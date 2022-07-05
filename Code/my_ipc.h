#ifndef MY_IPC_H
#define MY_IPC_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "my_structure.h"


typedef struct msgbuf {
    long msgType;
    REQ req;
  } MSGBUF;

#define MSG_SIZE sizeof(MSGBUF)-sizeof(long)

  	/*CREATE MESSAGE QUEUE*/
	int create_message_queue(key_t key, int msqflag);
	/*REMOVE MESSAGE QUEUE*/
	int remove_message_queue(key_t key, struct msqid_ds *msqds);
	/*SEND REQUEST BY MESSAGE QUEUE*/
	int send_request_by_MSQ(key_t key, REQ *r, long msgtype);
	/*RECEIVE REQUEST BY MESSAGE QUEUE*/
	int recv_request_by_MSQ(key_t key, REQ *r, long msgtype);





#endif
