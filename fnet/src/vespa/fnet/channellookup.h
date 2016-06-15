// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/vespalib/stllike/hash_map.h>

/**
 * This class handles registration/deregistration and lookup of
 * Channel objects. Note that locking must be done by the users of
 * this class in order to obtain thread safety. This class is used by
 * the @ref FNET_Connection class to keep track of channels. NOTE:
 * this class is not intended for direct use; use at own risk.
 **/
class FNET_ChannelLookup
{
private:
    typedef vespalib::hash_map<uint32_t, FNET_Channel *> ChannelMap;
    ChannelMap _map;

    FNET_ChannelLookup(const FNET_ChannelLookup &);
    FNET_ChannelLookup &operator=(const FNET_ChannelLookup &);

public:
    /**
     * Construct a channel lookup.
     *
     * @param hashSize Size of hash table used for lookup. Currently the
     *        hash size may not be modified during the lifetime of a
     *        ChannelLookup object.
     **/
    FNET_ChannelLookup(uint32_t hashSize = 16);

    /**
     * Delete hash table. NOTE: The hash table must be empty when this
     * method is called.
     **/
    ~FNET_ChannelLookup();

    /**
     * @return number of registered channels.
     **/
    uint32_t GetEntryCnt() { return _map.size(); }

    /**
     * Register a channel. If you register several channels with the
     * same ID, only the last registered channel will be availible by
     * calling the Lookup method.
     *
     * @param channel the channel you want to register.
     **/
    void Register(FNET_Channel *channel);

    /**
     * Find a channel given the channel ID.
     *
     * @return channel with correct id or nullptr if not found.
     * @param id channel ID of the channel you are looking for.
     **/
    FNET_Channel *Lookup(uint32_t id);

    /**
     * Broadcast a control packet to all channels registered with this
     * channel lookup. The given control packet will be sent on all
     * channels currently registered. The control packet is sent
     * sequentially (using the calling thread) to all channels in no
     * special order. When receiving a packet, the packet handler
     * registered for a channel may use the return code of the
     * HandlePacket method to indicate that the channel should be kept
     * open, closed or even freed. Keeping channels open and closing
     * them (unregistering them) is performed internally by this
     * method. Freeing channels, however, is left to the caller of this
     * method. The channels that are to be freed are linked together and
     * returned from this method.
     *
     * @return vector of all channels to be freed.
     * @param cpacket the control packet you want to broadcast.
     **/
    std::vector<FNET_Channel::UP> Broadcast(FNET_ControlPacket *cpacket);

    /**
     * Unregister a channel. This method uses both the channel ID and
     * the object identity of the parameter to ensure that the correct
     * object is unregistered even if there are several channels
     * currently registered with the same channel ID.
     *
     * @return true(ok)/false(not found)
     * @param channel the channel you want to unregister.
     **/
    bool Unregister(FNET_Channel *channel);
};

