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
    char buffer[4096];
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
	printf("sequential Time: %ld us\n", diff);
    map=0;
    //sequential write
    gettimeofday(&start,NULL);
    for(i=0;i<count;i++){
        memcpy(buffer,map,4096);//4kb
        map+=4096;
        fsync(f1);
    }
    gettimeofday(&end,NULL);
	diff = 1000000*(end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("sequential Time: %ld us\n", diff);


}