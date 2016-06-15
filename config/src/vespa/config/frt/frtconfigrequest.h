// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/config/common/configrequest.h>
#include <vespa/config/common/configresponse.h>
#include <vespa/config/common/configkey.h>
#include <vespa/vespalib/stllike/string.h>

class FRT_Values;
class FRT_RPCRequest;

namespace config {

class ConfigKey;
class Connection;

/**
 * Class representing a FRT config request.
 */
class FRTConfigRequest : public ConfigRequest {
public:
    typedef std::unique_ptr<FRTConfigRequest> UP;
    FRTConfigRequest(Connection * connection, const ConfigKey & key);
    virtual ~FRTConfigRequest();
    virtual bool verifyKey(const ConfigKey & key) const = 0;
    virtual bool verifyState(const ConfigState & state) const = 0;

    bool abort();
    bool isAborted() const;
    void setError(int errorCode);
    const ConfigKey & getKey() const;

    FRT_RPCRequest* getRequest() { return _request; }
    virtual ConfigResponse::UP createResponse(FRT_RPCRequest * request) const = 0;
protected:
    FRT_RPCRequest *_request;
    FRT_Values & _parameters;
private:
    Connection * _connection;
    const ConfigKey _key;
};

class FRTConfigRequestV1 : public FRTConfigRequest {
public:
    FRTConfigRequestV1(const ConfigKey & key,
                     Connection * connection,
                     const vespalib::string & configMd5,
                     int64_t generation,
                     int64_t serverTimeout);
    bool verifyKey(const ConfigKey & key) const;
    bool verifyState(const ConfigState & state) const;
    ConfigResponse::UP createResponse(FRT_RPCRequest * request) const;
private:
    static const vespalib::string REQUEST_TYPES;
};

}

