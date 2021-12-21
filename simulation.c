#include <stdlib.h>
#include <stdio.h>
#include <string.h>
typedef struct write_buffer
{
  struct write_buffer *block[1184];
  int physical_block_number=-1;
  int ppn=-1;//total 64 page...each block ppn from 0~63
  int full=0//0..not full,1...is full         
  float benefit;
  int free_block=40;//free block
  int duration=0;
}buf;
int main(){
    //write buffer size set to 10MB,contained 40 blocks
    int free_block[1184]; 
    int count=0,i;
    char buffer[1024],buffer1[1024];
    char *substr=NULL,*substr1=NULL;
    int sector_number,sector_number1;
    const char *const delim=" ";
    int physical_block_num,dur_count=0;
    buf *wb;
    int b=0,b1=0;
    char dur[1184][100]={0};
    // write buffer total 1184 blocks, 1 block=64 pages,  1 req=4kb=1 page=8 sectors
    FILE *a=fopen("run1_Postmark_2475.txt","r");
    while (fgets(buffer,1024,a)!=NULL)
    {
        substr=strtok(buffer,delim);//first number
        //physical_block_num=atoi(buffer);
        substr=strtok(NULL,delim);//second num
        substr=strtok(NULL,delim);//third...sector_num
        sector_number=atoi(buffer);
        FILE *b=fopen("secotr and physical_block_num and benefit.txt","r");
        while(fgets(buffer1,1024,b)!=NULL){
            substr1=strtok(buffer1,delim);//first...sector_num
            sector_number1=atoi(buffer1);
            if(sector_number==sector_number1){
                substr1=strtok(NULL,delim);//second---physical_block_number
                if(wb->block[count]->ppn<63){  //ppn=62時進入，此時會將新的data寫滿ppn 63                 
                    for(i=0;i<count;i++){
                        b1=1;
                        if(atoi(buffer1)==wb->block[count]->physical_block_number){//write in same block
                            wb->block[count]->ppn++;
                            b1=2;
                        }                      
                    }
                    if(b1==1){//進入for loop但沒進入condition----add a new block
                      count++;
                      if(count==1184){
                           //kick block
                           int min=10000,min_block_num;
                           int tmp_block_num;
                           float tmp_benefit;
                           tmp_block_num=atoi(buffer1);
                           substr1=strtok(NULL,delim);//third...benefit
                           tmp_benefit=atof(buffer1);
                        for(k=0;k<count-1;k++){
                          if (min>wb->block[k]->benefit){
                            min=wb->block[k]->benefit;
                            min_block_num=wb->block[k]->physicak_block_number;
                          }
                        }
                        //min=min benefit block in write buffer
                        if(tmp_benefit>min){
                          //kick min block from write buffer
                          
                          sprintf(dur[dur_count],"%d",wb->block[min_block_num]->physical_block_number);
                          strcat(dur[dur_count]," ");
                          sprintf(dur[dur_count],"%d",wb->block[min_block_num]->duration);
                          dur_count++;
                          wb->block[min_block_num]->physical_block_number=-1;
                          wb->block[min_block_num]->duration=0;
                          wb->block[min_block_num]->free_block++;
                          wb->block[min_block_num]->ppn=-1;
                          //current request write into block
                          
                        }
                        else{
                          //ignore current request...do nothing
                          count--;
                        }

                      }
                      wb->block[count]->ppn++
                      wb->block[count]->physical_block_number=atoi(buffer1);
                      substr1=strtok(NULL,delim);//third...benefit
                      wb->block[count]->benefit=atof(buffer1);
                      wb->free_block--;                     
                    }

                    ///****************/////
                    if(b==1){//第二個request到來時，會進入.....此時count還沒更改
                        if(atoi(buffer1)==wb->block[count]->physical_block_number){//write in same block
                            wb->block[count]->ppn++;
                        }
                        else{
                            count++;//write in new block
                            wb->block[count]->physical_block_number=atoi(buffer1);
                            substr1=strtok(NULL,delim);//third...benefit
                            wb->block[count]->benefit=atof(buffer1); 
                            wb->block[count]->ppn++;                         
                            wb->free_block--;
                            b=0;
                        }
                        
                    }
                    if(count==0 && b==0){//一開始會執行這裡
                        wb->block[0]->physical_block_number=atoi(buffer1);
                        substr1=strtok(NULL,delim);//third
                        wb->block[0]->benefit=atof(buffer1);
                        wb->block[count]->ppn++;
                        wb->free_block--;
                        b=1;
                    }    
                    ////////**************////////////// 以星號圈出的區間，只有第1~2個request會進入 
                                                     
                }
                else{//block full.....ppn=63進入，此時已滿，無法再寫
                    wb->block[count]->full=1;
                }
              int j,k,min=10000;
              for(j=0;j<count;j++)
                wb->block[j]->duration++;    
            }

        }
    }
    fclose(a);
    return 0;
}
