#include <sys.mman.h>//mmap
#include <sys/types.h>
#include <sys/stat.h>//open()
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>//memcpy()
#include <errno.h>
int main(){
    char buffer[4];
    int i,count=1024*1024*100/4096;
    char *map;//mapping variable
    f1=open("file.txt","O_RDWR");
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
        perror("error"\n);
        return 0;
    }
    //sequential read
    for(i=0;i<count;i++){
        memcpy(buffer,map,4);
        printf("%s\n",buffer);
        exit(0);
    }
    
    


}