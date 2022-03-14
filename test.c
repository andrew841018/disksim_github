#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef struct _profit{
	int channel_num;
	int plane;
	double benefit;
	struct _profit *next;
}profit;
typedef struct _buffer_cache
{
  double benefit;
  struct _profit *p,*q;
  unsigned int count; 
  unsigned int total_buffer_page_num;     //current buffer page number in the cache
  unsigned int total_buffer_block_num;
  unsigned int max_buffer_page_num;     //max buffer page count
  unsigned int w_hit_count;         //hit count for write (for statistics)
  unsigned int w_miss_count;          //miss count for write (for statistics)
  unsigned int r_hit_count;         //hit count for read (for statistics)
  unsigned int r_miss_count;          //miss count for read (for statistics)
   unsigned int current_mark_offset;     //the offset of current mark node ,using with ptr_current_mark_node together
}buffer_cache;
void insert_node(int channel,int plane,double benefit,buffer_cache *ptr_buffer_cache){
  profit *insert,*prev,*current,*start=malloc(sizeof(profit)); 
  int first=0;
  insert=ptr_buffer_cache->p;            
  current=malloc(sizeof(profit));
  prev=malloc(sizeof(profit));  
  if(ptr_buffer_cache->p==NULL){
	current->benefit=benefit;
	current->channel_num=channel;
	current->plane=plane;
	ptr_buffer_cache->p=current;
	return; 
  }
  insert=ptr_buffer_cache->p;            
  current=malloc(sizeof(profit));
  prev=malloc(sizeof(profit));
//insert current block to profit pointer--->according to the benefit value.

//only have one node
  if(insert!=NULL && insert->next==NULL){
    current->benefit=benefit;
    current->channel_num=channel;
    current->plane=plane;  			
    if(benefit>insert->benefit){
      insert->next=current;
      start=insert;
    }
    else{
      current->next=insert;
      start=current;
    }
    printf("HAC\n");
    first=1;
  }	
  //exist more than one node
  if(insert->next!=NULL){

    //the new block is first(benefit is min)
    if(benefit<=insert->benefit){
      printf("insert to the first node\n");
      current->benefit=benefit;
      current->channel_num=channel;
      current->plane=plane;
      current->next=insert;
      start=current;
      first=1;//tmp[i][j] is the first node in profit pointer
      printf(".........\n");
      }					

    //insert node is not the first one.
    int enter=0;
    while(insert!=NULL && first==0){ 					             				
      if(benefit>insert->benefit){
        prev=insert;
        insert=insert->next;
        enter=1;
      }
      else if(enter==1){
        printf("insert in the middle of profit pointer\n");
        current->benefit=benefit;
        current->channel_num=channel;
        current->plane=plane;
        prev->next=current;
        current->next=insert;
        break;
      }
      else if(enter==0){
		prev=insert;
		current->benefit=benefit;
        current->channel_num=channel;
        current->plane=plane;
        prev->next=current;
        current->next=insert->next;
        break;
	  }										
    }		  
  }    		
  if(first==1){
	ptr_buffer_cache->p=start;
	printf("yoyo~\n");
  }   
}
int main(){
	profit *wb=malloc(sizeof(profit)),*order,*prev;
	buffer_cache *ptr_buffer_cache=malloc(sizeof(buffer_cache));
	ptr_buffer_cache->p=malloc(sizeof(profit));
	ptr_buffer_cache->p->benefit=0.4515;
	ptr_buffer_cache->p->channel_num=7;
	ptr_buffer_cache->p->plane=5;
	ptr_buffer_cache->p->next=malloc(sizeof(profit));
	ptr_buffer_cache->p->next->channel_num=4;
	ptr_buffer_cache->p->next->plane=6;
	ptr_buffer_cache->p->next->benefit=0.62154;
	insert_node(3,5,0.53,ptr_buffer_cache);
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
