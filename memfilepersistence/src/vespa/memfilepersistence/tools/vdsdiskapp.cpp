// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/defaults.h>
#include <vespa/log/log.h>
#include <vespa/memfilepersistence/tools/vdsdisktool.h>

LOG_SETUP(".vdsdisktool");

namespace {
    struct DiskApp : public FastOS_Application {
        int Main() {
            try {
                std::string dir = vespa::Defaults::vespaHome();
                dir.append("var/db/vespa/vds");
                return storage::memfile::VdsDiskTool::run(
                        _argc, _argv, dir.c_str(),
                        std::cout, std::cerr);
            } catch (std::exception& e) {
                std::cerr << "Application aborted with exception:\n" << e.what()
                          << "\n";
                return 1;
            }
        }
    };
} // anonymous

int main(int argc, char **argv) {
    DiskApp app;
    return app.Entry(argc, argv);
}

