#include "main.h"

double systemtime;
double ssdReqCompleteTime;
int u1_sign = 0;
int user1 = 0;
//adjustable parameters***********
double periodlength = 1000.0;  //**
double currentperiod = 1000.0; //**
//********************************

double ssd_scheduler(USRQUE *ssd_que , UserStatistics *u1_statistics){
	REQ *request = calloc(1, sizeof(REQ));
	double latancy = 0.0;
	REQ tail = ssd_que->tail->r;
	if(send_request_by_MSQ(KEY_MSQ_DISKSIM_1, &tail, MSG_TYPE_DISKSIM_1) == -1) {
      	printf("A request not sent to MSQ in send_request_by_MSQ()");  
    }
    if(recv_request_by_MSQ(KEY_MSQ_DISKSIM_1, request, MSG_TYPE_DISKSIM_1_SERVED) == -1){
      	printf("A request not receive from MSQ in recv_request_by_MSQ()");      
   	}
   	latancy = request->responseTime;
    evictUSRQUEUE(ssd_que);
    if(testUSRQUEUE(ssd_que) == 0 && request->reqFlag == WriteReq){
    	systemtime += DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + latancy;
    	u1_statistics->WriteReq_PerPeriod++;
    }
    else if(testUSRQUEUE(ssd_que) == 1 && request->reqFlag == WriteReq){
    	systemtime += latancy;
    }
    else if(testUSRQUEUE(ssd_que) == 0 && request->reqFlag == ReadReq){
    	systemtime += NVRAM_PAGE_READ_TIME + latancy + NVRAM_PAGE_WRITE_TIME ;
    	u1_statistics->ReadReq_PerPeriod++; 
    }
	free(request);
	if(u1_sign == 1 && testUSRQUEUE(ssd_que) == 0){
		u1_sign = 0;
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
	if(argc!=4){
		fprintf(stderr,"usage: %s <DRAM Size> <NVRAM Size> <user1 trace file>\n",argv[0]);
		exit(1);		
	}
	
	char fname[80] = "throughput_runalnoe_";
	char fnametype[3]="csv";
	DRAM_size = atoll(argv[1]);
	NVRAM_size = atoll(argv[2]);
	
	int u1_flag = 0;
	double latancy;
	double t1,t2,min;
	int decision;
	int periodcount = 0;
	int stopperiod = -1; // -1 = disable

	FILE *u1_trace = fopen(argv[3],"r");
	if(u1_trace == NULL){
		printf("fail to open user1 trace file\n");
		exit(0);
	}

	//strncat(fname,argv[3]+31,strlen(argv[3])-5);
	//strcat(fname,fnametype);
	FILE *IOPSperperiod = fopen("throughput_runalnoe_fin2.csv","w");
	if(IOPSperperiod == NULL){
		printf("fail to open IOPSperperiod trace file\n");
		exit(0);
	}

	UserStatistics *u1_statistics = initUserStatistics();

	Queue *u1_DRAMQ = createQueue(DRAM_size);	
	Queue *u1_NVRAMQ = createQueue(NVRAM_size);	
	
	Hash *u1_hash = createHash(100000000);

	UserQueueArray *queuearr = createUserQueueArray(1);

  	queuearr->DRAMQueue[0] = u1_DRAMQ;
  	queuearr->NVRAMQueue[0] = u1_NVRAMQ;

  	UserHashArray *hasharr = createUserHashArray(1);
  	hasharr->array[0] = u1_hash;
  
  	USRQUE *ssd_que = createUSRQUEUE();
  	USRQUE *u1_hostque = createUSRQUEUE();


	//FILE *u1_reqlog = fopen("u1_alone_reqlog.txt","w");

	REQ r1;
	r1.userno = 0;
	r1.responseTime = 0;

	fscanf(u1_trace,"%lf %u %lu %u %u", &r1.arrivalTime, &r1.devno, &r1.diskBlkno, &r1.reqSize, &r1.reqFlag);
	r1.arrivalTime *=10; //s -> ms

	systemtime = r1.arrivalTime;
	ssdReqCompleteTime = r1.arrivalTime;
	
	while(1){	
	//printf("123\n");	
		if(feof(u1_trace)  && testUSRQUEUE(ssd_que) == 0  && testUSRQUEUE(u1_hostque) == 0){
			if(shutdown_SSDSimulator() == 0){
				break;
			}
		}
		//printf("456\n");
		while(r1.arrivalTime < currentperiod && !feof(u1_trace)){
			insertUSRQUEUE(u1_hostque,&r1);
			fscanf(u1_trace,"%lf %u %lu %u %u", &r1.arrivalTime, &r1.devno, &r1.diskBlkno, &r1.reqSize, &r1.reqFlag);
			r1.arrivalTime *=10; //s -> ms
		}
		//printf("789\n");
		while(1){
			t1 = -1;
			t2 = -1;
			min = -1;
			decision = 0;
			if(u1_sign == 0 && testUSRQUEUE(u1_hostque) == 1 && u1_hostque->tail->r.arrivalTime < currentperiod){
				t1 = u1_hostque->tail->r.arrivalTime;
			}
			if(testUSRQUEUE(ssd_que) == 1 && ssdReqCompleteTime < currentperiod){
				t2 = ssdReqCompleteTime;
			}
			if(t1!=-1){
				min = t1;
				decision = 1;
				if(t2!=-1 && t2<min){
					min = t2;
					decision = 2;
				}
			}
			else if(t2!=-1){
				min = t2;
				decision = 2;
			}
			if(decision == 1){
				if(systemtime < min){
					systemtime = min;
				}
				if(testUSRQUEUE(ssd_que) == 0 && ssdReqCompleteTime < min){
					ssdReqCompleteTime = min;
				}
				u1_sign = ReferencePage(user1,queuearr,hasharr,u1_statistics,&u1_hostque->tail->r,ssd_que,&systemtime);			
				evictUSRQUEUE(u1_hostque);
			}
			else if(decision == 2){
				latancy = ssd_scheduler(ssd_que,u1_statistics);
				ssdReqCompleteTime += latancy;
			}
			else{
				break;
			}
		}
		periodcount++;
		//printf("012\n");
		fprintf(IOPSperperiod, "%d , %lld, %lld, %lf\n" , 
			u1_statistics->ReadReq_PerPeriod + u1_statistics->WriteReq_PerPeriod ,
			u1_DRAMQ->NumOfFrames ,
			u1_NVRAMQ->NumOfFrames,
			currentperiod);		

		//PrintUserStatisticsPerPeriod(user1,u1_statistics);
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
		
	}

	//fprintf(IOPSperperiod, "%d , %lld, %lld, %lf\n" , u1_statistics->ReadReq_PerPeriod + u1_statistics->WriteReq_PerPeriod ,u1_DRAMQ->NumOfFrames ,u1_NVRAMQ->NumOfFrames, currentperiod);		
										
	PrintUserStatistics(user1,u1_DRAMQ,u1_NVRAMQ);
	fclose(u1_trace);
	fclose(IOPSperperiod);
	FreeQueue(u1_DRAMQ);
	FreeQueue(u1_NVRAMQ);
	FreeHash(u1_hash);
	freeUSRQUEUE(u1_hostque);
	freeUSRQUEUE(ssd_que);
	FreeQueueArray(queuearr);
  	FreeHashArray(hasharr);
  	FreeUserStatistics(u1_statistics);
	return 0;
}