#include <stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/time.h>
#include<unistd.h>
#include<string.h>
int main ()
{
	FILE *fp;
	int k,i;
	for(k=0;k<10000000;k++)
	{
		strcat(wbuffer,"aaaaaaaaaa");//1 character = 1 byte.
	}
	fp = fopen("file.txt","a");
	for(i=0;i<10000000;i++){
		fprintf(fp,"aaaaaaaaaa");
	}
	fclose(fp);
	return(0);
}

