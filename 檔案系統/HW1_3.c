#include <sys/mman.h>//mmap
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>//open()
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>//memcpy()
#include <errno.h>
int main(){
    struct timeval start;
	struct timeval end;
	unsigned long diff;
    char buffer[4096],disk[4096];
    int i,count=1024*1024*100/4096;
    char *map;//mapping variable
    int  f1=open("file.txt",O_RDWR);
    if(f1==-1){
        perror("error:\n");
    }
    //get file size
    struct stat st;
    stat("file.txt",&st);
    long int file_size=st.st_size;
    //mmap
    map=mmap(NULL,file_size,PROT_WRITE | PROT_READ,MAP_SHARED,f1,0);
    if(map==MAP_FAILED){
        perror("error\n");
        return 0;
    }
    //sequential read
    gettimeofday(&start,NULL);
    for(i=0;i<count;i++){
        memcpy(buffer,map,4096);//4kb
        map+=4096;
    }
    gettimeofday(&end,NULL);
	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("sequential read: %ld us\n", diff);
    //sequential write
    map=mmap(NULL,file_size,PROT_WRITE | PROT_READ,MAP_SHARED,f1,0);
    gettimeofday(&start,NULL);
    for(i=0;i<1024;i++){
        strcat(disk,"ar");
    }
    
    for(i=0;i<count*2;i++){
        memcpy(map,disk,2048);//4kb
        map+=2048;
        fsync(f1);
    }
    gettimeofday(&end,NULL);
	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("sequential write: %ld us\n", diff);
    //random read
    gettimeofday(&start,NULL);

    map=mmap(NULL,file_size,PROT_WRITE | PROT_READ,MAP_SHARED,f1,0);
    for(i=0;i<50000;i++){
        map+=rand()%25600;
        memcpy(buffer,map,4096);
        map=mmap(NULL,file_size,PROT_WRITE | PROT_READ,MAP_SHARED,f1,0);
    }
    gettimeofday(&end,NULL);
	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("random read: %ld us\n", diff);
    return 0;
}