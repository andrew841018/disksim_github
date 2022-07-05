#include "my_ghostbuffer.h"

int OverlookThreshold = 8;

GhostStatistics *initGhostStatistics(int DRAMSize, int DRAMPartitionNumber , int NVRAMSize , int NVRAMPartitionNumber){
	GhostStatistics *temp = (GhostStatistics*)malloc(sizeof(GhostStatistics));
	temp->total_read_access_count = 0;
	temp->total_write_access_count = 0;
	temp->HitCountArray = (int ***)calloc(DRAMPartitionNumber,sizeof(int**));
  int i,j,k;
  for(i = 0 ; i < DRAMPartitionNumber ; i++){
    temp->HitCountArray[i] = (int **)calloc(NVRAMPartitionNumber,sizeof(int*));
    for (j = 0; j < NVRAMPartitionNumber; j++)
    {
      temp->HitCountArray[i][j] = (int*)calloc(4,sizeof(int));
    }
  }

  temp->HitRatioArray = (double ***)calloc(DRAMPartitionNumber,sizeof(double**));
  for(i = 0;i<DRAMPartitionNumber;i++){
    temp->HitRatioArray[i] = (double **)calloc(NVRAMPartitionNumber,sizeof(double*));
    for(j=0;j<NVRAMPartitionNumber;j++){
      temp->HitRatioArray[i][j] = (double *)calloc(4,sizeof(double));
    }
  }

  temp->DRAMSize = DRAMSize;
  temp->DRAMPartitionNumber = DRAMPartitionNumber;
  temp->NVRAMSize = NVRAMSize;
  temp->NVRAMPartitionNumber = NVRAMPartitionNumber;

	return temp;
}

RANode *newRANode(unsigned long pagenumber){
	RANode *node = (RANode*)malloc(sizeof(RANode));
	node->PageNumber = pagenumber;
	node->PartitionCount = -1;
	node->Next = NULL;
	node->Prev = NULL;

	return node;
}

RWNode *newRWNode(unsigned long pagenumber){
	RWNode *node = (RWNode*)malloc(sizeof(RWNode));
	node->PageNumber = pagenumber;
	node->PartitionCount = -1;
	node->Frequency = 0;
	node->Next = NULL;
	node->Prev = NULL;
	node->OverlookCount = 0;

	return node;
}

RANodeQueue *createRANodeQueue(int capacity){
	RANodeQueue *queue = (RANodeQueue*)malloc(sizeof(RANodeQueue));
	queue->Count = 0;
	queue->Capacity = capacity;
	queue->MRU = NULL;
	queue->LRU = NULL;

	return queue;
}

RWNodeQueue *createRWNodeQueue(int capacity){
	RWNodeQueue *queue = (RWNodeQueue*)malloc(sizeof(RWNodeQueue));
	queue->Count = 0;
	queue->Capacity = capacity;
	queue->MRU = NULL;
	queue->LRU = NULL;

	return queue;
}

RANodeHash *createRANodeHash(long long int capacity){
	RANodeHash *hash = (RANodeHash*)malloc(sizeof(RANodeHash));
	hash->Capacity = capacity;

	hash->Array = (RANode **) malloc(hash->Capacity * sizeof(RANode*));
	int i ;
	for(i=0;i<hash->Capacity;++i){
		hash->Array[i] = NULL;
	}

	return hash;
}

RWNodeHash *createRWNodeHash(long long int capacity){
	RWNodeHash *hash = (RWNodeHash*)malloc(sizeof(RWNodeHash));
	hash->Capacity = capacity;

	hash->Array = (RWNode **) malloc(hash->Capacity * sizeof(RWNode*));
	int i ;
	for(i=0;i<hash->Capacity;++i){
		hash->Array[i] = NULL;
	}

	return hash;
}

int isRANodeQueueFull(RANodeQueue *queue){
  return queue->Count >= queue->Capacity;
}

int isRWNodeQueueFull(RWNodeQueue *queue){
  return queue->Count >= queue->Capacity;
}

void RANodeQueueReplacement(RANodeQueue *queue ,RANodeHash *hash){
	RANode *lru = queue->LRU;
	RANode *previous = lru->Prev;

	previous->Next = NULL;
	queue->LRU = previous;
	hash->Array[lru->PageNumber] = NULL;
	free(lru);
	queue->Count--;
}

void RWNodeQueueReplacement(RWNodeQueue *queue ,RWNodeHash *hash){
	int i = 0;
  //int stop = queue->count / 2;
	
	//RWNode *stopnode = queue->MRU;
	/*for(i=0;i<DoNotReplaceNumber-1;i++){
		stopnode = stopnode->Next;
	}*/

	float hot_page_threshold = 0;
	RWNode *curr = queue->MRU;
	while(curr!=NULL){
		hot_page_threshold += curr->Frequency;
		curr = curr->Next;
	}
	hot_page_threshold = hot_page_threshold/queue->Count;

	RWNode *victim = queue->LRU;
	
	while(victim->Frequency > hot_page_threshold && victim->OverlookCount < OverlookThreshold){
		victim->OverlookCount++;
		victim = victim->Prev;
    i++;
		if(i == queue->Count / 2){
			victim = queue->LRU;
      i = 0;
		}
	}

	RWNode *victimnext = victim->Next;
	RWNode *victimprev = victim->Prev;

	if(victimnext == NULL){
		victimprev->Next = NULL;
		queue->LRU = victimprev;
	}
	else{
		victimnext->Prev = victimprev;
		victimprev->Next = victimnext;
	}

	hash->Array[victim->PageNumber] = NULL;
	free(victim);
	queue->Count--;

}

void EnRANodeQueue(RANodeQueue *queue , RANodeHash *hash , unsigned long pagenumber){
	if(isRANodeQueueFull(queue)){
		RANodeQueueReplacement(queue,hash);
	}
	RANode *temp = newRANode(pagenumber);
	RANode *lru = queue->LRU;
	RANode *mru = queue->MRU;

	if(lru == NULL && mru == NULL){
		queue->LRU = temp;
		queue->MRU = temp;
	}
	else{
		temp->Next = mru;
		mru->Prev = temp;
		queue->MRU = temp;
	}
	queue->Count++;
	hash->Array[pagenumber] = temp;
}

void EnRWNodeQueue(RWNodeQueue *queue , RWNodeHash *hash , unsigned long pagenumber){
	if(isRWNodeQueueFull(queue)){
		RWNodeQueueReplacement(queue,hash);
	}
	RWNode *temp = newRWNode(pagenumber);
	RWNode *mru = queue->MRU;
	RWNode *lru = queue->LRU;

	if(lru == NULL && mru == NULL){
		queue->MRU = temp;
		queue->LRU = temp;
	}
	else{
		temp->Next = mru;
		mru->Prev = temp;
		queue->MRU = temp;
	}
	queue->Count++;
	hash->Array[pagenumber] = temp;
}

void InitialAllNodes(RANodeQueue *ranqueue , RWNodeQueue *rwnqueue){
	int i;
	RANode *temp = ranqueue->MRU;
	for(i=0;i<ranqueue->Count;i++){
		temp->PartitionCount = -1;
		temp = temp->Next;
	}

	RWNode *temp1 = rwnqueue->MRU;
	for(i=0;i<rwnqueue->Count;i++){
		temp1->PartitionCount = -1;
		temp1 = temp1->Next;
	}
}

void InitialHitCountArray(GhostStatistics *ghoststatistics){
  int i,j,k;
  ghoststatistics->total_read_access_count = 0;
  ghoststatistics->total_write_access_count = 0;
  for(i=0;i<ghoststatistics->DRAMPartitionNumber;i++){
    for(j=0;j<ghoststatistics->NVRAMPartitionNumber;j++){
      for(k=0;k<4;k++){
        ghoststatistics->HitCountArray[i][j][k] = 0;
      }
    }
  }
}

void HitCountArrayUpdate(RANodeQueue *ranqueue , RANodeHash *ranhash , RWNodeQueue *rwnqueue , RWNodeHash *rwnhash , unsigned long pagenumber , unsigned reqtype ,GhostStatistics *ghoststatistics){
  int i,j,k;

  for(i=0;i<ghoststatistics->DRAMPartitionNumber;i++){ 
    InitialAllNodes(ranqueue,rwnqueue);
    int DRAMCurrentSize = (i + 1) * (ghoststatistics->DRAMSize / ghoststatistics->DRAMPartitionNumber);

    if(DRAMCurrentSize>=rwnqueue->Count){
      RWNode *temp = rwnqueue->MRU;
      for(j=0;j<rwnqueue->Count;j++){
        temp->PartitionCount = 0;
        temp=temp->Next;
      }

    }
    else{
      RWNode *mru = rwnqueue->MRU;
      mru->PartitionCount = 0;
      DRAMCurrentSize--;
      
      float HotPageThreshold = 0;
      RWNode *CalAvgFreq = rwnqueue->MRU;
      while(CalAvgFreq!=NULL){
        HotPageThreshold+=CalAvgFreq->Frequency;
        CalAvgFreq = CalAvgFreq->Next;
      }
      HotPageThreshold =  HotPageThreshold / rwnqueue->Count;

      RWNode *AboveThresholdNode = mru->Next;
      while(DRAMCurrentSize>0 && AboveThresholdNode!=NULL){
        if(AboveThresholdNode->Frequency >= HotPageThreshold && AboveThresholdNode->OverlookCount<OverlookThreshold){
          AboveThresholdNode->PartitionCount = 0;
          DRAMCurrentSize--;
        }
        AboveThresholdNode = AboveThresholdNode->Next;
      }
      RWNode *PatchingNode = mru->Next;
      while(DRAMCurrentSize>0 && PatchingNode!=NULL){
        if(PatchingNode->PartitionCount == -1){
          PatchingNode->PartitionCount = 0;
          DRAMCurrentSize--;
        }
        PatchingNode = PatchingNode->Next;
      }
    }   

    RANode *temp1 = ranqueue->MRU;
    int target = ghoststatistics->NVRAMSize;
    int count = 0;

    while(target>0 && temp1!=NULL){
      RWNode *test = rwnhash->Array[temp1->PageNumber];
      if(test == NULL || test->PartitionCount==-1){
        temp1->PartitionCount = (count / (ghoststatistics->NVRAMSize / ghoststatistics->NVRAMPartitionNumber)) + 1;
        count++;
        target--;
      }
      temp1=temp1->Next;
    }

    RWNode *nodeindram = rwnhash->Array[pagenumber];
    if(nodeindram != NULL && nodeindram->PartitionCount==0){
      if(reqtype==ReadReq){ //read hit in dram
        for(j=i;j<ghoststatistics->DRAMPartitionNumber;j++){
          for(k=0;k<ghoststatistics->NVRAMPartitionNumber;k++){
            ghoststatistics->HitCountArray[j][k][0]++;
          }
        }
        break;
      }
      else{//write hit in dram
        for(j=i;j<ghoststatistics->DRAMPartitionNumber;j++){
          for(k=0;k<ghoststatistics->NVRAMPartitionNumber;k++){
            ghoststatistics->HitCountArray[j][k][1]++;
          }
        }
        break;
      }
    }
    else{
      RANode *nodeinnvram = ranhash->Array[pagenumber];
      if(nodeinnvram != NULL && nodeinnvram->PartitionCount > 0){
        if(reqtype == ReadReq){ //read hit in nvram
          for(k=nodeinnvram->PartitionCount;k<=ghoststatistics->NVRAMPartitionNumber;k++){
            ghoststatistics->HitCountArray[i][k-1][2]++;
          }
        }
        else{//write hit in nvram
          for(k=nodeinnvram->PartitionCount;k<=ghoststatistics->NVRAMPartitionNumber;k++){
            ghoststatistics->HitCountArray[i][k-1][3]++;
          }
        }
      }
    }
    
    
  }

}

void UpdateRANQueue(RANodeQueue *queue , RANodeHash *hash , unsigned long pagenumber){
  RANode *temp = hash->Array[pagenumber];
  RANode *next = temp->Next;
  RANode *prev = temp->Prev;
  RANode *mru = queue->MRU;

  if(temp != mru){ 
    if(next == NULL){
      prev->Next = NULL;
      queue->LRU = prev;
      temp->Prev = NULL;
      temp->Next = mru;
      mru->Prev = temp;
      queue->MRU = temp;
    }
    else{
      prev->Next = next;
      next->Prev = prev;
      temp->Prev = NULL;
      temp->Next = mru;
      mru->Prev = temp;
      queue->MRU = temp;
    }
    
  }
}

void UpdateRWNQueue(RWNodeQueue *queue , RWNodeHash *hash , unsigned long pagenumber){
  RWNode *temp = hash->Array[pagenumber];
  temp->OverlookCount = 0;
  temp->Frequency++;
  RWNode *next = temp->Next;
  RWNode *prev = temp->Prev;
  RWNode *mru = queue->MRU;

  if(temp != mru){ 
    if(next == NULL){
      prev->Next = NULL;
      queue->LRU = prev;
      temp->Prev = NULL;
      temp->Next = mru;
      mru->Prev = temp;
      queue->MRU = temp;
    }
    else{
      prev->Next = next;
      next->Prev = prev;
      temp->Prev = NULL;
      temp->Next = mru;
      mru->Prev = temp;
      queue->MRU = temp;
    }
    
  }
}

void GhostReferencePage(RANodeQueue *ranqueue , RANodeHash *ranhash , RWNodeQueue *rwnqueue , RWNodeHash *rwnhash , unsigned long pagenumber , unsigned reqtype , GhostStatistics *ghoststatistics){
  if(reqtype == ReadReq){
    ghoststatistics->total_read_access_count++;
    //printf("-------------------ReferencePage-----------PageNumber:%u----------Reqtype:%s",pagenumber,printread);
  }
  else{
    ghoststatistics->total_write_access_count++;
    //printf("-------------------ReferencePage-----------PageNumber:%u----------Reqtype:%s",pagenumber,printwrite);
  }
  if(ranhash->Array[pagenumber] == NULL && rwnhash->Array[pagenumber] == NULL){
    if(reqtype == ReadReq){
      EnRANodeQueue(ranqueue,ranhash,pagenumber);
    }
    else{
      EnRANodeQueue(ranqueue,ranhash,pagenumber);
      EnRWNodeQueue(rwnqueue,rwnhash,pagenumber);
    }
  }
  else{
    HitCountArrayUpdate(ranqueue,ranhash,rwnqueue,rwnhash,pagenumber,reqtype,ghoststatistics);
    if(rwnhash->Array[pagenumber] == NULL){
      if(reqtype == WriteReq){
        EnRWNodeQueue(rwnqueue,rwnhash,pagenumber);
      }
    }
    else{
      if(reqtype == WriteReq){
        UpdateRWNQueue(rwnqueue,rwnhash,pagenumber);
      }
      
    }

    if(ranhash->Array[pagenumber] == NULL){
      EnRANodeQueue(ranqueue,ranhash,pagenumber);
    }
    else{
      UpdateRANQueue(ranqueue,ranhash,pagenumber);
    }
  }
}

void FreeRANodeHash(RANodeHash *hash){
  free(hash->Array);
  free(hash);
}

void FreeRWNodeHash(RWNodeHash *hash){
  free(hash->Array);
  free(hash);
}

void FreeRANodeQueue(RANodeQueue *queue){
  RANode *current = queue->MRU;

  while(current!=NULL){
    RANode *temp = current;
    current = current->Next;
    free(temp);
  }
  free(queue);
}

void FreeRWNodeQueue(RWNodeQueue *queue){
  RWNode *current = queue->MRU;

  while(current!=NULL){
    RWNode *temp = current;
    current = current->Next;
    free(temp);
  }
  free(queue);
}

void FreeGhostStatistics(GhostStatistics *ghoststatistics){
  int i,j;
  for (i = 0; i < ghoststatistics->DRAMPartitionNumber; i++){
    for (j = 0; j < ghoststatistics->NVRAMPartitionNumber; j++){
      free(ghoststatistics->HitCountArray[i][j]);
      free(ghoststatistics->HitRatioArray[i][j]);
    }
  }
  for(i = 0 ; i < ghoststatistics->DRAMPartitionNumber ; i++){
    free(ghoststatistics->HitCountArray[i]);
    free(ghoststatistics->HitRatioArray[i]);
  }
  free(ghoststatistics->HitCountArray);
  free(ghoststatistics->HitRatioArray);
  free(ghoststatistics);
}

void PrintGhostStatistics(GhostStatistics *ghoststatistics){
  int i,j;

  for (i = 0 ; i < ghoststatistics->DRAMPartitionNumber ; i++)
  {
    for(j = 0 ; j < ghoststatistics->NVRAMPartitionNumber ; j++){
      printf("DRAM current size:%d   ,   NVRAM current size:%d\n", (i+1)*(ghoststatistics->DRAMSize/ghoststatistics->DRAMPartitionNumber), (j+1)*(ghoststatistics->NVRAMSize/ghoststatistics->NVRAMPartitionNumber));
      printf("DRAM Read Hit Count:%d\n",ghoststatistics->HitCountArray[i][j][0]);
      printf("DRAM Write Hit Count:%d\n",ghoststatistics->HitCountArray[i][j][1]);
      printf("NVRAM Read Hit Count:%d\n",ghoststatistics->HitCountArray[i][j][2]);
      printf("NVRAM Write Hit Count:%d\n\n\n",ghoststatistics->HitCountArray[i][j][3]);
    }
  }
  printf("total read count                 : %d\n", ghoststatistics->total_read_access_count);
  printf("total write count                : %d\n", ghoststatistics->total_write_access_count);
}

void updateGhostStatistics(GhostStatistics *ghoststatistics,int initFlag){
  int i,j;
  double dram_read_hit_ratio;
  double dram_write_hit_ratio;
  double nvram_read_hit_ratio;
  double nvram_write_hit_ratio;
 
  for (i = 0 ; i < ghoststatistics->DRAMPartitionNumber ; i++){
    for(j = 0 ; j < ghoststatistics->NVRAMPartitionNumber ; j++){
      dram_read_hit_ratio = 0.0;
      dram_write_hit_ratio = 0.0;
      nvram_read_hit_ratio = 0.0;
      nvram_write_hit_ratio = 0.0;

      if(ghoststatistics->total_read_access_count > 0){
        dram_read_hit_ratio   = (double)ghoststatistics->HitCountArray[i][j][0] / ghoststatistics->total_read_access_count;
        nvram_read_hit_ratio  = (double)ghoststatistics->HitCountArray[i][j][2] / ghoststatistics->total_read_access_count;  
      }

      if(ghoststatistics->total_write_access_count > 0){
        dram_write_hit_ratio  = (double)ghoststatistics->HitCountArray[i][j][1] / ghoststatistics->total_write_access_count;
        nvram_write_hit_ratio = (double)ghoststatistics->HitCountArray[i][j][3] / ghoststatistics->total_write_access_count;  
      }
       
      if(initFlag == 0){    
        ghoststatistics->HitRatioArray[i][j][0] = dram_read_hit_ratio;
        ghoststatistics->HitRatioArray[i][j][2] = nvram_read_hit_ratio;
        ghoststatistics->HitRatioArray[i][j][1] = dram_write_hit_ratio;
        ghoststatistics->HitRatioArray[i][j][3] = nvram_write_hit_ratio;
      }
      else{
        ghoststatistics->HitRatioArray[i][j][0] = dram_read_hit_ratio * alpha + ghoststatistics->HitRatioArray[i][j][0] * (1 - alpha);
        ghoststatistics->HitRatioArray[i][j][2] = nvram_read_hit_ratio * alpha + ghoststatistics->HitRatioArray[i][j][2] * (1 - alpha);
        ghoststatistics->HitRatioArray[i][j][1] = dram_write_hit_ratio * alpha + ghoststatistics->HitRatioArray[i][j][1] * (1 - alpha);
        ghoststatistics->HitRatioArray[i][j][3] = nvram_write_hit_ratio * alpha + ghoststatistics->HitRatioArray[i][j][3] * (1 - alpha);
        
      }
    }
  }   
  
}
/*double* ghostbuffer(FILE *trace,int dramsize , int nvramsize){
  //printf("enter ghostbuffer\n");
  static double HitRatio[4];
  double arrivaltime;
  unsigned devno;
  unsigned long diskblkno;
  unsigned reqsize;
  unsigned reqflag;
  int i;
  
  GhostStatistics *ghoststatistics = initGhostStatistics();

  RANodeQueue *ranqueue = createRANodeQueue(dramsize + nvramsize);
  RWNodeQueue *rwnqueue = createRWNodeQueue(dramsize);

  RWNodeHash *rwnhash = createRWNodeHash(100000000);
  //printf("test\n");
  RANodeHash *ranhash = createRANodeHash(100000000);
  
  

  while(!feof(trace)){
    fscanf(trace,"%lf %u %lu %u %u", &arrivaltime, &devno, &diskblkno, &reqsize, &reqflag);
    for(i = 0 ; i < reqsize ; i+=8){
      GhostReferencePage(ranqueue,ranhash,rwnqueue,rwnhash,(diskblkno+i)/8,reqflag,ghoststatistics);
    }
    
  }
  
  HitRatio[0] = (double)ghoststatistics->DRAMReadHit / ghoststatistics->total_read_access_count; //dram read hit ratio
  HitRatio[1] = (double)ghoststatistics->DRAMWriteHit / ghoststatistics->total_write_access_count; //dram write hit ratio
  HitRatio[2] = (double)ghoststatistics->NVRAMReadHit / ghoststatistics->total_read_access_count; // nvram read hit ratio
  HitRatio[3] = (double)ghoststatistics->NVRAMWriteHit / ghoststatistics->total_write_access_count; //nvram write hit ratio
  //HitRatio[4] = 1 - HitRatio[0] - HitRatio[2] ; // read miss ratio
  //HitRatio[5] = 1 - HitRatio[1] - HitRatio[3] ; //write miss ratio

  //printf("total_read_access_count :%d\n", ghoststatistics->total_read_access_count);
  //printf("total_write_access_count : %d\n", ghoststatistics->total_write_access_count);
  //printf("DRAMReadHit : %d (%lf)\n", ghoststatistics->DRAMReadHit , HitRatio[0]);
  //printf("DRAMWriteHit : %d (%lf)\n", ghoststatistics->DRAMWriteHit , HitRatio[1]);
  //printf("NVRAMReadHit : %d (%lf)\n", ghoststatistics->NVRAMReadHit , HitRatio[2]);
  //printf("NVRAMWriteHit : %d (%lf)\n\n", ghoststatistics->NVRAMWriteHit , HitRatio[3]);

  FreeRWNodeQueue(rwnqueue);
  FreeRANodeQueue(ranqueue);
  FreeRWNodeHash(rwnhash);
  FreeRANodeHash(ranhash);
  FreeGhostStatistics(ghoststatistics);
  //printf("exit ghostbuffer\n");
  return HitRatio;
}*/