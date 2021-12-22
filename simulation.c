#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
typedef struct write_buffer
{
  struct write_buffer *block[4000];//write buffer size=1184 blocks, 超過1183，用來存放那些不會被放入write buffer的資訊
  int physical_block_number;
  int ppn;//total 64 page...each block ppn from 0~63
  int full;//0..not full,1...is full         
  float benefit;
  int free_block;//free block
  int duration;
  int buffer_or_not;
}buf;
int main(){
    //write buffer size set to 10MB,contained 40 blocks
    int count=0,i,k;
    buf *wb;
    char buffer[1024],buffer1[1024];
    char *substr=NULL,*substr1=NULL;
    int sector_number,sector_number1;
    const char *const delim=" ";
    int physical_block_num,dur_count=0;//the index of char array which store duration of each block
    wb=malloc(sizeof(buf));
    wb->free_block=40;
    for(i=0;i<4000;i++)
		wb->block[i]=malloc(sizeof(buf));
    for(i=0;i<4000;i++){
		wb->block[i]->ppn=-1;
		wb->block[i]->duration=0;
		wb->block[i]->buffer_or_not=0;
	}
    int b=0,b1=0;
    char dur[1184][100]={0};
    // write buffer total 1184 blocks, 1 block=64 pages,  1 req=4kb=1 page=8 sectors
    FILE *a=fopen("trace(run1_Postmark_2475).txt","r");
    while (fgets(buffer,1024,a)!=NULL)
    {
        substr=strtok(buffer,delim);//first number
        substr=strtok(NULL,delim);//second num
        substr=strtok(NULL,delim);//third...sector_num
        sector_number=atoi(substr);
        FILE *a1=fopen("secotr and physical_block_num and benefit.txt","r");
        while(fgets(buffer1,1024,a1)!=NULL){
            substr1=strtok(buffer1,delim);//first...sector_num
            sector_number1=atoi(substr1);
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
                           int min=10000,min_block_num,index;
                           int tmp_block_num;
                           float tmp_benefit;
                           tmp_block_num=atoi(substr1);//current block number
                           substr1=strtok(NULL,delim);//third...benefit
                           tmp_benefit=atof(substr1);//current block benefit
                           //find min benefit block in write buffer
                        for(k=0;k<count-1;k++){
                          if (min>wb->block[k]->benefit){
                            min=wb->block[k]->benefit;
                            index=k;
                            min_block_num=wb->block[k]->physical_block_number;
                          }
                        }
                        //min=min benefit block in write buffer
                        if(tmp_benefit>min){
                          //kick min block from write buffer
                          
                          sprintf(dur[dur_count],"%d",min_block_num);
                          strcat(dur[dur_count]," ");
                          sprintf(dur[dur_count],"%d",wb->block[index]->duration);
                          dur_count++;
                          wb->block[min_block_num]->physical_block_number=-1;
                          //current bug...after physical_block_number set to -1
                          //it will be used to min_block_num
                          //and further use for wb->block[min_block_num]->...
                          // it cause bug becuase use -1 as array index.
                          wb->block[min_block_num]->duration=0;
                          wb->block[min_block_num]->free_block++;
                          wb->block[min_block_num]->ppn=-1;
                          count--;
                          //current request write into block
                          wb->block[count]->physical_block_number=tmp_block_num;
                          wb->block[count]->buffer_or_not=1;
                          wb->block[count]->benefit=tmp_benefit;
                          wb->block[count]->ppn++;
                          wb->block[count]->free_block--;
                        }
                        else{
                          //ignore current request...do nothing
                          //此時count=1184 
                          wb->block[count]->physical_block_number=tmp_block_num;
                          wb->block[count]->buffer_or_not=0;                      
                        }

                      }
                      else{
                      
						  wb->block[count]->buffer_or_not=1;
						  wb->block[count]->ppn++;
						  wb->block[count]->physical_block_number=atoi(substr1);
						  substr1=strtok(NULL,delim);//third...benefit
						  wb->block[count]->benefit=atof(buffer1);
						  wb->free_block--;  
				  }  
                                       
                    }
                    ///****************/////
                    if(b==1){//第二個request到來時，會進入.....此時count還沒更改
                        if(atoi(buffer1)==wb->block[count]->physical_block_number){//write in same block
                            wb->block[count]->ppn++;
                        }
                        else{
                            count++;//write in new block
                            wb->block[count]->buffer_or_not=1;
                            wb->block[count]->physical_block_number=atoi(substr1);
                            substr1=strtok(NULL,delim);//third...benefit
                            wb->block[count]->benefit=atof(buffer1); 
                            wb->block[count]->ppn++;                         
                            wb->free_block--;
                            wb->block[count]->buffer_or_not=1;
                            b=0;
                        }
                        
                    }
                    if(count==0 && b==0){//一開始會執行這裡
                        wb->block[0]->physical_block_number=atoi(substr1);
                        substr1=strtok(NULL,delim);//third
                        wb->block[0]->benefit=atof(substr1);
                        wb->block[count]->ppn++;
                        wb->free_block--;
                        wb->block[count]->buffer_or_not=1;
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
            fclose(a1);
    }
    fclose(a);
    FILE *result=fopen("duration.txt","a+");
    for(i=0;i<1184;i++){
      if(dur[i]!="0")
        fprintf(result,"%s\n",dur[i]);
    }
    fclose(result);
    FILE *result1=fopen("buffer_or_not.txt","a+");
    for(i=0;i<count;i++){
      if(wb->block[i]->physical_block_number!=-1)
        fprintf(result1,"%d %d\n",wb->block[i]->physical_block_number,wb->block[i]->buffer_or_not);
    }
    fclose(result1);
    return 0;
}
