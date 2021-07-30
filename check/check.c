#include <stdio.h>
#include <string.h>
#define N 1000000

unsigned long SB[N];

char s[16];
char t[16];

unsigned long tid;
int sb;
unsigned long long lineNO = 0;

#define NONE    "\e[0m"         // clear the color
#define GREEN   "\e[1;32m"      // set to green
#define RED     "\e[1;31m"      // set to red

int main()
{
    for(int i=0; i<N; i++)
        SB[i] = 0;
    while(1){
        
        lineNO++;
        
        scanf("%s",s);
        
        if(strcmp(s,"Thread")==0)
            ;
        else if(strcmp(s,"\e[1;32m")==0)
            break;
        else{
            printf(RED);
            printf("Syntax error in line %llu : %s\n\n",lineNO, s);
            printf(NONE);
            return 0;
        }

        scanf("%lu",&tid);
        scanf("%s",s);
        scanf("%s",t);
        scanf("%d",&sb);
        if(strcmp(s,"occupied")==0){
            if(SB[sb] == 0){
                SB[sb] = tid;
            }else{
                printf(RED);
                printf("ERROR : Thread %lu attempted to occupy SB %d, but it's occupied by thread %lu (line %llu)\n",
                                        tid, sb, SB[sb], lineNO);
                printf(NONE);
            }
        }
        else if(strcmp(s,"freed")==0){
            if(SB[sb] == tid){
                SB[sb] = 0;
            }else{
                printf(RED);
                printf("ERROR : Thread %lu attempted to free SB %d, but it's not occupied by it (line %llu)\n",
                                        tid, sb, lineNO);
                printf(NONE);
            }
        }
        else{
            printf(RED);
            printf("Syntax error in line %llu : %s\n\n",lineNO,s);
            printf(NONE);
            return 0;
        }
    }

    int flag = 0;
    for(int i=0; i<N; i++){
        if(SB[i]){
            printf(RED);
            printf("ERROR : SB %d is not freed, it's still occupied by thread %lu\n",i, SB[i]);
            printf(NONE);
            flag = 1;
        }
    }
    if(!flag){
        printf("Check finished %llu lines, no bug found.\n ",lineNO);
        printf(GREEN);
        printf("PASS!\n\n");
        printf(NONE);
    }
        
    return 0;
}