#include "def.h"

extern char *BaseAddress;
extern struct GlobalDescriptor *GD;

extern void *offset2ptr(offset_t offset);
extern offset_t CachePop(struct ThreadCache *cache, int SCindex);
extern void SBfree(struct SuperBlockDescriptor *desc);
extern void SBpartial(struct SuperBlockDescriptor *desc);
extern struct SuperBlockDescriptor *GetSBdescriptor(offset_t block);
extern int CAS64();

/*
**  Get current thread's cache
**  Return a pointer of the ThreadCache
*/
struct ThreadCache *GetThreadCache()
{
    int i;
    pthread_t tid = pthread_self();
    struct ThreadCache *cache =
        (struct ThreadCache *)offset2ptr(GD->ThreadCacheOffset);

    for (i = 0; i < MAX_THREAD_NUMBER && cache[i].TID != tid; i++)
        ; // Find this thread's ThreadCache
    if (i == MAX_THREAD_NUMBER)
    { // Not Found
        printf("Unexpected error in GetThreadCache() !\n");
        exit(0);
    }
    return &(cache[i]);
}

/*
** Initialize a thread after its born
** User applications should call this function immediately after creating a thread.
*/
void ThreadInit()
{
    /* Increase the tag in global descriptor */
    ((struct GlobalDescriptor *)BaseAddress)->tag++;
    pmem_persist(&(((struct GlobalDescriptor *)BaseAddress)->tag), 4);

    /* Get a ThreadCache */
    int i;
    pthread_t tid = pthread_self();
    struct ThreadCache *cache =
        (struct ThreadCache *)offset2ptr(GD->ThreadCacheOffset);

    do{
        for(i = 0; i < MAX_THREAD_NUMBER; i++)
            if(cache[i].TID == 0) // Find a free ThreadCache.
                break;
        if (i == MAX_THREAD_NUMBER)
        {
            printf("Unexpected error in ThreadInit() !\n");
            exit(0);
        }
    }while ( ! CAS64( &(cache[i].TID), 0, tid) ); // Occupy the ThreadCache.

    printf("Thread %lu has been initialized\n\n",tid);

}

void ThreadDestroy()
{
    struct ThreadCache *cache = GetThreadCache();

    // 1. Reclaim unused blocks in the cache
    for (int i = 0; i < SIZE_CLASS_NUMBER; i++)
    {
        while (cache->blockCount[i])
        {
            offset_t block = CachePop(cache, i);
            struct SuperBlockDescriptor *desc = GetSBdescriptor(block);
            *(offset_t *)offset2ptr(block) = desc->firstBlock;
            desc->firstBlock = block;
            (desc->availableCount)++;
            if (desc->availableCount == desc->maxCount)
                SBfree(desc);
            else
                SBpartial(desc);
        }
    }

    // 2. Initialize the ThreadCache.
    pthread_t tid = cache->TID;
    cache->TID = 0;

    ((struct GlobalDescriptor *)BaseAddress)->tag--;
    pmem_persist(&(((struct GlobalDescriptor *)BaseAddress)->tag), 4);

    printf("Thread %lu has been Destroyed\n\n",tid);
}
