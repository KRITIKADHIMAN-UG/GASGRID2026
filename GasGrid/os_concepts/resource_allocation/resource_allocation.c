#include <stdio.h>
int main(void) {
    int total,allocated,request;
    printf("Total resource units: ");if(scanf("%d",&total)!=1||total<0)return 1;
    printf("Currently allocated units: ");if(scanf("%d",&allocated)!=1||allocated<0||allocated>total)return 1;
    printf("Requested additional units: ");if(scanf("%d",&request)!=1||request<0)return 1;
    printf("Available units: %d\\n",total-allocated);
    if(request<=total-allocated)printf("Request granted. Remaining after allocation: %d\\n",total-allocated-request);
    else puts("Request denied: insufficient available resources.");return 0;
}
