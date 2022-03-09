#include <stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/time.h>
#include<unistd.h>
#include<string.h>
int main(){
	struct timeval start;
	struct timeval end;
	unsigned long diff;
	FILE *fp;
	char buffer[4096];//4096 byte=1024*4 byte=4KB
	char wbuffer[4096];
	srand(time(NULL));
 	
	int k;
	for(k=0;k<32;k++)
	{
		strcat(wbuffer,"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
	}
	
	
	fp = fopen("file.txt", "rw");
	if(fp == NULL)
	{
		printf("file open fail.\n");
		return 1;
	}
	
	int i;
	unsigned int count =100*1024*1024/4096;//1024*1024*100 byte=1024*100 KB=100MB. 100MB file,each time read/write 4kb
	//總共需要count次來讀/寫該檔案
 
//sread
	gettimeofday(&start,NULL);
	for(i=0; i<count; i++)
	{
		fread(buffer, sizeof(char), 4096, fp);//sizeof(char)=每個element多大, 4096=每次存取的element數量
	}
	gettimeofday(&end,NULL);
 
	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("sread Time: %ld us\n", diff);


//swrite
	fseek(fp,0,SEEK_SET);//前面讀完整個檔案後，指標在檔案的尾端，這裡是將指標移回檔案開頭
	gettimeofday(&start,NULL);
	for(i=0; i<count*2; i++)//因為寫入每次只寫2KB,而count是以4KB來做計算的，所以在這邊要*2
	{
		fwrite(wbuffer, sizeof(char), 2048, fp);
	}
	fsync(fileno(fp));//write into HDD
	gettimeofday(&end,NULL);
 
	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("swrite Time: %ld us\n", diff);
 	
//rread	
	gettimeofday(&start,NULL);
	for(i=0; i<50000; i++)
	{
		//25600=1024*1024*100/4096..->代表100MB,每次讀4kb,要讀的次數，可以想成將100MB切成25600個區塊，隨機選擇某個區塊
		//因此最大的區塊不能>=25600，然後一旦選出區塊，因為一個區塊4kb，所以要*4096
		fseek(fp,rand()%25600*4096,SEEK_SET);
		fread(buffer, sizeof(char), 4096, fp);//each time read 4kb
	}
	gettimeofday(&end,NULL);
 
	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("rread Time: %ld us\n", diff);

//rwrite1
	gettimeofday(&start,NULL);
	for(i=0; i<50000; i++)
	{
		fseek(fp,rand()%25600*4096,SEEK_SET);
		fwrite(wbuffer, sizeof(char), 2048, fp);//each time write 2kb
	}
	gettimeofday(&end,NULL);
 
	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("rwrite1 Time: %ld us\n", diff);

//rwrite2
	gettimeofday(&start,NULL);
	for(i=0; i<50000; i++)
	{
		fseek(fp,rand()%25600*4096,SEEK_SET);
		fwrite(wbuffer, sizeof(char), 2048, fp);
		fsync(fileno(fp));
	}
	gettimeofday(&end,NULL);
 
	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("rwrite2 Time: %ld us\n", diff);
	
	fclose(fp);
 
	return 0;
}

