#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef struct  _lru_node
{
  struct _lru_node *prev;           //link lru list
  struct _lru_node *next;
  int k;

}lru_node;
int main(){
	lru_node *a=malloc(sizeof(lru_node)),*b=malloc(sizeof(lru_node));
	b->prev=malloc(sizeof(lru_node));
	b->next=malloc(sizeof(lru_node));
	a->prev=malloc(sizeof(lru_node));
	a->next=malloc(sizeof(lru_node));
	a->prev->k=5;
	a->next->k=8;
	a->k=13;
	b=a;
	
	while(a!=NULL){
		a->k--;
		a=a->next;
	}
	a=b;
	
	printf("1:%d\n",a->k);
	printf("2:%d\n",a->next->k);
	printf("3:%d\n",a->prev->k);
	/*
	char *tmp[100];
	//strcpy(tmp,"ssss");
	FILE *a=fopen("a.txt","w");
	//for(i=0;i<120;i++){
		//fprintf(a,"%d\n",i);			
	//}
	fprintf(a,"%d %s %d\n",15,"sss",32);
	fclose(a);
	//a=fopen("a.txt","w");
	
	
	
	float benefit;
	benefit=(float)5/3040;
	benefit/=4500;
	printf("%.19f\n",benefit);*/
	return 0;
}
