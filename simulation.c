#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
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
int exist[1000000]={0};
char tmp[1000000][1024];
int block[1000000],write_count[1000000];
double benefit[1000000];
int main(){	
	int req_count=0;
    //write buffer size set to 10MB,contained 40 blocks
    clock_t start,end;
    start=clock();
    int count=0,i,k,j,kick_count=0;
    for(i=0;i<1000000;i++){
        block[i]=-1;
        write_count[i]=0;
        benefit[i]=0;
    }
    buf *wb;
    int enter=0;
    int testing[100]={0};
    int test=0;
    char buffer[1024],buffer1[1024],buffer0[1024];
    char *substr=NULL,*substr1=NULL,*substr0=NULL;
    int tmp_block_num;
    float tmp_benefit;
    int sector_number,sector_number1;
    const char *const delim=" ";
    const char *const delim1=":";
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
    char dur[50000][100]={0},temp[100]={0};  
    int req_type;   
    // write buffer total 1184 blocks, 1 block=64 pages,  1 req=4kb=1 page=8 sectors
	FILE *info=fopen("collected data(from disksim)/sector num-logical block num-benefit-sector count.txt","r");
	while(fgets(buffer0,1024,info)!=NULL){
		substr0=strtok(buffer0,delim);//sector number	
		sector_number=atoi(substr0);
		exist[sector_number]=1;				
		substr0=strtok(NULL,delim);//physical block number
        block[sector_number]=atoi(substr0);
		substr0=strtok(NULL,delim);//benefit
        benefit[sector_number]=atof(substr0);
		substr0=strtok(NULL,delim);//sector count;
        write_count[sector_number]=atoi(substr0);		
		req_count+=atoi(substr0);
	}
	fclose(info);
    FILE *a=fopen("collected data(from disksim)/trace(run1_Postmark_2475).txt","r");
    FILE *result=fopen("duration.txt","a+");
    while (fgets(buffer,1024,a)!=NULL)
    {		
        substr=strtok(buffer,delim);//time
        substr=strtok(NULL,delim);//disk_number
        substr=strtok(NULL,delim);//third...sector_num
        sector_number=atoi(substr);
        substr=strtok(NULL,delim);//total sector
        substr=strtok(NULL,delim);//req_type
        if(exist[sector_number]==1){																				
            enter++;
            for(j=0;j<count;j++){
                if(wb->block[j]->sector_num[0]!=-1){//write buffer block
                    wb->block[j]->duration++;
                }
            }
            for(j=0;j<count;j++){
                if(wb->block[j]->sector_index>=63){
                    wb->block[j]->full=1;
                }
            }  
            if(b==0){  //ppn=62時進入，此時會將新的data寫滿ppn 63 
                for(i=0;i<count;i++){
                    b1=1;                                                       
                    if(block[sector_number]==wb->block[i]->physical_block_number){//write in same block
                        for(j=0;j<64;j++){
                            if(wb->block[i]->sector_num[j]==sector_number){//judge whether new req hit the same page.									
                                b2=1;
                            }						
                        }
                        if(b2==0 && wb->block[i]->full==0){//access the same block,but different page
                            wb->block[i]->sector_index++;
                            wb->block[i]->sector_num[wb->block[i]->sector_index]=sector_number;
                        }	                      											
                        b1=2; 
                        break;                                             															
                    }
                }          
                if(b1==0){//first time will enter here.
                    count++;											
                    wb->block[count-1]->physical_block_number=block[sector_number];
                    wb->block[count-1]->benefit=benefit[sector_number];
                    wb->free_block--;
                    wb->block[count-1]->sector_num[0]=sector_number; 													
                }						 		
                if(b1==1){//進入for loop但沒進入condition----add a new block 
                    if(wb->free_block==0){						   
                        //kick block                          
                        int min_block_num,block_index; 
                        float min=10000;   
                        tmp_block_num=block[sector_number];//current block number
                        tmp_benefit=benefit[sector_number];//current block benefit
                                                 
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
                    fprintf(result,"%d %d\n",min_block_num,wb->block[block_index]->duration);               
                    for(i=0;i<64;i++){
                        wb->block[block_index]->sector_num[i]=-1;                      
                    }
                    wb->block[block_index]->physical_block_number=-1;                    
                    wb->block[block_index]->duration=0;
                    wb->free_block++;
                    wb->block[block_index]->benefit=0;
                    //current request write into block.....create new block
                    wb->block[block_index]->physical_block_number=tmp_block_num;
                    wb->block[block_index]->benefit=tmp_benefit;
                    wb->block[block_index]->sector_num[0]=sector_number;
                    wb->free_block--;                   
                    }                        
                    else if(wb->free_block>0){  // create new block  
                        count++;
                        wb->block[count-1]->sector_num[0]=sector_number;
                        // printf("%d\n",atoi(substr1));
                        wb->block[count-1]->physical_block_number=block[sector_number];
                        wb->block[count-1]->benefit=benefit[sector_number];
                        wb->free_block--; 	
                        				
                }                                                                            				 	  								
                }                                                  
            } 
        }            
    }
    fclose(a); 
   fclose(result);
    end=clock();
    double diff=end-start;
    printf("req_count:%d enter:%d\n",req_count,enter);
    printf("total excution time(s):%20.f\n",diff/CLOCKS_PER_SEC); 
     
    return 0;
}
