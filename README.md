# Virtual-Memory-Simulator

UGA CSCI4730 Project detailed around understanding how virtual memory is used in conjunction with physical memory to improve code's runtime. 

Usage: ./vm 3 1 ./input_file  // 3 Physical frames, replacement policy 1 (FIFO)
Number of virtual pages and processes are defined in the input file

Tracks separate arrays of virtual and physical memory storing chars. Initially all physical memory is empty. When a section of virtual memory is needed to be used it's loaded into physical memory and tracked using a pagetable. If all physical frames are full when physical memory is needed then a victim physical frame is selected using a page replacement algorithm (First-In-First-Out, Least-Recently-Used, or Second-Chance).

Output is recorded in output.txt tracking page hits and misses as well as the number of writes and reads to physical memory.
