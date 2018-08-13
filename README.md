# Mempool_os
Handling memory pool, and allocate some part of that to Threads as per requirement.
'memory splitting' is used (referred many experts codes for this!). 

**Basic purpose is to mimic the OS (is some sense :P) and understand how it works.


Prerequisites :

gcc (c compiler)



Compiling and execution :

1. for compiling use,
gcc mempool.c -o mempool -lpthread
-lpthread will enable pthread library link at compile time

2. for executing,
./mempool
