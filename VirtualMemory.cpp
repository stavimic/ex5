#include <cstdlib>
#include "VirtualMemory.h"
#include "PhysicalMemory.h"




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
}


/*
 * Get value of address within the range (left, right)
 */
uint64_t getBits(uint64_t address, uint64_t left, uint64_t right)
{
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


int swap_page()
{
    return 1;
}




/*
 * Finds an unused Frame on the RAM.
 * Returns it's address upon success
 * Returns 0 upon failure.
 */
uint64_t find_unused_frame()
{
    for(uint64_t index = 1; index < NUM_FRAMES; index++)
    {
        int value;
        PMread(0 + (index * PAGE_SIZE), &value);
        if (value == 0)
        {
            return index * PAGE_SIZE;  // Return the address of the unused Frame
        }
    }
    return FAILURE_VALUE;  // No available Frame on the RAM
}



int traverse(uint64_t virtualAddress, int& parent_addr, word_t* value, uint64_t depth, actions action)
{

    uint64_t left;
    uint64_t right;
    // Get the range of the offset bits:
    get_range(depth, &left, &right);
    // Get the value of the address within the given range:
    uint64_t relevant_address = getBits(virtualAddress, left, right);
    int current_address;

    // bottom depth, we need to read the actual value todo check depth:
    if(depth == TABLES_DEPTH + 1)
    {
        switch (action)
        {
            case READ:
            {
                // Read from the physical memory:
                PMread((virtualAddress * PAGE_SIZE) + relevant_address, value);
                break;
            }
            case REMOVE_REFERENCE:
            {
                PMwrite(parent_addr, 0);
                break;
            }
            case WRITE:
            {
                break;
            }
        }
        return SUCCESS_VALUE;

    }

    else if(depth == 1)
    {
        switch (action)
        {
            case READ:
            {
                PMread(0 + relevant_address, &current_address);
                if(current_address == 0) // The page we're looking for does'nt exist
                {
                    // todo find place and load the page
                }
                traverse(virtualAddress, current_address, value, depth + 1, action);
                break;

            }

            case REMOVE_REFERENCE:
            {
                PMread(0 + relevant_address, &current_address);
                if(current_address != 0) // Nothing to do here
                {
                    traverse(virtualAddress, current_address, value, depth + 1, action);
                }
                break;

            }
            case WRITE:
            {
                break;
            }
        }
        return SUCCESS_VALUE;
    }

    else
    {
        switch (action)
        {
            case READ:
            {
                PMread(parent_addr * PAGE_SIZE + relevant_address, &current_address);
                if(current_address == 0) // The page we're looking for does'nt exist
                {
                    // todo find place and load the page



                    // todo update addr_i to point to the new page
                }
                traverse(virtualAddress, current_address, value, depth + 1, action);
                break;

            }

            case REMOVE_REFERENCE:
            {
                PMread(0 + relevant_address, &current_address);

                if(current_address == 0)
                {
                    PMwrite(parent_addr, 0);
                }
                else
                {
                    traverse(virtualAddress, current_address, value, depth + 1, action);
                }
                break;
            }
            case WRITE:
            {
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
int vmReadHelper(uint64_t virtualAddress, int& adrr_i, word_t* value, uint64_t depth)
{
    uint64_t left;
    uint64_t right;
    // Get the range of the offset bits:
    get_range(depth, &left, &right);
    // Get the value of the address within the given range:
    uint64_t relevant_address = getBits(virtualAddress, left, right);
    int current_address;

    // bottom depth, we need to read the actual value todo check depth:
    if(depth == TABLES_DEPTH + 1)
    {
        // Read from the physical memory:
        PMread((virtualAddress * PAGE_SIZE) + relevant_address, value);
        return SUCCESS_VALUE;
    }

    if(depth == 1)
    {
        PMread(0 + relevant_address, &current_address);
        if(current_address == 0) // The page we're looking for does'nt exist
        {
            // todo find place and load the page
        }

        vmReadHelper(virtualAddress, current_address, value, depth + 1);
    }

    else
    {
        PMread(adrr_i * PAGE_SIZE + relevant_address, &current_address);
        if(current_address == 0) // The page we're looking for does'nt exist
        {
            // todo find place and load the page



            // todo update addr_i to point to the new page
        }
        vmReadHelper(virtualAddress, current_address, value, depth + 1);
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
    return 1;
}

/* writes a word to the given virtual address
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */

int VMwrite(uint64_t virtualAddress, word_t value)
{
    return 1;
}
