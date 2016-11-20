// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/config/subscription/configuri.h>
#include <vespa/memfilepersistence/tools/dumpslotfile.h>
#include <iostream>

namespace {

struct DumpSlotFileApp : public FastOS_Application {
    int Main() {
        try{
            config::ConfigUri config("");
            return storage::memfile::SlotFileDumper::dump(
                    _argc, _argv, config, std::cout, std::cerr);
        } catch (std::exception& e) {
            std::cerr << "Aborting due to exception:\n" << e.what() << "\n";
            return 1;
        }
    }
};

} // anonymous

int main(int argc, char **argv) {
    DumpSlotFileApp app;
    return app.Entry(argc, argv);
}
