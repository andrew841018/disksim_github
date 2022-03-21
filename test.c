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
	profit *wb=malloc(sizeof(profit)),*order=malloc(sizeof(profit)),*prev;
	wb->plane=55;
	order=wb;
	wb->next=malloc(sizeof(profit));
	wb->next->plane=36;
	wb=wb->next;
	return 0;
}
