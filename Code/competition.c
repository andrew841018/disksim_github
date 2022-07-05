#include "main.h"

//double nextReplenishCreditTime = 1000.0;//units:ms
//double u1_credit = 500.0;
//double u2_credit = 500.0;
double systemtime;
double ssdReqCompleteTime;

int u1_sign = 0;
int u2_sign = 0;
int user1 = 0;
int user2 = 1;
int queueid = 0;

//adjustable parameters***********
double periodlength = 1000.0;  //**
double currentperiod = 1000.0; //**
//********************************

double ssd_scheduler(USRQUE *ssd_que , UserStatistics *u1_statistics , UserStatistics *u2_statistics){
	REQ *request = calloc(1, sizeof(REQ));
	double latancy = 0.0;
	REQ tail = ssd_que->tail->r;
	if(tail.userno == user1){
		if(send_request_by_MSQ(KEY_MSQ_DISKSIM_1, &tail, MSG_TYPE_DISKSIM_1) == -1) {
      		printf("A request not sent to MSQ in send_request_by_MSQ()");  
    	}
    	if(recv_request_by_MSQ(KEY_MSQ_DISKSIM_1, request, MSG_TYPE_DISKSIM_1_SERVED) == -1){
      		printf("A request not receive from MSQ in recv_request_by_MSQ()");      
   		}
   		latancy = request->responseTime;
    	//u1_credit -= request->responseTime;
    	evictUSRQUEUE(ssd_que);
    	if(testUserRequestInUSRQUE(user1,ssd_que) == 0 && request->reqFlag == WriteReq){
    		systemtime += DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + latancy;
    		u1_statistics->WriteReq_PerPeriod++;
    	}
    	else if(testUserRequestInUSRQUE(user1,ssd_que) == 1 && request->reqFlag == WriteReq){
    		systemtime += latancy;
    	}
    	else if(testUserRequestInUSRQUE(user1,ssd_que) == 0 && request->reqFlag == ReadReq){
    		systemtime += NVRAM_PAGE_READ_TIME + latancy + NVRAM_PAGE_WRITE_TIME ;
    		u1_statistics->ReadReq_PerPeriod++; 
    	}
	}
	else{
		if(send_request_by_MSQ(KEY_MSQ_DISKSIM_1, &tail, MSG_TYPE_DISKSIM_1) == -1) {
      		printf("A request not sent to MSQ in send_request_by_MSQ()");  
    	}
    	if(recv_request_by_MSQ(KEY_MSQ_DISKSIM_1, request, MSG_TYPE_DISKSIM_1_SERVED) == -1){
      		printf("A request not receive from MSQ in recv_request_by_MSQ()");      
   		}
   		latancy = request->responseTime;
    	//u1_credit -= request->responseTime;
    	evictUSRQUEUE(ssd_que);
    	if(testUserRequestInUSRQUE(user2,ssd_que) == 0 && request->reqFlag == WriteReq){
    		systemtime += DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + latancy;
    		u2_statistics->WriteReq_PerPeriod++;
    	}
    	else if(testUserRequestInUSRQUE(user2,ssd_que) == 1 && request->reqFlag == WriteReq){
    		systemtime += latancy;
    	}
    	else if(testUserRequestInUSRQUE(user2,ssd_que) == 0 && request->reqFlag == ReadReq){
    		systemtime += NVRAM_PAGE_READ_TIME + latancy + NVRAM_PAGE_WRITE_TIME ;
    		u2_statistics->ReadReq_PerPeriod++; 
    	}
	}
	free(request);
	if(u1_sign == 1 && testUserRequestInUSRQUE(user1,ssd_que) == 0){
		u1_sign = 0;
	}
	if(u2_sign == 1 && testUserRequestInUSRQUE(user2,ssd_que) == 0){
		u2_sign = 0;
	}
	return latancy;
}





int shutdown_SSDSimulator(){
	REQ *request = calloc(1, sizeof(REQ));
	request->arrivalTime = 0.0;   
    request->devno = 0;     
    request->diskBlkno = 0;
    request->reqSize = 8;   
    request->reqFlag = 2;   //terminate:2
    request->userno = 0;
    request->responseTime = 0.0; 
    if(send_request_by_MSQ(KEY_MSQ_DISKSIM_1, request, MSG_TYPE_DISKSIM_1) == -1) {
      	printf("A request not sent to MSQ in send_request_by_MSQ()");  
      	return -1;
    }
    free(request);
    return 0;

}


int main(int argc,char *argv[]){
	if(argc!=5){
		fprintf(stderr,"usage: %s <DRAM Size> <NVRAM Size> <user1 trace file> <user2 trace file>\n",argv[0]);
		exit(1);		
	}
		

	DRAM_size = atoll(argv[1]);
	NVRAM_size = atoll(argv[2]);
	double latancy;
	double t1,t2,t3,min;
	int decision;
	int initFlag = 0;
	int u1_tot_req , u2_tot_req;
	FILE *IOPSperperiod;
 	int periodcount = 0;
	int stopperiod = -1; // -1 = disable

	UserStatistics *u1_statistics = initUserStatistics();
	UserStatistics *u2_statistics = initUserStatistics();

	Queue *u1_DRAMQ = createQueue(DRAM_size);	
	Queue *u1_NVRAMQ = createQueue(NVRAM_size);	
	
	Hash *u1_hash = createHash(100000000);

	UserQueueArray *queuearr = createUserQueueArray(1);

  	queuearr->DRAMQueue[0] = u1_DRAMQ;
  	queuearr->NVRAMQueue[0] = u1_NVRAMQ;

  	UserHashArray *hasharr = createUserHashArray(1);
  	hasharr->array[0] = u1_hash;
  
  	//USRQUE *u1_usrque = createUSRQUEUE(); //user ssd device queue
  	//USRQUE *u2_usrque = createUSRQUEUE(); //user ssd device queue
  	USRQUE *ssd_que = createUSRQUEUE();

  	USRQUE *u1_hostque = createUSRQUEUE();
  	USRQUE *u2_hostque = createUSRQUEUE();

	FILE *u1_trace = fopen(argv[3],"r");
	if(u1_trace == NULL){
		printf("fail to open user1 trace file\n");
		exit(0);
	}
	FILE *u2_trace = fopen(argv[4],"r");
	if(u2_trace == NULL){
		printf("fail to open user2 trace file\n");
		exit(0);
	}
	
	IOPSperperiod = fopen("throughput_competition.csv","w");
	
	
	//FILE *u1_reqlog = fopen("u1_share_reqlog.txt","w");
	//FILE *u2_reqlog = fopen("u2_share_reqlog.txt","w");

	REQ r1;
	r1.userno = user1;
	r1.responseTime = 0;

	REQ r2;
	r2.userno = user2;
	r2.responseTime = 0;

	fscanf(u1_trace,"%lf %u %lu %u %u", &r1.arrivalTime, &r1.devno, &r1.diskBlkno, &r1.reqSize, &r1.reqFlag);
	r1.arrivalTime *=10; //s -> ms
	fscanf(u2_trace,"%lf %u %lu %u %u", &r2.arrivalTime, &r2.devno, &r2.diskBlkno, &r2.reqSize, &r2.reqFlag);
	r2.arrivalTime *=10; //s -> ms

	if(r1.arrivalTime <= r2.arrivalTime){
		systemtime = r1.arrivalTime;
		ssdReqCompleteTime = r1.arrivalTime;
	}
	else{
		systemtime = r2.arrivalTime;
		ssdReqCompleteTime = r2.arrivalTime;
	}

	while(!feof(u1_trace)){
		insertUSRQUEUE(u1_hostque,&r1);
		fscanf(u1_trace,"%lf %u %lu %u %u", &r1.arrivalTime, &r1.devno, &r1.diskBlkno, &r1.reqSize, &r1.reqFlag);
		r1.arrivalTime *=10; //s -> ms
	}
	while(!feof(u2_trace)){
		insertUSRQUEUE(u2_hostque,&r2);
		fscanf(u2_trace,"%lf %u %lu %u %u", &r2.arrivalTime, &r2.devno, &r2.diskBlkno, &r2.reqSize, &r2.reqFlag);
		r2.arrivalTime *=10; //s -> ms
	}
	while(1){		
		if(testUSRQUEUE(ssd_que) == 0  && testUSRQUEUE(u1_hostque) == 0 && testUSRQUEUE(u2_hostque) == 0){
			if(shutdown_SSDSimulator() == 0){
				break;
			}
		}
		
		while(1){
			t1 = -1;
			t2 = -1;
			t3 = -1;
			min = -1;
			decision = 0;
			if(u1_sign == 0 && testUSRQUEUE(u1_hostque) == 1 && u1_hostque->tail->r.arrivalTime < currentperiod){
				t1 = u1_hostque->tail->r.arrivalTime;
			}
			if(u2_sign == 0 && testUSRQUEUE(u2_hostque) == 1 && u2_hostque->tail->r.arrivalTime < currentperiod){
				t2 = u2_hostque->tail->r.arrivalTime;
			}
			if(testUSRQUEUE(ssd_que) == 1 && ssdReqCompleteTime < currentperiod){
				t3 = ssdReqCompleteTime;
			}

			if(t1!=-1){
				min = t1;
				decision = 1;
				if(t2!=-1 && t2<min){
					min = t2;
					decision = 2;
				}
				if(t3!=-1 && t3 < min){
					min = t3;
					decision = 3;
				}
			}
			else if(t2!=-1){
				min = t2;
				decision = 2;
				if(t3!=-1 && t3 < min){
					min = t3;
					decision = 3;
				}
			}
			else if(t3!=-1){
				min = t3;
				decision = 3;
			}

			if(decision == 1){
				if(systemtime < min){
					systemtime = min;
				}
				if(testUSRQUEUE(ssd_que) == 0 && ssdReqCompleteTime < min){
					ssdReqCompleteTime = min;
				}
				u1_sign = ReferencePage(queueid,queuearr,hasharr,u1_statistics,&u1_hostque->tail->r,ssd_que,&systemtime);
				
				//fprintf(u1_reqlog, "%lf %u %lu %u %u\n", r1.arrivalTime/1000, r1.devno, r1.diskBlkno, r1.reqSize, r1.reqFlag);
				//GhostReferencePage(u1_ranqueue,u1_ranhash,u1_rwnqueue,u1_rwnhash,u1_hostque->tail->r.diskBlkno/8,u1_hostque->tail->r.reqFlag,u1_ghoststatitics);
				evictUSRQUEUE(u1_hostque);
			}
			else if(decision == 2){
				if(systemtime < min){
					systemtime = min;
				}
				if(testUSRQUEUE(ssd_que) == 0 && ssdReqCompleteTime < min){
					ssdReqCompleteTime = min;
				}
				u2_sign = ReferencePage(queueid,queuearr,hasharr,u2_statistics,&u2_hostque->tail->r,ssd_que,&systemtime);
				//fprintf(u2_reqlog, "%lf %u %lu %u %u\n", r2.arrivalTime/1000, r2.devno, r2.diskBlkno, r2.reqSize, r2.reqFlag);
				//GhostReferencePage(u2_ranqueue,u2_ranhash,u2_rwnqueue,u2_rwnhash,u2_hostque->tail->r.diskBlkno/8,u2_hostque->tail->r.reqFlag,u2_ghoststatitics);
				evictUSRQUEUE(u2_hostque);
			}
			else if(decision == 3){
				latancy = ssd_scheduler(ssd_que,u1_statistics,u2_statistics);
				ssdReqCompleteTime += latancy;
			}
			else{
				break;
			}
		}
		periodcount++;
		fprintf(IOPSperperiod, "%d, %d , %lld, %lld, %lf\n" , 
			u1_statistics->ReadReq_PerPeriod + u1_statistics->WriteReq_PerPeriod , 
			u2_statistics->ReadReq_PerPeriod + u2_statistics->WriteReq_PerPeriod ,
			u1_DRAMQ->NumOfFrames,
			u1_NVRAMQ->NumOfFrames , 
			currentperiod);		


		//PrintUserStatisticsPerPeriod(user1,u1_statistics);
		//PrintUserStatisticsPerPeriod(user2,u2_statistics);
		if(stopperiod != -1 && periodcount == stopperiod){
			if(shutdown_SSDSimulator() == 0){
				break;
			}
		}	

		if(systemtime < currentperiod){
			systemtime = currentperiod;
		}
		if(ssdReqCompleteTime < currentperiod){
			ssdReqCompleteTime = currentperiod;
		}

		currentperiod += periodlength;	
		cleanUserStatistics(u1_statistics);
		cleanUserStatistics(u2_statistics);

		
	}


	//fprintf(IOPSperperiod, "%d, %d , %lld, %lld, %lf\n" , u1_statistics->ReadReq_PerPeriod + u1_statistics->WriteReq_PerPeriod , u2_statistics->ReadReq_PerPeriod + u2_statistics->WriteReq_PerPeriod ,u1_DRAMQ->NumOfFrames,u1_NVRAMQ->NumOfFrames , currentperiod);								
	PrintUserStatistics(user1,u1_DRAMQ,u1_NVRAMQ);
	fclose(u1_trace);
	fclose(u2_trace);
	fclose(IOPSperperiod);
	FreeQueue(u1_DRAMQ);
	FreeQueue(u1_NVRAMQ);
	FreeHash(u1_hash);
	freeUSRQUEUE(ssd_que);
	freeUSRQUEUE(u1_hostque);
	freeUSRQUEUE(u2_hostque);
	FreeQueueArray(queuearr);
  	FreeHashArray(hasharr);
  	FreeUserStatistics(u1_statistics);
  	FreeUserStatistics(u2_statistics);
	return 0;
}