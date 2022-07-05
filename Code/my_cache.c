#include "my_cache.h"

long long int DRAM_size = 64;
long long int NVRAM_size = 64;
int expiration = 8;

UserStatistics *initUserStatistics(){
  UserStatistics *temp = (UserStatistics*)malloc(sizeof(UserStatistics));
  temp->ReadReq_PerPeriod = 0;
  temp->WriteReq_PerPeriod = 0;
  temp->ReadReq_Predict = 0;
  temp->WriteReq_Predict = 0;
  temp->Evictioncount = 0;
  temp->DRAMReadHit_PerPeriod = 0;
  temp->DRAMWriteHit_PerPeriod = 0;
  temp->NVRAMReadHit_PerPeriod = 0;
  temp->NVRAMWriteHit_PerPeriod = 0;
  temp->DRAMReadHitRatio_Predict = 0;
  temp->DRAMWriteHitRatio_Predict = 0;
  temp->NVRAMReadHitRatio_Predict = 0;
  temp->NVRAMWriteHitRatio_Predict = 0;
  temp->DirtyRatio_Predict = 0;

  return temp;
}

Node *newNode(unsigned long pageNumber,int PageType,int ClearOrDirty){
  Node *temp = (Node*)malloc(sizeof(Node));
  temp->PageNumber = pageNumber;
  temp->next = NULL;
  temp->reference_bit = 0;
  temp->dirty_bit = 0;
  
  if(PageType == DRAMPage){ 
    temp->type = DRAMPage;
    temp->Frequency = 0;
    temp->OverlookRotation = 0;
  }
  else{ 
    if(ClearOrDirty == DirtyPage){
      temp->dirty_bit = 1;
    }
    else{
      temp->dirty_bit = 0;
    }
    temp->type = NVRAMPage;
    temp->Frequency = -1;
    temp->OverlookRotation = -1;
  }
  return temp;
}

Queue *createQueue(long long int NumOfFrames){
  Queue *queue = (Queue *)malloc(sizeof(Queue));
  queue->count = 0;
  queue->prev = NULL;
  queue->NumOfFrames = NumOfFrames;
  queue->read_hitcount = 0;
  queue->write_hitcount = 0;
  queue->misscount = 0;
  //queue->evictioncount = 0;

  return queue;
}

Hash *createHash(long long int capacity){
  Hash *hash = (Hash *)malloc(sizeof(Hash));
  hash->capacity = capacity;

  hash->array = (Node **) malloc(hash->capacity * sizeof(Node*));
  int i ;
  for(i=0;i<hash->capacity;++i){
    hash->array[i] = NULL;
  }
  return hash;
}

UserQueueArray *createUserQueueArray(int NumofUsers){

  UserQueueArray *userqueuearray = (UserQueueArray *)malloc(sizeof(UserQueueArray));
  userqueuearray->NumofUsers = NumofUsers;

  userqueuearray->DRAMQueue = (Queue **) malloc(NumofUsers * sizeof(Queue*));
  userqueuearray->NVRAMQueue = (Queue **) malloc(NumofUsers * sizeof(Queue*));
  int i;
  for(i = 0;i<NumofUsers;i++){
    userqueuearray->DRAMQueue[i] = NULL;
  }
  for(i = 0;i<NumofUsers;i++){
    userqueuearray->NVRAMQueue[i] = NULL;
  }
  return userqueuearray; 
}

UserHashArray *createUserHashArray(int NumofUsers){
  UserHashArray *userhasharray = (UserHashArray *)malloc(sizeof(UserHashArray));
  userhasharray->NumofUsers = NumofUsers;

  userhasharray->array = (Hash **)malloc(NumofUsers * sizeof(Hash*));
  int i;
  for(i = 0;i<NumofUsers;i++){
    userhasharray->array[i] = NULL;
  }
  return userhasharray;
}

int isDRAMQueueFull(Queue *Dqueue){
  return Dqueue->count >= Dqueue->NumOfFrames;
}

int isNVRAMQueueFull(Queue *Nqueue){
  return Nqueue->count >= Nqueue->NumOfFrames;
}

Node *FindDRAMVictim(Queue *Dqueue){ 
  float hot_page_threshold = 0;
  int count = Dqueue->count;
  Node *temp = Dqueue->prev;
  while(count>0){
    hot_page_threshold += temp->Frequency;
    count--;
    temp = temp->next;
  }
  hot_page_threshold = hot_page_threshold / Dqueue->count;
  
  Node *prev = Dqueue->prev;
  Node *current = prev->next;
  
  while(1){
    if(current->dirty_bit == 0){
      if(current->Frequency >= hot_page_threshold && current->OverlookRotation < expiration){
        current->OverlookRotation++;
      }
      else{
        break;
      } 
    }
    else{
      current->dirty_bit = 0;
      current->Frequency++;
      current->OverlookRotation = 0;
      hot_page_threshold = (hot_page_threshold * (Dqueue->count-1) + current->Frequency) / Dqueue->count;
    }
    prev = current;
    current = current->next;
  }
  return prev;
}

Node *FindNVRAMVictim(Queue *Nqueue){  
  Node *prev = Nqueue->prev;
  Node *current = Nqueue->prev->next;
  while(1){
    if(current->reference_bit == 0){
      break;
    }
    else{
      current->reference_bit = 0 ;
    }
    prev = current;
    current = current->next;
  }
  return prev;
}

EvictPage NVRAMreplacement(int userid , UserQueueArray *queuearray , UserHashArray *hasharray){
  EvictPage victim;
  Queue *Nqueue = queuearray->NVRAMQueue[userid];
  Hash *hash = hasharray->array[userid];
  Node *prev = FindNVRAMVictim(Nqueue);
  Node *current = prev->next;
  if(current->dirty_bit == 1){
      victim.dirty_bit = 1;
      victim.PageNumber = current->PageNumber;
  }
  else{
      victim.dirty_bit = 0;
      victim.PageNumber = current->PageNumber;
  }
  prev->next = current->next;   
  Nqueue->prev = prev;
  hash->array[current->PageNumber] = NULL;
  free(current);
  Nqueue->count--;  

  return victim;
}

int CheckNVRAMSpace(int userid , UserQueueArray *queuearray){
  Queue *Nqueue = queuearray->NVRAMQueue[userid];
  int i;
  int numofusers = queuearray->NumofUsers;
  int totalcount = 0;
  Queue *tempqueue;
  for(i=0;i<numofusers;i++){
    tempqueue = queuearray->NVRAMQueue[i];
    totalcount+=tempqueue->count;
  }

  if(isNVRAMQueueFull(Nqueue)){ 
    return userid;
  }   
  else if(totalcount == NVRAM_size && Nqueue->count < Nqueue->NumOfFrames){
    
    for (i = 0; i < numofusers; i++)
    {
      if(i!=userid){
        tempqueue = queuearray->NVRAMQueue[i];
        if(tempqueue->count > tempqueue->NumOfFrames){
          break;
        }
      }
    }
    return i;
  }

  return -1;
}

void EnNVRAMqueue(int userid , UserQueueArray *queuearray , UserHashArray *hasharray , unsigned long pageNumber , int ClearOrDirty){
  Queue *Nqueue = queuearray->NVRAMQueue[userid];
  Hash *hash = hasharray->array[userid];
  Node *temp = newNode(pageNumber,NVRAMPage,ClearOrDirty);
  Node *prev = Nqueue->prev;
  if(prev == NULL){
    temp->next = temp;  
  } 
  else{
    temp->next = prev->next;
    prev->next = temp;
  }   
  Nqueue->prev = temp;  
  Nqueue->count++;
  hash->array[pageNumber] = temp;
}

EvictPage DRAMreplacement(int userid , UserQueueArray *queuearray , UserHashArray *hasharray){
  EvictPage victim;
  Queue *Dqueue = queuearray->DRAMQueue[userid];
  Hash *hash = hasharray->array[userid];
  Node *prev = FindDRAMVictim(Dqueue);
  Node *current = prev->next;
  victim.dirty_bit = 1;
  victim.PageNumber = current->PageNumber;
  prev->next = current->next;
  Dqueue->prev = prev;
  free(current);
  Dqueue->count--; 
  return victim;
}

int CheckDRAMSpace(int userid , UserQueueArray *queuearray){
  Queue *Dqueue = queuearray->DRAMQueue[userid];
  Queue *tempqueue;
  int i;
  int numofusers = queuearray->NumofUsers;
  int totalcount = 0;
  for(i=0;i<numofusers;i++){
    tempqueue = queuearray->DRAMQueue[i];
    totalcount+=tempqueue->count;
  }

  if(isDRAMQueueFull(Dqueue)){  
    return userid;
  }
  else if(Dqueue->count < Dqueue->NumOfFrames && totalcount==DRAM_size){
    for (i = 0; i < numofusers; i++)
    {
      if(i!=userid){
        tempqueue = queuearray->DRAMQueue[i];
        if(tempqueue->count > tempqueue->NumOfFrames){
          break;
        }
      }
    }
    return i;
  }
  return -1;
}

void EnDRAMqueue(int userid , UserQueueArray *queuearray , UserHashArray *hasharray , unsigned long pageNumber){
  Queue *Dqueue = queuearray->DRAMQueue[userid];
  Queue *Nqueue = queuearray->NVRAMQueue[userid];
  Hash *hash = hasharray->array[userid];

  Node *temp = newNode(pageNumber,DRAMPage,DirtyPage);
  Node *prev = Dqueue->prev;
  if(prev == NULL){
    temp->next = temp;    
  }
  else{
    temp->next = prev->next;
    prev->next = temp;
  }
  Dqueue->prev = temp;
  Dqueue->count++;
  hash->array[pageNumber] = temp;
}

Node *FindPreviousPage(Node *refPage){
  Node *previous = refPage->next;
  while(previous->next->PageNumber != refPage->PageNumber){
    previous = previous->next;
  }
  return previous;
}

void PageSwapping(int userid , UserQueueArray *queuearray , UserHashArray *hasharray, UserStatistics *userstatistics, Node *refPage,double *systime){
  Node *Nprev = FindPreviousPage(refPage); 
  Node *Ncurrent = refPage;
  Node *Nnext = refPage->next;
  EvictPage victim;
  
  Queue *Nqueue = queuearray->NVRAMQueue[userid]; 
  Queue *Dqueue = queuearray->DRAMQueue[userid]; 

  if(Nqueue->count == 1){
    Nqueue->count--;
    Nqueue->prev = NULL;
    if(isDRAMQueueFull(Dqueue)){  
      victim = DRAMreplacement(userid,queuearray,hasharray);
      EnNVRAMqueue(userid,queuearray,hasharray,victim.PageNumber,DirtyPage); 
      *systime += DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME;  
    }
    EnDRAMqueue(userid,queuearray,hasharray,Ncurrent->PageNumber);
    *systime += NVRAM_PAGE_READ_TIME + DRAM_PAGE_ACCESS_TIME;
    free(Ncurrent);
  }
  else if(Nqueue->count == 2){
    Nqueue->prev = Nprev;
    Nprev->next = Nprev;
    Nqueue->count--;
    if(isDRAMQueueFull(Dqueue)){  
      victim = DRAMreplacement(userid,queuearray,hasharray);
      EnNVRAMqueue(userid,queuearray,hasharray,victim.PageNumber,DirtyPage);   
      *systime += DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME;  
    }
    EnDRAMqueue(userid,queuearray,hasharray,Ncurrent->PageNumber);
    *systime += NVRAM_PAGE_READ_TIME + DRAM_PAGE_ACCESS_TIME;
    free(Ncurrent);
  }
  else{
    Nprev->next = Nnext;
    if(Nqueue->prev == Ncurrent){
      Nqueue->prev = Nprev;
    } 
    Nqueue->count--;
    if(isDRAMQueueFull(Dqueue)){  
      victim = DRAMreplacement(userid,queuearray,hasharray);
      EnNVRAMqueue(userid,queuearray,hasharray,victim.PageNumber,DirtyPage);    
      *systime += DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME; 
    }
    EnDRAMqueue(userid,queuearray,hasharray,Ncurrent->PageNumber);
    *systime += NVRAM_PAGE_READ_TIME + DRAM_PAGE_ACCESS_TIME;
    free(Ncurrent);
  }
}

int ReferencePage(int userid , UserQueueArray *queuearray , UserHashArray *hasharray, UserStatistics *userstatistics, REQ *r , USRQUE *que , double *systime){
  Hash *hash = hasharray->array[userid];
  Queue *Dqueue = queuearray->DRAMQueue[userid];
  Queue *Nqueue = queuearray->NVRAMQueue[userid];
  unsigned long pageNumber = (r->diskBlkno)/8;
  unsigned Req_type = r->reqFlag;
  int flag = 0;

  int victimuserid;
  EvictPage victim;
  REQ victimreq;

  int victimuserid2;
  EvictPage victim2;
  /*if(Req_type == ReadReq){
    userstatistics->ReadReq_PerPeriod++;
  }
  else{
    userstatistics->WriteReq_PerPeriod++;
  }*/


  Node *refPage = hash->array[pageNumber];
  if(refPage == NULL){ // cache miss
    if(Req_type == ReadReq){ //read miss
      Nqueue->misscount++;
      //userstatistics->ReadReq_PerPeriod++;    
      victimuserid = CheckNVRAMSpace(userid,queuearray);
      if(victimuserid != -1){
        victim = NVRAMreplacement(victimuserid,queuearray,hasharray);
        if(victim.dirty_bit == 1){
          userstatistics->Evictioncount++;
          copyRequest(r,&victimreq);
          victimreq.diskBlkno = victim.PageNumber*8;
          victimreq.reqFlag = 0;
          insertUSRQUEUE(que,&victimreq);
          *systime += NVRAM_PAGE_READ_TIME;
        }
      }
      insertUSRQUEUE(que,r);
      EnNVRAMqueue(userid,queuearray,hasharray,pageNumber,CleanPage);    
      return 1;
    }
    else {       //write miss
      Dqueue->misscount++;
      //userstatistics->WriteReq_PerPeriod++;   
      victimuserid = CheckDRAMSpace(userid,queuearray);
      if(victimuserid != -1){
        victim = DRAMreplacement(victimuserid,queuearray,hasharray);
        victimuserid2 = CheckNVRAMSpace(victimuserid,queuearray);
        if(victimuserid2!=-1){
          victim2 = NVRAMreplacement(victimuserid2,queuearray,hasharray);
          if(victim2.dirty_bit == 1){
            userstatistics->Evictioncount++;
            copyRequest(r,&victimreq);
            victimreq.diskBlkno =victim2.PageNumber*8;
            victimreq.reqFlag = 0;
            insertUSRQUEUE(que,&victimreq);
            flag = 1;
            *systime += NVRAM_PAGE_READ_TIME;
          }
        }
        EnNVRAMqueue(victimuserid,queuearray,hasharray,victim.PageNumber,DirtyPage);    
      }
      EnDRAMqueue(userid,queuearray,hasharray,pageNumber);
      if(flag == 1) {
        return 1;
      }
      else{
        *systime += DRAM_PAGE_ACCESS_TIME;
        userstatistics->WriteReq_PerPeriod++; 
      }
    }
  }
  else{ //cache hit
    if(Req_type == ReadReq){  //read hit
      refPage->reference_bit = 1;
      if(refPage->type == NVRAMPage){ //read hit on nvram
        Nqueue->read_hitcount++;
        userstatistics->ReadReq_PerPeriod++;
        userstatistics->NVRAMReadHit_PerPeriod++;
        *systime += NVRAM_PAGE_READ_TIME;
      }
      else{ //read hit on dram
        Dqueue->read_hitcount++;
        userstatistics->ReadReq_PerPeriod++;
        userstatistics->DRAMReadHit_PerPeriod++;
        *systime += DRAM_PAGE_ACCESS_TIME;
      }
      
    }
    else { //write hit
      if(refPage->type == NVRAMPage){ //write hit on nvram
        Nqueue->write_hitcount++;
        userstatistics->WriteReq_PerPeriod++;
        userstatistics->NVRAMWriteHit_PerPeriod++;
        PageSwapping(userid,queuearray,hasharray,userstatistics,refPage,systime);
      }
      else{ //write hit on dram
      	refPage->dirty_bit = 1;
        Dqueue->write_hitcount++;
        userstatistics->WriteReq_PerPeriod++;
        userstatistics->DRAMWriteHit_PerPeriod++;
        *systime += DRAM_PAGE_ACCESS_TIME;
      }
    }
  }
  return 0;
}

void FreeQueue(Queue *queue){
  if(queue->count > 0){
      Node *prev = queue->prev;
      Node *curr = prev->next;
      prev->next = NULL;
      while(curr != NULL){
        Node *temp = curr;
        curr = curr->next;
        free(temp);
      }
  }
  free(queue);
}

void FreeHash(Hash *hash){
  free(hash->array); 
  free(hash);
}

void FreeQueueArray(UserQueueArray *queuearray){
  free(queuearray->DRAMQueue);
  free(queuearray->NVRAMQueue);
  free(queuearray);

}

void FreeHashArray(UserHashArray *hasharray){
  free(hasharray->array);
  free(hasharray);
}

void FreeUserStatistics(UserStatistics *userstatistics){
  free(userstatistics);
}

void PrintUserStatistics(int userid , Queue *Dqueue, Queue *Nqueue){
  printf("---------------user %d statistic---------------\n",userid);
  printf("-----------------DRAM------------------\n");
  printf("dram  read hitcount              : %d\n", Dqueue->read_hitcount);
  printf("dram  write hitcount             : %d\n", Dqueue->write_hitcount);
  printf("write misscount                  : %d\n", Dqueue->misscount);
  printf("-----------------NVRAM-----------------\n");
  printf("nvram  read hitcount             : %d\n", Nqueue->read_hitcount);
  printf("nvram  write hitcount            : %d\n", Nqueue->write_hitcount);
  printf("read misscount                     %d\n\n", Nqueue->misscount);
}

void PrintUserStatisticsPerPeriod(int userid , UserStatistics *user_statistics){
  printf("---------------user %d statistic perperiod---------------\n",userid);
  printf("-----------------DRAM------------------\n");
  printf("write req:                       : %d\n", user_statistics->WriteReq_PerPeriod);
  printf("dram  read hitcount              : %d\n", user_statistics->DRAMReadHit_PerPeriod);
  printf("dram  write hitcount             : %d\n",  user_statistics->DRAMWriteHit_PerPeriod);
  printf("write misscount                  : %d\n", user_statistics->WriteReq_PerPeriod - user_statistics->NVRAMWriteHit_PerPeriod - user_statistics->DRAMWriteHit_PerPeriod);
  printf("-----------------NVRAM-----------------\n");
  printf("read req:                        : %d\n", user_statistics->ReadReq_PerPeriod);
  printf("nvram  read hitcount             : %d\n", user_statistics->NVRAMReadHit_PerPeriod);
  printf("nvram  write hitcount            : %d\n", user_statistics->NVRAMWriteHit_PerPeriod);
  printf("read misscount                   :%d\n\n",  user_statistics->ReadReq_PerPeriod - user_statistics->NVRAMReadHit_PerPeriod - user_statistics->DRAMReadHit_PerPeriod);
}

void cleanUserStatistics(UserStatistics *user_statistics){
  user_statistics->WriteReq_PerPeriod = 0;
  user_statistics->ReadReq_PerPeriod = 0;
  user_statistics->Evictioncount = 0;
  user_statistics->DRAMReadHit_PerPeriod = 0;
  user_statistics->DRAMWriteHit_PerPeriod = 0;
  user_statistics->NVRAMReadHit_PerPeriod = 0;
  user_statistics->NVRAMWriteHit_PerPeriod = 0;
}

void UpdateUserStatistics(UserStatistics *user_statistics,int initflag){
  double dram_read_hit_ratio = 0.0;
  double dram_write_hit_ratio = 0.0;
  double nvram_read_hit_ratio = 0.0;
  double nvram_write_hit_ratio = 0.0;
  double dirtyratio = 0.0;
  int misscount;

  if(user_statistics->ReadReq_PerPeriod > 0){
    dram_read_hit_ratio = (double)user_statistics->DRAMReadHit_PerPeriod / user_statistics->ReadReq_PerPeriod;
    nvram_read_hit_ratio = (double)user_statistics->NVRAMReadHit_PerPeriod / user_statistics->ReadReq_PerPeriod;
  }

  if(user_statistics->WriteReq_PerPeriod > 0){
    dram_write_hit_ratio = (double)user_statistics->DRAMWriteHit_PerPeriod / user_statistics->WriteReq_PerPeriod;
    nvram_write_hit_ratio = (double)user_statistics->NVRAMWriteHit_PerPeriod / user_statistics->WriteReq_PerPeriod;    
  }

  
  misscount = user_statistics->ReadReq_PerPeriod + 
                  user_statistics->WriteReq_PerPeriod - 
                  user_statistics->DRAMReadHit_PerPeriod - 
                  user_statistics->NVRAMReadHit_PerPeriod - 
                  user_statistics->DRAMWriteHit_PerPeriod - 
                  user_statistics->NVRAMWriteHit_PerPeriod;
  if(misscount > 0){
    dirtyratio = (double)user_statistics->Evictioncount / misscount;
  }

  if(initflag == 0){
    user_statistics->ReadReq_Predict = user_statistics->ReadReq_PerPeriod;
    user_statistics->WriteReq_Predict = user_statistics->WriteReq_PerPeriod;
    user_statistics->DRAMReadHitRatio_Predict = dram_read_hit_ratio;
    user_statistics->DRAMWriteHitRatio_Predict = dram_write_hit_ratio;
    user_statistics->NVRAMReadHitRatio_Predict = nvram_read_hit_ratio;
    user_statistics->NVRAMWriteHitRatio_Predict = nvram_write_hit_ratio;
    user_statistics->DirtyRatio_Predict = dirtyratio;
  }
  else{
    user_statistics->ReadReq_Predict = user_statistics->ReadReq_PerPeriod * alpha + user_statistics->ReadReq_Predict * (1.0 - alpha);
    user_statistics->WriteReq_Predict = user_statistics->WriteReq_PerPeriod * alpha + user_statistics->WriteReq_Predict * (1.0 - alpha);
    user_statistics->DRAMReadHitRatio_Predict = dram_read_hit_ratio * alpha + user_statistics->DRAMReadHitRatio_Predict * (1.0 - alpha);
    user_statistics->DRAMWriteHitRatio_Predict = dram_write_hit_ratio * alpha + user_statistics->DRAMWriteHitRatio_Predict * (1.0 - alpha);
    user_statistics->NVRAMReadHitRatio_Predict = nvram_read_hit_ratio * alpha + user_statistics->NVRAMReadHitRatio_Predict * (1.0 - alpha);
    user_statistics->NVRAMWriteHitRatio_Predict = nvram_write_hit_ratio * alpha + user_statistics->NVRAMWriteHitRatio_Predict * (1.0 - alpha);
    user_statistics->DirtyRatio_Predict = dirtyratio * alpha + user_statistics->DirtyRatio_Predict * (1.0 - alpha);
  }

}