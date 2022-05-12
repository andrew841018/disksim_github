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
	int arr[2][2]={{1,4},{4,3}};
	int s=3,d=5;
	char a='0';
	int g=a-48;
	printf("%d\n",g);
	return 0;
}
