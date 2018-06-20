#include "VirtualMemory.h"

#include <cstdio>
#include <cassert>
#include <bitset>
#include <iostream>

void get_range(uint64_t depth, uint64_t* lower, uint64_t* upper)
{
    uint64_t remainder = ((VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH) % OFFSET_WIDTH) + depth - depth;
    if((remainder != 0)&(depth == 1)){
        *lower = VIRTUAL_ADDRESS_WIDTH - 1;
        *upper = VIRTUAL_ADDRESS_WIDTH - remainder;
        return;
    }

    if(remainder == 0){
        *lower = (VIRTUAL_ADDRESS_WIDTH - 1) - (OFFSET_WIDTH*(depth-1));
        *upper = *lower - OFFSET_WIDTH + 1;
        return;
    }

    *lower = (VIRTUAL_ADDRESS_WIDTH - 1) - (OFFSET_WIDTH*(depth-2)) - remainder;
    *upper = *lower - OFFSET_WIDTH + 1;
    return;
}

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
//    std::cout << getBits(t, 4, 7) << std::endl;
//    std::cout << getBits(t, 1, 3) << std::endl;
//    std::cout << getBits(t, 0, 10) << std::endl;

    uint64_t lower = 0;
    uint64_t upper = 0;

    get_range(2, &lower, &upper);

    std::cout << lower << std::endl;
    std::cout << upper << std::endl;
    return 0;
}

