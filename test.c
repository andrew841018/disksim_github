#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef struct _a{
	int l;
}a;
int main(){
	float benefit;
	benefit=(float)5/3040;
	benefit/=4500;
	printf("%.19f\n",benefit);
	return 0;
}
