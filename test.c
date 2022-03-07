#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main(){
	int a[100];
	int i;
	for(i=0;i<100;i++)
		a[i]=-1;
	for(i=0;i<100;i++)
		printf("%d\n",a[i]);
	return 0;
}
