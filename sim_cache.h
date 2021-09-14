//
// Created by ianpi on 9/6/2021.
//

#ifndef SIM_CACHE_H
#define SIM_CACHE_H
//#define DEBUG
using namespace std;

typedef struct cache_params{
    unsigned long int block_size;
    unsigned long int l1_size;
    unsigned long int l1_assoc;
    unsigned long int vc_num_blocks;
    unsigned long int l2_size;
    unsigned long int l2_assoc;
}cache_params;

typedef struct local_cache_params{
    unsigned long int block_size;
    unsigned long int size;
    unsigned long int assoc;
    unsigned long int vc_num_blocks;
    unsigned int num_sets;
    unsigned int index_bits;
    unsigned int block_offset_bits;
    int name;
}local_cache_params;

typedef struct simulation_results{
    int L1_reads;
    int L1_read_misses;
    int L1_writes;
    int L1_write_misses;
    int swap_requests;
    float swap_request_rate;
    int number_of_swaps;
    float L1_VC_miss_rate;
    int writebacks_from_L1_VC;
    int L2_reads;
    int L2_read_misses;
    int L2_writes;
    int L2_write_misses;
    float L2_miss_rate;
    int writebacks_from_L2;
    int total_memory_traffic;
}simulation_results;

typedef struct block_info{
    bool valid;
    bool dirty;
    unsigned long int address;
    unsigned long int tag;
    unsigned long int victim_tag;
}block_info;


#endif //SIM_CACHE_H
