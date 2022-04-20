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
	int i,j;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			if(j==q){
				i=5;
				q=11;
				break;
			}
		}
		printf("%d\n",i);
	}

	return 0;
}
