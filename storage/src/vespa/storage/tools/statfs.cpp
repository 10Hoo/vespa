// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <errno.h>
#include <iostream>
#include <sys/vfs.h>
#include <vespa/vespalib/util/programoptions.h>
#include <vespa/vespalib/io/fileutil.h>

struct Options : public vespalib::ProgramOptions {
    bool showSyntaxPage;
    std::string _filename;

    Options(int argc, const char* const* argv)
        : vespalib::ProgramOptions(argc, argv),
          showSyntaxPage(false)
        {
            setSyntaxMessage(
                    "Utility program for checking output of statfs."
            );
            addOption("h help", showSyntaxPage, false,
                      "Shows this help page");
            addArgument("file", _filename, "File to use when calling statfs()");
        }
    };

int main(int argc, char** argv) {
    Options o(argc, argv);
    o.parse();

    if (o.showSyntaxPage) {
        o.writeSyntaxPage(std::cerr);
        exit(1);
    }

    if (!vespalib::fileExists(o._filename)) {
        std::cerr << "Cannot use statfs on non-existing file '" << o._filename
                  << "'.\n";
        exit(1);
    }

    struct statfs buf;
    if (statfs(o._filename.c_str(), &buf) == 0) {
        std::cerr << "f_type " << buf.f_type << "\n"
                  << "f_bsize " << buf.f_bsize << "\n"
                  << "f_blocks " << buf.f_blocks << "\n"
                  << "f_bfree " << buf.f_bfree << "\n"
                  << "f_bavail " << buf.f_bavail << "\n"
                  << "f_files " << buf.f_files << "\n"
                  << "f_ffree " << buf.f_ffree << "\n"
                  << "f_namelen " << buf.f_namelen << "\n";

        uint64_t available = buf.f_bavail;
        uint64_t total = buf.f_blocks;
        available *= buf.f_bsize;
        total *= buf.f_bsize;

        std::cerr << "\nAvailable " << available << " of total " << total
                  << "\n" << (100.0 * (total - available) / (double) total)
                  << " % full\n";
    } else {
        std::cerr << "statfs() failed: " << errno << "\n";
    }
}
