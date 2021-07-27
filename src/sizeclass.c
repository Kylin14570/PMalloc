#include "def.h"

int sizeclass[SIZE_CLASS_NUMBER] =
    {16, 20, 24, 28,
     32, 40, 48, 56,
     64, 80, 96, 112,
     128, 160, 192, 224,
     256, 320, 384, 448,
     512, 640, 768, 896,
     1024, 1280, 1536, 1792,
     2048, 2560, 3072, 3584,
     4096, 5120, 6144, 7168,
     8192};

/*
** Given a size, return an index i such that
** sizeclass[i-1] < size <= sizeclass[i]
**
** If the size is lager than the LIMIT_SIZE, return -1
*/
int GetSizeClassIndex(int size)
{
    if (size > LIMIT_SIZE) // Too large size
        return -1;         // It will be allocated from the file-mapped memory space directly

    if (size <= sizeclass[0])
        return 0;

    /* Binary search */
    int left = 0, right = SIZE_CLASS_NUMBER - 1;
    while (right - left > 1)
    {
        int mid = (left + right) >> 1;
        if (size > sizeclass[mid])
            left = mid;
        else
            right = mid;
    }
    return right;
}