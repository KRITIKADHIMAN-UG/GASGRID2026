#include <stdio.h>
#define SIZE 5
int main(void) {
    int buffer[SIZE],in=0,out=0,count=0,choice,value;
    do {
        printf("\\n1 Produce  2 Consume  3 Display  0 Exit\\nChoice: ");
        if(scanf("%d",&choice)!=1)return 1;
        if(choice==1){if(count==SIZE)puts("Buffer full: producer must wait.");else{printf("Value: ");if(scanf("%d",&value)!=1)return 1;buffer[in]=value;in=(in+1)%SIZE;count++;puts("Produced.");}}
        else if(choice==2){if(count==0)puts("Buffer empty: consumer must wait.");else{printf("Consumed: %d\\n",buffer[out]);out=(out+1)%SIZE;count--;}}
        else if(choice==3){int i;printf("Buffer count: %d\\n",count);for(i=0;i<count;i++)printf("%d ",buffer[(out+i)%SIZE]);puts("");}
    } while(choice!=0);return 0;
}
