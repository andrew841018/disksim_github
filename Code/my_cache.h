#ifndef MY_CACHE_H
#define MY_CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#include "my_ipc.h"
#include "my_structure.h"
#include "cpff_parameter.h"

#define DRAMPage  0
#define NVRAMPage 1

#define CleanPage 0
#define DirtyPage 1

#define ReadReq   1
#define WriteReq  0

extern long long int DRAM_size;
extern long long int NVRAM_size;
extern int expiration;

typedef struct evictpage
{
  unsigned dirty_bit;
  unsigned long PageNumber;
}EvictPage;

typedef struct node
{
  struct node *next;
  unsigned type;
  unsigned reference_bit;
  unsigned dirty_bit;
  int Frequency;
  int OverlookRotation;
  unsigned long PageNumber;
} Node;

typedef struct queue
{
  long long int count;
  long long int NumOfFrames;
  Node *prev;
  int read_hitcount;
  int write_hitcount;
  int misscount;
  //int evictioncount;
} Queue;

typedef struct hash
{
  long long int capacity;
  Node **array;
} Hash;

typedef struct userqueuearray
{
  int NumofUsers;
  Queue **DRAMQueue;
  Queue **NVRAMQueue;
} UserQueueArray;

typedef struct userhasharray
{
  int NumofUsers;
  Hash **array;
} UserHashArray;

typedef struct userstatistics
{
  int ReadReq_PerPeriod;
  int WriteReq_PerPeriod;
  int DRAMReadHit_PerPeriod;
  int DRAMWriteHit_PerPeriod;
  int NVRAMReadHit_PerPeriod;
  int NVRAMWriteHit_PerPeriod;
  int Evictioncount;
  double ReadReq_Predict;
  double WriteReq_Predict;
  double DRAMReadHitRatio_Predict;
  double DRAMWriteHitRatio_Predict;
  double NVRAMReadHitRatio_Predict;
  double NVRAMWriteHitRatio_Predict;
  double DirtyRatio_Predict;
  //double total_exec_time;
} UserStatistics;



UserStatistics *initUserStatistics();
Node *newNode(unsigned long pageNumber,int PageType,int ClearOrDirty);
Queue *createQueue(long long int NumOfFrames);
Hash *createHash(long long int capacity);
UserQueueArray *createUserQueueArray(int NumofUsers);
UserHashArray *createUserHashArray(int NumofUsers);
int isDRAMQueueFull(Queue *Dqueue);
int isNVRAMQueueFull(Queue *Nqueue);
Node *FindDRAMVictim(Queue *Dqueue);
Node *FindNVRAMVictim(Queue *Nqueue);
EvictPage NVRAMreplacement(int userid , UserQueueArray *queuearray , UserHashArray *hasharray);
int CheckNVRAMSpace(int userid , UserQueueArray *queuearray);
void EnNVRAMqueue(int userid , UserQueueArray *queuearray , UserHashArray *hasharray , unsigned long pageNumber , int ClearOrDirty);
EvictPage DRAMreplacement(int userid , UserQueueArray *queuearray , UserHashArray *hasharray);
int CheckDRAMSpace(int userid , UserQueueArray *queuearray);
void EnDRAMqueue(int userid , UserQueueArray *queuearray , UserHashArray *hasharray , unsigned long pageNumber);
Node *FindPreviousPage(Node *refPage);
void PageSwapping(int userid , UserQueueArray *queuearray , UserHashArray *hasharray, UserStatistics *userstatistics, Node *refPage,double *systime);
int ReferencePage(int userid , UserQueueArray *queuearray , UserHashArray *hasharray, UserStatistics *userstatistics, REQ *r , USRQUE *que , double *systime);
void FreeQueue(Queue *queue);
void FreeHash(Hash *hash);
void FreeQueueArray(UserQueueArray *queuearray);
void FreeHashArray(UserHashArray *hasharray);
void FreeUserStatistics(UserStatistics *userstatistics);
void PrintUserStatistics(int userid , Queue *Dqueue, Queue *Nqueue);
void PrintUserStatisticsPerPeriod(int userid , UserStatistics *user_statistics);
void cleanUserStatistics(UserStatistics *user_statistics);
void UpdateUserStatistics(UserStatistics *user_statistics,int initflag);
#endif