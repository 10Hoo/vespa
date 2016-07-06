// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/defaults.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <variable>\n", argv[0]);
        fprintf(stderr, "  the variable must be 'home' or 'portbase' currently\n");
        return 1;
    }
    if (strcmp(argv[1], "home") == 0) {
        printf("%s\n", vespa::Defaults::vespaHome());
        return 0;
    } else if (strcmp(argv[1], "portbase") == 0) {
        printf("%d\n", vespa::Defaults::vespaPortBase());
        return 0;
    } else if (strcmp(argv[1], "configservers") == 0) {
        for (std::string v : vespa::Defaults::vespaConfigServerHosts()) {
            printf("%s\n", v.c_str());
        }
    } else if (strcmp(argv[1], "configservers_rpc") == 0) {
        size_t count = 0;
        for (std::string v : vespa::Defaults::vespaConfigServerRpcAddrs()) {
            if (count++ > 0) printf(",");
            printf("%s", v.c_str());
        }
        printf("\n");
    } else if (strcmp(argv[1], "configservers_http") == 0) {
        for (std::string v : vespa::Defaults::vespaConfigServerRestUrls()) {
            printf("%s\n", v.c_str());
        }
    } else if (strcmp(argv[1], "configsources") == 0) {
        size_t count = 0;
        for (std::string v : vespa::Defaults::vespaConfigSourcesRpcAddrs()) {
            if (count++ > 0) printf(",");
            printf("%s", v.c_str());
        }
        printf("\n");
    } else if (strcmp(argv[1], "configproxy_rpc") == 0) {
        std::string v = vespa::Defaults::vespaConfigProxyRpcAddr();
        printf("%s\n", v.c_str());
    } else {
        fprintf(stderr, "Unknown variable '%s'\n", argv[1]);
        return 1;
    }
}
