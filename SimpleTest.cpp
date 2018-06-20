#include "VirtualMemory.h"

#include <cstdio>
#include <cassert>
#include <bitset>
#include <iostream>

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

    std::string binary = std::bitset<32>(999990).to_string();
    std::cout<<binary;

    return 0;
}