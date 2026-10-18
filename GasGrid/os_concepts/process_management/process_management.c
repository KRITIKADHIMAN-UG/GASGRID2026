#include <stdio.h>
int main(void) {
    int n,i,pid[20],burst[20];
    printf("Number of processes (1-20): ");if(scanf("%d",&n)!=1||n<1||n>20)return 1;
    for(i=0;i<n;i++){pid[i]=i+1;printf("Enter burst time for process P%d: ",pid[i]);if(scanf("%d",&burst[i])!=1||burst[i]<1)return 1;}
    printf("Process table\\nPID\\tState\\tBurst\\n");
    for(i=0;i<n;i++)printf("P%d\\tReady\\t%d\\n",pid[i],burst[i]);
    puts("Simulation complete: processes are represented in the ready state.");return 0;
}
