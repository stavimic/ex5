#include <bitset>
#include <iostream>
#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#define SUCCESS_VALUE 1
enum actions {READ, WRITE};


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
    uint64_t left = OFFSET_WIDTH - 1;
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

uint64_t min2(uint64_t a, uint64_t b)
{
    if(a >= b)
    {
        return b;
    }
    return a;
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
void find_unused_frame(uint64_t root, uint64_t cur_frame_index, int& max_index_frame, uint64_t& min_cyclic,
                       uint64_t& min_frame, uint64_t& page_num, uint64_t depth, const std::string& cur_virtual_address,
                       uint64_t & parent_page_cyclic, uint64_t& chosen_page,
                       uint64_t& empty_frame, uint64_t& parent_empty_frame_addr,
                       uint64_t& mega_parent, uint64_t& cyclic_frame_reference,
                       uint64_t& empty_frame_reference, uint64_t& physical_addr, uint64_t forbidden_frame)
{
    int value;
    int amount_Zeros = 0;
    // Get the value of the address within the current depth:
    if(depth == TABLES_DEPTH)
    {
        for(int j = 0; j < PAGE_SIZE; j++)
        {
            PMread(root + j, &value);
            uint64_t cur_address =  std::bitset<64>(concat_addresses(cur_virtual_address, j)).to_ullong();

            if (value > max_index_frame)
            {
                max_index_frame = value;
            }
            if(value == 0)  // empty registry
            {
                amount_Zeros++;
                continue;
            }

            auto minimal_value = min2(static_cast<uint64_t>(NUM_PAGES - std::abs(static_cast<int>(page_num) - static_cast<int>(cur_address))),
                    static_cast<uint64_t>(std::abs(static_cast<int>(page_num) -  static_cast<int>(cur_address))));

            if((minimal_value > min_cyclic) && (value != 0))
            {
                min_cyclic = static_cast<uint64_t>(minimal_value);
                min_frame = static_cast<uint64_t>(value);
                chosen_page = cur_address;
                parent_page_cyclic = std::bitset<64>(cur_virtual_address).to_ullong();
                cyclic_frame_reference = root + j;  // reference to the chosen page
            }
        }
        if(amount_Zeros == PAGE_SIZE)
        {
            if((cur_frame_index != forbidden_frame) and  (empty_frame == 0))
            {
                empty_frame = cur_frame_index;
                parent_empty_frame_addr = mega_parent;
                empty_frame_reference = physical_addr;
            }
        }
        return;
    }
    for(int i = 0; i < PAGE_SIZE; i++)
    {
        int cur_row;
        PMread(root + i, &cur_row);
        if(cur_row == 0)
        {
            amount_Zeros++;
            continue;
        }
        if (cur_row > max_index_frame)
        {
            max_index_frame = cur_row;
        }
        physical_addr = root + i;
        find_unused_frame(cur_row*PAGE_SIZE, cur_row, max_index_frame, min_cyclic, min_frame,
                          page_num, depth + 1, concat_addresses(cur_virtual_address, static_cast<uint64_t>(i)),
                          parent_page_cyclic, chosen_page, empty_frame, parent_empty_frame_addr, cur_frame_index,
                          cyclic_frame_reference, empty_frame_reference, physical_addr, forbidden_frame);
    }

    if(amount_Zeros == PAGE_SIZE)
    {
        if((cur_frame_index != forbidden_frame) and (empty_frame == 0))
        {
            empty_frame = cur_frame_index;
            parent_empty_frame_addr = mega_parent;
            empty_frame_reference = physical_addr;
        }
    }
}


uint64_t get_frame(uint64_t addr, uint64_t forbidden_frame)
{
    int max_used = 0;
    uint64_t max_cyclic = 0;
    uint64_t min_frame = 0;
    uint64_t page_num = remove_offset(addr);
    uint64_t chosen_page_cyclic = 0;
    uint64_t parent_frame_cyclic = 0;
    uint64_t empty_frame = 0;
    uint64_t parent_empty_frame_addr = 0;
    uint64_t mega_parent_frame_index = 0;
    uint64_t cyclic_frame_ref = 0;
    uint64_t empty_frame_ref = 0;
    uint64_t physical_addr = 0;

    find_unused_frame(0, 0, max_used, max_cyclic, min_frame, page_num, 1, "", parent_frame_cyclic,
                      chosen_page_cyclic, empty_frame, parent_empty_frame_addr
            , mega_parent_frame_index, cyclic_frame_ref, empty_frame_ref, physical_addr, forbidden_frame);

    if(empty_frame != 0)
    {
        PMwrite(empty_frame_ref, 0);
        return empty_frame;

    }
    if(max_used < NUM_FRAMES - 1)
    {
        return static_cast<uint64_t>(max_used + 1);
    }
    if(min_frame != 0)
    {
        PMevict(min_frame, chosen_page_cyclic);
        PMwrite(cyclic_frame_ref, 0);
        return min_frame;
    }
    return 1;
}

uint64_t add_offset(int addr, uint64_t offset)
{

    std::string a = std::bitset<VIRTUAL_ADDRESS_WIDTH>(static_cast<unsigned long long int>(addr)).to_string();
    std::string b = std::bitset<OFFSET_WIDTH>(offset).to_string();
    std::string total = a + b;
    uint64_t decimal = std::bitset<64>(total).to_ullong();
    return decimal;
}

int traverse(uint64_t virtualAddress, int& parent_addr, word_t* value, uint64_t depth, actions action, int forbidden_frame)
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
                    uint64_t victim_frame_index = parent_addr;
                    uint64_t page_index = remove_offset(virtualAddress);
                    PMrestore(victim_frame_index, page_index);
                }
                PMread(add_offset(parent_addr, get_offset(virtualAddress)), value);
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
                }
                PMwrite(add_offset(parent_addr, get_offset(virtualAddress)), *value);
                break;
            }
            default:
            {
                break;
            }
        }
        return SUCCESS_VALUE;
    }
    else if(depth == 1)
    {
        PMread(0 + relevant_address, &current_address);
        uint64_t victim_frame_index = 0;
        if(current_address == 0) // The page we're looking for does'nt exist
        {
            // Find place and load the page:
            victim_frame_index = get_frame(virtualAddress, forbidden_frame);
            clearTable(victim_frame_index);
            current_address = static_cast<int>(victim_frame_index);
            PMwrite(relevant_address, static_cast<word_t>(victim_frame_index));
        }
        traverse(virtualAddress, current_address, value, depth + 1, action, victim_frame_index);
        return SUCCESS_VALUE;
    }
    else
    {
        switch (action)
        {
            default:  // Read or Write
            {
                PMread(parent_addr * PAGE_SIZE + relevant_address, &current_address);
                uint64_t victim_frame_index = 0;

                if(current_address == 0) // The page we're looking for does'nt exist
                {
                    // Find place and load the page:
                    victim_frame_index = get_frame(virtualAddress, forbidden_frame);
                    clearTable(victim_frame_index);
                    current_address = static_cast<int>(victim_frame_index);
                    PMwrite(parent_addr * PAGE_SIZE + relevant_address, static_cast<word_t>(victim_frame_index));

                }
                traverse(virtualAddress, current_address, value, depth + 1, action, victim_frame_index);
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
    return traverse(virtualAddress, addr, value, 1, READ, 0);
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
    return traverse(virtualAddress, addr, &value, 1, WRITE, 0);
}

