# FlexibleCacheMemorySimulator

The project was completed as part of ECE 563 (Microprocessor Architecture) at NC State. In this project, I implemented a flexible cache and memory hierarchy simulator in C++ and used it to compare the performance, area, and energy of different memory hierarchy configurations.

The simulator is able to simulate any combination of an L1 Cache, L1 Victim Cache, and L2 Cache. Command-line parameters which govern the specifics of the memory being simulated include block size, L1 cache size, L1 cache associativity, number of victim cache blocks, L2 cache size, and L2 cache associativity.

Memory Heirarchy specifications:
Replacement Policy: LRU
Write policy: write-back + write-allocate
Victim cache associativity: Fully associative

The simulator outputs the following:
1. Memory hierarchy configuration and trace filename.
2. The final contents of all caches.
3. The following measurements:
  a. number of L1 reads
  
  b. number of L1 read misses
  
  c. number of L1 writes
  
  d. number of L1 write misses
  
  e. number of swap requests from L1 to its VC
  f. swap request rate
  g. number of swaps between L1 and its V
  h. combined L1+VC miss rate
  i. number of writebacks from L1 or its VC (if enabled), to next level
  j. number of L2 reads
  k. number of L2 read misses
  l. number of L2 writes
  m. number of L2 write misses
  n. L2 miss rate 
  o. number of writebacks from L2 to memory
  p. total memory traffic = number of blocks transferred to/from memory
