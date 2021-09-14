# FlexibleCacheMemorySimulator

The project was completed as part of ECE 563 (Microprocessor Architecture) at NC State. In this project, I implemented a flexible cache and memory hierarchy simulator in C++ and used it to compare the performance, area, and energy of different memory hierarchy configurations.

The simulator is able to simulate any combination of an L1 Cache, L1 Victim Cache, and L2 Cache. Command-line parameters which govern the specifics of the memory being simulated include block size, L1 cache size, L1 cache associativity, number of victim cache blocks, L2 cache size, and L2 cache associativity.

### Memory Heirarchy specifications:
Replacement Policy: LRU
Write policy: write-back + write-allocate
Victim cache associativity: Fully associative

### The simulator outputs the following:
* Memory hierarchy configuration and trace filename.
* The final contents of all caches.
* The following measurements:
  * number of L1 reads
  * number of L1 read misses
  * number of L1 writes
  * number of L1 write misses
  * number of swap requests from L1 to its VC
  * swap request rate
  * number of swaps between L1 and its V
  * combined L1+VC miss rate
  * number of writebacks from L1 or its VC (if enabled), to next level
  * number of L2 reads
  * number of L2 read misses
  * number of L2 writes
  * number of L2 write misses
  * L2 miss rate 
  * number of writebacks from L2 to memory
  * total memory traffic = number of blocks transferred to/from memory
