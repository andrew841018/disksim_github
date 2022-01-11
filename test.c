#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main(){
	int i;
	
	char tmp[100];
	for(i=0;i<10;i++){
		FILE *a=fopen("a.txt","w+");
		sprintf(tmp,"%d",i);
		fprintf(a,"%s\n",tmp);
		fclose(a);
	}
	
	/*
	float benefit;
	benefit=(float)5/3040;
	benefit/=4500;
	printf("%.19f\n",benefit);*/
	return 0;
}
