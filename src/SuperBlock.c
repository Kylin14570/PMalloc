#include "def.h"

extern char *BaseAddress;
extern struct GlobalDescriptor *GD;
extern void *offset2ptr(offset_t offset);
extern offset_t ptr2offset(void *ptr);

struct SuperBlockDescriptor *GetAPartialSB(int SCindex)
{
    int First, Next;
    struct SuperBlockDescriptor *desc =
        (struct SuperBlockDescriptor *)offset2ptr(GD->SuperBlockDescriptorOffset);

    /* Don't consider concurrency control at present */
    First = GD->firstPartialSB[SCindex];
    if (First == -1)
           return NULL;
    /* Find next partial SB */
    for (Next = First + 1; Next < GD->SBnumber; Next++)
        if (desc[Next].SizeClassIndex == SCindex && desc[Next].superBlcok && desc[Next].availableCount < desc[Next].maxCount)
            break;
    if (Next == GD->SBnumber)
        Next = -1;
    GD->firstPartialSB[SCindex] = Next;

    printf("Allocate a partial SB %d\n\n",First);

    return desc + First;

/*
    do
    {
        First = GD->firstPartialSB[SCindex];
        if (First == -1)
            return NULL;
        // Find next partial SB
        for (Next = First + 1; Next < GD->SBnumber; Next++)
            if (desc[Next].SizeClassIndex == SCindex && desc[Next].availableCount < desc[Next].maxCount)
                break;
        if (Next == GD->SBnumber)
            Next = -1;
    } while (!CAS(&(GD->firstPartialSB[SCindex]), First, Next));
*/

}

struct SuperBlockDescriptor *GetANewSB()
{
    int First, Next;
    struct SuperBlockDescriptor *desc =
        (struct SuperBlockDescriptor *)offset2ptr(GD->SuperBlockDescriptorOffset);

    /* Don't consider concurrency control at present */
    First = GD->firstFreeSB;
    if (First == -1)
        return NULL;
    /* Find next free SB */
    for (Next = First + 1; Next < GD->SBnumber; Next++)
        if (desc[Next].superBlcok == 0)
            break;
    if (Next == GD->SBnumber)
        Next = -1;
    GD->firstFreeSB = Next;

    printf("Allocate a new SB %d\n\n",First);

    return desc + First;
/*
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
    } while (!CAS(&(GD->firstFreeSB), First, Next));
*/
    
}

void SBfree(struct SuperBlockDescriptor *desc)
{
    int i = (ptr2offset((void *)desc) - GD->SuperBlockDescriptorOffset) / sizeof(struct SuperBlockDescriptor);

    printf("SB %d is freed\n\n",i);

    pmem_memset_persist(desc, 0, sizeof(struct SuperBlockDescriptor));
    if (i < GD->firstFreeSB)
        GD->firstFreeSB = i;
}

void SBpartial(struct SuperBlockDescriptor *desc)
{
    int i = (ptr2offset((void *)desc) - GD->SuperBlockDescriptorOffset) / sizeof(struct SuperBlockDescriptor);
    if (i < GD->firstPartialSB[desc->SizeClassIndex])
        GD->firstPartialSB[desc->SizeClassIndex] = i;
}

struct SuperBlockDescriptor *GetSBdescriptor(offset_t offset)
{
    if (offset < GD->UserSpaceOffset ||
        offset >= GD->UserSpaceOffset + GD->SBnumber * SUPER_BLOCK_SIZE)
        return NULL;
    
    struct SuperBlockDescriptor * desc = (struct SuperBlockDescriptor *)offset2ptr(GD->SuperBlockDescriptorOffset);
    
    int i = (offset - GD->UserSpaceOffset) / SUPER_BLOCK_SIZE;

    return  &desc[i];
}
