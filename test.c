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
	profit *wb=malloc(sizeof(profit)),*order,*prev;
	wb->next=malloc(sizeof(profit));
	order=wb;
	order->next=NULL;
	return 0;
}
