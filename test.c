#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef struct _a{
	int l;
}a;
int main(){
	char *substr=NULL,buffer[1024];
	const char *const delim=" ";
	int sector_number,i=55,k=22;
	char a[1000][100]={0},b[100];
	sprintf(a[1],"%d",i);
	strcat(a[1]," ");
	sprintf(b,"%d",k);
	strcat(a[1],b);
	if(1==1 && 2!=3)
		printf("oh no\n");
/*	FILE *a=fopen("run1_Postmark_2475.txt","r");	
	while (fgets(buffer,1024,a)!=NULL){
		substr=(char*)strtok(buffer,delim);
		substr=(char*)strtok(NULL,delim);//second num
        	substr=(char*)strtok(NULL,delim);//third...sector_num
//        	sector_number=atoi(substr);*/
//}
	return 0;
}
