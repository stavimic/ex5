#include <bitset>
#include <iostream>
#include "VirtualMemory.h"
#include "PhysicalMemory.h"


uint64_t get_bits(uint64_t address, uint64_t depth)
{
    uint64_t left, right;
    uint64_t remainder = ((VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH) % OFFSET_WIDTH) + depth - depth;
    if((remainder != 0)&(depth == 1)){
        left = VIRTUAL_ADDRESS_WIDTH - 1;
        right = VIRTUAL_ADDRESS_WIDTH - remainder;
    }
    else if(remainder == 0){
        left = (VIRTUAL_ADDRESS_WIDTH - 1) - (OFFSET_WIDTH * (depth-1));
        right = left - OFFSET_WIDTH + 1;
    }
    else
    {
        left = (VIRTUAL_ADDRESS_WIDTH - 1) - (OFFSET_WIDTH * (depth-2)) - remainder;
        right = left - OFFSET_WIDTH + 1;
    }

    uint64_t num = (address >> right) & ((1 << (left - right + 1))-1);
    return num;
}


uint64_t remove_offset(uint64_t address)
{
    uint64_t left = VIRTUAL_ADDRESS_WIDTH - 1;
    uint64_t right = OFFSET_WIDTH;
    uint64_t num = (address >> right) & ((1 << (left - right + 1))-1);
    return num;
}


uint64_t get_offset(uint64_t address)
{
    uint64_t left = OFFSET_WIDTH;
    uint64_t right = 0;
    uint64_t num = (address >> right) & ((1 << (left - right + 1))-1);
    return num;
}

void clearTable(uint64_t frameIndex)
{
    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
    {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}


/*
 * Initialize the virtual memory
 */
void VMinitialize()
{
    clearTable(0);
}

int swap_page(uint64_t oldAddress, uint64_t virtualAddress)
{
    // todo remove reference to old_address, and read the page in virtualAddress to the same place
    return 1;
}

uint64_t min2(uint64_t a, uint64_t b)
{
    if(a >= b)
    {
        return a;
    }
    return b;
}



std::string concat_addresses(const std::string& addr, uint64_t current_num)
{
    std::string right = std::bitset<OFFSET_WIDTH>(current_num).to_string();
    std::string total = addr + right;
    return total;
}



/*
 * Finds an unused Frame on the RAM.
 * Returns it's address upon success.
 * Returns 0 upon failure.
 */
void find_unused_frame(uint64_t root, uint64_t parent_addr, uint64_t& max_index_frame, uint64_t& min_cyclic,
                       uint64_t& min_frame, uint64_t& page_num, uint64_t depth, const std::string& cur_virtual_address, uint64_t& chosen_page_address)
{
    int value;
    // Get the value of the address within the current depth:
    if(depth == TABLES_DEPTH)
    {
        for(int j = 0; j < PAGE_SIZE; j++)
        {
            PMread(root + j, &value);
            uint64_t cur_address =  std::bitset<64>(concat_addresses(cur_virtual_address, j)).to_ullong();
            if (value > max_index_frame)
            {
                max_index_frame = static_cast<uint64_t>(value);
            }
//            auto current_value = min2(static_cast<uint64_t>(NUM_PAGES - std::abs(static_cast<int>(page_num) - value)), static_cast<uint64_t>(std::abs(static_cast<int>(page_num) - value)));
            auto current_value = min2(static_cast<uint64_t>(NUM_PAGES - std::abs(static_cast<int>(page_num) - static_cast<int>(cur_address))),
                    static_cast<uint64_t>(std::abs(static_cast<int>(page_num) -  static_cast<int>(cur_address))));

            if(current_value < min_cyclic)
            {
                min_cyclic = static_cast<uint64_t>(current_value);
                min_frame = static_cast<uint64_t>(value);
                chosen_page_address = cur_address;
            }
        }
        return;
    }
    for(int i = 0; i < PAGE_SIZE; i++)
    {
        int cur_frame_index;
        PMread(root + i, &cur_frame_index);
        if(cur_frame_index == 0)
        {
            continue;
        }
        if (cur_frame_index > max_index_frame)
        {
            max_index_frame = static_cast<uint64_t>(cur_frame_index);
        }
//        auto m = min2(static_cast<uint64_t>(NUM_PAGES - std::abs(static_cast<int>(p_num) - value)),
//                      static_cast<uint64_t>(std::abs(static_cast<int>(p_num) - value)));
//        if( m < min_cyclic){
//            min_cyclic = static_cast<uint64_t>(m);
//            min_page = static_cast<uint64_t>(cur_frame);
//
//        }
        find_unused_frame(cur_frame_index*PAGE_SIZE, root, max_index_frame, min_cyclic, min_frame,
                          page_num, depth + 1, concat_addresses(cur_virtual_address, static_cast<uint64_t>(i)), chosen_page_address);
    }
}


uint64_t get_frame(uint64_t addr)
{
    // todo we need to evict in cyclic min page ! and not frame
    uint64_t max_used = 0;
    uint64_t min_cyclic = 0;
    uint64_t min_frame = 0;
    uint64_t page_num = remove_offset(addr);
    uint64_t chosen_page_address = 0;
    find_unused_frame(0, 0, max_used, min_cyclic, min_frame, page_num, 1, "", chosen_page_address);
    if(max_used < NUM_FRAMES - 1)
    {
        return static_cast<uint64_t>(max_used + 1);
    }
    VMwrite(chosen_page_address, 0); // Remove the reference from the parent node to the removed page

    return min_frame; // todo need to swap
}

uint64_t add_offset(int addr, uint64_t offset)
{

    std::string a = std::bitset<VIRTUAL_ADDRESS_WIDTH>(static_cast<unsigned long long int>(addr)).to_string();
    std::string b = std::bitset<OFFSET_WIDTH>(offset).to_string();
    std::string total = a + b;
    uint64_t decimal = std::bitset<64>(total).to_ullong();
    return decimal;
}

int traverse(uint64_t virtualAddress, int& parent_addr, word_t* value, uint64_t depth, actions action)
{
    int current_address;
    // Get the value of the address within the current depth:
    uint64_t relevant_address = get_bits(virtualAddress, depth);

    // bottom depth, we need to read the actual value todo check depth:
    if(depth == TABLES_DEPTH + 1)
    {
        switch (action)
        {
            case READ:
            {
                PMread(parent_addr * PAGE_SIZE + relevant_address, &current_address);
                if(current_address == 0) // The page we're looking for does'nt exist
                {
                    // Find place and load the page:
//                    uint64_t victim_frame_index = get_frame();
                    uint64_t victim_frame_index = parent_addr;
                    uint64_t page_index = remove_offset(virtualAddress);
                    PMrestore(victim_frame_index, page_index);
                    clearTable(victim_frame_index);
                    current_address = static_cast<int>(victim_frame_index);
//                    PMwrite(parent_addr * PAGE_SIZE + relevant_address, static_cast<word_t>(victim_frame_index));
                }
                PMread(add_offset(parent_addr, get_offset(virtualAddress)), value);
                break;
            }
            case REMOVE_REFERENCE:
            {
                PMwrite(static_cast<uint64_t>(parent_addr), 0);
                break;
            }
            case WRITE:
            {
                PMread(parent_addr * PAGE_SIZE + relevant_address, &current_address);
                if(current_address == 0) // The page we're looking for does'nt exist
                {
                    // Find place and load the page:
                    uint64_t victim_frame_index = parent_addr;
                    uint64_t page_index = remove_offset(virtualAddress);
                    PMrestore(victim_frame_index, page_index);
                    clearTable(victim_frame_index);
                    current_address = static_cast<int>(victim_frame_index);
//                    uint64_t victim_frame_index = get_frame(virtualAddress);
//                    uint64_t page_index = remove_offset(virtualAddress);
//                    PMrestore(victim_frame_index, page_index);
//                    clearTable(victim_frame_index);
//                    current_address = static_cast<int>(victim_frame_index);
//                    PMwrite(parent_addr * PAGE_SIZE + relevant_address, static_cast<word_t>(victim_frame_index));
                }
                PMwrite(add_offset(parent_addr, get_offset(virtualAddress)), *value);
                break;
            }
        }
        return SUCCESS_VALUE;
    }
    else if(depth == 1)
    {
        switch (action)
        {
            case REMOVE_REFERENCE:
            {
                PMread(0 + relevant_address, &current_address);
                if(current_address != 0) // Nothing to do here
                {
                    traverse(virtualAddress, current_address, value, depth + 1, action);
                }
                break;
            }
            default:  // Read or Write
            {
                PMread(0 + relevant_address, &current_address);
                if(current_address == 0) // The page we're looking for does'nt exist
                {
                    // Find place and load the page:
                    auto victim_frame_index = get_frame(virtualAddress);
//                    uint64_t page_index = remove_offset(virtualAddress);
//                    PMrestore(victim_frame_index, page_index);
                    clearTable(victim_frame_index);
                    current_address = static_cast<int>(victim_frame_index);
                    PMwrite(relevant_address, static_cast<word_t>(victim_frame_index));
                }

                traverse(virtualAddress, current_address, value, depth + 1, action);
                break;
            }
        }
        return SUCCESS_VALUE;
    }
    else
    {
        switch (action)
        {
            case REMOVE_REFERENCE:
            {
                PMread(0 + relevant_address, &current_address);
                if(current_address == 0)
                {
                    PMwrite(static_cast<uint64_t>(parent_addr), 0);
                }
                else
                {
                    traverse(virtualAddress, current_address, value, depth + 1, action);
                }
                break;
            }
            default:  // Read or Write
            {
                PMread(parent_addr * PAGE_SIZE + relevant_address, &current_address);
                if(current_address == 0) // The page we're looking for does'nt exist
                {
                    // Find place and load the page:
                    uint64_t victim_frame_index = get_frame(virtualAddress);
//                    uint64_t page_index = remove_offset(virtualAddress);
//                    PMrestore(victim_frame_index, page_index);
                    clearTable(victim_frame_index);
                    current_address = static_cast<int>(victim_frame_index);
                    PMwrite(parent_addr * PAGE_SIZE + relevant_address, static_cast<word_t>(victim_frame_index));
                }
                traverse(virtualAddress, current_address, value, depth + 1, action);
                break;
            }
        }
    }
    return SUCCESS_VALUE;
}


/* reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value)
{
    int addr;
    return traverse(virtualAddress, addr, value, 1, READ);
}

/* writes a word to the given virtual address
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */

int VMwrite(uint64_t virtualAddress, word_t value)
{
    int addr=0;
    return traverse(virtualAddress, addr, &value, 1, WRITE);
}























//
///* reads a word from the given virtual address
// * and puts its content in *value.
// *
// * returns 1 on success.
// * returns 0 on failure (if the address cannot be mapped to a physical
// * address for any reason)
// */
//int vmReadHelper(uint64_t virtualAddress, int& adrr_i, word_t* value, uint64_t depth)
//{
//    uint64_t left;
//    uint64_t right;
//    // Get the range of the offset bits:
//    get_range(depth, &left, &right);
//    // Get the value of the address within the given range:
//    uint64_t relevant_address = getBits(virtualAddress, left, right);
//    int current_address;
//
//    // bottom depth, we need to read the actual value todo check depth:
//    if(depth == TABLES_DEPTH + 1)
//    {
//        // Read from the physical memory:
//        PMread((virtualAddress * PAGE_SIZE) + relevant_address, value);
//        return SUCCESS_VALUE;
//    }
//
//    if(depth == 1)
//    {
//        PMread(0 + relevant_address, &current_address);
//        if(current_address == 0) // The page we're looking for does'nt exist
//        {
//            // todo find place and load the page
//        }
//
//        vmReadHelper(virtualAddress, current_address, value, depth + 1);
//    }
//
//    else
//    {
//        PMread(adrr_i * PAGE_SIZE + relevant_address, &current_address);
//        if(current_address == 0) // The page we're looking for does'nt exist
//        {
//            // todo find place and load the page
//
//
//
//            // todo update addr_i to point to the new page
//        }
//        vmReadHelper(virtualAddress, current_address, value, depth + 1);
//    }
//
//    return SUCCESS_VALUE;
//}
