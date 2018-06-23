#include "VirtualMemory.h"
#include "PhysicalMemory.h"

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

void print_vec(){
    int counter = 0;
    for (uint64_t i = 0; i < NUM_FRAMES; ++i)
    {
        for (uint64_t j = 0;j < PAGE_SIZE; ++j) {
            word_t val;
            PMread(i*PAGE_SIZE + j, &val);
            std::cout<<"VAL"<<counter<<": " << val << std::endl;
            counter++;
        }
        std::cout<<"----" << std::endl;
    }
}

//int main(int argc, char **argv) {
//    VMinitialize();
//
//    int val1 = 3;
//    int val2 = 500;
//
//    VMwrite(13, val1);
//
//    print_vec();
//    word_t ans;
//    VMread(13, &ans);
//    if (ans == val1)
//    {
//        printf("success\n");
//    }
//    else{
//        printf("big fail\n");
//    }
//
//    print_vec();
//
//    printf("SECONDDD\n");
//    word_t ans2;
//    VMread(6, &ans2);
//    print_vec();
//
//    VMread(31, &ans2);

//}





int main(int argc, char **argv) {
    VMinitialize();
    print_vec();
    std::cout<<"--------------------------" << std::endl;
    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        printf("writing to %llu\n", (long long int) i);
        VMwrite(5 * i * PAGE_SIZE, i);
//        print_vec();
//        std::cout<<"--------------------------" << std::endl;
    }

    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        word_t value;
        VMread(5 * i * PAGE_SIZE, &value);
        printf("reading from %llu %d\n", (long long int) i, value);
//        print_vec();
//        std::cout<<"--------------------------" << std::endl;
        assert(uint64_t(value) == i);
    }
    printf("success\n");

    return 0;
}