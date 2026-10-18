#include <stdio.h>
int main(void) {
    int n, burst[20], wait[20], turn[20], i;
    float avgw=0, avgt=0;
    printf("Number of processes (1-20): ");
    if (scanf("%d", &n)!=1 || n<1 || n>20) return 1;
    for(i=0;i<n;i++){ printf("Burst time for P%d: ",i+1); if(scanf("%d",&burst[i])!=1||burst[i]<1)return 1; }
    wait[0]=0;
    for(i=1;i<n;i++) wait[i]=wait[i-1]+burst[i-1];
    printf("PID\\tBurst\\tWaiting\\tTurnaround\\n");
    for(i=0;i<n;i++){turn[i]=wait[i]+burst[i];avgw+=wait[i];avgt+=turn[i];printf("P%d\\t%d\\t%d\\t%d\\n",i+1,burst[i],wait[i],turn[i]);}
    printf("Average waiting time: %.2f\\nAverage turnaround time: %.2f\\n",avgw/n,avgt/n);
    return 0;
}
