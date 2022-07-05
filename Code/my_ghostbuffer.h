#ifndef MY_GHOSTBUFFER_H
#define MY_GHOSTBUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "cpff_parameter.h"

#define ReadReq   1
#define WriteReq  0

extern int OverlookThreshold;


typedef struct ghoststatistics
{
	int total_read_access_count;
	int total_write_access_count;
	int ***HitCountArray;
	int DRAMPartitionNumber; 
	int NVRAMPartitionNumber;
	int DRAMSize;
	int NVRAMSize;
	double ***HitRatioArray;
} GhostStatistics;

typedef struct RecentlyAccessNode
{
	struct RecentlyAccessNode *Next;
	struct RecentlyAccessNode *Prev;
	unsigned long PageNumber;
	int PartitionCount;
} RANode;

typedef struct RecentlyWriteNode
{
	struct RecentlyWriteNode *Next;
	struct RecentlyWriteNode *Prev;
	unsigned long PageNumber;
	int Frequency;
	int PartitionCount;
	int OverlookCount;
} RWNode;

typedef struct RecentlyAccessNodeQueue
{
	int Count;
	int Capacity;
	RANode *MRU;
	RANode *LRU;
} RANodeQueue;

typedef struct RecentlyWriteNodeQueue
{
	int Count;
	int Capacity;
	RWNode *MRU;
	RWNode *LRU;
} RWNodeQueue;

typedef struct RecentlyAccessNodeHash
{
	long long int Capacity;
	RANode **Array;
} RANodeHash;

typedef struct RecentlyWriteNodeHash
{
	long long int Capacity;
	RWNode **Array;
} RWNodeHash;



GhostStatistics *initGhostStatistics(int DRAMSize, int DRAMPartitionNumber , int NVRAMSize , int NVRAMPartitionNumber);
RANode *newRANode(unsigned long pagenumber);
RANodeQueue *createRANodeQueue(int capacity);
RWNodeQueue *createRWNodeQueue(int capacity);
RANodeHash *createRANodeHash(long long int capacity);
RWNodeHash *createRWNodeHash(long long int capacity);
int isRANodeQueueFull(RANodeQueue *queue);
int isRWNodeQueueFull(RWNodeQueue *queue);
void RANodeQueueReplacement(RANodeQueue *queue ,RANodeHash *hash);
void RWNodeQueueReplacement(RWNodeQueue *queue ,RWNodeHash *hash);
void EnRANodeQueue(RANodeQueue *queue , RANodeHash *hash , unsigned long pagenumber);
void EnRWNodeQueue(RWNodeQueue *queue , RWNodeHash *hash , unsigned long pagenumber);
void InitialAllNodes(RANodeQueue *ranqueue , RWNodeQueue *rwnqueue);
void InitialHitCountArray(GhostStatistics *ghoststatistics);
void HitCountArrayUpdate(RANodeQueue *ranqueue , RANodeHash *ranhash , RWNodeQueue *rwnqueue , RWNodeHash *rwnhash , unsigned long pagenumber , unsigned reqtype ,GhostStatistics *ghoststatistics);
void UpdateRANQueue(RANodeQueue *queue , RANodeHash *hash , unsigned long pagenumber);
void UpdateRWNQueue(RWNodeQueue *queue , RWNodeHash *hash , unsigned long pagenumber);
void GhostReferencePage(RANodeQueue *ranqueue , RANodeHash *ranhash , RWNodeQueue *rwnqueue , RWNodeHash *rwnhash , unsigned long pagenumber , unsigned reqtype , GhostStatistics *ghoststatistics);
void FreeRANodeHash(RANodeHash *hash);
void FreeRWNodeHash(RWNodeHash *hash);
void FreeRANodeQueue(RANodeQueue *queue);
void FreeRWNodeQueue(RWNodeQueue *queue);
void FreeGhostStatistics(GhostStatistics *ghoststatistics);
void PrintGhostStatistics(GhostStatistics *ghoststatistics);
void updateGhostStatistics(GhostStatistics *ghoststatistics,int initFlag);
//double* ghostbuffer(FILE *trace,int dramsize , int nvramsize);





#endif