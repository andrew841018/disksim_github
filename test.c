#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main(){
	char b[100];
	int i=50;
	FILE *a=fopen("test.txt","a+");
	sprintf(b,"%d",i);
	fprintf(a,"%s\n",b);
	i++;
	sprintf(b,"%d",i);
	fprintf(a,"%s\n",b);
	fclose(a);
	/*
	float benefit;
	benefit=(float)5/3040;
	benefit/=4500;
	printf("%.19f\n",benefit);*/
	return 0;
}
