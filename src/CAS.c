int CAS32(volatile unsigned int *addr, unsigned int old,
                  unsigned int new)
{
    int ret = 0;
    
    __asm__ volatile (" lock; cmpxchg %2, %3"
                          : "=a" (ret), "=m"(*addr)
                          : "r" (new), "m" (*addr), "0" (old)
                          : "cc"
                      );
    return ret==old;
}

int CAS64(volatile unsigned long long *addr, unsigned long long old,
                  unsigned long long new)
{
    int ret = 0;
    
    __asm__ volatile (" lock; cmpxchg %2, %3"
                          : "=a" (ret), "=m"(*addr)
                          : "r" (new), "m" (*addr), "0" (old)
                          : "cc"
                      );
    return ret==old;
}