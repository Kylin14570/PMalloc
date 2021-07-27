#include "def.h"
#include <pthread.h>

#define N 10

extern char *PMalloc_map_file(const char *filePath, int len);
extern void ThreadInit();
extern void ThreadDestroy();
extern offset_t PMalloc(int size);
extern int PMfree(offset_t offset);

const char path[] = "./pool";
const int size = 100*1024*1024;




void *test(void *arg)
{
    offset_t p[N];

    ThreadInit();
    
    for(int i=0; i<N; i++){
        p[i] = PMalloc(rand()%8000);
        if(p[i])
            printf("Thread %lu got a block at %llx\n\n",pthread_self(), p[i]);
        else{
            printf("Thread %lu failed to allocate a block.\n\n",pthread_self());
            exit(0);
        }
    }
    
    for(int i=0; i<N; i++)
        if(p[i]){
            if(PMfree(p[i])==0)
                printf("Thread %lu freed %llx\n\n",pthread_self(),p[i]);
            else{
                printf("Thread %lu failed to free %llx\n\n",pthread_self(),p[i]);
                exit(0);
            }
        }

    ThreadDestroy();
}

int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("Usage : main threadnumber\n\n");
        return 0;
    }

    int threadnumber = atoi(argv[1]);
    if(threadnumber > MAX_THREAD_NUMBER){
        printf("No more than %d threads !\n",MAX_THREAD_NUMBER);
        return 0;
    }

    srand(1214);

    char *base = PMalloc_map_file(path, size);
    
    pthread_t tid[MAX_THREAD_NUMBER];

    for(int i = 0; i < threadnumber; i++){
        pthread_create( &tid[i], NULL, test, NULL);
        printf("Thread %lu was born!\n\n",tid[i]);
    }
    
    for(int i = 0; i < threadnumber; i++)
        pthread_join( tid[i], NULL);

    printf("Finished successfully!\n\n");

    return 0;
}