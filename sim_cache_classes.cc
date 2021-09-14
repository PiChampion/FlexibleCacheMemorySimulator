//
// Created by ianpi on 9/6/2021.
//

#include <cstdio>
#include <iostream>
#include "sim_cache.h"
#include "sim_cache_classes.h"
#include <cmath>

using namespace std;

bool dirty_swap = false;
bool has_victim = false;
unsigned int offset_bits;
extern unsigned long int writeback;
extern unsigned long int writeback_bit;
extern bool has_victim_cache;
extern simulation_results results;

// Initialization for CACHE class
CACHE::CACHE(int name, unsigned long int block_size_in, unsigned long int size_in,
             unsigned long int assoc_in, unsigned long int vc_num_blocks_in) {

    // Initialize cache parameters
    my_params.name = name;
    my_params.block_size = block_size_in;
    my_params.size = size_in;
    my_params.assoc = assoc_in;
    my_params.vc_num_blocks = vc_num_blocks_in;
    my_params.num_sets = (my_params.size)/((my_params.assoc)*(my_params.block_size));
    my_params.index_bits = (unsigned int)log2(my_params.num_sets);
    my_params.block_offset_bits = (unsigned int)log2(my_params.block_size);
    offset_bits = my_params.block_offset_bits;

    // Initialize a victim cache. In there is no victim cache, don't push the victim cache to the victim cache vector
    victim_CACHE emptyVictimCache(vc_num_blocks_in);
    if(vc_num_blocks_in) {
        victim_caches.push_back(emptyVictimCache);
    }

    // Initialize a vector of empty sets
    set emptySet(my_params.assoc);
    for (std::size_t i = 0; i < (my_params.num_sets); i++) {
        sets.push_back(emptySet);
    }
}

// Function for searching for a block in the cache
int CACHE::Execute(char str, unsigned long int addr) {
    /*************************************
    Function 	returns 0 if read miss
                returns 1 if read hit
                returns 2 if write miss
                returns 3 if write hit
                returns 4 if read VC hit
                returns 5 if write VC hit
    **************************************/
    unsigned long int index = (((1 << my_params.index_bits) - 1)&(addr >> my_params.block_offset_bits));
    unsigned long int tag = addr >> (my_params.block_offset_bits + my_params.index_bits);
    unsigned long int victim_tag = addr >> (my_params.block_offset_bits);       // Tag for victim cache
    block_info block;
#if defined(DEBUG)
    if(str == 'r') {
        printf("L%d read: %lx (tag %lx, index %lu)\n", my_params.name, ((addr >> my_params.block_offset_bits) <<my_params.block_offset_bits), tag, index);
    }
    else {
        printf("L%d write: %lx (tag %lx, index %lu)\n", my_params.name, ((addr >> my_params.block_offset_bits) <<my_params.block_offset_bits), tag, index);
    }
#endif
    // Hit //
    if(sets[index].search(my_params.name, tag, str, my_params.assoc)) {
        if(str == 'r') return 1;    // Read Hit
        if(str == 'w') return 3;    // Write Hit
    }
    // Miss //
    else{
        // Checks to make sure there is a victim cache and that this is the L1 cache before searching victim cache
        if(my_params.vc_num_blocks && has_victim && (my_params.name == 1)) {

            // Victim Cache Hit //
            if(victim_caches[0].Execute(victim_tag)) {
                // Get info for LRU block in L1 Cache
                block = sets[index].getLRUinfo(my_params.assoc);
                // Set the victim cache tag for the new block to be swapped
                block.victim_tag = block.address >> (my_params.block_offset_bits);
#if defined(DEBUG)
                printf("VC swap req: [%lx, %lx]\n",((addr >> my_params.block_offset_bits) <<my_params.block_offset_bits), (block.address >> my_params.block_offset_bits) <<my_params.block_offset_bits);
                if(dirty_swap) {
                    printf("VC hit: %lx (dirty)\n",
                           ((addr >> my_params.block_offset_bits) << my_params.block_offset_bits));
                }
                else {
                    printf("VC hit: %lx (clean)\n",
                           ((addr >> my_params.block_offset_bits) << my_params.block_offset_bits));
                }
#endif
                // Swap the LRU block with the desired block
                victim_caches[0].swap(victim_tag, block.valid, block.dirty, block.address, block.victim_tag);
                // Insert the desired block in L1 cache
                sets[index].insert(str, addr, tag, my_params.assoc);
                if(str == 'r') return 4;    // Read Victim Cache Hit
                if(str == 'w') return 5;    // Write Victim Cache Hit
            }

            // Victim Cache Miss //
            else {
                // Get info for LRU block in L1 Cache
                block = sets[index].getLRUinfo(my_params.assoc);
                // Set the victim cache tag for the new block to be swapped
                block.victim_tag = block.address >> (my_params.block_offset_bits);
                // Is the LRU block is valid, give the block to the victim cache
                if(block.valid) {
                    results.swap_requests++;
                    victim_caches[0].Receive(block.victim_tag, block.dirty, block.address);
#if defined(DEBUG)
                        printf("VC swap req: [%lx, %lx]\n",((addr >> my_params.block_offset_bits) <<my_params.block_offset_bits), (block.address >> my_params.block_offset_bits) <<my_params.block_offset_bits);
                        printf("VC miss\n");
#endif
                }
            }
            has_victim = false;     // If there was a victim, reset this global variable used to determine the need to search the victim cache
        }
        if(str == 'r') return 0;        // Read Miss
        if(str == 'w') return 2;        // Write miss
    }
    return 0;
}

// Function for receiving a block for a cache
void CACHE::Receive(char str, unsigned long int addr) {
    unsigned long int index = (((1 << my_params.index_bits) - 1)&(addr >> my_params.block_offset_bits));
    unsigned long int tag = addr >> (my_params.block_offset_bits + my_params.index_bits);

    // Insert the block into the desired set
    sets[index].insert(str, addr, tag, my_params.assoc);
}

// Function for printing the contents of a cache
void CACHE::print(void) {
    auto interator = sets.begin();
    auto last = sets.end();

    int set_number = 0;

    // Iterate through the sets, printing each
    while (interator!=last) {
        printf("  set   %d:   ", set_number);
        (*interator).print();
        ++interator;
        set_number++;
        printf("\n");
    }
}

// Function for printing the contents of a victim cache
void CACHE::VCprint(void) {
    victim_caches[0].sets[0].VCprint();
}

// Initialization for victim_CACHE class
victim_CACHE::victim_CACHE(unsigned long int vc_num_blocks_in) {
    vc_num_blocks = vc_num_blocks_in;   // Record the size of victim cache
    set emptySet(vc_num_blocks);        // Create empty set with the needed number of blocks
    sets.push_back(emptySet);           // Place the empty set in a vector of sets
}

// Function for searching for a block in the victim cache
int victim_CACHE::Execute(unsigned long int victim_tag) {
    /*************************************
    Function 	returns 0 if miss
                returns 1 if hit
    **************************************/
    return sets[0].VictimSearch(victim_tag, vc_num_blocks);
}

// Function for receiving a block for a victim cache
void victim_CACHE::Receive(unsigned long int victim_tag, bool dirty_in, unsigned long int addr) {
    sets[0].VictimInsert(victim_tag, vc_num_blocks, addr, dirty_in);
}

// Function for swapping a desired block in the victim cache with the given block
void victim_CACHE::swap(unsigned long int victim_tag, bool new_valid, bool new_dirty, unsigned long int new_address, unsigned long int new_victim_tag) {
    sets[0].VictimSwap(victim_tag, new_valid, new_dirty, new_address, new_victim_tag);
}

// Initialization for set class
set::set(unsigned long int assoc) {
    block emptyBlock;                   // Create empty block
    for (std::size_t i = 0; i < assoc; i++) {
        emptyBlock.setLRU(i); 			// Initialize the LRUs to be in initial order
        blocks.push_back(emptyBlock);   // Push the blocks to the set
    }
}

// Function for searching for a block in a set
int set::search(int name, unsigned long int tag, char str, unsigned long int assoc) {
    /*************************************
    Function 	returns 0 if miss
                returns 1 if hit
    **************************************/
    auto first = blocks.begin();
    auto LRU = blocks.begin();
    auto interator = blocks.begin();
    auto last = blocks.end();
    unsigned long int LRU_block = assoc - 1;        // Calculate LRU number

    // Iterate through the set looking for the block
    while (interator!=last) {

        // Save the LRU block in case of a miss
        if ((*interator).getLRU()==LRU_block) {
            LRU = interator;
        }

        // Check to see if the block is found
        if (((*interator).getTag()==tag)&&((*interator).isValid())) {
#if defined(DEBUG)
            printf("L%d hit\n", name);
#endif
            // Update the LRUs of the blocks
            while (first!=last) {
                if ((*first).getLRU() < (*interator).getLRU()) {
                    (*first).increment_LRU_counter();
                }
                ++first;
            }
            (*interator).setLRU(0);     // Set the LRU of the found block to 0 (MRU)
#if defined(DEBUG)
            printf("L%d update LRU\n", name);
#endif

            if(str == 'w') {
                (*interator).setDirty(true);
#if defined(DEBUG)
                printf("L%d set dirty\n", name);
#endif
            }
            return 1;
        }
        ++interator;    // Move to the next block to check
    }

    // Check is the LRU is valid to know if the victim cache should be used
    if((*LRU).isValid()) has_victim = true;
    else has_victim = false;

#if defined(DEBUG)
    printf("L%d miss\n", name);
        if((*LRU).isValid()) {
            if((*LRU).isDirty()) {
                printf("L%d victim: %lx (tag %lx, index %lu, dirty)\n", name, ((*LRU).getAddr() >> offset_bits << offset_bits), (*LRU).getTag(), index);
            }
            else {
                printf("L%d victim: %lx (tag %lx, index %lu, clean)\n", name, ((*LRU).getAddr() >> offset_bits << offset_bits), (*LRU).getTag(), index);
            }
        }
        else {
            printf("L%d victim: none\n", name);
        }
#endif

    // If the cache is L2 or L1 without a victim cache, check to see if there should be a writeback to the next memory hierarchy
    if(!has_victim_cache || name == 2) {
        if ((*LRU).isDirty() && (*LRU).isValid()) {
            writeback = (*LRU).getAddr();       // Get the address of the LRU and place in global to be written back by next memory in the hierarchy
            writeback_bit = 1;
            (*LRU).setDirty(false);     // Set block to clean after the writeback has occurred
        }
    }
    return 0;
}

// Function for searching for a block in the victim cache set
int set::VictimSearch(unsigned long int victim_tag, unsigned long int assoc) {
    /*************************************
    Function 	returns 0 if miss
                returns 1 if hit
    **************************************/

    auto first = blocks.begin();
    auto LRU = blocks.begin();
    auto interator = blocks.begin();
    auto last = blocks.end();
    unsigned long int LRU_block = assoc - 1;        // Calculate LRU number

    // Iterate through the set looking for the block
    while (interator!=last) {

        // Save the LRU block in case of a dirty eviction
        if ((*interator).getLRU()==LRU_block) {
            LRU = interator;
        }

        // Check to see if the block is found
        if (((*interator).getVictimTag()==victim_tag)&&((*interator).isValid())) {
            // Update the LRUs of the blocks
            while (first!=last) {
                if ((*first).getLRU() < (*interator).getLRU()) {
                    (*first).increment_LRU_counter();
                }
                ++first;
            }
            if((*interator).isDirty()) dirty_swap = true;   // If the evicted block is dirty, set dirty_swap to true to signal the inserted block to be dirty in the L1 cache
            (*interator).setLRU(0);     // Set the LRU of the found block to 0 (MRU)
            return 1;
        }
        ++interator;        // Move to the next block to check
    }

    // Check to see if there should be a writeback to the next memory hierarchy
    if((*LRU).isDirty() && (*LRU).isValid()) {
        writeback = (*LRU).getAddr();       // Get the address of the LRU and place in global to be written back by next memory in the hierarchy
        writeback_bit = 1;
        (*LRU).setDirty(false);     // Set block to clean after the writeback has occurred
    }
    return 0;
}

// Function for inserting a block in a set
void set::insert(char str, unsigned long int addr, unsigned long int tag, unsigned long int assoc) {
    auto first = blocks.begin();
    auto last = blocks.end();
    unsigned long int LRU_block = assoc - 1;    // Calculate LRU number

    // Search for LRU block
    while (first!=last) {
        // When LRU block is found, update its contents
        if ((*first).getLRU()==LRU_block) {
            (*first).setAddr(addr);
            (*first).setTag(tag);
            (*first).setLRU(0);
            (*first).setValid(true);
#if defined(DEBUG)
            printf("L%d update LRU\n", name);
#endif
            if(str == 'w') {
#if defined(DEBUG)
                printf("L%d set dirty\n", name);
#endif
                (*first).setDirty(true);
                dirty_swap = false;
            }
            else if (str == 'r') {
                // If this is a victim swap and the block from the victim cache is dirty, set the block as dirty
                if(dirty_swap) {
                    (*first).setDirty(true);
                    dirty_swap = false;
                }
                else {
                    (*first).setDirty(false);
                }
            }
        }
        // If the block is not the LRU, increment its LRU counter
        else {
            (*first).increment_LRU_counter();
        }
        ++first;        // Move to the next block
    }
}

void set::VictimInsert(unsigned long int victim_tag, unsigned long int assoc, unsigned long int addr, bool dirty_in) {

    auto first = blocks.begin();
    auto last = blocks.end();
    unsigned long int LRU_block = assoc - 1;    // Calculate LRU number

    // Search for LRU block
    while (first!=last) {
        // When LRU block is found, update its contents
        if ((*first).getLRU()==LRU_block) {
#if defined(DEBUG)
            if((*first).isValid()) {
                if(writeback_bit) {
                    printf("VC victim: %lx (tag %lx, index 0, dirty)\n", (*first).getAddr() >> offset_bits << offset_bits,
                           (*first).getVictimTag());
                }
                else {
                    printf("VC victim: %lx (tag %lx, index 0, clean)\n", (*first).getAddr() >> offset_bits << offset_bits,
                           (*first).getVictimTag());
                }
            }
            else {
                printf("VC victim: none\n");
            }
#endif
            (*first).setAddr(addr);
            (*first).setVictimTag(victim_tag);
            (*first).setLRU(0);
            (*first).setValid(true);
            (*first).setDirty(dirty_in);
        }
            // If the block is not the LRU, increment its LRU counter
        else {
            (*first).increment_LRU_counter();
        }
        ++first;// Move to the next block
    }
#if defined(DEBUG)
    printf("VC update LRU\n");
#endif
}

// Function for swapping a desired block in the victim cache with another block
void set::VictimSwap(unsigned long int victim_tag, bool new_valid, bool new_dirty, unsigned long int new_address, unsigned long int new_victim_tag) {

    auto interator = blocks.begin();
    auto last = blocks.end();

    // Look for the desired block
    while (interator!=last) {
        // Check to see if the block is found
        if (((*interator).getVictimTag()==victim_tag)&&((*interator).isValid())) {
            // Update the block in victim cache
            (*interator).setDirty(new_dirty);
            (*interator).setVictimTag(new_victim_tag);
            (*interator).setAddr(new_address);
            (*interator).setValid(new_valid);
#if defined(DEBUG)
            printf("VC update LRU\n");
#endif
        }
        ++interator;        // Move to the next block
    }
}

// Function for retrieving the info of the LRU block in a given set
block_info set::getLRUinfo(unsigned long int assoc) {

    auto first = blocks.begin();
    auto last = blocks.end();
    unsigned long int LRU_block = assoc - 1;    // Calculate LRU number
    block_info block;       // Block to be returned

    // Search for LRU block
    while (first!=last) {
        // When LRU block is found, update the info of the block to be returned
        if ((*first).getLRU()==LRU_block) {
            block.valid = (*first).isValid();
            block.dirty = (*first).isDirty();
            block.address = (*first).getAddr();
            block.tag = (*first).getTag();
            return block;
        }
        ++first;    // Move to the next block
    }
    return block;
}

// Function for printing the contents of a set
void set::print(void) {
    auto first = blocks.begin();
    auto interator = blocks.begin();
    auto last = blocks.end();

    // For each block in the set, print its tag
    for(unsigned int i = 0; i < blocks.size(); i++) {
        interator = first;
        // Search for the desired LRU counter to print
        while (interator != last) {
            // Print the blocks from MRU to LRU
            if((*interator).getLRU() == i) {
                printf("%lx", (*interator).getTag());
                // Print dirty information
                if ((*interator).isDirty()) {
                    printf(" D  ");
                } else {
                    printf("    ");
                }
                break;
            }
            ++interator;        // Move to the next block
        }
    }
}

// Function for printing the contents of a victim cache
void set::VCprint(void) {
    auto first = blocks.begin();
    auto interator = blocks.begin();
    auto last = blocks.end();

    // For each block in the set, print its tag
    for(unsigned int i = 0; i < blocks.size(); i++) {
        interator = first;
        // Search for the desired LRU counter to print
        while (interator != last) {
            // Print the blocks from MRU to LRU
            if((*interator).getLRU() == i) {
                printf("%lx", (*interator).getAddr() >> offset_bits);   // Victim cache tag printed
                // Print dirty information
                if ((*interator).isDirty()) {
                    printf(" D  ");
                } else {
                    printf("    ");
                }
                break;
            }
            ++interator;        // Move to the next block
        }
    }
}

// initialization for block class
block::block() {
    valid = false;
    dirty = false;
    address = 0;
    victim_tag = 0;
    tag = 0;
    LRU_counter = 0;
}
