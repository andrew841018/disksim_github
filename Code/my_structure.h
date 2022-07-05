#ifndef MY_STRUCTURE_H
#define MY_STRUCTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct req {
    double arrivalTime;   //抵達時間
    unsigned devno;     //裝置編號(預設為0)
    unsigned long diskBlkno;//Block編號(根據Disksim格式)
    unsigned reqSize;   //Block連續數量(至少為1)
    unsigned reqFlag;   //terminate:2 讀:1;寫:0
    unsigned userno;
    double responseTime;  //反應時間(初始為0)
  } REQ;

  typedef struct usrque_item {
  	struct usrque_item *back;
  	REQ r;
  	struct usrque_item *front;
  } USRQUE_ITEM;

  typedef struct usrque {
  	USRQUE_ITEM *head;
  	USRQUE_ITEM *tail;
  } USRQUE;


  void copyRequest(REQ *r, REQ *copy);
  USRQUE_ITEM *newUSRQUEITEM(REQ *r);
  USRQUE *createUSRQUEUE();
  void insertUSRQUEUE(USRQUE *que , REQ *r);
  void evictUSRQUEUE(USRQUE *que);
  void freeUSRQUEUE(USRQUE *que);
  int printUSRQUEUE(USRQUE *que);
  int testUSRQUEUE(USRQUE *que);
  int testUserRequestInUSRQUE(int userno,USRQUE *que);

  #endif