#include "def.h"

extern char *BaseAddress;
extern struct GlobalDescriptor *GD;
extern int sizeclass[];

extern void *offset2ptr(offset_t offset);
extern offset_t ptr2offset(void *ptr);
extern struct SuperBlockDescriptor *GetANewSB();
extern struct SuperBlockDescriptor *GetAPartialSB(int SCindex);
extern void blockFree(offset_t block);

int CacheEmpty(struct ThreadCache *cache, int SCindex)
{
    return cache->blockCount[SCindex] == 0;
}

int CacheFull(struct ThreadCache *cache, int SCindex)
{
    return cache->blockCount[SCindex] * sizeclass[SCindex] >= CACHE_LIMIT;
}

void CachePush(struct ThreadCache *cache, int SCindex, offset_t block)
{
    if(block == 0){
        printf("In CachePush, block == 0!\n");
        exit(0);
    }
    *(offset_t *)offset2ptr(block) = cache->sc[SCindex];
    cache->sc[SCindex] = block;
    cache->blockCount[SCindex]++;
}

offset_t CachePop(struct ThreadCache *cache, int SCindex)
{
    if (cache->blockCount[SCindex] == 0)
        return 0;
    if(cache->sc[SCindex]==0){
        printf("Thread %lu blockCount[%d]=%d but sc[%d]=0\n",
            pthread_self(), SCindex, cache->blockCount[SCindex], SCindex);
        exit(0);
    }
    offset_t firstBlock = cache->sc[SCindex];
    cache->sc[SCindex] = *(offset_t *)offset2ptr(firstBlock);
    cache->blockCount[SCindex]--;
    return firstBlock;
}

int CacheFillFromPartial(struct ThreadCache *cache, int SCindex)
{
    struct SuperBlockDescriptor *descriptor = GetAPartialSB(SCindex);
    if (!descriptor)
        return 0;
    int cnt = descriptor->availableCount;
    offset_t FirstBlock, NextBlock;
    FirstBlock = descriptor->firstBlock;
    for (int i = 0; i < cnt; i++)
    {
        NextBlock = *(offset_t *)offset2ptr(FirstBlock);
        CachePush(cache, SCindex, FirstBlock);
        FirstBlock = NextBlock;
    }
    descriptor->availableCount = 0;
    descriptor->firstBlock = 0;
    return cnt;
}

int CacheFillFromNewSB(struct ThreadCache *cache, int SCindex)
{
    struct SuperBlockDescriptor *desc = GetANewSB();
    if (!desc)
        return 0;

    int i = (ptr2offset((void *)desc) - GD->SuperBlockDescriptorOffset) 
            / sizeof(struct SuperBlockDescriptor);
    desc->superBlcok =  GD->UserSpaceOffset + i * SUPER_BLOCK_SIZE;
    desc->blockSize = sizeclass[SCindex];
    desc->maxCount = SUPER_BLOCK_SIZE / sizeclass[SCindex];
    desc->SizeClassIndex = SCindex;
    desc->firstBlock = 0;
    desc->availableCount = 0;
    printf("Thread %lu occupied SB %d\n",pthread_self(),i);
    for (int i = desc->maxCount - 1; i >= 0; i--){
        offset_t blk = desc->superBlcok + i * desc->blockSize;
        CachePush(cache, SCindex, blk);
    }

    return desc->maxCount;
}

void CacheFlush(struct ThreadCache *cache, int SCindex)
{
    int cnt = cache->blockCount[SCindex] / 2;
    for (int i = 0; i < cnt; i++)
    {
        offset_t block = CachePop(cache, SCindex);
        blockFree(block);
    }
}

void CacheDisplay(struct ThreadCache *cache)
{
    for(int i=0; i<SIZE_CLASS_NUMBER; i++){
        printf("%2d : ",i);
        offset_t blk = cache->sc[i];
        for(int j=0; j<cache->blockCount[i]; j++){
            printf("%llx  ",blk);
            blk = *(offset_t *)offset2ptr(blk);
        }
        putchar('\n');
    }
    putchar('\n');
}