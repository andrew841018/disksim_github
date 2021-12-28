#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
typedef struct write_buffer
{
  struct write_buffer *block[40];//write buffer size=1184 blocks, 超過1183，用來存放那些不會被放入write buffer的資訊
  int physical_block_number;
  int full;//0..not full,1...is full         
  float benefit;
  int free_block;//free block
  int duration;
  int buffer_or_not;
  int sector_index;
  int sector_num[64];;//total 64 page...each block ppn from 0~63
}buf;
int main(){
    //write buffer size set to 10MB,contained 40 blocks
    int count=0,i,k,j,kick_count=0;
    buf *wb;
    int test=0;
    char buffer[1024],buffer1[1024];
    char *substr=NULL,*substr1=NULL;
    int tmp_block_num;
    float tmp_benefit;
    int sector_number,sector_number1;
    const char *const delim=" ";
    int physical_block_num,dur_count=0;//dur_count is the index of duration array, it mean which lpn 
    wb=malloc(sizeof(buf));
    wb->free_block=40;	
    for(i=0;i<wb->free_block;i++){
		wb->block[i]=malloc(sizeof(buf));
		wb->block[i]->duration=0;
		wb->block[i]->full=0;
		wb->block[i]->sector_index=0;
		wb->block[i]->physical_block_number=-1;
		for(j=0;j<64;j++){
			wb->block[i]->sector_num[j]=-1;
		}
	}
    int b1=0,b=0,b2=0,full_block_num,page_count=0;
    char dur[10000][100]={0},temp[100]={0};
    int sector_count=0,block_count=0;
    // write buffer total 1184 blocks, 1 block=64 pages,  1 req=4kb=1 page=8 sectors
    FILE *a=fopen("collected data(from disksim)/trace(run1_Postmark_2475).txt","r");
    while (fgets(buffer,1024,a)!=NULL)
    {		
        substr=strtok(buffer,delim);//first number
        substr=strtok(NULL,delim);//second num
        substr=strtok(NULL,delim);//third...sector_num
        sector_number=atoi(substr);
        FILE *a1=fopen("collected data(from disksim)/secotr and physical_block_num and benefit.txt","r");
        while(fgets(buffer1,1024,a1)!=NULL){
            substr1=strtok(buffer1,delim);//first...sector_num
            sector_number1=atoi(substr1);
            if(sector_number==sector_number1){
				for(j=0;j<count;j++){
					if(wb->block[j]->sector_num[0]!=-1){//write buffer block
						wb->block[j]->duration++;
					}
				}
                substr1=strtok(NULL,delim);//second---physical_block_number 
                for(j=0;j<count;j++){
					if(wb->block[j]->sector_index>=63){
						wb->block[j]->full=1;
					}
				}  
                if(b==0){  //ppn=62時進入，此時會將新的data寫滿ppn 63 
                    for(i=0;i<count;i++){
                        b1=1;
                        if(atoi(substr1)==wb->block[i]->physical_block_number && wb->block[i]->full==0){//write in same block
							for(j=0;j<64;j++){
								if(wb->block[i]->sector_num[j]==sector_number1){//judge whether new req hit the same page.									
									b2=1;
								}						
							}
							if(b2==0){//access the same block,but different page
								wb->block[i]->sector_index++;
								wb->block[i]->sector_num[wb->block[i]->sector_index]=sector_number;
							}						
							b1=2;
							break;															
                        }
                    }
                    if(b1==0){//first time will enter here.
						count++;						
						wb->block[count-1]->physical_block_number=atoi(substr1);
						substr1=strtok(NULL,delim);//third...benefit
						wb->block[count-1]->benefit=atof(substr1);
						wb->free_block--;
						wb->block[count-1]->sector_num[0]=sector_number; 													
					}
                    if(b1==1){//進入for loop但沒進入condition----add a new block                    
                      if(wb->free_block==0){
                           //kick block                          
                           int min_block_num,block_index; 
                           float min=10000;   
                           tmp_block_num=atoi(substr1);//current block number
                           substr1=strtok(NULL,delim);//third...benefit
                           tmp_benefit=atof(substr1);//current block benefit
                           //find min benefit block in write buffer
                        for(k=0;k<count;k++){
                          if (min>wb->block[k]->benefit && wb->block[k]->benefit!=0){
                            min=wb->block[k]->benefit;
                            block_index=k;
                            min_block_num=wb->block[k]->physical_block_number;
                          }
                        }                        
                        //min=min benefit block in write buffer
                          //kick min block from write buffer
                          
                          
                        sprintf(dur[dur_count],"%d",min_block_num);                   
                        strcat(dur[dur_count]," ");
                        sprintf(temp,"%d",wb->block[block_index]->duration);
                        strcat(dur[dur_count],temp);                       
                        dur_count++;                                             
                        for(i=0;i<64;i++)
							wb->block[block_index]->sector_num[i]=-1;                      
                        wb->block[block_index]->physical_block_number=-1;                    
                        wb->block[block_index]->duration=0;
                        wb->free_block++;
                        wb->block[block_index]->benefit=0;
                          //current request write into block.....create new block
                        wb->block[block_index]->physical_block_number=tmp_block_num;
                        wb->block[block_index]->benefit=tmp_benefit;
                        wb->block[block_index]->sector_num[0]=sector_number;
                        wb->free_block--; 
						sleep(1);
                      }
                      else if(wb->free_block>0){  // create new block  
						  count++;            
						  wb->block[count-1]->sector_num[0]=sector_number;
						  wb->block[count-1]->physical_block_number=atoi(substr1);
						  substr1=strtok(NULL,delim);//third...benefit
						  wb->block[count-1]->benefit=atof(substr1);
						  wb->free_block--; 					
				  }	  								
                    }                                                  
                }              					     		
            }
        }
            fclose(a1);
    }
    
    fclose(a);/*
    printf("count:%d\n",count);
    printf("block number:%d\n",wb->block[count-1]->physical_block_number);
    int real_buffer_count=0,real_not_buffer_count=0;
    FILE *result=fopen("duration.txt","a+");
    for(i=0;i<=dur_count;i++){
      if(dur[i]!="0")
        fprintf(result,"%s\n",dur[i]);
    }
    fclose(result);*/
    return 0;
}
