#include <stdlib.h>
#include <stdio.h>
int main(){
    char buffer[1024];
    char *substr=NULL;
    const char *const delim=" ";
    FILE *a=fopen("physical_block_number and benefit.txt","r");
    while (fgets(buffer,1024,a)!=NULL)
    {
        substr=strtok(buffer,delim);    
        substr=strtok(NULL,delim);//move to next line
    }
  
    fclose(a);
    return 0;
}