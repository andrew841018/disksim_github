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
int block[1000000],write_count[1000000];
int ignore_block_count[1000000]={0},ignore_num[1000000];
int ig=0;
double benefit[1000000];
int main(){	
	int req_count=0;
    //write buffer size set to 10MB,contained 40 blocks
    clock_t start,end;
    start=clock();
    int count=0,i,k,j,kick_count=0;
    for(i=0;i<1000000;i++){
        block[i]=-1;
        ignore_num[i]=-1;
        write_count[i]=0;
        benefit[i]=0;
    }
    buf *wb;
    
    int enter=0;
    int test=0,hit_count=0,testing[100]={0};
    char buffer[1024],buffer0[1024];
    char *substr=NULL,*substr0=NULL;
    int tmp_block_num,free_block;
    float tmp_benefit;
    int block_store[10000];
    int block_store_index=0;
    int sector_number,sector_number1;
    const char *const delim=" ";
    const char *const delim1=":";
    int physical_block_num,dur_count=0;//dur_count is the index of duration array, it mean which lpn 
    wb=malloc(sizeof(buf));
    free_block=40;   	
    for(i=0;i<20000;i++){
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
	
    FILE *a=fopen("collected data(from disksim)/trace(for testing)/info(iozone2).txt","r");
    FILE *result=fopen("duration.txt","w");
   // FILE *buffer_tag=fopen("buffer_or_not.txt","w");
    while (fgets(buffer,1024,a)!=NULL)
    {		
        substr=strtok(buffer,delim);//time
        substr=strtok(NULL,delim);//disk_number
        substr=strtok(NULL,delim);//third...sector_num
        sector_number=atoi(substr);
        substr=strtok(NULL,delim);//total sector
        substr=strtok(NULL,delim);//req_type
        req_type=atoi(substr);        
        if(exist[sector_number]==1){	
            if(block[sector_number]==79759){
                testing[3]++;
                //printf("%d\n",testing[3]);
                }																						
            enter++;
            if(count<=40){
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
		}
            if(b==0){  //ppn=62時進入，此時會將新的data寫滿ppn 63 
				if(count<=40){
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
                            testing[2]++;
							break;															
						}
					}
			}
                if(b1==0){//first time will enter here.					
                    count++;
                    testing[0]++;
                    wb->block[count-1]->physical_block_number=block[sector_number];
                    wb->block[count-1]->benefit=benefit[sector_number];
                    free_block--;                   
                    wb->block[count-1]->sector_num[0]=sector_number; 
                    block_store[block_store_index]=block[sector_number];
                    block_store_index++;
                    //fprintf(buffer_tag,"%d %d\n",block[sector_number],1);													
                }						 		
                if(b1==1){//進入for loop但沒進入condition----add a new block 
					
                    if(free_block==0){												   
                            //kick block                          
                        int min_block_num,block_index; 
                        float min=10000;   
                        tmp_block_num=block[sector_number];//current block number
                        tmp_benefit=benefit[sector_number];//current block benefit                         
                        //find min benefit block in write buffer
                        if(count<=40){
                            for(k=0;k<count;k++){
                                if (min>wb->block[k]->benefit && wb->block[k]->benefit!=0){
                                    min=wb->block[k]->benefit;
                                    block_index=k;
                                    min_block_num=wb->block[k]->physical_block_number;
                                }
                            }
                        }                                                                     
                        //min=min benefit block in write buffer
                            //kick min bloif(tmp_benefit>min){
                        //kick min block from write buffer                
                        if(tmp_benefit>min){
                        //kick min block from write buffer
                           fprintf(result,"%d %d\n",min_block_num,wb->block[block_index]->duration);
                            testing[0]++;
                            testing[1]++;
                            //test++;
                            //printf("%s %d\n",dur[dur_count],test);
                            for(i=0;i<64;i++){
                                wb->block[block_index]->sector_num[i]=-1; 
                            }                     
                            wb->block[block_index]->physical_block_number=-1;                    
                            wb->block[block_index]->duration=0;
                            free_block++;
                            wb->block[block_index]->benefit=0;
                            //current request write into block.....create new block
                            wb->block[block_index]->physical_block_number=tmp_block_num;
                            wb->block[block_index]->buffer_or_not=1;
                            wb->block[block_index]->benefit=tmp_benefit;
                            wb->block[block_index]->sector_num[0]=sector_number;
                           // fprintf(buffer_tag,"%d %d\n",tmp_block_num,1);
                            block_store[block_store_index]=tmp_block_num;
							block_store_index++;
                            free_block--;
                    }                 
                    else{
                        //test++;
                        //ignore current request...do nothing						
                        int ignore=0;
                        for(k=0;k<ig;k++){
                            if(ignore_num[k]==tmp_block_num){
                                ignore=1;
                                hit_count++;
                                ignore_block_count[tmp_block_num]++;                               
                                if(tmp_block_num==79759){
                                  //  printf("sector:%d count:%d\n",sector_number,ignore_block_count[tmp_block_num]);
                                }
                            //  printf("hit in ssd:%d %d\n",ignore_num[k],hit_count);
                                break;
                            }
                        }    
                        int store=0;   
                                  
                        //count>41 mean:after entrer here once,for loop will execute.
                        if(ignore==0){
							for(i=0;i<=block_store_index;i++){
								if(tmp_block_num==block_store[i]){
									store=1;
									break;
								}
							}
							if(store==0){ 
								ignore_num[ig]=tmp_block_num;
								ignore_block_count[tmp_block_num]++;
								//fprintf(buffer_tag,"%d %d\n",tmp_block_num,0);
								test++;
								testing[0]++;
								// printf("%d %d\n",ignore_num[ig],test);
								ig++;
						}
                    }                                 
                    }
				}
				else if(free_block>0){  // create new block  
					count++;    
                    testing[0]++;	
					wb->block[count-1]=malloc(sizeof(buf)); 
					wb->block[count-1]->sector_num[0]=sector_number;                   
					// printf("%d\n",atoi(substr1));
					wb->block[count-1]->physical_block_number=block[sector_number];
					wb->block[count-1]->benefit=benefit[sector_number];
                  //  fprintf(buffer_tag,"%d %d\n",block[sector_number],1);
                    block_store[block_store_index]=block[sector_number];
                    block_store_index++;
					free_block--; 
                    
			}				 	  								
                }                                                  
            } 
        }    
    }              
    //若仔細檢查ignore部分，會發現有max block count in ignore>min block count in write buffer，
    //那是因為在模擬時，會將disksim當中不寫入write buffer的request也一併寫入，因為只判斷是否寫入和該sector是否存在於write buffer
    //因此有可能出現以下情形:某些write request會寫入write buffer同時也會做某些大量寫入處理(但不在write buffer)
    //此時我的判斷會認為該request是寫入，且存在於write buffer，因此將這個request放入，但其實這個reuqest實際寫入write buffer的次數
    //不會那麼多(程式將write buffer外處理的那些寫入也一併算入)
 

    //但我在抓資料時，只抓寫入write buffer的次數，因此benefit是正確的，整個simulation也是正確，只是如果特別去注意max ignore count
    //會發現有點矛盾，但不影響結果
/*
    float max=0;
    for(i=0;i<1000000;i++){
        if(max<ignore_block_count[i] && ignore_block_count[i]!=0){
            max=ignore_block_count[i];
            tmp_block_num=i;
        }
    }
    float min=10000;
    int t;
    for(i=0;i<40;i++){
        if(wb->block[i]->benefit<min){
            min=wb->block[i]->benefit;
            t=wb->block[i]->physical_block_number;
        }
    }
    printf("write buffer min block:%d count:%f block num:%d block_count:%f\n",t,min*64*64,tmp_block_num,max);
 */ fclose(a); 
    //fclose(buffer_tag);
    fclose(result);
    end=clock();
    double diff=end-start;
    printf("req_count:%d enter:%d\n",req_count,enter);
    printf("total excution time(s):%f\n",diff/CLOCKS_PER_SEC); 
    printf("ignore hit count:%d ignore new block:%d total block:%d\n",hit_count,test,testing[0]); 
    printf("number of block kick from write buffer:%d\n",testing[1]);  
    printf("hit in write buffer block:%d\n",testing[2]);
    return 0;
}
