/************************************
 * 
 *  Concurrency Control Needed !
 * 
 * **********************************/

#include "def.h"

extern char *BaseAddress;
extern struct GlobalDescriptor *GD;
extern void *offset2ptr(offset_t offset);
extern offset_t ptr2offset(void *ptr);
extern int CAS32();
extern int CAS64();


struct SuperBlockDescriptor *GetSBdescriptor(offset_t offset)
{
    if (offset < GD->UserSpaceOffset ||
        offset >= GD->UserSpaceOffset + GD->SBnumber * SUPER_BLOCK_SIZE)
        return NULL;
    
    struct SuperBlockDescriptor * desc = (struct SuperBlockDescriptor *)offset2ptr(GD->SuperBlockDescriptorOffset);
    
    int i = (offset - GD->UserSpaceOffset) / SUPER_BLOCK_SIZE;

    return  &desc[i];
}

struct SuperBlockDescriptor *GetAPartialSB(int SCindex)
{
    int First, Next;
    struct SuperBlockDescriptor *desc =
        (struct SuperBlockDescriptor *)offset2ptr(GD->SuperBlockDescriptorOffset);

/*
    // Don't consider concurrency control at present
    First = GD->firstPartialSB[SCindex];
    if (First == -1)
           return NULL;
    // Find next partial SB
    for (Next = First + 1; Next < GD->SBnumber; Next++)
        if (desc[Next].SizeClassIndex == SCindex && desc[Next].superBlcok && desc[Next].availableCount < desc[Next].maxCount)
            break;
    if (Next == GD->SBnumber)
        Next = -1;
    GD->firstPartialSB[SCindex] = Next;

    printf("Allocate a partial SB %d\n\n",First);

    return desc + First;
*/



    do
    {
        First = GD->firstPartialSB[SCindex];
        if (First == -1)
            return NULL;
        // Find next partial SB
        for (Next = First + 1; Next < GD->SBnumber; Next++)
            if (desc[Next].SizeClassIndex == SCindex && desc[Next].superBlcok && desc[Next].availableCount < desc[Next].maxCount)
                break;
        if (Next == GD->SBnumber)
            Next = -1;
    } while ( ! CAS32( &(GD->firstPartialSB[SCindex]), First, Next) );

    return desc + First;
}

struct SuperBlockDescriptor *GetANewSB()
{
    int First, Next;
    struct SuperBlockDescriptor *desc =
        (struct SuperBlockDescriptor *)offset2ptr(GD->SuperBlockDescriptorOffset);

/*
    // Don't consider concurrency control at present 
    First = GD->firstFreeSB;
    if (First == -1)
        return NULL;
    // Find next free SB 
    for (Next = First + 1; Next < GD->SBnumber; Next++)
        if (desc[Next].superBlcok == 0)
            break;
    if (Next == GD->SBnumber)
        Next = -1;
    GD->firstFreeSB = Next;

    printf("Allocate a new SB %d\n\n",First);

    return desc + First;
*/


    do
    {
        First = GD->firstFreeSB;
        if (First == -1)
            return NULL;
        // Find next free SB
        for (Next = First + 1; Next < GD->SBnumber; Next++)
            if (desc[Next].superBlcok == 0)
                break;
        if (Next == GD->SBnumber)
            Next = -1;
    } while ( ! CAS32(&(GD->firstFreeSB), First, Next) );

    printf("Allocate a new SB %d\n\n",First);
    
    return desc + First;
}

void SBfree(struct SuperBlockDescriptor *desc)
{
    int i = (ptr2offset((void *)desc) - GD->SuperBlockDescriptorOffset) / sizeof(struct SuperBlockDescriptor);

    printf("SB %d is freed\n\n",i);

    pmem_memset_persist(desc, 0, sizeof(struct SuperBlockDescriptor));

    int old, new;
    do{
        new = old = GD->firstFreeSB;
        if( i < new )
            new = i;
    }while ( ! CAS32( &(GD->firstFreeSB), old, new) );
}

void SBpartial(struct SuperBlockDescriptor *desc)
{
    int i = (ptr2offset((void *)desc) - GD->SuperBlockDescriptorOffset) / sizeof(struct SuperBlockDescriptor);
    
    int old, new;
    do{
        new = old = GD->firstPartialSB[desc->SizeClassIndex];
        if( i < new )
            new = i;
    }while ( ! CAS32( &(GD->firstPartialSB[desc->SizeClassIndex]), old, new) );
}


void blockFree(offset_t block)
{
    struct SuperBlockDescriptor *desc = GetSBdescriptor(block);

    offset_t FirstBlock;
    int AvailabelCount;

    do{
        FirstBlock = desc->firstBlock;
        *(offset_t *)offset2ptr(block) = FirstBlock;

    }while ( ! CAS64( &(desc->firstBlock), FirstBlock, block) );

    do{
        AvailabelCount = desc->availableCount;

    }while( !CAS32( &(desc->availableCount), AvailabelCount, AvailabelCount+1) );

    if (desc->availableCount == desc->maxCount)
        SBfree(desc);
    else
        SBpartial(desc);
}



