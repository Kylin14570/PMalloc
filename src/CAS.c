#include <stdbool.h>

bool CAS32(volatile unsigned int *addr, unsigned int old, unsigned int new)
{
    return    __sync_bool_compare_and_swap(addr, old, new);
}

bool CAS64(volatile unsigned long long *addr, unsigned long long old, unsigned long long new)
{
    return    __sync_bool_compare_and_swap(addr, old, new);
}