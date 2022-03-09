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
    char buffer[];
    char *map;//mapping variable
    f1=open("file.txt","O_RDWR");
    if(f1==-1){
        perror("error:");
    }
    //get file size


}