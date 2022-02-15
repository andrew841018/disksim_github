#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef struct  _lru_node
{
  struct _lru_node *prev;           //link lru list
  struct _lru_node *next;

}lru_node;
int main(){
	lru_node *a=malloc(sizeof(lru_node));
	a=a->prev;
	if(a==NULL)
		printf("hi\n");
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
