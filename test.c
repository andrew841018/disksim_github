#include<stdio.h>
#include<stdlib.h>
typedef struct _a{
	int l;
}a;
int main(){
	char *substr=NULL,buffer[1024];
	const char *const delim=" ";
	int sector_number,i;
	FILE *a=fopen("run1_Postmark_2475.txt","r");	
	while (fgets(buffer,1024,a)!=NULL){
		substr=(char*)strtok(buffer,delim);
/*		substr=(char*)strtok(NULL,delim);//second num
        	substr=(char*)strtok(NULL,delim);//third...sector_num
//        	sector_number=atoi(substr);*/
		printf("%s\n",substr);
		sleep(5);
}
	return 0;
}
