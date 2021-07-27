#include "def.h"

#define N 10

extern char *PMalloc_map_file(const char *filePath, int len);
extern void ThreadInit();
extern void ThreadDestroy();
extern offset_t PMalloc(int size);
extern int PMfree(offset_t offset);

const char path[] = "./pool";
const int size = 100*1024*1024;

offset_t p[N];

int main()
{
    srand(1214);

    char *base = PMalloc_map_file(path, size);
    
    ThreadInit();
    
    for(int i=0; i<N; i++){
        p[i] = PMalloc(rand()%8000);
        printf("%2d -- ",i);
        if(p[i])
            printf("Allocate a block at %llx\n\n",p[i]);
        else{
            printf("Failed to allocate.\n\n");
            return 0;
        }
    }
    
    for(int i=0; i<N; i++)
        if(p[i]){
            if(PMfree(p[i])==0)
                printf("%2d -- Free %llx\n\n",i,p[i]);
            else{
                printf("Failed to free %llx\n\n",p[i]);
                return 0;
            }
        }

    ThreadDestroy();

    printf("Finished successfully!\n\n");

    return 0;
}