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
	int a=1;
	if(a){
		return;
	}
	profit *wb=malloc(sizeof(profit)),*order=malloc(sizeof(profit)),*prev;
	wb->plane=55;
	wb->next=malloc(sizeof(profit));
	wb->next->plane=36;
	order->plane=123;
	order->next=wb;
	wb=order;
	return 0;
}
