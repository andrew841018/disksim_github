#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<assert.h>
typedef struct _profit{
	int channel_num;
	int plane;
	double benefit;
	struct _profit *next;
}profit;
typedef struct _lru_node{
}lru_node;
void a(int *b,int *c){
	int g=11,k=12;
	*b=g;
	*c=k;
}
int main(){	
	int p=2,q=3;
	a(&p,&q);
	int i=5,k;
	double h=5.5;
	if(h*i>27)
		printf("aa\n");
	else
		printf("bb\n");

	return 0;
}
