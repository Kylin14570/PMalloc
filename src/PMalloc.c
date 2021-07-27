#include "def.h"

extern int sizeclass[];
extern struct GlobalDescriptor *GD;
extern struct ThreadCache *GetThreadCache();
extern int GetSizeClassIndex(int size);
extern offset_t allocLargeSize(int size);
extern void CacheFlush(struct ThreadCache *cache, int SCindex);
extern void CachePush(struct ThreadCache *cache, int SCindex, offset_t block);
extern offset_t CachePop(struct ThreadCache *cache, int SCindex);
extern int CacheFillFromPartial(struct ThreadCache *cache, int SCindex);
extern int CacheFillFromNewSB(struct ThreadCache *cache, int SCindex);
extern int CacheEmpty(struct ThreadCache *cache, int SCindex);
extern int CacheFull(struct ThreadCache *cache, int SCindex);
extern void CacheDisplay(struct ThreadCache *cache);
extern struct ROOT *allocROOT(void);
extern void freeROOT(struct ROOT * root);
extern void *offset2ptr(offset_t offset);
extern offset_t ptr2offset(void *ptr);
extern struct SuperBlockDescriptor *GetSBdescriptor(offset_t offset);

offset_t PMalloc(int size)
{
    if (size <= 0) // Stupid calling.
        return 0;
    if (size > LIMIT_SIZE) // Large size directly allocated from the memory space.
        return allocLargeSize(size);

    size += 8; // 8 more bytes for ROOT item address.

    int SizeClassIndex = GetSizeClassIndex(size);
    
    struct ThreadCache *cache = GetThreadCache();

    if (CacheEmpty(cache, SizeClassIndex))
    {   // Fill the cache from a SuperBlcok
        if (!CacheFillFromPartial(cache, SizeClassIndex))
            if (!CacheFillFromNewSB(cache, SizeClassIndex))
                printf("Exhausted memory!\n");
    }

    //CacheDisplay(cache);

    // pop the first block from the cache
    offset_t firstBlock = CachePop(cache, SizeClassIndex);

    // add a root item to record this object
    struct ROOT *root = allocROOT();
    *(offset_t *)offset2ptr(firstBlock) = ptr2offset((void *)root);
    root->objectAddress = firstBlock;

    return firstBlock + 8;
}

int PMfree(offset_t offset)
{
    if(offset - 8 < GD->UserSpaceOffset
    || offset >= GD->MemorySize)
        return -1;  // Illegal address
    
    offset -= 8;    // 8 more bytes for ROOT item address.

    struct ROOT *root = (struct ROOT *)offset2ptr(*(offset_t *)offset2ptr(offset));
    if(root->objectAddress != offset)
        return -1;
    
    freeROOT(root);

    struct ThreadCache *cache = GetThreadCache();
    struct SuperBlockDescriptor *SB = GetSBdescriptor(offset);
    if( CacheFull(cache, SB->SizeClassIndex) )
        CacheFlush(cache, SB->SizeClassIndex);
    CachePush(cache, SB->SizeClassIndex, offset);
    return 0;
}

offset_t allocLargeSize(int size)
{
    return 0;
}