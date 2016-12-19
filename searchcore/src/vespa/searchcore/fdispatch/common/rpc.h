// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
// Copyright (C) 1998-2003 Fast Search & Transfer ASA
// Copyright (C) 2003 Overture Services Norway AS

#pragma once

#include <vespa/fnet/frt/frt.h>
#include <vespa/searchcore/fdispatch/common/appcontext.h>
#include <vespa/vespalib/stllike/string.h>
#include <vespa/slobrok/sbregister.h>

class FastS_RPC : public FRT_Invokable
{
private:
    FastS_RPC(const FastS_RPC &);
    FastS_RPC& operator=(const FastS_RPC &);

    FastS_AppContext         *_appCtx;
    FNET_Transport            _transport;
    FRT_Supervisor            _supervisor;
    slobrok::api::RegisterAPI _sbregister;

public:
    FastS_RPC(FastS_AppContext *appCtx);
    virtual ~FastS_RPC() {}

    FastS_AppContext *GetAppCtx() { return _appCtx; }
    FRT_Supervisor *GetSupervisor() { return &_supervisor; }
    bool Init(int port, const vespalib::string& myHeartbeatId);
    bool Start() { return _transport.Start(_appCtx->GetThreadPool()); }
    void ShutDown() { _transport.ShutDown(true); }

    // Register RPC Methods

    virtual void RegisterMethods(FRT_ReflectionBuilder *rb);

    // RPC methods implemented here

    void RPC_GetCompileInfo(FRT_RPCRequest *req);
    void RPC_GetResultConfig(FRT_RPCRequest *req);

    // RPC Proxy Methods

    void RPC_GetNodeType_Proxy(FRT_RPCRequest *req);

    // RPC methods to be implemented by subclasses

    virtual void RPC_GetNodeType(FRT_RPCRequest *req) = 0;
};

