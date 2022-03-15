/*做"memory cache"發生的事,(計算dirty ratio and 定時flush/hint,計算timer and 定量flush/hint,計算sum_block_count*/
/*應該要export給write buffer的:sum_block_count,定時hint,定量hint*/
/*141行跟822行有問題*/ 
/*       
 * DiskSim Storage Subsystem Simulation Environment (Version 4.0)
 * Revision Authors: John Bucy, Greg Ganger
 * Contributors: John Griffin, Jiri Schindler, Steve Schlosser
 *         
 * Copyright (c) of Carnegie Mellon University, 2001-2008.
 *         
 * This software is being provided by the copyright holders under the
 * following license. By obtaining, using and/or copying this software,
 * you agree that you have read, understood, and will comply with the
 * following terms and conditions:  
 *           
 * Permission to reproduce, use, and prepare derivative works of this
 * software is granted provided the copyright and "No Warranty" statements
 * are included with all reproductions and derivative works and associated
 * documentation. This software may also be redistributed without charge
 * provided that the copyright and "No Warranty" statements are included
 * in all redistributions.
 * 
 * NO WARRANTY. THIS SOFTWARE IS FURNISHED ON AN "AS IS" BASIS.
 * CARNEGIE MELLON UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER
 * EXPRESSED OR IMPLIED AS TO THE MATTER INCLUDING, BUT NOT LIMITED
 * TO: WARRANTY OF FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY
 * OF RESULTS OR RESULTS OBTAINED FROM USE OF THIS SOFTWARE. CARNEGIE 
 * MELLON UNIVERSITY DOES NOT MAKE ANY WARRANTY OF ANY KIND WITH RESPECT
 * TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.
 * COPYRIGHT HOLDERS WILL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE
 * OR DOCUMENTATION. 
 *  
 */         
 
/*
 * A sample skeleton for a system simulator that calls DiskSim as
 * a slave.
 *
 * Contributed by Eran Gabber of Lucent Technologies - Bell Laboratories
 *
 * Usage:
 *  syssim <parameters file> <output file> <max. block number>
 * Example:
 *  syssim parv.seagate out 2676846
 */

/*
one block=64 pages
*/ 
 
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m" 
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m" 
#define YELLOW "\033[1;33m"

#define NF 0
#define TF 1
#define RP 2
#define FLUSHBACK 1024 //8 page
#define HINTGROUP 5
#define alpha 0.8
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
       
#include "syssim_driver.h"
#include "disksim_interface.h"
#include "disksim_rand48.h"
extern int sum_block_count[1000000] = {100}; //在my_ssd.c裡宣告
extern int clean_replace = 0;
extern int clean_flush1 = 0;
extern int clean_flush2 = 0;
extern long long int replace_time = 0;
extern long long int flush1_time = 0;
extern long long int flush2_time = 0;
extern int ssd_ARR = 0;
int flush_direct = 0 ;
int test_count2 = 0;
double Ref_temp = 0;
double Ref_diff = 0,diff = 0,diff1=0;
double Ref_dr = 0,dr = 0;
//static float flush_count = 10.0; //with ssd buffer = 256
//static float Mem =50.0;
static float flush_count = FLUSHBACK; //with ssd buffer = 16000
static float Mem = 0;//65535; //131072;//16384;//202025.0;//131072.0;512MB//262144;1GB//1048576;4GB
static float dirty_ratio = 0.4;
static float dirty_count = 0.0;
static double Hitcount = 0.0,Sumcount = 0.0,Hitcount100=0.0, Sumcount100=0.0, Replace100=0.0,misscount=0.0;
static int write_back_type; // 0->replacement 1->rule 1 2->rule 2 ( more than 30s)
static int old_dirty_time = 30 ; // 30 
static int per_time = 5;
int read_miss = 0,read_hit = 0,write_miss = 0,write_hit = 0;
unsigned int clean2dirty=0;
SysTime predict_Ntime=0, predict_Ttime=0, Ntime=0, Ttime=0, Rtime=0, sum_req_time=0, replacement_diff=0,sum_time=0;
int Thint_times=0;
int ReqCount = 0;
long long int evict_count = 0;
long long int count_to_buffer = 0; 
int Rpre_blk=-1;
int Rstart_blk=-1;
double Rstart_time=-1;
int Rstart_size=-1;
unsigned alln=0;
unsigned MAXREQ;
 
int unique_page=0;
struct timeval start,start1;
struct timeval end,end1;

int fa_hint_count=0;
int ft_hint_count=0;
int rp_hint_count=0;
 

#define BLOCK 4096
#define SECTOR  512 
#define BLOCK2SECTOR  (BLOCK/SECTOR)

FILE *Nflush_flow;
FILE *Nflush_hintflow;
int flushgj4=0;
int replacegj4=0;
int check_R=0;

unsigned long tot_Rtime;
float Rcount=0,write_ratio=0,hit_ratio=0,vdratio=0;
float NR=0,Nwr=0,Nhr=0,NVdr=0;
float Rcount2=0,write_ratio2=0,hit_ratio2=0,vdratio2=0;
int firstopen=0;
int WBH=0,RH=0;
int pdFA_count=0,pdR_count=0;
int FA_count=0,R_count=0;
typedef struct  {
  int n; 
  double sum;
  double sqr;
} Stat;


SysTime now = 0;   /* current time */
SysTime CacheTime = 0;   /* current time */
static SysTime next_event = -1; /* next event */
static int completed = 0; /* last request was completed */
static Stat st;
static Stat wst;
static Stat rst;
static Stat wstp;
static Stat rstp;

// A Queue Node (Queue is implemented using Doubly Linked List)
typedef struct QNode
{
  struct QNode *prev, *next;
  unsigned blockNumber; // the page number stored in this QNode(±qtxtÅª¤Jªº) 
  unsigned Req_type; // 0 -> write , 1 -> read
  unsigned Dirty ; // 0 -> clean , 1 -> dirty
  unsigned Hint_Dirty;// 0 -> clean , 1 -> dirty
  unsigned write_type; // 0->replacement 1->rule 1 2->rule 2 ( more than 30s)
  unsigned size;
  unsigned devno;
  int block_count;
  long long int timestamp ;
  unsigned r_count;
  unsigned w_count;
} QNode;
      
// A Queue (A FIFO collection of Queue Nodes)
typedef struct Queue
{
  unsigned count; // Number of filled frames
  //long long int count; // total number of frames
  long long int numberOfFrames; // total number of frames
  QNode *front, *rear;
} Queue;
    
//Q:Queue指的是hint Queue還是cache Queue
//感覺是Cache Queue
 
// A hash (Collection of pointers to Queue Nodes)
typedef struct Hash
{
  int capacity; // how many pages can be there
  QNode **array; // an array of queue nodes
} Hash;
/*
typedef struct Hint_page //all dirty page
{
  unsigned type; // NF/TF/RP
  unsigned page_num;
  double predict_time;
  struct Hint_page *prev, *next;
} Hint_page;
  
typedef struct Hint_Queue
{
  unsigned count; // number flush array offset
  long long int numberOfFrames; // total number of frames
  Hint_page *front, *rear;
} Hint_Queue;*/
 
kick_req *req_RW_count = NULL;
RW_count *page_RW_count = NULL;

Hint_Queue* createHint_Queue( long long int numberOfFrames )//新增一個Queue(Queue的大小)
{
  Hint_Queue* queue = (Hint_Queue *)malloc( sizeof( Hint_Queue ) );//分配空間

  // The queue is empty
  queue->count = 0;
  queue->front = queue->rear = NULL;

  // Number of frames that can be stored in memory
  queue->numberOfFrames = numberOfFrames;
  return queue;
}

// A utility function to check if queue is empty
int isHintQueueEmpty( Hint_Queue* queue )
{
  return queue->rear == NULL;
}

//blockNumberCount:計算目前"Queue中"每個Node各屬於哪個LBN,計算LBN目前在Queue持有幾個Node(sum_block_count[Blk])
void blockNumberCount(Queue *queue){
  QNode *block_count_Point = queue->rear;//block_count_Point=Queue的最後一個Node
  int i = 0;
  for(i = 0 ; i < 1000000 ; i++) //把sum_block_count清空
    sum_block_count[i] = NULL;

  while(1) {
    if(block_count_Point->Dirty == 1 ) {//如果最後一個Node是Dirty的話
      int Blk = block_count_Point->blockNumber / 64;//Blk=最後一個Node的blockNumber/block size 64 (屬於第幾個Logical BN)
      sum_block_count[Blk] ++;//sum_block_count[這個LBN]++(表示這個LBN目前上層持有dirty page個數)
    }
   // printf("blockNumber[%d] = %d\n" , block_count_Point->block_count,sum_block_count[block_count_Point->block_count]);
    //if(block_count_Point != NULL )printf("blkno = %d\n",block_count_Point->blockNumber );
    if(block_count_Point == queue->front || block_count_Point == NULL) break;//測完Queue中所有Node或者Queue是空的,break
    if(block_count_Point->prev != NULL) block_count_Point = block_count_Point->prev; //往前一個繼續測
    else break;
  }
  
}   
             
//------------------------------flush 1024 hint ---------------------------------------------- 
QNode *global_write_back_Point = NULL;
void write_back_hint(Hint_Queue* HintQ, Queue* queue , FILE *fwrite, struct disksim_interface *disksim ,int rep) { 
  //printf(YELLOW"write_back_hint|time=%lf\n"NONE, CacheTime);
  //fprintf(outputfd, "write_back_hint|time=%lf\n", CacheTime);
  int flush_Number = 0;
  int extra_flush_Number = 1; 
  QNode *current_Point = global_write_back_Point;//目前的Node=最後的Node
  //on != 1 //hint flush也是定量flush但是write type=16 (extra_flush)
    clean_flush1 = 1;//????
    flush1_time = CacheTime;
    if(rep==1)
    {
      current_Point = queue->rear;
      int i;
      for(i=0;i<5;i++)
      {
        if(current_Point->Dirty == 1 && current_Point->Hint_Dirty == 1)
        {
          global_HQ[global_HQ_size]=current_Point->blockNumber/8;
          //fprintf(myoutput4, "WBH dirty incoming hint page = %d\n", global_HQ[global_HQ_size] );
          
          global_HQ_size++;
          int i,h=0;
          for(i=0;i<global_HQ_node_size;i++)
          {
            if((current_Point->blockNumber/8)/64==global_HQ_node[i])
            {
              h=1;
              break;
            }
          }
          if(h==0)
          {
            global_HQ_node[global_HQ_node_size]=(current_Point->blockNumber/8)/64;
            global_HQ_node_size++;
            fa_hint_count++;
          }
          
          current_Point->Hint_Dirty = 0;
          //HintQ->count++;//queue大小增加
          current_Point=current_Point->prev;
        }
        else if(current_Point->Dirty == 0)
        {
          current_Point=current_Point->prev;
        }
      }
      return;
    }   
    //fprintf(outputfd, "global_write_back_Point = %d\n", global_write_back_Point->blockNumber);
    //fprintf(myoutput, "global_write_back_Point = %d\n", global_write_back_Point->blockNumber );
    while(current_Point != NULL) {//目前page不是第一個
      //printf("in Nhint while\n");
//////////////////////////////////////////////////    
      if(current_Point->Dirty == 1 && current_Point->Hint_Dirty == 1) {//dirty page
         
        global_HQ[global_HQ_size]=current_Point->blockNumber/8;
        //fprintf(myoutput4, "WBH dirty incoming hint page = %d\n", global_HQ[global_HQ_size] );
        global_HQ_size++;
        int i,h=0;
        for(i=0;i<global_HQ_node_size;i++)
        {
          if((current_Point->blockNumber/8)/64==global_HQ_node[i])
          {
            h=1;
            break;
          }
        }
        if(h==0)
        {
          global_HQ_node[global_HQ_node_size]=(current_Point->blockNumber/8)/64;
          global_HQ_node_size++;
          fa_hint_count++;
        }
         
        current_Point->Hint_Dirty = 0;
        //HintQ->count++;//queue大小增加
        //-----------------------------------------------------------------------------
        extra_flush_Number ++;//extra_flush次數+1 ???
      }
      if(extra_flush_Number == FLUSHBACK && current_Point != queue->front) 
      {
        global_write_back_Point=current_Point->prev;
        break;//flush了1024個dirty page
      }
      else if (current_Point == queue->front) 
      {
        //printf("in else if (current_Point == queue->front) \n");
        global_write_back_Point=current_Point;
        //fprintf(outputfd, "current_Point == queue->front global_write_back_Point = %d\n", global_write_back_Point->blockNumber);
        break;
      }
      current_Point = current_Point->prev;
    }
} 
          
int compare1 (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}
void sort_FB(int point,int* arr)
{
  int i;
  qsort (arr, point+1, sizeof(int), compare1);
}     
//----------------------flush 1024 dirty pages---------------------  
void write_back(Queue* queue , FILE *fwrite, struct disksim_interface *disksim, int on ) { 
 //printf(YELLOW"write_back|on=%d|time=%lf\n"NONE,on, now);
  //fprintf(outputfd, "write_back|on=%d|time=%lf\n",on, now);
  int flush_Number = 0 , i;
  int extra_flush_Number = 1;
  int FBpoint=-1;
  int FlushBlock[FLUSHBACK]={0};   
  QNode *current_Point = queue->rear;//目前的Node=最後的Node
  if(on == 1){ //定量flush
    //fprintf(outputfd, "r->blkno = ");
    //fprintf(fwrite,"write_back\n");
    while(flush_Number < FLUSHBACK) {//目前flush page數量
      //printf("in while\n");
      if(current_Point->Dirty == 1 ) {//目前的page是dirty,表示要flush
        //printf("%d->Dirty=1\n",current_Point->blockNumber);
        flushgj4++;
        current_Point->write_type = 0;//write_type=0表示??;read(1)write(0)
        FBpoint ++ ;
        FlushBlock[FBpoint] = current_Point->blockNumber;
        req_RW_count->page[req_RW_count->page_count].page_num = current_Point->blockNumber/8;
        req_RW_count->page[req_RW_count->page_count].r_count = current_Point->r_count;
        req_RW_count->page[req_RW_count->page_count].w_count = current_Point->w_count;
        req_RW_count->page_count++;
 
        // struct disksim_request *r = malloc(sizeof(struct disksim_request));//動態配置disksim_request空間
        // r->start = now;//現在時間
        // r->flags = current_Point->write_type;//=0 
        // r->devno = current_Point->devno;//device number
        // r->blkno = current_Point->blockNumber;//LPN
        // r->bytecount = (current_Point->size) * 512;  // ssd 4096 //page size 4KB ??
        // page_RW_count->page_num = current_Point->blockNumber/8;
        // page_RW_count->r_count = current_Point->r_count;
        // page_RW_count->w_count = current_Point->w_count;
        // //fprintf(outputfd, "%d|", r->blkno);
        // disksim_interface_request_arrive(disksim, now, r);//把r送進disksim裡

        // // Process events until this I/O is completed 
        // while(next_event >= 0) {
        //   now = next_event;
        //   next_event = -1;
        //   disksim_interface_internal_event(disksim, now, 0);
        // }
  
        // fprintf(fwrite,"%lf\t%ld\t%ld\t%ld\t%ld\n",now,current_Point->devno,current_Point->blockNumber,current_Point->size,current_Point->write_type);
        //fprintf(myoutput,"Block_num:%ld\n",current_Point->blockNumber);
        current_Point->Dirty = 0;//flush後變為clean
        current_Point->Hint_Dirty = 0;
        //printf("flush_1 = %d\n",current_Point->blockNumber);
        dirty_count -= 1.0 ;//????
        flush_Number ++;//flush個數+1,若=1024則break
      
        if(current_Point != queue->front && current_Point != NULL) {//往前還有Node
          current_Point = current_Point->prev;
        }
        else if(current_Point == queue->front && current_Point->Dirty == 0 ) break;//遇到clean page跳過
        else if(current_Point == NULL) break;
      }
      else 
      {
        current_Point=current_Point->prev;
      }
    }
    //fprintf(myoutput,"///////////////////////////////////1024///////////////////////////////////////\n");

    check_R = 1;

    sort_FB(FBpoint, FlushBlock);
    int seqBlock=1, startBlock=FlushBlock[0];
    //assert(0);
    for(i=0;i<FBpoint+1;i++)
    {
      if(FlushBlock[i+1]==FlushBlock[i]+8 && seqBlock < 16)
      {
        seqBlock++;
      }
      else
      {
        struct disksim_request *r = malloc(sizeof(struct disksim_request));
        r->start = now;
        r->flags = 0;
        r->devno = 0;
        r->blkno = startBlock;
        r->bytecount = (seqBlock*8) * 512;  // ssd 4096
        fprintf(fwrite,"%lf\t%ld\t%ld\t%ld\t%ld\n",now,0,startBlock,seqBlock*8,0);
        unsigned long diffget;
        gettimeofday(&start, NULL);
        disksim_interface_request_arrive(disksim, now, r);
        while(next_event >= 0) {
          now = next_event;
          next_event = -1;
          disksim_interface_internal_event(disksim, now, 0);
        }

        gettimeofday(&end, NULL);
        diffget=1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        //fprintf(outputfd,"!!!!!!write disksim_interface_request_arrive = %ld\n",diffget);
        
        //**************************************************************
        seqBlock=1;
        startBlock=FlushBlock[i+1];
      }
    }
    //fprintf(outputfd, "\n");
  }
}
QNode *global_flush_Point = NULL;
QNode *global_flush_MRU_Point = NULL;
QNode *global_flush_LRU_Point = NULL;
QNode *global_hint_TPoint = NULL;
int record_flag = 0 ;
int all_Thint_clear=0;

void period_write_back_hint(Hint_Queue* HintQ, Hash *hash , Queue* queue , FILE *fwrite , struct disksim_interface *disksim) {

  //fprintf(outputfd, "+++++++++++++++++++period_write_back_hint++++++++++++++++++++\n");

  //printf(YELLOW"+++++++++++++++++++period_write_back_hint++++++++++++++++++++++++\n"NONE);

  struct  timeval tv;
  long long int local_time ;
  gettimeofday(&tv,NULL);
  local_time = tv.tv_sec;
  int diff = 0 ;
  int tec;
  float i = 1.0;
  float extra_i = flush_count;
  int extra_flush_direct ;
  int change_direction=0;

  
  //printf("period_write_back\n");
  //printf("dirty_count=%f\n",dirty_count);
  QNode *current_Point2 ;

  //                 MRU            LRU
  //record_flag == 2   |<-----------
  //record_flag == 3    ----------->|
  
  
  

  if(global_hint_TPoint == NULL){
    global_hint_TPoint = queue->rear;//global_flush_Point=最後的page
  }
  current_Point2 = global_hint_TPoint;
 //printf("global_hint_TPoint=%d\n", global_hint_TPoint->blockNumber);
  //fprintf(outputfd, "global_hint_TPoint=%d\n", global_hint_TPoint->blockNumber);
  //???
  if(global_flush_MRU_Point == queue->rear){
    current_Point2 = queue->front;
    global_flush_MRU_Point = NULL;
  }
  if(global_flush_LRU_Point == queue->front){
    current_Point2 = queue->rear;
    global_flush_LRU_Point = NULL;
  }

    clean_flush2 = 1;
    flush2_time = CacheTime;
    extra_flush_direct = flush_direct ;//上次實際定時flush的方向(0/1)
    if(extra_flush_direct == 0 && current_Point2->prev != NULL){//MRU<-LRU && current_Point2前面有page
      current_Point2 = current_Point2->prev;//(global_flush_Point沒有動!!)
    }else if(extra_flush_direct == 1 && current_Point2->next != NULL){//MRU->LRU && current_Point2後面有page
      current_Point2 = current_Point2->next;
    }
   //fprintf(outputfd, "current_Point2=%d\n", current_Point2->blockNumber);
    while(1) {
      diff1 = CacheTime - ( current_Point2->timestamp + old_dirty_time-(Thint_times*5) );//how long page in cache
      if(current_Point2->Dirty == 1 && current_Point2->Hint_Dirty == 1 && diff1 >= 0) {//dirty && time longer then 30
       //printf("\t$$$$$$$dirty && time longer then %d, current_Point2->blockNumber=%d\n", old_dirty_time-(Thint_times*5),current_Point2->blockNumber);
       //fprintf(outputfd, "\t$$$$$$$dirty && time longer then %d, current_Point2->blockNumber=%d\n", old_dirty_time-(Thint_times*5),current_Point2->blockNumber);
        // Allocate memory and assign 'blockNumber'
        global_HQ[global_HQ_size]=current_Point2->blockNumber/8;
        global_HQ_size++;
        int i,h=0;
        for(i=0;i<global_HQ_node_size;i++)
        {
          if((current_Point2->blockNumber/8)/64==global_HQ_node[i])
          {
            h=1;
            break;
          }
        }
        if(h==0)
        {
          global_HQ_node[global_HQ_node_size]=(current_Point2->blockNumber/8)/64;
          global_HQ_node_size++;
          ft_hint_count++;
        }
        /*Hint_page *temp = (Hint_page*)malloc( sizeof( Hint_page ) );//分配空間
        temp->page_num = current_Point2->blockNumber/8;
        // Initialize prev and next as NULL
        temp->prev = NULL;
        temp->next = HintQ->front;//把temp加到queue的最前面
        // If queue is empty, change both front and rear pointers
        if ( isHintQueueEmpty( HintQ ) )
          HintQ->rear = HintQ->front = temp;
        else // Else change the front
        {
          HintQ->front->prev = temp; //µ¥©ó¬O¸ò blnum->next = queue->front; ¹ïÀ³ 
          HintQ->front = temp;       //©Ò¥H¦¹®Éblnum³o­Ó·sªºpage´NÅÜ¦¨MRU 
        }*/
        current_Point2->Hint_Dirty = 0;
        // increment number of full frames
        //HintQ->count++;//queue大小增加
        //-----------------------------------------------------------------------------------------
      // fprintf(fwrite,"%ld\t%d\n",current_Point2->blockNumber,current_Point2->write_type);
        if((current_Point2 == queue->front && change_direction==1) || (current_Point2 == queue->rear && change_direction==1))
        {
          all_Thint_clear=1;
          break;//碰到底之後break//可能hint不到1024個
        }
        if(extra_flush_direct == 0){//MRU<-LRU
          if(current_Point2 != queue->front && current_Point2 != NULL){
            current_Point2 = current_Point2->prev;
          }
          else if(current_Point2 == queue->front)
          {
            extra_flush_direct = 1;
            change_direction ++;
            current_Point2=global_flush_Point;
          }
        }else if (extra_flush_direct == 1){//MRU->LRU
          if(current_Point2 != queue->rear && current_Point2 != NULL){
            current_Point2 = current_Point2->next;
          }
          else if(current_Point2 == queue->rear)
          {
            extra_flush_direct = 0;
            change_direction ++;
            current_Point2=global_flush_Point;
          }
        }
      }else{//clean or 沒有超過30秒
        //printf("\t$$$$$clean or 沒有超過%d秒, current_Point2->blockNumber=%d\n",old_dirty_time-(Thint_times*5), current_Point2->blockNumber);
       //fprintf(outputfd, "\t$$$$$clean or 沒有超過%d秒, current_Point2->blockNumber=%d\n",old_dirty_time-(Thint_times*5), current_Point2->blockNumber);
        // Allocate memory and assign 'blockNumber'
        if((current_Point2 == queue->front && change_direction==1) || (current_Point2 == queue->rear && change_direction==1)) 
        {
          all_Thint_clear=1;
          break;//碰到底之後break//可能hint不到1024個
        }
        if(extra_flush_direct == 0){
          if(current_Point2 != queue->front && current_Point2 != NULL){
            current_Point2 = current_Point2->prev;
          }
          else if(current_Point2 == queue->front)
          {
            extra_flush_direct = 1;
            change_direction ++;
            current_Point2=global_flush_Point;
          }
        } 
        else if (extra_flush_direct == 1){
          if(current_Point2 != queue->rear && current_Point2 != NULL){
            current_Point2 = current_Point2->next;
          }
          else if(current_Point2 == queue->rear)
          {
            extra_flush_direct = 0;
            change_direction ++;
            current_Point2=global_flush_Point;
          }
        }

      }
      if(extra_i == 1.0 || current_Point2 == NULL) break;//找到hint第1024個
      extra_i -= 1.0; //從1024往下減
    }
    global_hint_TPoint=current_Point2;

   //printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
   //fprintf(outputfd, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

 // blockNumberCount(queue);
}
//----------------------flush 1024 location of pages -----------------------
//定時FLUSH
void period_write_back(Hash *hash , Queue* queue , FILE *fwrite , struct disksim_interface *disksim ,int on) {

 //printf(YELLOW"period_write_back|on=%d\n"NONE,on);
 //fprintf(outputfd, "period_write_back|on=%d\n",on);

  struct  timeval tv;
  long long int local_time ;
  gettimeofday(&tv,NULL);
  local_time = tv.tv_sec;
  int diff = 0 ;
  int tec;
  float i = 1.0;
  float extra_i = flush_count;
  int extra_flush_direct ;
  int FBpoint=-1,j;
  int FlushBlock[FLUSHBACK]={0};
  
  //printf("period_write_back\n");
  //printf("dirty_count=%f\n",dirty_count);
  //fprintf(fwrite,"record_flag = %d\n",record_flag);
  QNode *current_Point2 ;
  //fprintf(outputfd, "record_flag = %d\n", record_flag);

  //                 MRU            LRU
  //record_flag == 2   |<-----------
  //record_flag == 3    ----------->|
  if(record_flag == 0){
    current_Point2 = queue->rear;//預設從最後一個page找起
    if(on == 1)record_flag = 1;//第一輪??
  }else if(record_flag == 2){//第一輪刷完碰MRU的牆,從MRU開始找
     current_Point2 = queue->front;//2=指到第一個
     record_flag = 5;
  }else if(record_flag == 3){//第二輪刷完碰LRU的牆,從LRU開始找
     current_Point2 = queue->rear;//3=指到最後一個
     record_flag = 5;
  }else{//1,4,5=指到上次指的(往下一個或往上一個)
     current_Point2 = global_flush_Point;
  }
  if(global_flush_Point == NULL){
    global_flush_Point = queue->rear;//global_flush_Point=最後的page
  }
  //fprintf(outputfd, "global_flush_Point = %d\n", global_flush_Point->blockNumber);
  //???
  if(global_flush_MRU_Point == queue->rear){
    current_Point2 = queue->front;
    global_flush_MRU_Point = NULL;
  }
  if(global_flush_LRU_Point == queue->front){
    current_Point2 = queue->rear;
    global_flush_LRU_Point = NULL;
  }
  if(on == 1){
    //fprintf(outputfd, "r->blkno = ");
    //fprintf(fwrite,"period_write_back global_flush_Point=%d record_flag = %d\n",global_flush_Point->blockNumber,record_flag);
    while(1){
      diff1 = CacheTime - ( (current_Point2)->timestamp + old_dirty_time );//現在時間-(候選page時間+30)>0 表示候選page待超過30秒
      //printf("now=%f\n",now);
      //printf("(current_Point2)->timestamp=%d\n",(current_Point2)->timestamp);
      //printf("diff=%d\n",diff);

      if(current_Point2->Dirty == 1 && diff1 >= 0) {
        flushgj4++;
        //struct disksim_request *r = malloc(sizeof(struct disksim_request));//刷下去
        current_Point2->write_type = 0;//write(0)
        
        FBpoint ++ ;
        FlushBlock[FBpoint] = current_Point2->blockNumber;
        req_RW_count->page[req_RW_count->page_count].page_num = current_Point2->blockNumber/8;
        req_RW_count->page[req_RW_count->page_count].r_count = current_Point2->r_count;
        req_RW_count->page[req_RW_count->page_count].w_count = current_Point2->w_count;
        req_RW_count->page_count++;
        /*r->start = now;
        r->flags = current_Point2->write_type;
        r->devno = current_Point2->devno;
        r->blkno = current_Point2->blockNumber;
        r->bytecount = (current_Point2->size) * 512;  // ssd 4096
        page_RW_count->page_num = current_Point2->blockNumber/8;
        page_RW_count->r_count = current_Point2->r_count;
        page_RW_count->w_count = current_Point2->w_count;
        //fprintf(outputfd, "%d|", r->blkno);
        ///* Process events until this I/O is completed 
        disksim_interface_request_arrive(disksim, now, r);
       //fprintf(outputfd, "disksim_interface_request_arrive now=%lf\n", now);
        while(next_event >= 0) {
          now = next_event;
          next_event = -1;
          disksim_interface_internal_event(disksim, now, 0);
         //fprintf(outputfd, "disksim_interface_internal_event now=%lf\n", now);
        }
        fprintf(fwrite,"%lf\t%ld\t%ld\t%ld\t%ld\n",now,current_Point2->devno,current_Point2->blockNumber,current_Point2->size,current_Point2->write_type);
        */
      //  fprintf(fwrite,"%ld\t%d\n",current_Point2->blockNumber,current_Point2->write_type);
        current_Point2->Dirty = 0;
        current_Point2->Hint_Dirty = 0;
        dirty_count -=1.0;
        //printf("dirty_count=%f\n",dirty_count);
        global_flush_Point = current_Point2;//global_flush_Point=目前"測過"正刷下去的page
        if(i == flush_count || current_Point2 == NULL) {break;}//i=1024||
        if(flush_direct == 0){//MRU<-LRU
          if(current_Point2 != queue->front && current_Point2 != NULL){//目前的page在中間
            current_Point2 = current_Point2->prev;//point往前一個
          }else if (current_Point2 == queue->front){//目前page=第一個
            flush_direct = 1;//MRU<-LRU刷完一輪,改為MRU->LRU
            global_flush_MRU_Point = current_Point2;//MRUpoint=第一個page
            record_flag = 2;//第二輪
          }
        }
        if (flush_direct == 1){//MRU->LRU
          if(current_Point2 != queue->rear && current_Point2 != NULL){//還沒到底
            current_Point2 = current_Point2->next;//point往後一個
          }else if (current_Point2 == queue->rear){//刷到底了
            flush_direct = 0;//MRU->LRU刷完一輪,改為MRU<-LRU
            global_flush_LRU_Point = current_Point2;//LRUpoint=最後一個page
            record_flag = 3;//第三輪
          }
        }
        //fprintf(fwrite,"ch1 flush_direct = %d\n",flush_direct);
      }else{ //clean or 沒有超過30秒
        global_flush_Point = current_Point2;//global_flush_Point=目前"測過"不用刷下去的page
        if(i == flush_count || current_Point2 == NULL) {break;}
        if(flush_direct == 0){//MRU<-LRU
          if(current_Point2 != queue->front && current_Point2 != NULL){
            current_Point2 = current_Point2->prev;
          }else if (current_Point2 == queue->front){
            flush_direct = 1;
            record_flag = 2;
          }
        }
        if (flush_direct == 1){//MRU->LRU
          if(current_Point2 != queue->rear && current_Point2 != NULL){
            current_Point2 = current_Point2->next;
          }else if (current_Point2 == queue->rear){
            flush_direct = 0;
            record_flag = 3;
          }
        }  
        //fprintf(fwrite,"ch2 flush_direct = %d i=%f\n",flush_direct,i);
      }
      i += 1.0;
    }//i=1024,break
    //fprintf(outputfd, "\n" );
    //fprintf(fwrite,"i=%f\n",i);
  
    sort_FB(FBpoint, FlushBlock);
    int seqBlock=1, startBlock=FlushBlock[0];
    //if(FBpoint>5)assert(0);
    for(j=0;j<FBpoint+1;j++)
    {
      if(FlushBlock[j+1]==FlushBlock[j]+8 && seqBlock < 16)
      {
        seqBlock++;
      }
      else
      {
        struct disksim_request *r = malloc(sizeof(struct disksim_request));
        r->start = now;
        r->flags = 0;
        r->devno = 0;
        r->blkno = startBlock;
        //printf("r->blkno = %d\n",r->blkno);
        assert(r->blkno%8 == 0);
        r->bytecount = (seqBlock*8) * 512;  // ssd 4096
        fprintf(fwrite,"%lf\t%ld\t%ld\t%ld\t%ld\n",now,0,startBlock,seqBlock*8,0);
        unsigned long diffget;
        gettimeofday(&start, NULL);
        disksim_interface_request_arrive(disksim, now, r);
        while(next_event >= 0) {
          now = next_event;
          next_event = -1;
          disksim_interface_internal_event(disksim, now, 0);
        }

        gettimeofday(&end, NULL);
        diffget=1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        //fprintf(outputfd,"!!!!!!write disksim_interface_request_arrive = %ld\n",diffget);
        
        //**************************************************************
        seqBlock=1;
        startBlock=FlushBlock[j+1];
      }
    }
  }
 // blockNumberCount(queue);
}
// A utility function to create a new Queue Node. The queue Node
// will store the given 'blockNumber'
QNode *newQNode( unsigned blockNumber ) //新增一個Node(Node 的 blockNumber)
{
  // Allocate memory and assign 'blockNumber'
  QNode *temp = (QNode*)malloc( sizeof( QNode ) );//分配空間

  temp->blockNumber = blockNumber;
  // Initialize prev and next as NULL
  temp->prev = NULL;
  temp->next = NULL;
  return temp;
}

// A utility function to create an empty Queue.
// The queue can have at most 'numberOfFrames' nodes
Queue* createQueue( long long int numberOfFrames )//新增一個Queue(Queue的大小)
{
  Queue* queue = (Queue *)malloc( sizeof( Queue ) );//分配空間

  // The queue is empty
  queue->count = 0;
  queue->front = queue->rear = NULL;

  // Number of frames that can be stored in memory
  queue->numberOfFrames = numberOfFrames;
 
  return queue;
}

// A utility function to create an empty Hash of given capacity
Hash* createHash( long long int capacity )
{
  // Allocate memory for hash
  Hash *hash = (Hash *) malloc( sizeof( Hash ) );
  hash->capacity = capacity;

  // Create an array of pointers for refering queue nodes
  hash->array = (QNode **) malloc( hash->capacity * sizeof( QNode* ) );

  // Initialize all hash entries as empty
  int i;
  for( i = 0; i < hash->capacity; ++i )
    hash->array[i] = NULL;
  return hash;
}

// A function to check if there is slot available in memory
int AreAllFramesFull( Queue* queue )
{
  // fprintf(myoutput,"queue->numberOfFrames:%d\n",queue->numberOfFrames);
  // fprintf(myoutput,"queue->count:%d\n",queue->count);
  return queue->count == queue->numberOfFrames;  
  //return queue->count == Mem; 
}

// A utility function to check if queue is empty
int isQueueEmpty( Queue* queue )
{
  return queue->rear == NULL;
}

void reMark_hint_dirty(Queue* queue) {
  QNode *current_Point = queue->rear;//current_Point初始指到LRU端
  
  //目前LRU page有東西 && extra_no=0
  while(current_Point != NULL) {
    if(current_Point == queue->front)break;
    if(current_Point->Dirty == 1 && current_Point->Hint_Dirty == 0 ) { 
     //fprintf(outputfd, "dirty incoming hint page = %d\n", current_Point->blockNumber );
      current_Point->Hint_Dirty = 1;
    }
    current_Point = current_Point->prev;
  }
}

//Hash 放cache裡的東西(=Queue) 為了搜尋快速 hash->array[block number]=QNode
//replacement(queue放page cache的page,hash,fwrite寫入檔案?,incoming_blockNumber新進來的的page的bn,disksim_interface)
void replacement_hint(Hint_Queue* HintQ, Queue* queue , FILE *fwrite , struct disksim_interface *disksim) {
  QNode *current_Point = queue->rear;//current_Point初始指到LRU端
  
  //目前LRU page有東西 && extra_no=0
  while(current_Point != NULL) {
    if(current_Point->Dirty == 1 && current_Point->Hint_Dirty == 1 ) { 
     //fprintf(outputfd, "dirty incoming hint page = %d\n", current_Point->blockNumber );
      global_HQ[global_HQ_size]=current_Point->blockNumber/8;
      //fprintf(myoutput, "RH dirty incoming hint page = %d\n", current_Point->blockNumber/8 );
      global_HQ_size++;
      int i,h=0;
        for(i=0;i<global_HQ_node_size;i++)
        {
          if((current_Point->blockNumber/8)/64==global_HQ_node[i])
          {
            h=1;
            break;
          }
        }
        if(h==0)
        {
          global_HQ_node[global_HQ_node_size]=(current_Point->blockNumber/8)/64;
          global_HQ_node_size++;
          rp_hint_count++;
        }
      /*Hint_page *temp = (Hint_page*)malloc( sizeof( Hint_page ) );//分配空間
      temp->page_num = current_Point->blockNumber/8;
      // Initialize prev and next as NULL
      temp->prev = NULL;
      temp->next = HintQ->front;//把temp加到queue的最前面
      // If queue is empty, change both front and rear pointers
      if ( isHintQueueEmpty( HintQ ) )
        HintQ->rear = HintQ->front = temp;
      else // Else change the front
      {
        HintQ->front->prev = temp; //µ¥©ó¬O¸ò blnum->next = queue->front; ¹ïÀ³ 
        HintQ->front = temp;       //©Ò¥H¦¹®Éblnum³o­Ó·sªºpage´NÅÜ¦¨MRU 
      }*/
      current_Point->Hint_Dirty = 0;
      // increment number of full frames
      //HintQ->count++;//queue大小增加
      break;
    }
    current_Point = current_Point->prev;
  }
  
}
void Clear_Global_hint_Queue()
{
  int i;
  for(i=0;i<10000;i++)
  {
    global_HQ[i]=-1;
    global_HQ_size=0;
    global_HQ_node[i]=-1;
    global_HQ_node_size=0;
  }
}

double Count_Next_Nflush_time(double pre_Nflush_time, int cleanTOdirty);
double Count_Next_Rep_time(double pre_Rep_time);
//Hash 放cache裡的東西(=Queue) 為了搜尋快速 hash->array[block number]=QNode
//replacement(queue放page cache的page,hash,fwrite寫入檔案?,incoming_blockNumber新進來的的page的bn,disksim_interface)
void replacement(Queue* queue , Hash* hash , FILE *fwrite , unsigned incoming_blockNumber ,struct disksim_interface *disksim )
{
  QNode *current_Point2 = queue->rear;//current_Point2初始指到LRU端
  QNode *current_Point = queue->rear;//current_Point初始指到LRU端
  write_back_type == 0;
  QNode *reqPage = hash->array[ incoming_blockNumber ];//找到array裡bn的QNode存進reqPage
  int extra_no = 0;
  Replace100++;
  Rtime = CacheTime;
  printf("incoming page = %d\n", incoming_blockNumber);
  printf(YELLOW"replacement\nincoming page = %d\n"NONE, incoming_blockNumber);
 //fprintf(outputfd, "replacement\nincoming page = %d\n", incoming_blockNumber );
  //LRU端Page=Dirty && LRU端Page不是新進page && hash->array[ incoming_blockNumber ]=null(目前ssd裡沒有??)
  // fprintf(myoutput, "/////////////////////////////////////////////////////////////////////////////////////\n");
  // fprintf(myoutput, "current_Point2->Dirty= %d\n", current_Point2->Dirty);
  // fprintf(myoutput, "current_Point2->blockNumber= %d,incoming_blockNumber:%d\n", current_Point2->blockNumber,incoming_blockNumber) ;
  // fprintf(myoutput, "reqPage= %d\n", reqPage);
  // fprintf(myoutput, "/////////////////////////////////////////////////////////////////////////////////////\n");
  if(current_Point2->Dirty==1 && current_Point2->blockNumber != incoming_blockNumber)
  {
    vdratio++;
  }
  if(current_Point2->Dirty == 1 && current_Point2->blockNumber != incoming_blockNumber && reqPage == NULL ) 
  //if(current_Point2->Dirty == 1) 
  {
      replacegj4++;
      R_count++;
     //fprintf(outputfd, "dirty incoming page = %d\n", current_Point2->blockNumber );
      printf("dirty incoming page = %d\n", current_Point2->blockNumber );
      struct disksim_request *r = malloc(sizeof(struct disksim_request));
        current_Point2->write_type = 0;//直接寫入LRU端Page,讓LRU端Page變成clean
        r->start = now;
        r->flags = current_Point2->write_type;
        r->devno = current_Point2->devno;
        r->blkno = current_Point2->blockNumber;
        r->bytecount = (current_Point2->size) * 512;  // ssd 4096
        current_Point2->Dirty = 0;
        current_Point2->Hint_Dirty = 0;
        dirty_count -=1.0;
        req_RW_count->page[req_RW_count->page_count].page_num = current_Point2->blockNumber/8;
        req_RW_count->page[req_RW_count->page_count].r_count = current_Point2->r_count;
        req_RW_count->page[req_RW_count->page_count].w_count = current_Point2->w_count;
        req_RW_count->page_count++;
        fprintf(fwrite,"%lf\t%ld\t%ld\t%ld\t%ld\n",now,current_Point2->devno,current_Point2->blockNumber,current_Point2->size,current_Point2->write_type);
        unsigned long diffget;
    gettimeofday(&start, NULL);
        disksim_interface_request_arrive(disksim, now, r);
       //fprintf(outputfd, "disksim_interface_request_arrive now=%lf\n", now);
        while(next_event >= 0) {
          now = next_event;
          next_event = -1;
          disksim_interface_internal_event(disksim, now, 0);
         //fprintf(outputfd, "disksim_interface_internal_event now=%lf\n", now);
        }
        
        gettimeofday(&end, NULL);
        diffget=1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        //fprintf(outputfd,"!!!!!!write disksim_interface_request_arrive = %ld\n",diffget);
//***************************************************************************************************2020
      Hint_Queue* HintQ = createHint_Queue(8192);//16 page
      if(WBH==1)
      {
        
        write_back_hint(HintQ, queue, fwrite, disksim, 0);
        //reMark_hint_dirty(queue);
      }
      reMark_hint_dirty(queue);
//***************************************************************************************************
  } 
  else
  {
    // fprintf(myoutput, "/////////////////////////////////////////////////////////////////////////////////////= %d\n");
    // fprintf(myoutput, "current_Point2->Dirty= %d\n", current_Point2->Dirty);
    // fprintf(myoutput, "current_Point2->Dirty= %d,incoming_blockNumber:%d\n", current_Point2->blockNumber,incoming_blockNumber) ;
    // fprintf(myoutput, "reqPage= %d\n", reqPage);
    // fprintf(myoutput, "/////////////////////////////////////////////////////////////////////////////////////= %d\n");
  }
  clean_replace = 1;
  replace_time = CacheTime; //全域變數,這時發生replacement
     
}
// A utility function to "delete a frame" from queue
void deQueue( Queue* queue ){
 //printf(YELLOW"deQueue\n"NONE);
 //fprintf(outputfd,"deQueue\n");
  QNode *current_Point = queue->rear;//current_Point初始指到LRU端
  if( isQueueEmpty( queue ) )
    return;
  if (queue->front == queue->rear) queue->front = NULL; // If this is the only node in list, then change front
 // printf("real_replacement page = %d\n", current_Point->blockNumber);
  queue->rear = queue->rear->prev;//1個node,queue->rear=null//2個node,queue->rear= queue->rear->prev
  
  
  if (queue->rear)//2個node的情況
    queue->rear->next = NULL;//刪掉第2個node

  // decrement the number of full frames by 1
  queue->count--;//queue大小-1
  free(current_Point);//釋放空間
}

// A function to add a page with given 'blockNumber' to both queue
// and "hash" replacement
//加入最新req,只有更改queue的指標,沒有做寫入
void Enqueue( Queue* queue, Hash* hash, unsigned blockNumber )
{
  //printf("Enqueue\n");
  // If all frames are full, remove the page at the rear
  if ( AreAllFramesFull ( queue ) ){
    hash->array[ queue->rear->blockNumber ] = NULL;// remove page from hash
    deQueue( queue );
  }

  // Create a new node with given page number,
  // And add the new node to the front of queue
  // newQNode¬O²£¥Í¤@­Ó¿W¥ßªº·s¸`ÂI¡A©Ò¥H¤@¶}©l¥LªºÀY§À³£¨SªF¦è
  // §â­ì¥»¦bMRUºÝªºpage²¾°Ê¨ìblnumªº¤U¤@­Ó
  // ¤@¶}©lªì©l¤Æ®É¡Aprev¬ONULL
  QNode *temp = newQNode( blockNumber  );//新node
  temp->next = queue->front;//把temp加到queue的最前面

  // If queue is empty, change both front and rear pointers
  //¦pªG¤@¶}©l¬OªÅªº¡A¥Ø«e­n·s¼W¤@­Ónode¶i¨Ó¡A¨ºquequ¸Ì­±ªºMRU©MLRU³£¬O¦P¤@­Ó 
  if ( isQueueEmpty( queue ) )
    queue->rear = queue->front = temp;
  else // Else change the front
  {
    queue->front->prev = temp; //µ¥©ó¬O¸ò blnum->next = queue->front; ¹ïÀ³ 
    queue->front = temp;       //©Ò¥H¦¹®Éblnum³o­Ó·sªºpage´NÅÜ¦¨MRU 
  }

  // Add page entry to hash also
  hash->array[ blockNumber ] = temp;//在hash的blocknumber位置也加上node
  
  // increment number of full frames
  queue->count++;//queue大小增加

}



// This function is called when a page with given 'blockNumber' is referenced
// from cache (or memory). There are two cases:
// 1. Frame is not there in memory, we bring it in memory and add to the front
// of queue
// 2. Frame is there in memory, we move the frame to front of queue

int  interval = 1,interval_clean2dirty = 0, ipre_clean2dirty = 0;
double avg_clean2dirty_time = 0, interval_now=0, ipre_now=0;
double predict_c2d_avgtime=0.1;
double next_c2d_avgtime=0.1;

int  Rinterval = 1,Rinterval_Rep = 0, Ripre_Rep = 0;
double Ravg_Rep_time = 0, Rinterval_now=0, Ripre_now=0;
double Rpredict_Rep_avgtime=0.1;
double Rnext_Rep_avgtime=0.1;

//                               上次發生flush的時間      目前c2d數量
double Count_Next_Nflush_time(double pre_Nflush_time, int cleanTOdirty)
{
  return pre_Nflush_time + (next_c2d_avgtime * FLUSHBACK);
  //預測下次flush的時間
}
double Count_Next_Rep_time(double pre_Rep_time)
{
  return pre_Rep_time + Rnext_Rep_avgtime;
  //預測下次Rep的時間
}

//1個clean2dirty page 的平均時間(每100秒)
//[0.12, 0.23, 0.25, 0.18, ...]紀錄10個
void Push_in_time_buf()
{
  int i;
  interval_now = CacheTime - ipre_now;
  ipre_now = CacheTime;
  interval_clean2dirty = clean2dirty - ipre_clean2dirty;
  ipre_clean2dirty = clean2dirty;

  avg_clean2dirty_time = interval_now / interval_clean2dirty;

  if(interval == 1)
  {
    next_c2d_avgtime = (alpha*avg_clean2dirty_time)+(1 - alpha)*predict_c2d_avgtime;
  }
  else
  {
    predict_c2d_avgtime = next_c2d_avgtime;
    next_c2d_avgtime = (alpha*avg_clean2dirty_time)+(1 - alpha)*predict_c2d_avgtime;
  }
  //fprintf(Nflush_hintflow, "next_c2d_avgtime=%lf\n",next_c2d_avgtime);
}
void Push_in_Rep_time_buf()
{
  int i;
  Rinterval_now = CacheTime - Ripre_now;
  Ripre_now = CacheTime;
  Rinterval_Rep = Replace100 - Ripre_Rep;//=100
  Ripre_Rep = Replace100;

  Ravg_Rep_time = Rinterval_now / Rinterval_Rep;

  if(Rinterval == 1)
  {
    Rnext_Rep_avgtime = (alpha*Ravg_Rep_time)+(1 - alpha)*Rpredict_Rep_avgtime;
  }
  else
  {
    Rpredict_Rep_avgtime = Rnext_Rep_avgtime;
    Rnext_Rep_avgtime = (alpha*Ravg_Rep_time)+(1 - alpha)*Rpredict_Rep_avgtime;
  }
  //fprintf(Nflush_hintflow, "next_Rep_avgtime=%lf\n",Rnext_Rep_avgtime);
}

void ReferencePage( Queue* queue, Hash* hash, double Req_time, long int Req_devno, long int blockNumber, long int Req_size, long int Req_type, struct disksim_interface *disksim,FILE *fwrite)
{
  struct  timeval    tv;
  long long int local_time;
  gettimeofday(&tv,NULL);
  local_time = tv.tv_sec;
  printf(YELLOW"ReferencePage\n"NONE);

  
 //fprintf(outputfd, "ReferencePage\n");
  int i,j;
  //fprintf(outputfd, "***CacheTime= %f \n", CacheTime);
  if(clean2dirty/100 == interval)
  {
    //fprintf(outputfd, "???CacheTime= %f (Sumcount/100 == interval)\n", CacheTime);
    Push_in_time_buf();
    interval ++;
  }
  if(Replace100/100 == Rinterval)
  {
    //fprintf(outputfd, "???CacheTime= %f (Sumcount/100 == interval)\n", CacheTime);
    Push_in_Rep_time_buf();
    Rinterval ++;
  }
 
  //printf("diff=%d\n",diff);
  //printf("ReferencePage\n");
  QNode *reqPage = hash->array[ blockNumber ]; //有沒有在記憶體裡

  struct disksim_request *r = malloc(sizeof(struct disksim_request));

  int down=0;
  if(Req_type == 1 && reqPage != NULL && Rpre_blk != -1)
  {
    //if(blockNumber >= Rstart_blk && blockNumber <= Rpre_blk && Rpre_blk != -1)
    //{
        //fprintf(fwrite,"if(blockNumber >= Rstart_blk && blockNumber <= Rpre_blk)\n");
        r->start = Rstart_time;
        r->flags = 1;
        r->devno = 0;
        r->blkno = Rstart_blk;
        r->bytecount = Rstart_size * 512;  // ssd 4096
        int i;
        for(i=0;i<Rstart_size/8;i++)
        {
          req_RW_count->page[req_RW_count->page_count].page_num = (Rstart_blk+(i*8))/8;
          req_RW_count->page[req_RW_count->page_count].r_count = 1;
          req_RW_count->page[req_RW_count->page_count].w_count = 0;
          req_RW_count->page_count++;
        }
        //printf("last\n");
        //去讀或寫page的時間
       //fprintf(outputfd, "before disksim_interface_request_arrive now=%lf\n", now);
        unsigned long diffget;
        gettimeofday(&start, NULL);
        disksim_interface_request_arrive(disksim, Rstart_time, r);
       //fprintf(outputfd, "disksim_interface_request_arrive now=%lf\n", now);
       //fprintf(outputfd, "next_event=%lf\n", next_event);
        while(next_event >= 0) {
         //fprintf(outputfd, "--next_event=%lf\n", next_event);
          now = next_event;
          next_event = -1;
         //fprintf(outputfd, "before disksim_interface_internal_event now=%lf\n", now);
          disksim_interface_internal_event(disksim, now, 0);
         //fprintf(outputfd, "disksim_interface_internal_event now=%lf\n", now);
        }

        gettimeofday(&end, NULL);
        diffget=1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        //fprintf(outputfd,"!!!!!!disksim_interface_request_arrive = %ld\n",diffget);
        //fprintf(myoutput5,"!!!!!!IN page cache = %ld\n",diffget);
        tot_Rtime = tot_Rtime + diffget;
        fprintf(fwrite,"%lf\t%d\t%d\t%d\t%d\n",Rstart_time,0,Rstart_blk,Rstart_size,1);
        Rpre_blk = -1;
        Rstart_blk = -1;
        Rstart_time = -1;
        Rstart_size = -1;
    //}
  }

  // the page is not in cache, bring it
  if ( reqPage == NULL ) 
  { //miss
    misscount++;
    unique_page++;
    Enqueue( queue, hash, blockNumber);//更改Queue指標
    if(Req_type==1)//read
    {
      if(Rpre_blk == -1 && Rstart_size ==-1 && Rstart_time == -1 && Rstart_blk == -1)
      {
        //fprintf(fwrite,"Rpre_blk == -1 && Rstart_size ==-1 && Rstart_time == -1 && Rstart_blk == -1\n");
        Rpre_blk = blockNumber;
        Rstart_blk = blockNumber;
        Rstart_time = now;
        Rstart_size = Req_size;
      }
      else if(blockNumber == Rpre_blk+8 && Rstart_size < 128)
      {
        //fprintf(fwrite,"else if(blockNumber == Rpre_blk+8)\n");
        Rpre_blk = blockNumber;
        Rstart_size += Req_size;
      }
      else if(blockNumber != Rpre_blk+8 || Rstart_size >= 128)
      {
        //fprintf(fwrite,"else if(blockNumber != Rpre_blk+8)\n");
        r->start = Rstart_time;
        r->flags = 1;
        r->devno = 0;
        r->blkno = Rstart_blk;
        r->bytecount = Rstart_size * 512;  // ssd 4096
        int i;
        for(i=0;i<Rstart_size/8;i++)
        {
          req_RW_count->page[req_RW_count->page_count].page_num = (Rstart_blk+(i*8))/8;
          req_RW_count->page[req_RW_count->page_count].r_count = 1;
          req_RW_count->page[req_RW_count->page_count].w_count = 0;
          req_RW_count->page_count++;
        }
        //printf("last\n");
        //去讀或寫page的時間
       //fprintf(outputfd, "before disksim_interface_request_arrive now=%lf\n", now);
        unsigned long diffget;
        gettimeofday(&start, NULL);
        disksim_interface_request_arrive(disksim, Rstart_time, r);
       //fprintf(outputfd, "disksim_interface_request_arrive now=%lf\n", now);
       //fprintf(outputfd, "next_event=%lf\n", next_event);
        while(next_event >= 0) {
         //fprintf(outputfd, "--next_event=%lf\n", next_event);
          now = next_event;
          next_event = -1;
         //fprintf(outputfd, "before disksim_interface_internal_event now=%lf\n", now);
          disksim_interface_internal_event(disksim, now, 0);
         //fprintf(outputfd, "disksim_interface_internal_event now=%lf\n", now);
        }
        gettimeofday(&end, NULL);
        diffget=1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        //fprintf(outputfd,"!!!!!!disksim_interface_request_arrive = %ld\n",diffget);
        //fprintf(myoutput5,"!!!!!!Read miss = %ld\n",diffget);
        tot_Rtime = tot_Rtime + diffget;
        fprintf(fwrite,"%lf\t%d\t%d\t%d\t%d\n",Rstart_time,0,Rstart_blk,Rstart_size,1);
        Rpre_blk = blockNumber;
        Rstart_blk = blockNumber;
        Rstart_time = now;
        Rstart_size = Req_size;
      }
     //printf("read req\n");
     //fprintf(outputfd, "read req\n");
      /*r->start = now;
      r->flags = Req_type;
      r->devno = Req_devno;
      r->blkno = blockNumber;
      r->bytecount = Req_size * 512;  // ssd 4096
      page_RW_count->page_num = blockNumber/8;
      page_RW_count->r_count = 1;
      page_RW_count->w_count = 0;
      //printf("last\n");
      //去讀或寫page的時間
     //fprintf(outputfd, "before disksim_interface_request_arrive now=%lf\n", now);
      disksim_interface_request_arrive(disksim, now, r);
     //fprintf(outputfd, "disksim_interface_request_arrive now=%lf\n", now);
     //fprintf(outputfd, "next_event=%lf\n", next_event);
      while(next_event >= 0) {
       //fprintf(outputfd, "--next_event=%lf\n", next_event);
        now = next_event;
        next_event = -1;
       //fprintf(outputfd, "before disksim_interface_internal_event now=%lf\n", now);
        disksim_interface_internal_event(disksim, now, 0);
       //fprintf(outputfd, "disksim_interface_internal_event now=%lf\n", now);
      }
      fprintf(fwrite,"%lf\t%ld\t%ld\t%ld\t%ld\n",now,Req_devno,blockNumber,Req_size,Req_type);*/
    }
    else 
    {
      write_ratio++;
      
      //fprintf(fwrite,"else\n"); 
      if(Rpre_blk != -1 && Rstart_size !=-1 && Rstart_time != -1 && Rstart_blk != -1)
      {
        //fprintf(fwrite,"if(Rpre_blk != -1 && Rstart_size !=-1 && Rstart_time != -1 && Rstart_blk != -1)\n");
        r->start = Rstart_time;
        r->flags = 1;
        r->devno = 0;
        r->blkno = Rstart_blk; 
        r->bytecount = Rstart_size * 512;  // ssd 4096
        int i;
        for(i=0;i<Rstart_size/8;i++)
        {
          req_RW_count->page[req_RW_count->page_count].page_num = (Rstart_blk+(i*8))/8;
          req_RW_count->page[req_RW_count->page_count].r_count = 1;
          req_RW_count->page[req_RW_count->page_count].w_count = 0;
          req_RW_count->page_count++;
        }
        //printf("last\n");
        //去讀或寫page的時間
       //fprintf(outputfd, "before disksim_interface_request_arrive now=%lf\n", now);
        unsigned long diffget;
        gettimeofday(&start, NULL);
        disksim_interface_request_arrive(disksim, Rstart_time, r);
       //fprintf(outputfd, "disksim_interface_request_arrive now=%lf\n", now);
       //fprintf(outputfd, "next_event=%lf\n", next_event);
        while(next_event >= 0) {
         //fprintf(outputfd, "--next_event=%lf\n", next_event);
          now = next_event;
          next_event = -1;
         //fprintf(outputfd, "before disksim_interface_internal_event now=%lf\n", now);
          disksim_interface_internal_event(disksim, now, 0);
         //fprintf(outputfd, "disksim_interface_internal_event now=%lf\n", now);
        }

        gettimeofday(&end, NULL);
        diffget=1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        //fprintf(outputfd,"!!!!!!disksim_interface_request_arrive = %ld\n",diffget);
        //fprintf(myoutput5,"!!!!!!Write miss = %ld\n",diffget);
        tot_Rtime = tot_Rtime + diffget;
        fprintf(fwrite,"%lf\t%d\t%d\t%d\t%d\n",Rstart_time,0,Rstart_blk,Rstart_size,1);
        Rpre_blk = -1;
        Rstart_blk = -1;
        Rstart_time = -1;
        Rstart_size = -1;
      }
    }

    for(i=0;i<FLUSHBACK;i++)
    {
      req_RW_count->page[i].page_num=0;
      req_RW_count->page[i].r_count=0;
      req_RW_count->page[i].w_count=0;
    }
    req_RW_count->page_count=0;
    page_RW_count->page_num=0;
    page_RW_count->r_count=0;
    page_RW_count->w_count=0;

    //printf("last\n");
    // if(reqPage == NULL && Req_type==0)
    // {
    //   reqPage = hash->array[ blockNumber ]; 
    //   reqPage->Req_type = Req_type;
    //   reqPage->Dirty = 1;
    // }
    reqPage = hash->array[ blockNumber ]; //記錄進hash
    reqPage->Req_type = Req_type;
    if(Req_type==1){reqPage->r_count=1;reqPage->w_count=0;}
    else if(Req_type==0){reqPage->w_count=1;reqPage->r_count=0;}
    reqPage->size = Req_size;
    reqPage->devno = Req_devno;
    reqPage->timestamp = CacheTime;
    reqPage->block_count = (blockNumber/ 64 ) ;//didn't used
    if (reqPage->Req_type == 1 ){ //read
      reqPage->Dirty = 0;
      reqPage->Hint_Dirty = 0;
    }else{ //write
      clean2dirty++;
      reqPage->Dirty = 1;
      reqPage->Hint_Dirty = 1;
      dirty_count += 1.0;//40%的那個
    }
  } 
  else if (reqPage == queue->front)
  { // hit!!,page is there and at front, change pointer
    hit_ratio++;
    if(Req_type == 0)
    {   //新進來的是write
      write_ratio++;
      if(reqPage->Req_type == 1 )
      {  //原本的是read
        clean2dirty++;
        reqPage->Req_type = Req_type;  //更新type
        reqPage->Dirty = 1; //設為dirty
        reqPage->Hint_Dirty = 1;
        dirty_count += 1.0; //40%的+1
      }
      else
      { //原本的是write
        reqPage->Req_type = Req_type; 
        if(reqPage->Dirty != 1)
        { //原本是clean(because flush),因為新的是write所以改成dirty
          clean2dirty++;
          reqPage->Dirty = 1; 
          reqPage->Hint_Dirty = 1;
          dirty_count += 1.0; 
        }
      }
      reqPage->w_count++;       
    }
    else
    {
      reqPage->r_count++;
    } 
    Hitcount++;// hit!!
    Hitcount100++;
  }
  else if (reqPage != queue->front)
  { // hit!!page is there and not at front, change pointer
    //做跟上面一樣的事
    hit_ratio++;
    if(Req_type == 0){  //if type of new request is 0(write) , update type of the new reqest , //and dirty always is true;
      write_ratio++;
      if(reqPage->Req_type == 1 ){  
        clean2dirty++;
        reqPage->Req_type = Req_type;  
        reqPage->Dirty = 1;
        reqPage->Hint_Dirty = 1;
        dirty_count += 1.0;  // 0->1 ,so dirty count ++
      }else{
        reqPage->Req_type = Req_type;  
        if(reqPage->Dirty != 1){
          clean2dirty++;
          reqPage->Dirty = 1;
          reqPage->Hint_Dirty = 1;
          dirty_count += 1.0;
        }
      }       
      reqPage->w_count++;       
    }
    else
    {
      reqPage->r_count++;
    } 
    Hitcount++;
    Hitcount100++;
    // Unlink rquested page from its current location
    // in queue.
    //比上面多出來的,要改Queue的point,拔出這個Node
    reqPage->prev->next = reqPage->next;
    if (reqPage->next)
      reqPage->next->prev = reqPage->prev;

    // If the requested page is rear, then change rear
    // as this node will be moved to front
    //如果是最LRU端的話還要再改Queue的rear指標
    if (reqPage == queue->rear){
      queue->rear = reqPage->prev;
      queue->rear->next = NULL;
    }
 
    // Put the requested page before current front
    //把拔出來的Node插到最前面
    reqPage->next = queue->front;
    reqPage->prev = NULL;

    // Change prev of current front
    reqPage->next->prev = reqPage;

    // Change front to the requested page
    queue->front = reqPage;
  }
  //------------------------------------------------------------------------2020
    // Hint_Queue* HintQ = createHint_Queue(8192);//16 page
    // // if(RH==1)
    // // {
    // //   replacement_hint(HintQ, queue, fwrite, disksim);
    // // }
    // if(WBH==1)
    // {
    //    write_back_hint(HintQ, queue, fwrite, disksim, 0);
    // }
    // reMark_hint_dirty(queue);
  //------------------------------------------------------------------------2020
  //計算什麼時候要下hint (39%dirty or timer=5sec)
  //if(HINTGROUP>0)
  //{
    if(Req_type == 0 )
    {//write 更改待在 queue裡的時間
      double next_Nflush_time, next_Tflush_time, next_Rep_time, Nflush_time_diff, next_R_time, AvgReplaceTime;
      Hint_page *p = (Hint_page*)malloc( sizeof( Hint_page ) );//分配空間
      
      if(diff == 0) Ref_diff = CacheTime + 4.99 ; //全域diff=現在時間-(候選page待在memory時間+30),預設diff=0
      else Ref_diff = diff -1;
      Ref_temp = 1 ;
      Ref_dr = CacheTime - Ref_diff;
     //fprintf(outputfd, "diff=%lf, now=%lf, Ref_diff=%lf, Ref_dr=%lf=%lf-%lf\n", diff, now, Ref_diff, Ref_dr, now, Ref_diff);
      // if(Sumcount100 == 100)
      // {  
      //   double missavg100;
      //   if(Hitcount100!=0)
      //     missavg100 = Sumcount100/Hitcount100;
      //   else
      //     missavg100 = Sumcount100;

      //   AvgReplaceTime = missavg100*(sum_req_time/(double)ReqCount);
      //   next_R_time = ((CacheTime/AvgReplaceTime)+1)*AvgReplaceTime;
      //   Hitcount100 = 0;
      //   Sumcount100 = 0;
      // }
      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      
      if( Ref_dr >=0 )
      {
        printf("Tflush hint\n");
        Hint_Queue* HintQ = createHint_Queue(8192);//16 page
       //fprintf(outputfd, "#######################!!!@@@定時flush hint@@@!!!#############################\n");
       //fprintf(outputfd, "Ref_dr=%lf >=0\n", Ref_dr);
        predict_Ttime=CacheTime;
        for(i=0;i<10000;i++)
        {
          global_HQ[i]=-1;
          global_HQ_size=0;
          global_HQ_node[i]=-1;
          global_HQ_node_size=0;
        }
        int hint_g=0;
        //---------------global_Hint_Queue free-------------------
        //----------------------------------------------------------
        global_hint_TPoint=global_flush_Point;
        Thint_times=0;
        //************************************************************
        all_Thint_clear=0;
        //printf("before period_write_back_hint( 1\n");
        period_write_back_hint(HintQ, hash, queue ,fwrite,disksim);//定時flush hint
        
            if(RH==1)
            {
              replacement_hint(HintQ, queue, fwrite, disksim);
            }
         reMark_hint_dirty(queue);
      }
      //fprintf(outputfd, "dirty_count/Mem = %f\n", (dirty_count/Mem));
      //if((dirty_count/Mem) >= (dirty_ratio - 0.0001)) { //定量flush hint
      if(WBH==1){
        printf("Nflush hint\n");
        Hint_Queue* HintQ = createHint_Queue(8192);//16 page
        int hint_g=0;
        Clear_Global_hint_Queue();
       //fprintf(outputfd, "###################!!!@@@定量flush hint@@@!!!#######################\n");
       //printf("dirty_count/Mem=%lf\n", dirty_count/Mem);
       //fprintf(outputfd, "dirty_count/Mem=%lf\n", dirty_count/Mem);
        //---------------------------------------------------
        predict_Ntime = CacheTime; 

        global_write_back_Point = queue->rear;
        //fprintf(outputfd, "in Nhint's write_back\n");
        write_back_hint(HintQ, queue, fwrite, disksim, 0);
        hint_g++;
        reMark_hint_dirty(queue);
      }
    }
  //}
//  fprintf(outputfd, "clean2dirty=%d\n",clean2dirty);

  //printf("diff=%d\n",diff);
}




void
panic(const char *s)
{
  perror(s);
  exit(1);
}


void
add_statistics_page(Stat *s, int p, double x)
{
  s->n+=p;
  s->sum += x;
  s->sqr += x*x;
}

void
add_statistics(Stat *s, double x)
{
  s->n++;
  s->sum += x;
  s->sqr += x*x;
}


void
print_statistics(Stat *s, Stat *ws, Stat *rs,Stat *wsp, Stat *rsp, const char *title)
{
  double avg, std;
  double wavg, wstd;
  double ravg, rstd;
  double wpavg, wpstd;
  double rpavg, rpstd;

  avg = s->sum/s->n;
  std = sqrt((s->sqr - 2*avg*s->sum + s->n*avg*avg) / s->n);

  wavg = ws->sum/ws->n;
  wstd = sqrt((ws->sqr - 2*wavg*ws->sum + ws->n*wavg*wavg) / ws->n);

  wpavg = wsp->sum/wsp->n;
  wpstd = sqrt((wsp->sqr - 2*wpavg*wsp->sum + wsp->n*wpavg*wpavg) / wsp->n);

  ravg = rs->sum/rs->n;
  rstd = sqrt((rs->sqr - 2*ravg*rs->sum + rs->n*ravg*ravg) / rs->n);

  rpavg = rsp->sum/rsp->n;
  rpstd = sqrt((rsp->sqr - 2*rpavg*rsp->sum + rsp->n*rpavg*rpavg) / rsp->n);

  printf("\nall req avg %s: n=%d average=%f std. deviation=%f\n", title, s->n, avg, std);
  printf("write req avg %s: n=%d average=%f std. deviation=%f\n", title, ws->n, wavg, wstd);
  printf("write_page avg %s: n=%d average=%f std. deviation=%f\n", title, wsp->n, wpavg, wpstd);
  printf("read req avg%s: n=%d average=%f std. deviation=%f\n", title, rs->n, ravg, rstd);
  printf("read_page avg %s: n=%d average=%f std. deviation=%f\n", title, rsp->n, rpavg, rpstd);
  
  fprintf(finaloutput,"\nall req avg %s: n=%d average=%f std. deviation=%f\n", title, s->n, avg, std);
  fprintf(finaloutput,"write req avg %s: n=%d average=%f std. deviation=%f\n", title, ws->n, wavg, wstd);
  fprintf(finaloutput,"write_page avg %s: n=%d average=%f std. deviation=%f\n", title, wsp->n, wpavg, wpstd);
  fprintf(finaloutput,"read req avg%s: n=%d average=%f std. deviation=%f\n", title, rs->n, ravg, rstd);
  fprintf(finaloutput,"read_page avg %s: n=%d average=%f std. deviation=%f\n", title, rsp->n, rpavg, rpstd);
}

/*
 * Schedule next callback at time t.
 * Note that there is only *one* outstanding callback at any given time.
 * The callback is for the earliest event.
 */
void
syssim_schedule_callback(disksim_interface_callback_t fn, 
       SysTime t, 
       void *ctx)
{
  next_event = t;
}
  
    
/*
 * de-scehdule a callback.
 */
void
syssim_deschedule_callback(double t, void *ctx)
{
  next_event = -1;
}
 

void
syssim_report_completion(SysTime t, struct disksim_request *r, void *ctx)
{
  completed = 1;
  now = t;
  alln++;
  if(alln > (int)((double)MAXREQ*0.6))
  {
    //add_statistics_page(&st, r->bytecount/4096, t - r->start);
    add_statistics(&st, t - r->start);
    if(r->flags == 0)
    {
      add_statistics_page(&wstp, r->bytecount/4096, t - r->start);
      add_statistics(&wst, t - r->start);
    }
    else if(r->flags == 1)
    {
      add_statistics_page(&rstp, r->bytecount/4096, t - r->start);
      add_statistics(&rst, t - r->start);
    }
  }
}

void predict_Arg()
{
  double myalpha=0.8,IR=100,IWr=0.5,Ihr=0.5,IVdr=0.5;
  if(firstopen == 0)
  {
    NR=IR;
    Nwr=IWr;
    Nhr=Ihr;
    NVdr=IVdr;
    // Rcount2 =20;
    // write_ratio2 =0.5;
    // hit_ratio2 =0.5; 
    // vdratio2 =0; 
  } 
  // Rcount2=Rcount;
  // write_ratio2=write_ratio/Rcount2;
  // hit_ratio2=hit_ratio/Rcount2;
  // vdratio2=vdratio/Rcount2;
  //fprintf(myoutput,"PR:%lf,PWr:%lf,PHr:%lf,Pvdr:%lf\n",Rcount2,write_ratio2/Rcount2,hit_ratio2/Rcount2,vdratio2/Rcount2);
  NR = (myalpha*Rcount2)+(1-myalpha)*NR;
  Nwr = (myalpha*write_ratio2)+(1-myalpha)*Nwr;
  Nhr = (myalpha*hit_ratio2)+(1-myalpha)*Nhr;
  NVdr =(myalpha*vdratio2)+(1-myalpha)*NVdr;
  //fprintf(myoutput2,"PR:%lf,PWr:%lf,PHr:%lf,Pvdr:%lf\n",NR,Nwr,Nhr,NVdr);
  calculate_predict();
  Rcount2=Rcount;
  write_ratio2=write_ratio/Rcount2;
  hit_ratio2=hit_ratio/Rcount2;
  vdratio2=vdratio/Rcount2;
  
}
void calculate_predict()
{
  float condition4=0,condition5=0,condition7=0,condition8=0,total_conF=0,total_conR;
  float NowDr=0;
  condition4 = Rcount2 * (1-Nwr) * (1-Nhr) * NVdr;
  condition5 = Rcount2 * Nwr * Nhr * (dirty_count/Mem);
  condition7 = Rcount2 * Nwr * (1-Nhr) * (1-NVdr);
  condition8 = Rcount2 * Nwr * (1-Nhr) * NVdr;
  total_conF = condition5 + condition7 - condition5;
  total_conF = ceil(total_conF);

  total_conR = condition4 + condition8;
  total_conR = ceil(total_conR);
  //fprintf(myoutput,"total_conR:%f\n",total_conR);
  NowDr = (dirty_count+total_conF)/Mem;
  //fprintf(myoutput3,"total_conF:%lf\n",total_conF); 
  //fprintf(myoutput3,"NowDr:%lf\n",dirty_count+total_conF); 
  if( NowDr >= dirty_ratio)
  {
    WBH=1;
    pdFA_count++;
  }
  else
  {
    WBH=0;
  }
  if(total_conR != 0)
  {
    RH=1;
    pdR_count = pdR_count + total_conR;
  }
  else
  {
    RH=0;
  }
  //fprintf(myoutput,"total_conF:%f\n",total_conF);
  
} 
int main(int argc, char *argv[]) 
{
  unsigned long diffall;
  gettimeofday(&start1, NULL); 
  int i;
  int nsectors = 1000;
  struct stat buf;
  struct disksim_request r;
  struct disksim_interface *disksim;
   
  struct  timeval    tv;
  long long int Global_time ;
  int temp = 0;


 
  int devno;
  int blnum=0,R_W=0,size;
  int temp_2 = 0 ;
    
  float evict_time;
  long int evict_devno , evict_blnum , evict_size , evict_flags;
 
  struct  timeval  time_period_s,time_period_e;
  unsigned long count_time;
 
  double time=0,mytime=0,my_prev_time=0,total_avg_time=0;
  double avg_time[10]={0.0};
  int avg_count=0,x=0;
  float req_temp_count=0,req_last_count=0;
  int reqamount=0,myreqcount=0;
  int test_RQ=0;
  
  double prstime=0;
     
  //printf("tv_sec:%ld\n",Global_time);
  printf("<<<ssd_ARR=%d>>\n",ssd_ARR);
  printf("tv_sec:%ld\n",Global_time);
  FILE *fwrite = fopen("flush1.txt","w");
  //FILE *fread = fopen("/home/osnet/num_ssd-postmark-aligned2.trace","r");
  Nflush_hintflow = fopen("src/Nflush_hintflow1.txt","w");
  Nflush_flow = fopen("src/Nflush_flow1.txt","w");
  outputfd = fopen("src/syssim_output1","w");
  outputssd = fopen("src/ssd_output1","w");
  myoutput = fopen("src/syssim_2020-flush.txt","w");
  myoutput2 = fopen("src/syssim_2020-TWO.txt","w");
  myoutput3 = fopen("src/syssim_2020-Three-hq.txt","w");
  myoutput4 = fopen("src/syssim_2020-four-wbh.txt","w");
  myoutput5 = fopen("src/syssim_rtime","w");
  finaloutput = fopen("final.txt","w");
  gc_info = fopen("gc_information.txt","w");
  gc_info2 = fopen("gc_information2.txt","w");
  //FILE *evict_fread = fopen("/home/osnet/disksim_blas_Yifen/src/BPLRU/iozone/2048","r");
  
    
///////////原本就有↓////////////////
  if (argc != 7 ) 
  {
    fprintf(stderr, "usage: %s <param file> <output file> <#sectors> <input trace file> <max_req> <memory_size(#pages)>\n",argv[0]);
    exit(1);
  }
  FILE *fread = fopen(argv[4],"r");
  MAXREQ = atoi(argv[5]);
  Mem = atoi(argv[6]);
  //reqamount = atoi(argv[7]);
  
  Queue *q = createQueue( Mem ); // Let cache can hold x pages (Mem全域)
  Hash *hash = createHash( 100000000 );// Let 10 different pages can be requested (pages to be referenced are numbered from 0 to 9 )

  if (stat(argv[1], &buf) < 0)
    panic(argv[1]);

  disksim = disksim_interface_initialize(argv[1], 
           argv[2],
           syssim_report_completion,
           syssim_schedule_callback,
           syssim_deschedule_callback,
           0,
           0,
           0);
/////////原本就有↑////////////////
    
  if (fread == NULL)
    perror ("Error opening file");
  else 
  {
    double perv_time = 0;
    kick_req* rw = (kick_req *)malloc( sizeof( kick_req ) );//分配空間
      req_RW_count = rw;
      RW_count* prw = (RW_count *)malloc( sizeof( RW_count ) );//分配空間
      page_RW_count = prw;
    
    while(fscanf(fread,"%lf%ld%ld%ld%ld",&time,&devno,&blnum,&size,&R_W)!= EOF) //接收新的request
    //while(myreqcount<reqamount)
    {
      //myreqcount++;
      //fscanf(fread,"%lf%ld%ld%ld%ld",&time,&devno,&blnum,&size,&R_W);
      //fprintf(myoutput,"time:%lf,devno:%ld,blnum:%ld,size:%ld,R_W:%ld\n",time,devno,blnum,size,R_W);
      ReqCount++;
      test_RQ++;
      printf("----接收新的request----|ReqCount=%d\n", ReqCount);
      FILE *wb=fopen("wb.txt","w");      
      fprintf(wb,"request:%d\n",ReqCount);
      fclose(wb);
      if(blnum > 56000000 && R_W == 0)
        continue;
      //fprintf(outputfd, "----接收新的request----|ReqCount=%d\n", ReqCount);
      // printf("%f\n",dirty_count );
      //printf("dirty_count=%f\n",dirty_count );
      CacheTime=time;
      //fprintf(fwrite, "^now=%lf\n", now);
      sum_req_time =  sum_req_time + (time - perv_time);
      
   
      mytime = time - my_prev_time;
      //fprintf(myoutput,"time:%lf,mytime:%lf,my_prev_time:%lf\n",time,mytime,my_prev_time);
      sum_time = sum_time + mytime;
      //fprintf(myoutput,"sum_time%lf,Reqcount:%d\n",sum_time,test_RQ);
      //if(sum_time >= 0.01 || test_RQ >=200)
      //if(sum_time>=10)
      //if(sum_time >= 0.001 )
      avg_time[avg_count]=mytime;
      avg_count++;
      if(avg_count==10)
      {
          
        for(x=0;x<avg_count;x++)
        {
          total_avg_time = total_avg_time + avg_time[x];
          avg_time[x]=0;
        }
        total_avg_time = total_avg_time/10;
        avg_count=0;
        //memset(avg_time,'/0',sizeof(avg_time))
      } 
      //fprintf(myoutput,"total_avg_time:%lf\n",total_avg_time);
      //if(sum_time>=total_avg_time)
      if(test_RQ==500)
      {
         
        req_temp_count=ReqCount-req_last_count;
        //Rcount=req_temp_count;
        Rcount = test_RQ;
        //fprintf(myoutput,"!!!!!!TIME:%lf,ReqCount:%d\n",sum_time,test_RQ);
        predict_Arg();
        
        //fprintf(myoutput,"Wr:%f,Hr:%f,vdr:%f\n",write_ratio/req_temp_count,hit_ratio/req_temp_count,vdratio/req_temp_count);
        write_ratio=0;
        hit_ratio=0;
        vdratio=0;
        
        test_RQ=0;
        sum_time = 0;
        //req_last_count = ReqCount;
        //req_temp_count= 0;
        Rcount=0;
        firstopen=1;
        //fprintf(myoutput,"!!!!!!req_count:%ld\n",req_temp_count);
        //req_temp_count = 0;
      }
      else
      {
        //fprintf(myoutput,"else!!!!!!TIME:%lf,ReqCount:%d\n",sum_time,test_RQ);
      }
      
      int count=size, size_b=8;
         
      for(i=0;i<FLUSHBACK;i++)
      {     
        req_RW_count->page[i].page_num=0;
        req_RW_count->page[i].r_count=0;
        req_RW_count->page[i].w_count=0;
      }  
      req_RW_count->page_count=0;
      page_RW_count->page_num=0;
      page_RW_count->r_count=0;
      page_RW_count->w_count=0;

      for(i=0;i<10000;i++)
      {
        global_HQ[i]=-1;
        global_HQ_size=0;
        global_HQ_node[i]=-1;
        global_HQ_node_size=0;
      }
      //printf("before while(count>0)\n");
      gettimeofday(&time_period_s, NULL);
      while(count>0)
      {
        //fprintf(outputfd, "^^^^^^^^^^^^^^^^^^^^^^^^next page^^^^^^^^^^^^^^^^^^^\n");
        Sumcount++; 
        Sumcount100++;
        test_count2 ++ ;
        if(temp == 0){ //第一次讀req
          // printf("----(temp == 0)接收新的request----\n");
          //printf("temp==0\n");
          diff = CacheTime + per_time ; //diff=now+5
          //printf("now=%f,diff=%d\n",now,diff);
          temp = 1 ;
        }
        //printf("before if ( AreAllFramesFull ( q ) ){ \n");
        if ( AreAllFramesFull ( q ) ) //memory是滿的(Queue是滿的)
        { 
            //fprintf(myoutput,"Replacement\n");
            replacement(q , hash, fwrite , blnum , disksim); //新的req進來要做replace
            //replacegj4++;
        }
        //fprintf(fwrite, "^^now=%lf\n", now);
        ReferencePage( q, hash, time, devno, blnum, size_b, R_W, disksim,fwrite); 
        //fprintf(fwrite, "^^^now=%lf\n", now);//把新的req放到MRU
        printf("ReferencePage\n");
        //printf("----把新的req放到MRU----\n");
        QNode *block_count_Point = q->rear; //取LRU的Node,從最後往前算block_count
        int i = 0;
        for(i = 0 ; i < 1000000 ; i++) sum_block_count[i] = 0; //整個ssd有1000000個block,sum_block_count[]歸零
        while(1) 
        {
          if(block_count_Point->Dirty == 1 ) 
          { //選定的Node是dirty
            int Blk = ((block_count_Point->blockNumber/8) / 64 ) ;//為什麼要除以8!!!!!
            sum_block_count[Blk] ++;
          }
          if(block_count_Point == q->front || block_count_Point == NULL) break;
          if(block_count_Point->prev != NULL) block_count_Point = block_count_Point->prev; //往前一個繼續算
          else break;
        } 
        //fprintf(outputfd, "dirty_count/Mem=%lf\n", dirty_count/Mem);
        if((dirty_count/Mem) >= dirty_ratio)
        { //定量flush, flush_1 number of dirty more than dirty ratio , exe flush 
          //fprintf(outputfd,"////////////////////實際執行定量flush//////////////////////\n");
          printf("////////////////////實際執行定量flush//////////////////////\n");
          // if(check_R==1)
          // {
          //   vdratio++;
          //   replacegj4++;
          //   check_R=0;
          // }
          FA_count++;
          Ntime=CacheTime;
          //fprintf(Nflush_flow, "%lf,%lf\n", now, Ntime);
          write_back(q ,fwrite,disksim,1); //實際執行定量flush
          //重算sum_block_count↓///
          QNode *block_count_Point = q->rear;
          int i = 0;
          for(i = 0 ; i < 1000000 ; i++) sum_block_count[i] = 0;
          while(1) 
          {
            if(block_count_Point->Dirty == 1 ) 
            {
              int Blk = ((block_count_Point->blockNumber/8) / 64 ) ;
              sum_block_count[Blk] ++;
            }
            if(block_count_Point == q->front || block_count_Point == NULL) break;
            if(block_count_Point->prev != NULL) block_count_Point = block_count_Point->prev; 
            else break;
          }
            
        }
        //fprintf(fwrite, "^^^^now=%lf\n", now);//把新的req放到MRU    
        dr = CacheTime - diff;//算時間的QQ 
        //fprintf(fwrite, "dr = %lf now =%lf - diff=%lf;\n", dr,now,diff);
        //printf("diff=%f\n",diff);
        //diff maybe (last exe timer flush) + 5 
        //ex.now=25,last flush=20,diff=20+5=25,now-diff>=0,next timer flish
        //printf("test_count2 = %d\n", test_count2 );
        //printf("test_count2 = %d\n", test_count2 );
        //printf("dr = %d \n", dr );
        for(i=0;i<FLUSHBACK;i++)
        {
          req_RW_count->page[i].page_num=0;
          req_RW_count->page[i].r_count=0;
          req_RW_count->page[i].w_count=0;
        }
        req_RW_count->page_count=0;
        page_RW_count->page_num=0;
        page_RW_count->r_count=0;
        page_RW_count->w_count=0;
        if( dr >= 0) { // per 5 second to wake up 
          temp = 0; //當再進一個新的req時重新計算時間
          //printf("dirty_count=%f\n",dirty_count);
          //sleep(10);
          printf("----實際執行定時flush----\n");
          //fprintf(outputfd,"////////////////////實際執行定時flush//////////////////////\n");
            
          Ttime=CacheTime;
          period_write_back(hash, q,fwrite,disksim,1); //實際執行定時flush
          //sleep(10);
          //重算sum_block_count↓///
          QNode *block_count_Point = q->rear;
          int i = 0;
          for(i = 0 ; i < 1000000 ; i++) sum_block_count[i] = 0;
          while(1) 
          {
            if(block_count_Point->Dirty == 1 ) 
            {
              int Blk = ((block_count_Point->blockNumber/8) / 64 ) ;
              sum_block_count[Blk] ++;
            } 
            if(block_count_Point == q->front || block_count_Point == NULL) break;
            if(block_count_Point->prev != NULL) block_count_Point = block_count_Point->prev; 
            else break;
          }
        }   
        //printf("Queue:");  
        /*fprintf(outputfd, "Queue:");
        QNode *p = q->front;
        while(1)
        {
          //printf("[%d|d=%d]",p->blockNumber,p->Dirty); 
          fprintf(outputfd, "[%d|d=%d]",p->blockNumber,p->Dirty);
          if(p->next==NULL)break;
          else p=p->next;
        } 
        //printf("\n");
        fprintf(outputfd, "\n");*/
        // int test=0;
        // for(test=0;test<global_HQ_size;test++)
        // {
        //   fprintf(myoutput, "Syssim HQ  = %d\n", global_HQ[test] );
        // }
        count=count-8;
        blnum=blnum+8;
      }
      // req_temp_count++;
      // gettimeofday(&time_period_e, NULL);     
      // count_time=1000000 * (time_period_e.tv_sec-time_period_s.tv_sec)+ time_period_e.tv_usec-time_period_s.tv_usec;
      // sum_time = sum_time + count_time;
      // if(sum_time>1000000)
      // {
      //   sum_time = 0;
      //   //fprintf(myoutput,"!!!!!!req_count:%ld\n",req_temp_count);
      //   req_temp_count = 0;
      // }
      //fprintf(myoutput,"!!!!!!ALL = %ld\n",count_time);
      my_prev_time = time;
      //mytime = 0;
       
    }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////scanf

  }
  // Let us print cache frames after the above referenced pages
/*
 while(fscanf(evict_fread,"%f%ld%ld%ld%ld",&evict_time,&evict_devno,&evict_blnum,&evict_size,&evict_flags)!= EOF){
  //kk++;
  //printf("kk = %d \n", kk);
  struct disksim_request *r = malloc(sizeof(struct disksim_request));
  // the page is not in cache, bring it
    r->start = evict_time;
    r->flags = evict_flags;
    r->devno = evict_devno;
    r->blkno = evict_blnum;
    r->bytecount = evict_size;  
    disksim_interface_request_arrive(disksim, evict_time, r);
    while(next_event >= 0) {
      now = next_event;
      next_event = -1;
      disksim_interface_internal_event(disksim, now, 0);
    }
  }*/
  int j=0;
  
  /*while(j<=1000){
    QNode *cur = hash->array[j];
    if(cur!= NULL)
     printf("blkno = %d , invalid = %d\n",cur->blockNumber ,cur->invalid);
     cur = cur->next;
    j++;
  }*/ 
 /* while(print_cur != NULL){
        printf ("%d\n", print_cur->F_blockNumber);
        print_cur = print_cur->F_next;
      }*/
 // printf ("invalid =%d\n", cur->invalid);
  //float acratio_FA=0,acratio_R=0; 
  gettimeofday(&end1, NULL);
  diffall=1000000 * (end1.tv_sec-start1.tv_sec)+ end1.tv_usec-start1.tv_usec;
  prstime = tot_Rtime/Sumcount;
  fprintf(outputfd,"!!!!!!ALL = %ld\n",diffall);
  printf("Unique page=%d\n",unique_page);
  printf("<<<ssd_ARR=%d>>>\n",ssd_ARR);
  printf("flush個數=%d, replacement個數=%d\n", flushgj4, replacegj4);
  printf("Sumcount = %.0lf,Hitcount = %.0lf Hit = %.2f\n,miss_ratio=%lf\n",Sumcount,Hitcount,(Hitcount/Sumcount)*100.0,(misscount/Sumcount)*100.0);
  printf("\n****參數顯示****\n");
  printf(LIGHT_GREEN"[CHEN] HINTGROUP=%d, alpha=%f\n"NONE, HINTGROUP, alpha);
  printf(LIGHT_GREEN"[CHEN] Trace File=%s, max_req=%d, MEM Size=%d\n"NONE, argv[4], atoi(argv[5]), atoi(argv[6]));
  //acratio_FA =  (abs(FA_count-pdFA_count)/pdFA_count)*100.0;
  //acratio_R  =  (abs(R_count-pdR_count)/pdR_count)*100.0;
  printf(LIGHT_GREEN"[CHEN] Pd_FA_count=%d,Pd_R_count=%d\n",pdFA_count,pdR_count);
  printf(LIGHT_GREEN"[CHEN] FA_count=%d,R_count=%d\n",FA_count,R_count);
  printf(LIGHT_GREEN"[CHEN] tot_Rtime=%ld,prstime=%lf\n",tot_Rtime,prstime);
  printf(LIGHT_GREEN"[CHEN] fa_hint_count=%d\n",fa_hint_count);
  printf(LIGHT_GREEN"[CHEN] ft_hint_count=%d\n",ft_hint_count);
  printf(LIGHT_GREEN"[CHEN] rp_hint_count=%d\n",rp_hint_count);
  printf(LIGHT_GREEN"[CHEN] total_hint_count=%d\n",fa_hint_count+ft_hint_count+rp_hint_count);
//-----------------------------------------------------------------------------------------------------------------------
  fprintf(finaloutput,"Unique page=%d\n",unique_page);
  fprintf(finaloutput,"flush個數=%d, replacement個數=%d\n", flushgj4, replacegj4);
  fprintf(finaloutput,"Sumcount = %.0lf,Hitcount = %.0lf Hit = %.2f\n,miss_ratio=%lf\n",Sumcount,Hitcount,(Hitcount/Sumcount)*100.0,(misscount/Sumcount)*100.0);
  fprintf(finaloutput,"\n****參數顯示****\n");
  fprintf(finaloutput,"[CHEN] HINTGROUP=%d, alpha=%f\n",HINTGROUP, alpha);
  fprintf(finaloutput,"[CHEN] Trace File=%s, max_req=%d, MEM Size=%d\n", argv[4], atoi(argv[5]), atoi(argv[6]));
  fprintf(finaloutput,"[CHEN] Pd_FA_count=%d,Pd_R_count=%d\n",pdFA_count,pdR_count);
  fprintf(finaloutput,"[CHEN] FA_count=%d,R_count=%d\n",FA_count,R_count);
  fprintf(finaloutput,"[CHEN] tot_Rtime=%ld,prstime=%lf\n",tot_Rtime,prstime);
  fprintf(finaloutput,"[CHEN] fa_hint_count=%d\n",fa_hint_count);
  fprintf(finaloutput,"[CHEN] ft_hint_count=%d\n",ft_hint_count);
  fprintf(finaloutput,"[CHEN] rp_hint_count=%d\n",rp_hint_count);
  fprintf(finaloutput,"[CHEN] total_hint_count=%d\n",fa_hint_count+ft_hint_count+rp_hint_count);
//-----------------------------------------------------------------------------------------------------------------------   
 
  /* NOTE: it is bad to use this internal disksim call from external... */
  DISKSIM_srand48(1);
 
   
  disksim_interface_shutdown(disksim, now);
  printf("\n****Response time****");
  fprintf(finaloutput,"\n****Response time****");
  print_statistics(&st, &wst, &rst, &wstp, &rstp, "response time");
  //fclose(evict_fread);
  fclose(fread);
  fclose(fwrite);
  fclose(Nflush_hintflow);
  fclose(Nflush_flow);
  fclose(outputfd);
  fclose(outputssd);
  fclose(myoutput);
  fclose(myoutput2);
  fclose(myoutput3);
  fclose(myoutput4);
  fclose(myoutput5);
  fclose(finaloutput);
  fclose(gc_info);
  fclose(gc_info2);
  exit(0);
}
