#include <stdio.h>
int main(void) {
    int shared=0,lock=0,choice;
    do {
        printf("\\nShared counter: %d | lock: %s\\n1 Enter critical section  2 Read counter  0 Exit\\nChoice: ",shared,lock?"locked":"unlocked");
        if(scanf("%d",&choice)!=1)return 1;
        if(choice==1){if(lock)puts("Critical section unavailable.");else{lock=1;shared++;puts("Lock acquired; counter incremented; lock released.");lock=0;}}
        else if(choice==2)printf("Counter = %d\\n",shared);
    }while(choice!=0);return 0;
}
