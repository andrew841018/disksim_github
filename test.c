#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<assert.h>
//#include "../build-model-after-simulation/a.h"
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
double s;
int main(){	
	int k=123,g=2345;
	s=(double)k/(g+999);
	printf("%f\n",s);
	return 0;
}
