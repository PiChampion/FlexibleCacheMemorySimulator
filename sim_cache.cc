#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "sim_cache.h"
#include "sim_cache_classes.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim_cache 32 8192 4 7 262144 8 gcc_trace.txt
    argc = 8
    argv[0] = "sim_cache"
    argv[1] = "32"
    argv[2] = "8192"
    ... and so on
*/

/* function declarations */
void L1miss(CACHE &L1_cache, char rw, unsigned long int addr);
void L1L2miss(CACHE &L1_cache, CACHE &L2_cache, char rw, unsigned long int addr);
void calculateResults(cache_params params);

/* global variables */
unsigned long int writeback;        // Global variable for transferring writeback data information
unsigned long int writeback_bit;    // Global variable for signalling that a writeback has occurred
bool has_victim_cache = false;      // Global variable for indicating if a victim cache is used in the system

// Global variable used to keep track of results used for printing
simulation_results results = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    cache_params params;    // look at sim_cache.h header file for the the definition of struct cache_params
    char rw;                // variable holds read/write type read from input file. The array size is 2 because it holds 'r' or 'w' and '\0'. Make sure to adapt in future projects
    unsigned long int addr; // Variable holds the address read from input file
	char str[2];            // Variable holds the read/write command read from input file
    int L1_name = 1;        // Variable holds the name passed to L1 Cache to let the cache know it is L1
    int L2_name = 2;        // Variable holds the name passed to L1 Cache to let the cache know it is L2
    unsigned long int saved_writeback;  // Variable holds previous writeback if multiple writebacks occur successively
    
    if(argc != 8)           // Checks if correct number of inputs have been given. Throw error and exit if wrong
    {
        printf("Error: Expected inputs:7 Given inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }

    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    params.block_size       = strtoul(argv[1], NULL, 10);
    params.l1_size          = strtoul(argv[2], NULL, 10);
    params.l1_assoc         = strtoul(argv[3], NULL, 10);
    params.vc_num_blocks    = strtoul(argv[4], NULL, 10);
    params.l2_size          = strtoul(argv[5], NULL, 10);
    params.l2_assoc         = strtoul(argv[6], NULL, 10);
    trace_file              = argv[7];
    if(params.vc_num_blocks) {
        has_victim_cache = true;
    }

    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == nullptr)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

    // Print params
    printf("  ===== Simulator configuration =====\n"
           "  BLOCKSIZE:                     %lu\n"
           "  L1_SIZE:                          %lu\n"
           "  L1_ASSOC:                         %lu\n"
           "  VC_NUM_BLOCKS:                    %lu\n"
           "  L2_SIZE:                          %lu\n"
           "  L2_ASSOC:                         %lu\n"
           "  trace_file:                       %s\n", params.block_size, params.l1_size, params.l1_assoc, params.vc_num_blocks, params.l2_size, params.l2_assoc, trace_file);

    CACHE L1_cache(L1_name, params.block_size, params.l1_size, params.l1_assoc, params.vc_num_blocks);  // Initialize L1 Cache
    if(!params.l2_assoc) params.l2_assoc = 1;     // Is there is no L2 Cache, set its assoc to 1 so initializing the cache doesn't divide by 0
    CACHE L2_cache(L2_name, params.block_size, params.l2_size, params.l2_assoc, 0); 					// Initialize L2 Cache
    int memory_access = 1;              // Variable to keep track of memory accesses
	
    while(fscanf(FP, "%s %lx", str, &addr) != EOF)
    {
#if defined(DEBUG)
        printf("----------------------------------------\n");
#endif
#if defined(DEBUG)
        if (rw == 'r')
                printf("#%d: %s %lx\n", memory_access, "read", addr);           // Print and test if file is read correctly
            else if (rw == 'w')
                printf("#%d: %s %lx\n", memory_access, "write", addr);          // Print and test if file is read correctly
#endif
        
		rw = str[0];				// Variable for holding read/write character
		// If there is an L2 Cache
        if(params.l2_size) {
            if(params.vc_num_blocks) {
                switch(L1_cache.Execute(rw, addr)) {
                    case 0:		// Read L1 Miss
                        L1L2miss(L1_cache, L2_cache, rw, addr);
                        results.L1_reads++;
                        results.L1_read_misses++;
                        break;
                    case 1:		// Read L1 Hit
                        results.L1_reads++;
                        break;
                    case 2:		// Write L1 Miss
                        L1L2miss(L1_cache, L2_cache, rw, addr);
                        results.L1_writes++;
                        results.L1_write_misses++;
                        break;
                    case 3:		// Write L1 Hit
                        results.L1_writes++;
                        break;
                    case 4:		// Read L1 miss VC hit
                        results.swap_requests++;
                        results.L1_reads++;
                        results.L1_read_misses++;
                        results.number_of_swaps++;
                        break;
                    case 5:		// Write L1 miss VC hit
                        results.swap_requests++;
                        results.L1_writes++;
                        results.L1_write_misses++;
                        results.number_of_swaps++;
                        break;
                    default:
                        break;
                }
            }
            else {
                switch(L1_cache.Execute(rw, addr)) {
                    case 0:		// Read L1 Miss
                        L1L2miss(L1_cache, L2_cache, rw, addr);
                        results.L1_reads++;
                        results.L1_read_misses++;
                        break;
                    case 1:		// Read L1 Hit
                        results.L1_reads++;
                        break;
                    case 2:		// Write L1 Miss
                        L1L2miss(L1_cache, L2_cache, rw, addr);
                        results.L1_writes++;
                        results.L1_write_misses++;
                        break;
                    case 3:		// Write L1 Hit
                        results.L1_writes++;
                        break;
                    default:
                        break;
                }
            }
        }
        else {
            if(params.vc_num_blocks) {
                switch(L1_cache.Execute(rw, addr)) {
                    case 0:		// Read L1 Miss
                        L1miss(L1_cache, rw, addr);
                        results.L1_reads++;
                        results.L1_read_misses++;
                        break;
                    case 1:		// Read L1 Hit
                        results.L1_reads++;
                        break;
                    case 2:		// Write L1 Miss
                        L1miss(L1_cache, rw, addr);
                        results.L1_writes++;
                        results.L1_write_misses++;
                        break;
                    case 3:		// Write L1 Hit
                        results.L1_writes++;
                        break;
                    case 4:		// Read L1 miss VC hit
                        results.swap_requests++;
                        results.L1_reads++;
                        results.L1_read_misses++;
                        results.number_of_swaps++;
                        break;
                    case 5:		// Write L1 miss VC hit
                        results.swap_requests++;
                        results.L1_writes++;
                        results.L1_write_misses++;
                        results.number_of_swaps++;
                        break;
                    default:
                        break;
                }
            }
            else {
                switch(L1_cache.Execute(rw, addr)) {
                    case 0:		// Read L1 Miss
                        L1miss(L1_cache, rw, addr);
                        results.L1_reads++;
                        results.L1_read_misses++;
                        break;
                    case 1:		// Read L1 Hit
                        results.L1_reads++;
                        break;
                    case 2:		// Write L1 Miss
                        L1miss(L1_cache, rw, addr);
                        results.L1_writes++;
                        results.L1_write_misses++;
                        break;
                    case 3:		// Write L1 Hit
                        results.L1_writes++;
                        break;
                    default:
                        break;
                }
            }
        }
        memory_access++;
    }

    // Calculate simulation results
    calculateResults(params);

    // Print L1 Cache contents
    printf("\n===== L1 contents =====\n");
    L1_cache.print();

    // If there was a victim cache, print its contents
    if(params.vc_num_blocks) {
        printf("\n===== VC contents =====\n  set   0:  ");
        L1_cache.VCprint();
        printf("\n");
    }

    // If there was an L2 cache, print its contents
    if(params.l2_size) {
        printf("\n===== L2 contents =====\n");
        L2_cache.print();
    }

    // Print simulation results
    printf("\n===== Simulation results =====\n"
           "  a. number of L1 reads:                       %d\n"
           "  b. number of L1 read misses:                  %d\n"
           "  c. number of L1 writes:                      %d\n"
           "  d. number of L1 write misses:                 %d\n"
           "  e. number of swap requests:                      %d\n"
           "  f. swap request rate:                       %0.4f\n"
           "  g. number of swaps:                              %d\n"
           "  h. combined L1+VC miss rate:                %0.4f\n"
           "  i. number writebacks from L1/VC:              %d\n"
           "  j. number of L2 reads:                           %d\n"
           "  k. number of L2 read misses:                     %d\n"
           "  l. number of L2 writes:                          %d\n"
           "  m. number of L2 write misses:                    %d\n"
           "  n. L2 miss rate:                            %0.4f\n"
           "  o. number of writebacks from L2:                 %d\n"
           "  p. total memory traffic:                     %d\n",
           results.L1_reads, results.L1_read_misses, results.L1_writes, results.L1_write_misses, results.swap_requests,
           results.swap_request_rate, results.number_of_swaps, results.L1_VC_miss_rate,
           results.writebacks_from_L1_VC, results.L2_reads, results.L2_read_misses, results.L2_writes,
           results.L2_write_misses, results.L2_miss_rate, results.writebacks_from_L2, results.total_memory_traffic);
    return 0;
}




 
// Function for handling a miss in L1 cache when the system does not contain an L2 cache
void L1miss(CACHE &L1_cache, char rw, unsigned long int addr) {

    // If writeback from L1 Cache, need to write data to main memory
    if(writeback_bit) {
        results.writebacks_from_L1_VC++;
        writeback_bit = 0;
    }
    L1_cache.Receive(rw, addr);     // L1 Cache receives the data from memory
}

// Function for handling a miss in L1 cache when the system contains an L2 cache
void L1L2miss(CACHE &L1_cache, CACHE &L2_cache, char rw, unsigned long int addr) {

    unsigned long int saved_writeback;  // Variable holds previous writeback if multiple writebacks occur successively

    // If writeback from L1 Cache, need to write data to L2 Cache
    if(writeback_bit) {
        writeback_bit = 0;		//Reset writeback bit since writeback is now being handled
        results.writebacks_from_L1_VC++;
        saved_writeback = writeback;		//Save writeback in case L2 needs to writeback to main memory

        // Write address to L2 Cache
        switch(L2_cache.Execute('w', writeback)) {
            case 2:		// Write L2 Miss

                // If writeback, need to write data to main memory
                if(writeback_bit) {
                    results.writebacks_from_L2++;
                    writeback_bit = 0;		//Reset writeback bit since writeback is now being handled
                }
                results.L2_writes++;
                results.L2_write_misses++;
                L2_cache.Receive('w', saved_writeback);		// Send the saved writeback information to L2 Cache
                break;
            case 3:		// Write L2 Hit
                results.L2_writes++;
                break;
            default:
                break;
        }
    }

    // If no writeback from L1 Cache, just read the block from L2 Cache
    switch(L2_cache.Execute('r', addr)) {
        case 0:		// Read L2 Miss

            // If writeback, need to write data to main memory
            if(writeback_bit) {
                results.writebacks_from_L2++;
                writeback_bit = 0;
            }
            L2_cache.Receive('r', addr);		// L2 Cache receives the data from memory
            results.L2_reads++;
            results.L2_read_misses++;
            break;
        case 1:		// Read L2 Hit
            results.L2_reads++;
            break;
        default:
            break;
    }
    L1_cache.Receive(rw, addr);			// L1 Cache receives the data from memory
}

void calculateResults(cache_params params) {
    // Calculate L1 + VC miss rate
    results.L1_VC_miss_rate = (float(results.L1_read_misses) + float(results.L1_write_misses) - float(results.number_of_swaps))/(float(results.L1_reads) + float(results.L1_writes));

    // If there was not an L2 cache used, calculate total memory traffic
    if(!(params.l2_size)) {
        results.total_memory_traffic = results.L1_read_misses + results.L1_write_misses - results.number_of_swaps + results.writebacks_from_L1_VC;
    }

        // If there was an L2 cache used, calculate L2 miss rate and total memory traffic
    else {
        results.L2_miss_rate = float(results.L2_read_misses)/float(results.L2_reads);
        results.total_memory_traffic = results.L2_read_misses + results.L2_write_misses + results.writebacks_from_L2;
    }

    // If there is a victim cache, calculate swap rate
    if(params.vc_num_blocks) {
        results.swap_request_rate = float(results.swap_requests)/(float(results.L1_reads) + float(results.L1_writes));
    }
}





