#include "my_ipc.h"


int remove_message_queue(key_t key, struct msqid_ds *msqds) {
  int msqid;
  msqid = msgget((key_t)key, IPC_CREAT);
  if(msgctl(msqid, IPC_RMID, msqds) == 0) {
    //PrintDebug(msqid, "Remove message queue:");
    return msqid;
  }
  else {
    return -1;
  }
}

int create_message_queue(key_t key, int msqflag) {
  int msqid = -1;
  msqid = msgget(key, msqflag | 0666);
  if(msqid >= 0) {
    //PrintDebug(msqid, "Message Queue Identifier:");
    return msqid;
  }
  else {
    return -1;
  }
}

int send_request_by_MSQ(key_t key, REQ *r, long msgtype) {
  int msqid;
  msqid = msgget((key_t)key, IPC_CREAT);

  MSGBUF msg;
  msg.msgType = msgtype;
  msg.req.arrivalTime = r->arrivalTime;
  msg.req.devno = r->devno;
  msg.req.diskBlkno = r->diskBlkno;
  msg.req.reqSize = r->reqSize;
  msg.req.reqFlag = r->reqFlag;
  msg.req.userno = r->userno;
  msg.req.responseTime = r->responseTime;
  if(msgsnd(msqid, (void *)&msg, MSG_SIZE, 0) == 0) {
    //PrintDebug(msqid, "A request sent to MSQ:");
    return 0;
  }
  else {
    return -1;
  }
}

int recv_request_by_MSQ(key_t key, REQ *r, long msgtype) { //msgtype can be used for receiving specific user message 
  int msqid;
  msqid = msgget((key_t)key, IPC_CREAT);

  MSGBUF buf;
  if(msgrcv(msqid, (void *)&buf, MSG_SIZE, msgtype, 0) >= 0) {/*return bytes sent*/
    //PrintDebug(msqid, "A request received from MSQ:");
    r->arrivalTime = buf.req.arrivalTime;
    r->devno = buf.req.devno;
    r->diskBlkno = buf.req.diskBlkno;
    r->reqSize = buf.req.reqSize;
    r->reqFlag = buf.req.reqFlag;
    r->userno = buf.req.userno;
    r->responseTime = buf.req.responseTime;
    return 0;
  }
  else {
    return -1;
  }
}
