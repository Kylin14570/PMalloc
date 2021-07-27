#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libpmem.h>
#include <pthread.h>

/*
** Support 16 threads at most
*/
#define MAX_THREAD_NUMBER 16

/*
** Each SuperBlock is 16KB
*/
#define SUPER_BLOCK_SIZE (16 * 1024)  // 16KB

/*
** Size limitation for malloc is 8KB
** If the memory that user application required is lager than 8KB,
** it will be allocated from the file-mapped memory space directly.
** Otherwise, it will be allocated from the ThreadCache.
*/
#define LIMIT_SIZE (SUPER_BLOCK_SIZE >> 1)  // 8KB


/*
** Capacity limitation of a ThreadCache
** When a cache's size is beyond this value, it should be flushed.
*/
#define CACHE_LIMIT (1024 * 1024)  // 1MB

/*
** There are 37 size classes
** More details in "sizeclass.c"
*/
#define SIZE_CLASS_NUMBER 37

/*
**  Magic number
*/
#define MAGIC 0x2021B510



/*****************************************************************************
**  DataStructure definitions 
******************************************************************************/
typedef unsigned long long offset_t;

    

struct SuperBlockDescriptor
{
    offset_t superBlcok;    // Start address of the SuperBlock
    int blockSize;       // Size of small blocks in the SuperBlock
    int maxCount;        // Total number of small blocks
    int SizeClassIndex;     // Index of the blocksize in size class
    int availableCount; // The number of available blocks in the SuperBlock
    offset_t firstBlock; // The first available block's offset
};

struct ThreadCache
{
    pthread_t TID;                      // ID of the thread the Cache belongs to
    
    offset_t sc[SIZE_CLASS_NUMBER]; // Array of pointers to all size classes in the Cache
    // For example, sc[1] points to the first available block in the size of sizeclass[1] in the Cache.

    int blockCount[SIZE_CLASS_NUMBER]; // Array of counters of each size class' available blocks
    // For example, blockCount[1] is the number of available blocks in the size of sizeclass[1] in the Cache.
};


struct ROOT
{
    offset_t objectAddress; // address of the object (an allocated memory block)
    offset_t nextRoot;      // pointer to the next ROOT node
};

struct GlobalDescriptor
{
    /*
    ** We use a MAGIC number to judge whether this is the first time to map a file.
    ** When we firstly map a file, we will write a 32bits MAGIC at the beginning of the file.
    ** So each time we map a file, we can compare the first 32bits with the MAGIC number.
    ** If they are equal, the file must have been mapped before.
    ** Otherwise, it's the first time to map it.
    */
    unsigned int magic;

    /*
    ** The tag is used to judge whether there was a crash before.
    ** It will increase one when a thread created and decrease one when a thread terminated.
    ** So it should remain 0 after each normal exit.
    ** Otherwise, there must be a crash so that some thread wasn't able to decrease it.
    */
    int tag;

    // Total size of the file-mapped memory space
    unsigned long long MemorySize;

    // The offset of the ThreadCache domain in the memory
    offset_t ThreadCacheOffset;

    // The offset of the SuperBlockDescriptor domain in the memory
    offset_t SuperBlockDescriptorOffset;

    // The offset of user space in the memory
    offset_t UserSpaceOffset;

    // Index of first partial SuperBlocks in each size class
    int firstPartialSB[SIZE_CLASS_NUMBER];

    // Index of first free SuperBlcok
    int firstFreeSB;

    // Address of the first ROOT item
    offset_t Root;

    // Address of the first free ROOT item
    offset_t FreeRoot;

    // Quantity of SBs
    unsigned int SBnumber;
};