#include "PhysicalMemory.h"
#include <vector>
#include <unordered_map>
#include <cassert>
#include <cstdio>

typedef std::vector<word_t> page_t;

std::vector<page_t> RAM;
std::unordered_map<uint64_t, page_t> swapFile;

void initialize() {
    RAM.resize(NUM_FRAMES, page_t(PAGE_SIZE));
}

/*
 * reads an integer from the given physical address and puts it in 'value'
 */
void PMread(uint64_t physicalAddress, word_t* value) {
    if (RAM.empty())
        initialize();

    assert(physicalAddress < RAM_SIZE);

    *value = RAM[physicalAddress / PAGE_SIZE][physicalAddress % PAGE_SIZE];
 }

/*
 * writes 'value' to the given physical address
 */
void PMwrite(uint64_t physicalAddress, word_t value) {
    if (RAM.empty())
        initialize();

    assert(physicalAddress < RAM_SIZE);

    RAM[physicalAddress / PAGE_SIZE][physicalAddress
             % PAGE_SIZE] = value;
}


/*
 * evicts a page from the RAM to the hard drive
 */
void PMevict(uint64_t frameIndex, uint64_t evictedPageIndex) {

    if (RAM.empty())
        initialize();

    assert(swapFile.find(evictedPageIndex) == swapFile.end());
    assert(frameIndex < NUM_FRAMES);
    assert(evictedPageIndex < NUM_PAGES);

    swapFile[evictedPageIndex] = RAM[frameIndex];
}

/*
 * restores a page from the hard drive to the RAM
 */
void PMrestore(uint64_t frameIndex, uint64_t restoredPageIndex) {
    if (RAM.empty())
        initialize();

    assert(frameIndex < NUM_FRAMES);

    // page is not in swap file, so this is essentially
    // the first reference to this page. we can just return
    // as it doesn't matter if the page contains garbage
    if (swapFile.find(restoredPageIndex) == swapFile.end())
        return;

    RAM[frameIndex] = std::move(swapFile[restoredPageIndex]);
    swapFile.erase(restoredPageIndex);
}
