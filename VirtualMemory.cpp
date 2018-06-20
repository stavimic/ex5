#include "VirtualMemory.h"
#include "PhysicalMemory.h"

void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}

void VMinitialize() {
    clearTable(0);
}


int getBits(uint64_t addr, int lower, int higher){
    uint64_t num = addr << lower;
}

int VMread(uint64_t virtualAddress, word_t* value) {

    return 1;
}


int VMwrite(uint64_t virtualAddress, word_t value) {
    return 1;
}
