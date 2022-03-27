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
	profit *wb=malloc(sizeof(profit)),*order=malloc(sizeof(profit)),*prev,*node;
	prev=wb;
	wb->plane=55;
	wb->next=malloc(sizeof(profit));
	wb->next->plane=36;
	wb->next->next=malloc(sizeof(profit));
	wb->next->next->plane=334;
	order=wb;	
	node=wb->next;
	order->next=order->next->next;
	node=NULL;
	return 0;
}
