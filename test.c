#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef struct  _profit
{
  struct _profit *next;           //link lru list
  int channel_num,plane;
  double benefit;

}profit;
typedef struct _buffer_cache
{
	struct profit *p;
}buffer_cache;
void insert_value(double benefit,buffer_cache *ptr_buffer_cache,int channel,int plane){
  if(ptr_buffer_cache->p==NULL){
    printf("error\n");
    sleep(1);
    return;
  }
  profit *prev,*curr,*data,*head;
  data=malloc(sizeof(profit));
  data->benefit=benefit;
  data->channel_num=channel;
  data->plane=plane;
  curr=ptr_buffer_cache->p;
  head=curr;
  while(curr!= NULL && benefit>curr->benefit){
    prev=curr;
    curr=curr->next;
  }
  //means benefit is min in link list--->not enter while
  data->next=curr;
  if(curr==head){
    ptr_buffer_cache->p=data;
  }
  else{
    prev->next=data; 
    ptr_buffer_cache->p=prev; 
  }
}
int main(){
	profit *order=malloc(sizeof(profit)),order1;
	buffer_cache *buffer=malloc(sizeof(buffer_cache));
	int i;
	double k=4.3;
	order1=order;
	for(i=0;i<10;i++){
		order->benefit=i;
		printf("benefit:%f\n",order->benefit);
		order->next=malloc(sizeof(profit));
		order=order->next;
	}
	buffer->p=order1;
	insert_value(k,buffer,k+1,k-1);
	while(buffer->p->next!=NULL){
		printf("benefit:%f\n",buffer->p->benefit);
		buffer->p=buffer->p->next;
	}
	return 0;
}
