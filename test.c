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
	int count=888;
	printf("dddd\n");
	count=0;
	count++;
	a:
		printf("%d\n",count);
	/*profit *wb=malloc(sizeof(profit)),*order,*prev;
	buffer_cache *ptr_buffer_cache=malloc(sizeof(buffer_cache));
	ptr_buffer_cache->p=malloc(sizeof(profit));
	ptr_buffer_cache->p->benefit=0.4515;
	ptr_buffer_cache->p->channel_num=7;
	ptr_buffer_cache->p->plane=5;
	ptr_buffer_cache->p->next=malloc(sizeof(profit));
	ptr_buffer_cache->p->next->channel_num=4;
	ptr_buffer_cache->p->next->plane=6;
	ptr_buffer_cache->p->next->benefit=0.62154;
	insert_node(3,5,0.53,ptr_buffer_cache);*/
	/*
	order=malloc(sizeof(profit));
	order->plane=5;
	order->next=malloc(sizeof(profit));
	order->next->plane=6;
	wb->next=order;
	
	for(i=0;i<10;i++){
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
