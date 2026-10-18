#include <stdio.h>
int main(void) {
    int n,m,alloc[10][10],need[10][10],avail[10],finish[10]={0},safe[10],count=0,i,j,progress;
    printf("Processes and resource types (max 10 each): ");
    if(scanf("%d%d",&n,&m)!=2||n<1||n>10||m<1||m>10)return 1;
    printf("Enter allocation matrix:\\n");
    for(i=0;i<n;i++)for(j=0;j<m;j++)if(scanf("%d",&alloc[i][j])!=1||alloc[i][j]<0)return 1;
    printf("Enter remaining-need matrix:\\n");
    for(i=0;i<n;i++)for(j=0;j<m;j++)if(scanf("%d",&need[i][j])!=1||need[i][j]<0)return 1;
    printf("Enter available resources:\\n");
    for(j=0;j<m;j++)if(scanf("%d",&avail[j])!=1||avail[j]<0)return 1;
    do { progress=0; for(i=0;i<n;i++) if(!finish[i]) { int can=1; for(j=0;j<m;j++)if(need[i][j]>avail[j])can=0; if(can){for(j=0;j<m;j++)avail[j]+=alloc[i][j];finish[i]=1;safe[count++]=i;progress=1;} } } while(progress);
    if(count==n){printf("Safe sequence: ");for(i=0;i<n;i++)printf("P%d%s",safe[i],i==n-1?"\\n":" -> ");}else printf("Unsafe state: deadlock may occur.\\n");
    return 0;
}
