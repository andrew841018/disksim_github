#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main(){
    int count=0;
    char buffer[1024];
    char *substr=NULL;
    const char *const delim=" ";
    int physical_block_num;
    int write_buffer[1184][64]//total 1184 blocks, 1 block=64 pages,  1 req=4kb=1 page=8 sectors
    //write_buffer[total block number][total page number],write_buffer[15][23]=771 表示第16個block的block number=771，
    //且該block中已經被放入了24個page
    FILE *a=fopen("run1_Postmark_2475.txt","r");
    while (fgets(buffer,1024,a)!=NULL)
    {
        substr=strtok(buffer,delim);
        //physical_block_num=atoi(buffer);
        count++;
        substr=strtok(NULL,delim);//move to next line
    }
    printf("%d\n",count);
    fclose(a);
    return 0;
}
