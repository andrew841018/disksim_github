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
	int i;
	int a[5]={0,2,4,3,3};
	int b[5];
	b=a;
	/*for(i=0;i<10;i++){
		FILE *f=fopen("a.txt","a+");
		fprintf(f,"%d\n",i);
		fclose(f);
	}*/

	return 0;
}
