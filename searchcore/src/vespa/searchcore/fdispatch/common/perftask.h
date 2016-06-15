// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
// Copyright (C) 2005 Overture Services Norway AS

#pragma once

class FastS_AppContext;

class FastS_PerfTask : public FNET_Task
{
private:
    FastS_AppContext &_ctx;
    double            _delay;
    bool              _valid;

    FastS_PerfTask(const FastS_PerfTask &);
    FastS_PerfTask &operator=(const FastS_PerfTask &);

public:
    FastS_PerfTask(FastS_AppContext &ctx, double delay);
    ~FastS_PerfTask();
    virtual void PerformTask();
    bool isValid() const { return _valid; }
};

