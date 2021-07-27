#include "def.h"

extern char *BaseAddress;
extern struct GlobalDescriptor *GD;

void *offset2ptr(offset_t offset)
{
    if(offset == 0)
        return NULL;
    return (void *)BaseAddress + offset;
}

offset_t ptr2offset(void *ptr)
{
    if(ptr == NULL)
        return 0;
    return ptr - (void *)BaseAddress;
}