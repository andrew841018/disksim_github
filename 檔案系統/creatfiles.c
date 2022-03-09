#include <stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/time.h>
#include<unistd.h>
#include<string.h>
int main ()
{
	FILE *fp;
	char wbuffer[4096];
	unsigned int count =100*1024*1024/4096;
	int k,i;
	for(k=0;k<64;k++)
	{
		strcat(wbuffer,"abcdefghijklmnopqrstuvwxyzbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
	}

	fp = fopen("file.txt","w+");
	for(i=0; i<count; i++)
	{
		fwrite(wbuffer, sizeof(char), 4096, fp);
	}
	fclose(fp);

	return(0);
}

