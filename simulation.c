#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
typedef struct write_buffer
{
  struct write_buffer *block[10000];//write buffer size=1184 blocks, 超過1183，用來存放那些不會被放入write buffer的資訊
  int physical_block_number;
  int ppn;//total 64 page...each block ppn from 0~63
  int full;//0..not full,1...is full         
  float benefit;
  int free_block;//free block
  int duration;
  int buffer_or_not;
  int sector_num;
}buf;
int main(){
    //write buffer size set to 10MB,contained 40 blocks
    int count=0,i,k,j;
    buf *wb;
    char buffer[1024],buffer1[1024];
    char *substr=NULL,*substr1=NULL;
    int tmp_block_num;
    float tmp_benefit;
    int sector_number,sector_number1;
    const char *const delim=" ";
    int physical_block_num,dur_count=0;//dur_count is the index of duration array, it mean which lpn 
    wb=malloc(sizeof(buf));
    wb->free_block=40;	
    for(i=0;i<10000;i++){
		wb->block[i]=malloc(sizeof(buf));
		wb->block[i]->ppn=-1;
		wb->block[i]->duration=0;
		wb->block[i]->buffer_or_not=-1;
		wb->block[i]->full=0;
		wb->block[i]->physical_block_number=-1;
	}
    int b1=0,b=0,full_block_num;
    char dur[10000][100]={0},temp[100]={0};
    int sector_count=0,block_count=0;
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
				sector_count++;
                substr1=strtok(NULL,delim);//second---physical_block_number 
                for(j=0;j<count;j++){
					b=0;
					if(wb->block[j]->ppn>=63){
						b=1;
						full_block_num=j;//which block is full
						break;
					}
				}  
                if(b==0){  //ppn=62時進入，此時會將新的data寫滿ppn 63 
                    for(i=0;i<count;i++){
                        b1=1;
                        if(atoi(substr1)==wb->block[i]->physical_block_number && wb->block[i]->full==0){//write in same block
							if(wb->block[i]->sector_num!=sector_number1)//judge whether new req hit the same page.
								wb->block[i]->ppn++;
							b1=2;
							block_count++;
							printf("a:%d %d\n",sector_count,block_count);
							break;														
                        }
                        printf("b:%d %d\n",sector_count,block_count);
                    }
                    if(b1==0){//first time will enter here.
						count++;
						block_count++;
						printf("c:%d %d\n",sector_count,block_count);
						wb->block[count-1]->buffer_or_not=1;
						wb->block[count-1]->ppn++;
						wb->block[count-1]->physical_block_number=atoi(substr1);
						substr1=strtok(NULL,delim);//third...benefit
						wb->block[count-1]->benefit=atof(substr1);
						wb->free_block--;  
						wb->block[count-1]->sector_num=sector_number;	
					}
                    if(b1==1){//進入for loop但沒進入condition----add a new block
                      count++;
                      block_count++;
                      printf("d:%d %d\n",sector_count,block_count);
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
                        if(tmp_benefit>min){
                          //kick min block from write buffer
                          sprintf(dur[dur_count],"%d",min_block_num);                   
                          strcat(dur[dur_count]," ");
                          sprintf(temp,"%d",wb->block[block_index]->duration);
                          strcat(dur[dur_count],temp);
                          dur_count++;
                          wb->block[block_index]->physical_block_number=-1;                    
                          wb->block[block_index]->duration=0;
                          wb->free_block++;
                          wb->block[block_index]->ppn=-1;    
                          wb->block[block_index]->benefit=0;
                          //current request write into block.....create new block
                          wb->block[block_index]->physical_block_number=tmp_block_num;
                          wb->block[block_index]->buffer_or_not=1;
                          wb->block[block_index]->benefit=tmp_benefit;
                          wb->block[block_index]->ppn++;
                          wb->free_block--;
                        }
                        else{
                          //ignore current request...do nothing
                          wb->block[count-1]->physical_block_number=tmp_block_num;
                          wb->block[count-1]->buffer_or_not=0;                      
                        }
                      }
                      else if(wb->free_block>0){  // create new block                
						  wb->block[count-1]->buffer_or_not=1;
						  wb->block[count-1]->ppn++;
						  wb->block[count-1]->physical_block_number=atoi(substr1);
						  substr1=strtok(NULL,delim);//third...benefit
						  wb->block[count-1]->benefit=atof(substr1);
						  wb->free_block--;  
						  wb->block[count-1]->sector_num=sector_number;
				  }	  								
                    }                                                  
                }
                else{//block full.....ppn=63進入，此時已滿，無法再寫
                    wb->block[full_block_num]->full=1;
                }
				for(j=0;j<count;j++)
					wb->block[j]->duration++;		
            }
        }
            fclose(a1);
    }
    fclose(a);
    
    printf("detail:%d %d\n",sector_count,block_count);
    /*
    FILE *result=fopen("duration.txt","a+");
    for(i=0;i<=dur_count;i++){
      if(dur[i]!="0")
        fprintf(result,"%s\n",dur[i]);
    }
    fclose(result);
    FILE *result1=fopen("buffer_or_not.txt","a+");
    for(i=0;i<count;i++){
      if(wb->block[i]->physical_block_number!=-1)
        fprintf(result1,"%d %d\n",wb->block[i]->physical_block_number,wb->block[i]->buffer_or_not);
    }
    fclose(result1);*/
    return 0;
}
