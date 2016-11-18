#include <vespa/vespalib/util/alloc.h>
#include <vector>

using namespace vespalib::alloc;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        return 77;
    }
    size_t virt = strtoul(argv[1], nullptr, 0);
    size_t blockSize = strtoul(argv[2], nullptr, 0);
    size_t numBlocks = strtoul(argv[3], nullptr, 0);
    rlimit virtualLimit;
    virtualLimit.rlim_cur = virt;
    virtualLimit.rlim_max = virt;
    assert(setrlimit(RLIMIT_AS, &virtualLimit) == 0);
    std::vector<Alloc> mappings;
    for (size_t i(0); i < numBlocks; i++) {
        mappings.emplace_back(Alloc::allocMMap(blockSize));
        memset(mappings.back().get(), 0xa5, mappings.back().size());
    }
    return 0;
}
