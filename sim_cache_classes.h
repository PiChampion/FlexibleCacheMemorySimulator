//
// Created by ianpi on 9/6/2021.
//

#ifndef SIM_CACHE_CLASSES_H
#define SIM_CACHE_CLASSES_H
#include <vector>

class victim_CACHE;
class CACHE;
class set;
class block;

class victim_CACHE {
public:
    unsigned int vc_num_blocks;
    vector<set> sets;
    explicit victim_CACHE(unsigned long int vc_num_blocks_in);
    int Execute(unsigned long int victim_tag);
    void swap(unsigned long int victim_tag, bool new_valid, bool new_dirty, unsigned long int new_address, unsigned long int new_victim_tag);
    void Receive(unsigned long int victim_tag, bool dirty_in, unsigned long int addr);
private:
};

class CACHE {
public:
    local_cache_params my_params;
    vector<set> sets;
    vector<victim_CACHE> victim_caches;
    CACHE(int name, unsigned long int block_size_in, unsigned long int size_in,
          unsigned long int assoc_in, unsigned long int vc_num_blocks_in);
    int Execute(char str, unsigned long int addr);
    void Receive(char str, unsigned long int addr);
    void print(void);
    void VCprint(void);
private:
};

class set {
public:
    int search(int name, unsigned long int tag, char str, unsigned long int assoc);
    int VictimSearch(unsigned long int tag, unsigned long int assoc);
    void insert(char str, unsigned long int addr, unsigned long int tag, unsigned long int assoc);
    void VictimInsert(unsigned long int tag, unsigned long int assoc, unsigned long int addr, bool dirty_in);
    void VictimSwap(unsigned long int tag, bool new_valid, bool new_dirty, unsigned long int new_address, unsigned long int new_tag);
    block_info getLRUinfo(unsigned long int assoc);
    void print(void);
    void VCprint(void);
    vector<block> blocks;
    explicit set(unsigned long int assoc);
private:

};

class block {
public:
    bool isValid() const {return valid;}
    bool isDirty() const {return dirty;}
    unsigned long int getAddr() const {return address;}
    unsigned long int getTag() const {return tag;}
    unsigned long int getVictimTag() const {return victim_tag;}
    unsigned long int getLRU() const {return LRU_counter;}
    void setLRU(unsigned long int LRU_number) {LRU_counter = LRU_number;}
    void setVictimTag(unsigned long int victim_tag_in) {victim_tag = victim_tag_in;}
    void setTag(unsigned long int tag_in) {tag = tag_in;}
    void setAddr(unsigned long int address_in) {address = address_in;}
    void setValid(bool valid_in) {valid = valid_in;}
    void setDirty(bool dirty_in) {dirty = dirty_in;}
    void increment_LRU_counter() {LRU_counter++;}
    block();
private:
    bool valid;
    bool dirty;
    unsigned long int address;
    unsigned long int victim_tag;
    unsigned long int tag;
    unsigned long int LRU_counter;
};

#endif //SIM_CACHE_CLASSES_H
