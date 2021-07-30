#include "def.h"

extern char *BaseAddress;
extern struct GlobalDescriptor *GD;
extern void *offset2ptr(offset_t offset);
extern offset_t ptr2offset(void *ptr);
extern int CAS32();
extern int CAS64();

extern pthread_mutex_t mutex;

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


/*
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
*/
    //printf("Thread %lu get into GetAPartialSB()\n",pthread_self());
    
    pthread_mutex_lock(&mutex);
    
    First = GD->firstPartialSB[SCindex];
    if (First == -1){
        pthread_mutex_unlock(&mutex);
       return NULL;
    }

    // Find next partial SB
    for (Next = First + 1; Next < GD->SBnumber; Next++){
        if (desc[Next].SizeClassIndex == SCindex && desc[Next].superBlcok && desc[Next].availableCount < desc[Next].maxCount)
            break;
    }
    if (Next == GD->SBnumber)
        Next = -1;
    
    GD->firstPartialSB[SCindex] = Next;
    
    pthread_mutex_unlock(&mutex);
    
    return &desc[First];
}

struct SuperBlockDescriptor *GetANewSB(int SCindex)
{
    int First, Next;
    struct SuperBlockDescriptor *desc =
        (struct SuperBlockDescriptor *)offset2ptr(GD->SuperBlockDescriptorOffset);


/*
    //-----------------------------------------------
    // Don't consider concurrency control 
    //-----------------------------------------------
    // Find next free SB 
    for (Next = First + 1; Next < GD->SBnumber; Next++)
        if (desc[Next].superBlcok == 0)
            break;
    if (Next == GD->SBnumber)
        Next = -1;
    GD->firstFreeSB = Next;
*/


/*
    //-----------------------------------------------
    // Concurrency Control with CAS 
    //-----------------------------------------------
    do
    {
        // Get the first free SB
        First = GD->firstFreeSB;
        if (First == -1)
            return NULL;
        // Find next free SB
        for (Next = First + 1; Next < GD->SBnumber; Next++)
            if (desc[Next].superBlcok == 0) // It's free
                break;
        if (Next == GD->SBnumber) // There is no free SB anymore
            Next = -1;

    } while ( ! CAS32( &(GD->firstFreeSB), First, Next) );
*/



    //-----------------------------------------------
    // Concurrency Control with mutex 
    //-----------------------------------------------
    pthread_mutex_lock(&mutex);
    
    First = GD->firstFreeSB;  // Get first free SB
    if (First == -1)  // There is no free SB
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    desc[First].superBlcok = GD->UserSpaceOffset + First * SUPER_BLOCK_SIZE;

    // Find next free SB 
    for (Next = First + 1; Next < GD->SBnumber; Next++)
        if (desc[Next].superBlcok == 0) // Find a free SB
            break;
    if (Next == GD->SBnumber)  // There is no free SB
        Next = -1;

    GD->firstFreeSB = Next;  // Update the first free SB to Next

    printf("Thread %lu occupied SB %d\n",pthread_self(),First);
    
    pthread_mutex_unlock(&mutex);

    return &desc[First];

}

void SBfree(struct SuperBlockDescriptor *desc)
{
    int old, new;
    int i = (ptr2offset((void *)desc) - GD->SuperBlockDescriptorOffset) // index of the SB
            / sizeof(struct SuperBlockDescriptor);

    int SCindex = desc->SizeClassIndex;

/*
    pmem_memset_persist(desc, 0, sizeof(struct SuperBlockDescriptor));
    
    do{
        new = old = GD->firstFreeSB;
        if( i < new )
            new = i;
        else
            break ;
    }while ( ! CAS32( &(GD->firstFreeSB), old, new) );

    printf("Thread %lu freed SB %d\n",pthread_self(), i);
*/

    pthread_mutex_lock(&mutex);

    desc->superBlcok = 0;
    
    if(i < GD->firstFreeSB || GD->firstFreeSB == -1)
        GD->firstFreeSB = i;
    
    if(GD->firstPartialSB[SCindex] == i){
        int j;
        struct SuperBlockDescriptor *d = (struct SuperBlockDescriptor *)offset2ptr(GD->SuperBlockDescriptorOffset);
        for(j = i+1 ; j<GD->SBnumber; j++)
            if(d[j].SizeClassIndex == SCindex && d[j].superBlcok && d[j].availableCount < d[j].maxCount)
                break;
        if(j == GD->SBnumber)
            j = -1;
        GD->firstPartialSB[SCindex] = j;
    }

    printf("Thread %lu freed SB %d\n",pthread_self(), i);
    
    pthread_mutex_unlock(&mutex);

}

void SBpartial(struct SuperBlockDescriptor *desc)
{
    int i = (ptr2offset((void *)desc) - GD->SuperBlockDescriptorOffset) / sizeof(struct SuperBlockDescriptor);
/*
    int old, new;
    do{
        new = old = GD->firstPartialSB[desc->SizeClassIndex];
        if( i < new )
            new = i;
        else    
            break;
    }while ( ! CAS32( &(GD->firstPartialSB[desc->SizeClassIndex]), old, new) );
*/
    pthread_mutex_lock(&mutex);
    if(i < GD->firstPartialSB[desc->SizeClassIndex] || GD->firstPartialSB[desc->SizeClassIndex] == -1)
        GD->firstPartialSB[desc->SizeClassIndex] = i;
    pthread_mutex_unlock(&mutex);
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



