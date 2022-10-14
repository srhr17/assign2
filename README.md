# Manual to run the buffer manager program #

### To run the buffer manager program in linux server execute the below commands in order ###

Navigate to assign2 folder and execute below commands

$ make                                                                                                                                                                        

$ ./buffermanager

### What is this repository for? ###

* This repository contains C program code files required to initiate buffer pool, Shutdown buffer pool and perform other operations such as get Dirty Flag value, get Fix counts, get number of read and write
* Version 1
* Repository contains, C program files, C test cases, C header files, output files, make file and ReadMe instrcution manual.

### Project Members ###

* Ramya Krishnan(rkrishnan1@hawk.iit.edu)
* Jason Scott(jscott30@hawk.iit.edu)
* Darek Nowak(dnowak2@hawk.iit.edu)

### Functions Implemented ###

#### Statistics Interface ####

##### PageNumber *getFrameContents (BM_BufferPool *const bm) #####
1. Check if the management data is valid
2. If no, return an error, RC_BUFFER_POOL_NOT_INIT
3. if yes, get number of pages in the buffer pool
4. Initiate PageNumber, loop through the buffer pool management data and assign the page num's i'th value to the frame content's i'th value.
5. Return the page number

##### bool *getDirtyFlags (BM_BufferPool *const bm) #####
1. Check if the management data is valid
2. If no, return an error, RC_BUFFER_POOL_NOT_INIT
3. if yes, get number of pages in the buffer pool
4. Initiate dirty flag as int, loop through the buffer pool management data and assign the data flag value of i'th  to the dirty flag's i'th value.
5. Return the array of boolean values containing dirty flag values

##### int *getFixCounts (BM_BufferPool *const bm) #####
1. Check if the management data is valid
2. If no, return an error, RC_BUFFER_POOL_NOT_INIT
3. if yes, get number of pages in the buffer pool
4. Initiate fix count, loop through the buffer pool management data and assign the fix count value of i'th  to the fix count's i'th value.
5. Return the array of int values containing fix count values of the page stored in the i'th page frame

##### int getNumReadIO (BM_BufferPool *const bm) #####
1. Check if the management data is valid
2. If no, return an error, RC_BUFFER_POOL_NOT_INIT
3. if yes, return the numReadIO from management data

##### int getNumWriteIO (BM_BufferPool *const bm) #####
1. Check if the management data is valid
2. If no, return an error, RC_BUFFER_POOL_NOT_INIT
3. if yes, return the numWriteIO from management data

#### Buffer Manager Interface Access Pages ####



#### Buffer Manager Interface Pool Handling functions ####



### Contribution guidelines ###

* Statistics Interface - Ramya Krishnan
* Buffer Manager Interface Pool Handling - Jason Scott
* Buffer Manager Interface Access Pages - Darek Nowak


