#include "def.h"

extern struct GlobalDescriptor *GD;
extern struct SuperBlockDescriptor *GetANewSB();
extern void *offset2ptr(offset_t offset);
extern offset_t ptr2offset(void *ptr);

struct ROOT * allocROOT(void)
{
    /* Need concurrency control */
    if(GD->FreeRoot == 0){
        struct SuperBlockDescriptor *SB = GetANewSB();
        int i = (ptr2offset((void *)SB) - GD->SuperBlockDescriptorOffset) / sizeof(struct SuperBlockDescriptor);
        SB->superBlcok =  GD->UserSpaceOffset + i * SUPER_BLOCK_SIZE;
        SB->SizeClassIndex = -1;
        int n = SUPER_BLOCK_SIZE / sizeof(struct ROOT);
        for(int i=0; i < n ; i++){
            struct ROOT *newROOT = (struct ROOT *)offset2ptr(SB->superBlcok + i * sizeof(struct ROOT));
            newROOT->nextRoot = GD->FreeRoot;
            GD->FreeRoot = ptr2offset((void *)newROOT);
        }
    }
    struct ROOT *firstROOT = (struct ROOT *)offset2ptr(GD->FreeRoot);
    GD->FreeRoot = firstROOT->nextRoot;
    firstROOT->nextRoot = GD->Root;
    GD->Root = ptr2offset((void *)firstROOT);
    return firstROOT;
}

void freeROOT(struct ROOT * root)
{
    /* Need concurrency control */
    root->objectAddress = 0;
    if(ptr2offset((void *)root) == GD->Root){
        GD->Root = root->nextRoot;
        root->nextRoot = GD->FreeRoot;
        GD->FreeRoot = ptr2offset((void *)root);
        return ;
    }
    struct ROOT *priorROOT = (struct ROOT *)offset2ptr(GD->Root);
    while( priorROOT->nextRoot !=  ptr2offset((void *)root))
        priorROOT = (struct ROOT *)offset2ptr(priorROOT->nextRoot);
    priorROOT->nextRoot = root->nextRoot;
    root->nextRoot = GD->FreeRoot;
    GD->FreeRoot = ptr2offset((void *)root);
}