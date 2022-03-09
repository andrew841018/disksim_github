#include <sys/types.h>  //
#include <sys/stat.h>   //open()
#include <fcntl.h>	//	
#include <unistd.h> //read(),write()
#include <stdio.h>
#include<sys/time.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
int main(){
	struct timeval start;
	struct timeval end;
	unsigned long diff;
	char buffer[4096];//4kb
	char wbuffer[4096];
	int fd,i;
	unsigned int count =100*1024*1024/4096;//total 100MB,each time access 4kb,how much time access all file-->count=number of time access all file
	srand(time(NULL));
	
	int k;
	for(k=0;k<32;k++)
	{
		strcat(wbuffer,"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
	}

	fd = open("file.txt", O_RDWR );
	if(fd == -1)
	{
		printf("open file fail.\n");
		return 1;
	}
//sread	
	gettimeofday(&start,NULL);
	for(i=0; i<count; i++)
	{
		read(fd, buffer, 4096);
	}
	gettimeofday(&end,NULL);

	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("sread Time: %ld us\n", diff);

//swrite
	lseek(fd,0,SEEK_SET);
	gettimeofday(&start,NULL);
	for(i=0; i<count*2; i++)
	{
		write(fd, wbuffer, 2048);
	}
	fsync(fd);
	gettimeofday(&end,NULL);

	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("swrite Time: %ld us\n", diff);

//rread	
	gettimeofday(&start,NULL);
	for(i=0; i<50000; i++)
	{
		lseek(fd,rand()%25600*4096,SEEK_SET);
		read(fd, buffer, 4096);
	}
	gettimeofday(&end,NULL);

	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("rread Time: %ld us\n", diff);

//rwrite1
	gettimeofday(&start,NULL);
	for(i=0; i<50000; i++)
	{
		lseek(fd,rand()%25600*4096,SEEK_SET);
		write(fd, wbuffer, 2048);
	}
	gettimeofday(&end,NULL);

	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("rwrite1 Time: %ld us\n", diff);

//rwrite2
	gettimeofday(&start,NULL);
	for(i=0; i<50000; i++)
	{
		lseek(fd,rand()%25600*4096,SEEK_SET);
		write(fd, wbuffer, 2048);
		fsync(fd);
	}
	gettimeofday(&end,NULL);

	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("rwrite2 Time: %ld us\n", diff);

	close(fd);

	return 0;
}

