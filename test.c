#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef struct _profit{
	int channel_num;
	int plane;
	double benefit;
	struct _profit *next;
}profit;
int main(){
	profit *wb=malloc(sizeof(profit)),*order;
	//order=wb;
	int i;
	wb=NULL;
	/*for(i=0;i<10;i++){
		wb->plane=i;
		wb->next=malloc(sizeof(profit));
		wb=wb->next;
	}
	free(wb);
	while(order->next!=NULL){
		printf("%d\n",order->plane);
		order=order->next;
	}*/
	return 0;
}
