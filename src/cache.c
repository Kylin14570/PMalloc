#include "def.h"

extern char *BaseAddress;
extern struct GlobalDescriptor *GD;
extern int sizeclass[];

extern void *offset2ptr(offset_t offset);
extern offset_t ptr2offset(void *ptr);
extern void SBfree(struct SuperBlockDescriptor *desc);
extern void SBpartial(struct SuperBlockDescriptor *desc);
extern struct SuperBlockDescriptor *GetANewSB();
extern struct SuperBlockDescriptor *GetAPartialSB(int SCindex);
extern struct SuperBlockDescriptor *GetSBdescriptor(offset_t offset);

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
    *(offset_t *)offset2ptr(block) = cache->sc[SCindex];
    cache->sc[SCindex] = block;
    cache->blockCount[SCindex]++;
}

offset_t CachePop(struct ThreadCache *cache, int SCindex)
{
   if (cache->blockCount[SCindex] == 0)
        return 0;
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

    int i = (ptr2offset((void *)desc) - GD->SuperBlockDescriptorOffset) / sizeof(struct SuperBlockDescriptor);
    desc->superBlcok =  GD->UserSpaceOffset + i * SUPER_BLOCK_SIZE;
    desc->blockSize = sizeclass[SCindex];
    desc->maxCount = SUPER_BLOCK_SIZE / sizeclass[SCindex];
    desc->SizeClassIndex = SCindex;
    desc->firstBlock = 0;
    desc->availableCount = 0;

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
        struct SuperBlockDescriptor *desc = GetSBdescriptor(block);
        *(offset_t *)offset2ptr(block) = desc->firstBlock;
        desc->firstBlock = block;
        desc->availableCount++;
        if (desc->availableCount == desc->maxCount)
            SBfree(desc);
        else
            SBpartial(desc);
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