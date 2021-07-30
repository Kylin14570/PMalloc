#include "def.h"
#include <pthread.h>

pthread_mutex_t mutex;

int T = 1;
int M = 1;
int N = 1;

extern char *PMalloc_map_file(const char *filePath, int len);
extern void ThreadInit();
extern void ThreadDestroy();
extern offset_t PMalloc(int size);
extern void PMfree(offset_t offset);

const char path[] = "./pool";
const int size = 1024*1024*1024;

void *test(void *arg)
{
    offset_t p[N];

    ThreadInit();

    for(int j=0; j<M; j++)
    {
        for(int i=0; i<N; i++){
            int size = rand()%1000 + 1;
            p[i] = PMalloc(size);
            if(!p[i]){
                printf("ERROR : Thread %lu failed to allocate %d bytes !\n\n",pthread_self(), size);
                exit(0);
            }
        }
    
        for(int i=0; i<N; i++){
            if(p[i])
                PMfree(p[i]);
        }
    }  

    ThreadDestroy();
}

int main(int argc, char *argv[])
{
    if(argc != 4){
        printf("Usage : main T M N\n");
        printf("T -- number of threads\n");
        printf("M -- number of loops\n");
        printf("N -- number of PMalloc operations\n");
        return 0;
    }

    time_t begin = clock();

    T = atoi(argv[1]);
    M = atoi(argv[2]);
    N = atoi(argv[3]);
    
    if(T > MAX_THREAD_NUMBER){
        printf("No more than %d threads !\n",MAX_THREAD_NUMBER);
        return 0;
    }

    srand(time(NULL));

    pthread_mutex_init(&mutex, NULL);

    char *base = PMalloc_map_file(path, size);
    
    pthread_t tid[MAX_THREAD_NUMBER];

    for(int i = 0; i < T; i++)
        pthread_create( &tid[i], NULL, test, NULL);
    
    for(int i = 0; i < T; i++)
        pthread_join( tid[i], NULL);

    pmem_unmap(base, size);
    
    pthread_mutex_destroy(&mutex);
    
    printf("\e[1;32m""\nFinished successfully!\n");
    printf("time = %ldms\n\n",(clock()-begin)*1000/CLOCKS_PER_SEC);
    printf("\e[0m");

    return 0;
}