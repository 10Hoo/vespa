// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <sys/types.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include "log.h"
LOG_SETUP("");

namespace ns_log {

void log_assert_fail(const char *assertion,
                     const char *file,
                     uint32_t line)
{
    LOG(error, "%s:%d: Failed assertion: '%s'",
        file, line, assertion);
    fprintf(stderr, "%s:%d: Failed assertion: '%s'\n",
            file, line, assertion);
    abort();
}


void log_abort(const char *message,
               const char *file,
               unsigned int line)
{
    LOG(error, "%s:%d: Abort called. Reason: %s",
        file, line, message);
    fprintf(stderr, "%s:%d: Abort called. Reason: %s\n",
            file, line, message);
    abort();
}

} // end namespace ns_log
