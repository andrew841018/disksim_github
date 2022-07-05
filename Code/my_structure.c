#include "my_structure.h"

void copyRequest(REQ *r, REQ *copy) {
	copy->arrivalTime = r->arrivalTime;
	copy->devno = r->devno;
	copy->diskBlkno = r->diskBlkno;
	copy->reqSize = r->reqSize;
	copy->reqFlag = r->reqFlag;
	copy->userno = r->userno;
	copy->responseTime = r->responseTime;
}


USRQUE_ITEM *newUSRQUEITEM(REQ *r){
	USRQUE_ITEM *temp = (USRQUE_ITEM*)malloc(sizeof(USRQUE_ITEM));
	temp->back = NULL;
	temp->front = NULL;
	copyRequest(r,&(temp->r));
	return temp;
}


USRQUE *createUSRQUEUE(){
	USRQUE *temp = (USRQUE*)malloc(sizeof(USRQUE));
	temp->head = NULL;
	temp->tail = NULL;

	return temp;
}

void insertUSRQUEUE(USRQUE *que , REQ *r){
	USRQUE_ITEM *head = que->head;
	USRQUE_ITEM *tail = que->tail;
	USRQUE_ITEM *temp = newUSRQUEITEM(r);

	if(head == NULL && tail == NULL){
		que->head = temp;
		que->tail = temp;
	}
	else {
		temp->back = head;
		head->front = temp;
		que->head = temp;
	}
}

void evictUSRQUEUE(USRQUE *que){
	USRQUE_ITEM *tail = que->tail;
	if(tail == NULL){
		return;
	}
	USRQUE_ITEM *front = tail->front;	
	if(front == NULL){
		free(tail);
		que->head = NULL;
		que->tail = NULL;
	}
	else{
		front->back = NULL;
		que->tail = front;
		free(tail);
	}
}

void freeUSRQUEUE(USRQUE *que){
	USRQUE_ITEM *head = que->head;
	USRQUE_ITEM *temp;
	while(head!=NULL){
		temp = head;
		head = head->back;
		free(temp);
	}
	free(que);
}

int printUSRQUEUE(USRQUE *que){
	USRQUE_ITEM *temp = que->head;
	int count = 0;
	while(temp!=NULL){
		count++;
		temp = temp->back;
	}
	return count;
}

int testUSRQUEUE(USRQUE *que){
	if(que->tail!=NULL){
		return 1;
	}
	else{
		return 0;
	}
}

int testUserRequestInUSRQUE(int userno,USRQUE *que){
	if(que->tail == NULL){
		return 0;
	}
	
	USRQUE_ITEM *temp = que->tail;
	while(temp != NULL){
		if(temp->r.userno == userno){
			return 1;
		}
		temp = temp->front;
	}
	return 0;
}
