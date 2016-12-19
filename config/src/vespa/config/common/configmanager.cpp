// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include "configmanager.h"
#include "exceptions.h"
#include "configholder.h"
#include <vespa/vespalib/util/atomic.h>
#include <memory>
#include <thread>
#include <chrono>

#include <vespa/log/log.h>
LOG_SETUP(".config.common.configmanager");

using namespace std::chrono_literals;

namespace config {

ConfigManager::ConfigManager(SourceFactory::UP sourceFactory, int64_t initialGeneration)
    : _idGenerator(0),
      _sourceFactory(std::move(sourceFactory)),
      _generation(initialGeneration),
      _subscriptionMap(),
      _lock(),
      _firstLock(),
      _first(true)
{ }

ConfigManager::~ConfigManager() { }

ConfigSubscription::SP
ConfigManager::subscribe(const ConfigKey & key, uint64_t timeoutInMillis)
{
    LOG(debug, "subscribing on def %s, configid %s", key.getDefName().c_str(), key.getConfigId().c_str());

    SubscriptionId id(vespalib::Atomic::postInc(&_idGenerator));

    IConfigHolder::SP holder(new ConfigHolder());
    Source::UP source = _sourceFactory->createSource(holder, key);
    source->reload(_generation);

    source->getConfig();
    ConfigSubscription::SP subscription(new ConfigSubscription(id, key, holder, std::move(source)));

    FastOS_Time timer;
    timer.SetNow();
    while (timer.MilliSecsToNow() < timeoutInMillis) {
        if (holder->poll())
            break;
        std::this_thread::sleep_for(10ms);
    }
    if (!holder->poll()) {
        std::ostringstream oss;
        oss << "Timed out while subscribing to '" << key.getDefNamespace() << "." << key.getDefName() << "', configid '" << key.getConfigId() << "'";
        throw ConfigTimeoutException(oss.str());
    }
    LOG(debug, "done subscribing");
    vespalib::LockGuard guard(_lock);
    _subscriptionMap[id] = subscription;
    return subscription;
}

void
ConfigManager::unsubscribe(const ConfigSubscription::SP & subscription)
{
    vespalib::LockGuard guard(_lock);
    const SubscriptionId id(subscription->getSubscriptionId());
    if (_subscriptionMap.find(id) != _subscriptionMap.end())
        _subscriptionMap.erase(id);
}

void
ConfigManager::reload(int64_t generation)
{
    _generation = generation;
    vespalib::LockGuard guard(_lock);
    for (SubscriptionMap::iterator it(_subscriptionMap.begin()), mt(_subscriptionMap.end()); it != mt; it++) {
        it->second->reload(_generation);
    }
}

} // namespace config
