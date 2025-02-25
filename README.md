# NXP-TWR
This is about a software module for performance measurements primarily on an NXP (ex Freescale) TWR-K70F120M platform
The multithreaded design of the software module is based upon an algorithm which generates iteratively multiple processes (jobs) and threads (tasks), which run concurrently in the same core. Thus, the Cortex-M4 core is shared by at least two software threads supported by a single hardware thread. The processes generated perform two types of computations: sorting and matrix operations. Each process contains a group of two threads assigned to run concurrently on a single core. Therefore, on every execution run, initially two software threads, and later multiples of twos are instantiated. The results obtained contribute towards primarily timing measurements upon which is evaluated the performance of the TWR-K70F120M device.

Adam, G.K. Timing and Performance Metrics for TWR-K70F120M Device. Computers 2023, 12, 163. https://doi.org/10.3390/computers12080163 
