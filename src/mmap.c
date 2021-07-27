#include "def.h"

char * BaseAddress;

struct GlobalDescriptor *GD;

extern void *offset2ptr(offset_t offset);

/*
** Function: Initialize the file-mapped memory space, writing into metadata.
** Argument1: Base address of the memory space 
** Argument2: Length of the memoryspace
*/
void Initialize(char *baseAddress, int len)
{
#ifdef DEBUG
    printf("Initialize the pool...\n");
#endif

    unsigned long long GlobalDescriptorLength = 
                        sizeof(struct GlobalDescriptor);
    
    unsigned long long ThreadCacheLength = 
                        MAX_THREAD_NUMBER * sizeof(struct ThreadCache);
    
    unsigned long long SBcount =
                        (len - GlobalDescriptorLength - ThreadCacheLength) 
                        /(SUPER_BLOCK_SIZE + sizeof(struct SuperBlockDescriptor));
    
    GD->tag = 0;
    
    GD->MemorySize = len;
    
    GD->ThreadCacheOffset = GlobalDescriptorLength;
    
    GD->SuperBlockDescriptorOffset = 
                                    GlobalDescriptorLength + ThreadCacheLength;
    
    GD->UserSpaceOffset = GlobalDescriptorLength 
                                     + ThreadCacheLength 
                                     + SBcount * sizeof(struct SuperBlockDescriptor);
    
    GD->firstFreeSB = 0;
    
    for(int i=0; i < SIZE_CLASS_NUMBER; i++)
        GD->firstPartialSB[i] = -1;
    
    GD->Root = 0;
    
    GD->FreeRoot = 0;
    
    GD->SBnumber = SBcount;
    
    pmem_persist(baseAddress, GlobalDescriptorLength);
    
    pmem_memset_persist(
        offset2ptr(GD->ThreadCacheOffset), 0,
        ThreadCacheLength
    );
    
    pmem_memset_persist(
        offset2ptr(GD->SuperBlockDescriptorOffset), 0,
        SBcount * sizeof(struct SuperBlockDescriptor)
    );

    GD->magic = MAGIC;
    pmem_persist(baseAddress, 4);

    printf("The memory pool has been initialized!\n");
    printf("Base address -- %llx\n", (unsigned long long)BaseAddress);
    printf("Size         -- %llu\n", GD->MemorySize);
    printf("Thread Cache -- %llx\n", GD->ThreadCacheOffset);
    printf("SBdescriptor -- %llx\n", GD->SuperBlockDescriptorOffset);
    printf("User Space   -- %llx\n", GD->UserSpaceOffset);
    printf("There are %llu SuperBlocks.\n\n", SBcount);

}

void Recovery(char *BaseAddress, int len)
{
    printf("recover\n\n");

    // TODO
    // 1. Garbage collection via reachability from the ROOT
    // 2. Initialize all the ThreadCaches.
    // 3. Clear the tag
}

/*
** Function: Memory map a file into a continous user memory space(If the file doesn't exist, it will be created).
**           If this is the first time to map this file, initialize the layout of the memory space.
**           Otherwise, check the persistence of the memory space and do some garbage collection if necessary.
** Argument1: Path of the mapped file
** Argument2: Length of the mapped file 
*/
char *PMalloc_map_file(const char *filePath, int len)
{
    size_t MappedLength;
    int IsPM;

    // Call PMDK's interface to map the file
    BaseAddress = (char *)pmem_map_file(
        filePath, len,
        PMEM_FILE_CREATE, 0666,
        &MappedLength, &IsPM);

    if (BaseAddress == NULL)
    {
        printf("Failed to map the file!\n\n");
        return NULL; // Something wrong
    }

    printf("Mapped the file successfully!\n\n");

    GD = (struct GlobalDescriptor *)BaseAddress;

    // This is the first time to map the file.
    if (GD->magic != MAGIC)
        Initialize(BaseAddress, len);
    // A crash happened last time.
    else if (GD->tag != 0)
        Recovery(BaseAddress, len);

    return BaseAddress;
}