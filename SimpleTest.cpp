#include "VirtualMemory.h"

#include <cstdio>
#include <cassert>
#include <bitset>
#include <iostream>

uint64_t getBits(uint64_t addr, int lower, int higher){
    uint64_t num = (addr >> lower) & ((1 << (higher - lower + 1))-1);
    return num;
}

int main(int argc, char **argv) {
//    VMinitialize();
//    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
//        printf("writing to %llu\n", i);
//        VMwrite(5 * i * PAGE_SIZE, i);
//    }
//
//    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
//        word_t value;
//        VMread(5 * i * PAGE_SIZE, &value);
//        printf("reading from %llu %d\n", i, value);
//        assert(uint64_t(value) == i);
//    }
//    printf("success\n");

    uint64_t t = 1302;
    std::cout << getBits(t, 4, 7) << std::endl;
    std::cout << getBits(t, 1, 3) << std::endl;
    std::cout << getBits(t, 0, 10) << std::endl;
    return 0;
}

