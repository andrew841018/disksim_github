#include "main.h"
#include <math.h>

//double nextReplenishCreditTime = 1000.0;//units:ms
//double u1_credit = 500.0;
//double u2_credit = 500.0;
double systemtime;
double ssdReqCompleteTime;

int u1_sign = 0;
int u2_sign = 0;
int numberofuser = 2;
int user1 = 0;
int user2 = 1;

int thr_cal_method = 1;

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

//version 1
/*void adjustcachespace(UserQueueArray *queuearray , UserStatistics *u1_statistics , UserStatistics *u2_statistics , GhostStatistics *u1_ghoststatitics , GhostStatistics *u2_ghoststatitics){
	if((u1_statistics->ReadReq_PerPeriod == 0 && u1_statistics->WriteReq_PerPeriod == 0) || (u2_statistics->ReadReq_PerPeriod == 0 && u2_statistics->WriteReq_PerPeriod == 0)){
		return;
	}
	double u1_throughput , u2_throughput;
	double u1_throughput_predict , u2_throughput_predict;
	double u1_executiontime , u2_executiontime ;
	double dram_readhit_ratio;
	double dram_writehit_ratio;
	double nvram_readhit_ratio;
	double nvram_writehit_ratio;
	double writemiss_ratio;
	double readmiss_ratio; 
	double u1_dirty_ratio;
	double u2_dirty_ratio;
	int misscount;
	int i,j;
	int u1_dram_space_allocate = -1;
	int u1_nvram_space_allocate = -1;
	double min_throughput_diff;
	double throughput_diff;
	FILE *debug = fopen("debug.txt","w");
	//calculate user1 exection time and throughput
	dram_readhit_ratio = 0;
	dram_writehit_ratio = 0;
	nvram_readhit_ratio = 0;
	nvram_writehit_ratio = 0;
	writemiss_ratio = 0;
	readmiss_ratio = 0;
	u1_dirty_ratio = 0;
	if(u1_statistics->ReadReq_PerPeriod != 0){
		dram_readhit_ratio   = (double)u1_statistics->DRAMReadHit_PerPeriod / u1_statistics->ReadReq_PerPeriod;
		nvram_readhit_ratio  = (double)u1_statistics->NVRAMReadHit_PerPeriod / u1_statistics->ReadReq_PerPeriod;
		readmiss_ratio       = 1.0 - dram_readhit_ratio - nvram_readhit_ratio;
	}
	if(u1_statistics->WriteReq_PerPeriod != 0){
		dram_writehit_ratio  = (double)u1_statistics->DRAMWriteHit_PerPeriod / u1_statistics->WriteReq_PerPeriod;
		nvram_writehit_ratio = (double)u1_statistics->NVRAMWriteHit_PerPeriod / u1_statistics->WriteReq_PerPeriod;
		writemiss_ratio      = 1.0 - dram_writehit_ratio - nvram_writehit_ratio;
	}
	misscount = u1_statistics->ReadReq_PerPeriod + u1_statistics->WriteReq_PerPeriod - u1_statistics->DRAMReadHit_PerPeriod - u1_statistics->NVRAMReadHit_PerPeriod - u1_statistics->DRAMWriteHit_PerPeriod - u1_statistics->NVRAMWriteHit_PerPeriod;
	if(misscount != 0){
		u1_dirty_ratio = (double)u1_statistics->Evictioncount / misscount;
	}

	u1_executiontime = u1_statistics->ReadReq_Predict * dram_readhit_ratio * DRAM_PAGE_ACCESS_TIME +
					   u1_statistics->ReadReq_Predict * nvram_readhit_ratio * NVRAM_PAGE_READ_TIME  + 
					   u1_statistics->ReadReq_Predict * readmiss_ratio * (SSD_READ_PRE_CHAREG_VALUE + NVRAM_PAGE_WRITE_TIME + NVRAM_PAGE_READ_TIME + u1_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE)) + 
					   u1_statistics->WriteReq_Predict * dram_writehit_ratio * DRAM_PAGE_ACCESS_TIME + 
					   u1_statistics->WriteReq_Predict * nvram_writehit_ratio * (2*DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_READ_TIME + NVRAM_PAGE_WRITE_TIME)+
					   u1_statistics->WriteReq_Predict * writemiss_ratio * (DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + u1_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE));

	u1_throughput = (u1_statistics->ReadReq_Predict + u1_statistics->WriteReq_Predict) / u1_executiontime;
	fprintf(debug, "u1 : \n" );		
	fprintf(debug, "dram_readhit_ratio : %lf \n" ,dram_readhit_ratio);		
	fprintf(debug, "nvram_readhit_ratio : %lf \n" ,nvram_readhit_ratio);	
	fprintf(debug, "readmiss_ratio : %lf \n" ,readmiss_ratio);	
	fprintf(debug, "dram_writehit_ratio : %lf \n" ,dram_writehit_ratio);		
	fprintf(debug, "nvram_writehit_ratio : %lf \n" ,nvram_writehit_ratio);	
	fprintf(debug, "writemiss_ratio : %lf \n" ,writemiss_ratio);	
	fprintf(debug, "u1_dirty_ratio : %lf \n" ,u1_dirty_ratio);	
	fprintf(debug, "u1_executiontime : %lf \n" ,u1_executiontime);	
	fprintf(debug, "u1_throughput : %lf \n\n" ,u1_throughput);	

	//calculate user2 exection time and throughput
	dram_readhit_ratio = 0;
	dram_writehit_ratio = 0;
	nvram_readhit_ratio = 0;
	nvram_writehit_ratio = 0;
	writemiss_ratio = 0;
	readmiss_ratio = 0;
	u2_dirty_ratio = 0;
	if(u2_statistics->ReadReq_PerPeriod != 0){
		dram_readhit_ratio   = (double)u2_statistics->DRAMReadHit_PerPeriod / u2_statistics->ReadReq_PerPeriod;
		nvram_readhit_ratio  = (double)u2_statistics->NVRAMReadHit_PerPeriod / u2_statistics->ReadReq_PerPeriod;
		readmiss_ratio       = 1.0 - dram_readhit_ratio - nvram_readhit_ratio;
	}
	if(u2_statistics->WriteReq_PerPeriod != 0){
		dram_writehit_ratio  = (double)u2_statistics->DRAMWriteHit_PerPeriod / u2_statistics->WriteReq_PerPeriod;
		nvram_writehit_ratio = (double)u2_statistics->NVRAMWriteHit_PerPeriod / u2_statistics->WriteReq_PerPeriod;
		writemiss_ratio      = 1.0 - dram_writehit_ratio - nvram_writehit_ratio;
	}
	misscount = u2_statistics->ReadReq_PerPeriod + u2_statistics->WriteReq_PerPeriod - u2_statistics->DRAMReadHit_PerPeriod - u2_statistics->NVRAMReadHit_PerPeriod - u2_statistics->DRAMWriteHit_PerPeriod - u2_statistics->NVRAMWriteHit_PerPeriod;
	if(misscount != 0){
		u2_dirty_ratio =  (double)u2_statistics->Evictioncount / misscount;
	}

	u2_executiontime = u2_statistics->ReadReq_Predict * dram_readhit_ratio * DRAM_PAGE_ACCESS_TIME +
					   u2_statistics->ReadReq_Predict * nvram_readhit_ratio * NVRAM_PAGE_READ_TIME  + 
					   u2_statistics->ReadReq_Predict * readmiss_ratio * (SSD_READ_PRE_CHAREG_VALUE + NVRAM_PAGE_WRITE_TIME + NVRAM_PAGE_READ_TIME + u2_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE)) + 
					   u2_statistics->WriteReq_Predict * dram_writehit_ratio * DRAM_PAGE_ACCESS_TIME + 
					   u2_statistics->WriteReq_Predict * nvram_writehit_ratio * (2*DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_READ_TIME + NVRAM_PAGE_WRITE_TIME)+
					   u2_statistics->WriteReq_Predict * writemiss_ratio * (DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + u2_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE));
	
	u2_throughput = (u2_statistics->ReadReq_Predict + u2_statistics->WriteReq_Predict) / u2_executiontime;
	fprintf(debug, "u2 : \n" );		
	fprintf(debug, "dram_readhit_ratio : %lf \n" ,dram_readhit_ratio);		
	fprintf(debug, "nvram_readhit_ratio : %lf \n" ,nvram_readhit_ratio);	
	fprintf(debug, "readmiss_ratio : %lf \n" ,readmiss_ratio);	
	fprintf(debug, "dram_writehit_ratio : %lf \n" ,dram_writehit_ratio);		
	fprintf(debug, "nvram_writehit_ratio : %lf \n" ,nvram_writehit_ratio);	
	fprintf(debug, "writemiss_ratio : %lf \n" ,writemiss_ratio);	
	fprintf(debug, "u2_dirty_ratio : %lf \n" ,u2_dirty_ratio);	
	fprintf(debug, "u1_executiontime : %lf \n" ,u2_executiontime);	
	fprintf(debug, "u1_throughput : %lf \n\n" ,u2_throughput);	

	if(u1_throughput >= u2_throughput){
		min_throughput_diff = u1_throughput - u2_throughput;
	}
	else{
		min_throughput_diff = u2_throughput - u1_throughput;
	}
	fprintf(debug, "min_throughput_diff : %lf \n\n", min_throughput_diff);	

	for(i = 1 ; i < u1_ghoststatitics->DRAMPartitionNumber ; i++){
		for(j = 1 ; j < u1_ghoststatitics->NVRAMPartitionNumber ; j++){
			fprintf(debug, "i:%d j:%d \n" ,i,j);	
			dram_readhit_ratio = 0;
			dram_writehit_ratio = 0;
			nvram_readhit_ratio = 0;
			nvram_writehit_ratio = 0;
			writemiss_ratio = 0;
			readmiss_ratio = 0;

			if(u1_ghoststatitics->total_read_access_count != 0){
				dram_readhit_ratio   = (double)u1_ghoststatitics->HitCountArray[i-1][j-1][0] / u1_ghoststatitics->total_read_access_count;
				nvram_readhit_ratio  = (double)u1_ghoststatitics->HitCountArray[i-1][j-1][2] / u1_ghoststatitics->total_read_access_count;
				readmiss_ratio       = 1.0 - dram_readhit_ratio - nvram_readhit_ratio;
			}
			if(u1_ghoststatitics->total_write_access_count != 0){
				dram_writehit_ratio  = (double)u1_ghoststatitics->HitCountArray[i-1][j-1][1] / u1_ghoststatitics->total_write_access_count;
				nvram_writehit_ratio = (double)u1_ghoststatitics->HitCountArray[i-1][j-1][3] / u1_ghoststatitics->total_write_access_count;
				writemiss_ratio      = 1.0 - dram_writehit_ratio - nvram_writehit_ratio;
			}
						
			u1_executiontime = u1_statistics->ReadReq_Predict * dram_readhit_ratio * DRAM_PAGE_ACCESS_TIME +
					   u1_statistics->ReadReq_Predict * nvram_readhit_ratio * NVRAM_PAGE_READ_TIME  + 
					   u1_statistics->ReadReq_Predict * readmiss_ratio * (SSD_READ_PRE_CHAREG_VALUE + NVRAM_PAGE_WRITE_TIME + NVRAM_PAGE_READ_TIME + u1_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE)) + 
					   u1_statistics->WriteReq_Predict * dram_writehit_ratio * DRAM_PAGE_ACCESS_TIME + 
					   u1_statistics->WriteReq_Predict * nvram_writehit_ratio * (2*DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_READ_TIME + NVRAM_PAGE_WRITE_TIME)+
					   u1_statistics->WriteReq_Predict * writemiss_ratio * (DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + u1_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE));

			u1_throughput_predict = (u1_statistics->ReadReq_Predict + u1_statistics->WriteReq_Predict) / u1_executiontime;
			fprintf(debug, "u1 : \n" );		
			fprintf(debug, "dram_readhit_ratio : %lf \n" ,dram_readhit_ratio);		
			fprintf(debug, "nvram_readhit_ratio : %lf \n" ,nvram_readhit_ratio);	
			fprintf(debug, "readmiss_ratio : %lf \n" ,readmiss_ratio);	
			fprintf(debug, "dram_writehit_ratio : %lf \n" ,dram_writehit_ratio);		
			fprintf(debug, "nvram_writehit_ratio : %lf \n" ,nvram_writehit_ratio);	
			fprintf(debug, "writemiss_ratio : %lf \n" ,writemiss_ratio);	
			fprintf(debug, "u1_dirty_ratio : %lf \n" ,u1_dirty_ratio);	
			fprintf(debug, "u1_executiontime : %lf \n" ,u1_executiontime);	
			fprintf(debug, "u1_throughput_predict : %lf \n" ,u1_throughput_predict);	

			dram_readhit_ratio = 0;
			dram_writehit_ratio = 0;
			nvram_readhit_ratio = 0;
			nvram_writehit_ratio = 0;
			writemiss_ratio = 0;
			readmiss_ratio = 0;

			if(u2_ghoststatitics->total_read_access_count != 0){
				dram_readhit_ratio   = (double)u2_ghoststatitics->HitCountArray[u2_ghoststatitics->DRAMPartitionNumber-i-1][u2_ghoststatitics->NVRAMPartitionNumber-j-1][0] / u2_ghoststatitics->total_read_access_count;
				nvram_readhit_ratio  = (double)u2_ghoststatitics->HitCountArray[u2_ghoststatitics->DRAMPartitionNumber-i-1][u2_ghoststatitics->NVRAMPartitionNumber-j-1][2] / u2_ghoststatitics->total_read_access_count;
				readmiss_ratio       = 1.0 - dram_readhit_ratio - nvram_readhit_ratio;
			}
			if(u2_ghoststatitics->total_write_access_count != 0){
				dram_writehit_ratio  = (double)u2_ghoststatitics->HitCountArray[u2_ghoststatitics->DRAMPartitionNumber-i-1][u2_ghoststatitics->NVRAMPartitionNumber-j-1][1] / u2_ghoststatitics->total_write_access_count;
				nvram_writehit_ratio = (double)u2_ghoststatitics->HitCountArray[u2_ghoststatitics->DRAMPartitionNumber-i-1][u2_ghoststatitics->NVRAMPartitionNumber-j-1][3] / u2_ghoststatitics->total_write_access_count;
				writemiss_ratio      = 1.0 - dram_writehit_ratio - nvram_writehit_ratio;
			}

			u2_executiontime = u2_statistics->ReadReq_Predict * dram_readhit_ratio * DRAM_PAGE_ACCESS_TIME +
					   u2_statistics->ReadReq_Predict * nvram_readhit_ratio * NVRAM_PAGE_READ_TIME  + 
					   u2_statistics->ReadReq_Predict * readmiss_ratio * (SSD_READ_PRE_CHAREG_VALUE + NVRAM_PAGE_WRITE_TIME + NVRAM_PAGE_READ_TIME + u2_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE)) + 
					   u2_statistics->WriteReq_Predict * dram_writehit_ratio * DRAM_PAGE_ACCESS_TIME + 
					   u2_statistics->WriteReq_Predict * nvram_writehit_ratio * (2*DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_READ_TIME + NVRAM_PAGE_WRITE_TIME)+
					   u2_statistics->WriteReq_Predict * writemiss_ratio * (DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + u2_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE));

			u2_throughput_predict = (u2_statistics->ReadReq_Predict + u2_statistics->WriteReq_Predict) / u2_executiontime;
			fprintf(debug, "u2 : \n" );		
			fprintf(debug, "dram_readhit_ratio : %lf \n" ,dram_readhit_ratio);		
			fprintf(debug, "nvram_readhit_ratio : %lf \n" ,nvram_readhit_ratio);	
			fprintf(debug, "readmiss_ratio : %lf \n" ,readmiss_ratio);	
			fprintf(debug, "dram_writehit_ratio : %lf \n" ,dram_writehit_ratio);		
			fprintf(debug, "nvram_writehit_ratio : %lf \n" ,nvram_writehit_ratio);	
			fprintf(debug, "writemiss_ratio : %lf \n" ,writemiss_ratio);	
			fprintf(debug, "u2_dirty_ratio : %lf \n" ,u2_dirty_ratio);	
			fprintf(debug, "u2_executiontime : %lf \n" ,u2_executiontime);	
			fprintf(debug, "u2_throughput_predict : %lf \n" ,u2_throughput_predict);	

			if(u1_throughput_predict >= u2_throughput_predict){
				throughput_diff = u1_throughput_predict - u2_throughput_predict;
			}
			else{
				throughput_diff = u2_throughput_predict - u1_throughput_predict;
			}
			fprintf(debug, "throughput_diff : %lf \n\n" ,throughput_diff);	

			if(throughput_diff < min_throughput_diff){
				u1_dram_space_allocate = i;
				u1_nvram_space_allocate = j;
				min_throughput_diff = throughput_diff;
			}
		}
	}

	fprintf(debug, "u1_dram_space_allocate : %d \n" ,u1_dram_space_allocate);	
	fprintf(debug, "u1_nvram_space_allocate : %d \n" ,u1_nvram_space_allocate);	
	fprintf(debug, "min_throughput_diff : %lf \n" ,min_throughput_diff);	
	if(u1_dram_space_allocate != -1 && u1_nvram_space_allocate != -1){
		Queue *u1_DRAMQ = queuearray->DRAMQueue[0];
  		Queue *u1_NVRAMQ = queuearray->NVRAMQueue[0];
  		Queue *u2_DRAMQ = queuearray->DRAMQueue[1];
  		Queue *u2_NVRAMQ = queuearray->NVRAMQueue[1];

  		u1_DRAMQ->NumOfFrames = (long long int)((u1_ghoststatitics->DRAMSize/u1_ghoststatitics->DRAMPartitionNumber) * u1_dram_space_allocate);
  		u1_NVRAMQ->NumOfFrames = (long long int)((u1_ghoststatitics->NVRAMSize/u1_ghoststatitics->NVRAMPartitionNumber) * u1_nvram_space_allocate);
  		u2_DRAMQ->NumOfFrames = (long long int)((u1_ghoststatitics->DRAMSize/u1_ghoststatitics->DRAMPartitionNumber) * (u1_ghoststatitics->DRAMPartitionNumber - u1_dram_space_allocate));
  		u2_NVRAMQ->NumOfFrames = (long long int)((u1_ghoststatitics->NVRAMSize/u1_ghoststatitics->NVRAMPartitionNumber) * (u1_ghoststatitics->NVRAMPartitionNumber - u1_nvram_space_allocate));
	}
	fclose(debug);
}*/


//version2
void adjustcachespace(UserQueueArray *queuearray , UserStatistics *u1_statistics , UserStatistics *u2_statistics , GhostStatistics *u1_ghoststatitics , GhostStatistics *u2_ghoststatitics){
	double u1_runalone_throughput , u2_runalone_throughput;
	double u1_executiontime , u2_executiontime;
	double u1_runalone_executiontime,u2_runalone_executiontime;
	double u1_throughput , u2_throughput;
	double dram_readhit_ratio;
	double dram_writehit_ratio;
	double nvram_readhit_ratio;
	double nvram_writehit_ratio;
	double writemiss_ratio;
	double readmiss_ratio; 
	double u1_dirty_ratio;
	double u2_dirty_ratio;
	int misscount;
	int i , j;
	double min_degradation_diff,degradation_diff;
	double u1_degradation , u2_degradation;
	int u1_dram_space_allocate = -1;
	int u1_nvram_space_allocate = -1;
	int length = (u1_ghoststatitics->DRAMPartitionNumber-1) * (u1_ghoststatitics->NVRAMPartitionNumber-1);
	int *dram_pos = calloc(length , sizeof(int));
	int *nvram_pos = calloc(length , sizeof(int));
	//double u1_degra[49] = {0.0};
	//double u2_degra[49] = {0.0};
	double *degra_diff = calloc(length , sizeof(double));
	double *u1_thr = calloc(length , sizeof(double));
	double *u2_thr = calloc(length , sizeof(double));	
	int k = 0;
	int flag = 0;

	// calculate user 1 runalone throughput
	dram_readhit_ratio = u1_ghoststatitics->HitRatioArray[u1_ghoststatitics->DRAMPartitionNumber-1][u1_ghoststatitics->NVRAMPartitionNumber-1][0];
	dram_writehit_ratio = u1_ghoststatitics->HitRatioArray[u1_ghoststatitics->DRAMPartitionNumber-1][u1_ghoststatitics->NVRAMPartitionNumber-1][1];
	nvram_readhit_ratio = u1_ghoststatitics->HitRatioArray[u1_ghoststatitics->DRAMPartitionNumber-1][u1_ghoststatitics->NVRAMPartitionNumber-1][2];
	nvram_writehit_ratio = u1_ghoststatitics->HitRatioArray[u1_ghoststatitics->DRAMPartitionNumber-1][u1_ghoststatitics->NVRAMPartitionNumber-1][3];
	writemiss_ratio = 1.0 - dram_writehit_ratio - nvram_writehit_ratio;
	readmiss_ratio = 1.0 - dram_readhit_ratio - nvram_readhit_ratio;
	u1_dirty_ratio = u1_statistics->DirtyRatio_Predict;	

	u1_runalone_executiontime = u1_statistics->ReadReq_Predict * dram_readhit_ratio * DRAM_PAGE_ACCESS_TIME +
					   u1_statistics->ReadReq_Predict * nvram_readhit_ratio * NVRAM_PAGE_READ_TIME  + 
					   u1_statistics->ReadReq_Predict * readmiss_ratio * (SSD_READ_PRE_CHAREG_VALUE + NVRAM_PAGE_WRITE_TIME + NVRAM_PAGE_READ_TIME + u1_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE)) + 
					   u1_statistics->WriteReq_Predict * dram_writehit_ratio * DRAM_PAGE_ACCESS_TIME + 
					   u1_statistics->WriteReq_Predict * nvram_writehit_ratio * (2*DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_READ_TIME + NVRAM_PAGE_WRITE_TIME)+
					   u1_statistics->WriteReq_Predict * writemiss_ratio * (DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + u1_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE));

	u1_runalone_throughput = (u1_statistics->ReadReq_Predict + u1_statistics->WriteReq_Predict) / u1_runalone_executiontime;

	// calculate user 2 runalone throughput
	dram_readhit_ratio = u2_ghoststatitics->HitRatioArray[u2_ghoststatitics->DRAMPartitionNumber-1][u2_ghoststatitics->NVRAMPartitionNumber-1][0];
	dram_writehit_ratio = u2_ghoststatitics->HitRatioArray[u2_ghoststatitics->DRAMPartitionNumber-1][u2_ghoststatitics->NVRAMPartitionNumber-1][1];
	nvram_readhit_ratio = u2_ghoststatitics->HitRatioArray[u2_ghoststatitics->DRAMPartitionNumber-1][u2_ghoststatitics->NVRAMPartitionNumber-1][2];
	nvram_writehit_ratio = u2_ghoststatitics->HitRatioArray[u2_ghoststatitics->DRAMPartitionNumber-1][u2_ghoststatitics->NVRAMPartitionNumber-1][3];
	writemiss_ratio = 1.0 - dram_writehit_ratio - nvram_writehit_ratio;
	readmiss_ratio = 1.0 - dram_readhit_ratio - nvram_readhit_ratio;
	u2_dirty_ratio = u2_statistics->DirtyRatio_Predict;
	
	u2_runalone_executiontime = u2_statistics->ReadReq_Predict * dram_readhit_ratio * DRAM_PAGE_ACCESS_TIME +
					   u2_statistics->ReadReq_Predict * nvram_readhit_ratio * NVRAM_PAGE_READ_TIME  + 
					   u2_statistics->ReadReq_Predict * readmiss_ratio * (SSD_READ_PRE_CHAREG_VALUE + NVRAM_PAGE_WRITE_TIME + NVRAM_PAGE_READ_TIME + u2_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE)) + 
					   u2_statistics->WriteReq_Predict * dram_writehit_ratio * DRAM_PAGE_ACCESS_TIME + 
					   u2_statistics->WriteReq_Predict * nvram_writehit_ratio * (2*DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_READ_TIME + NVRAM_PAGE_WRITE_TIME)+
					   u2_statistics->WriteReq_Predict * writemiss_ratio * (DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + u2_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE));
	
	u2_runalone_throughput = (u2_statistics->ReadReq_Predict + u2_statistics->WriteReq_Predict) / u2_runalone_executiontime;

	//user 1 w/o adjustment
	/*dram_readhit_ratio = 0;
	dram_writehit_ratio = 0;
	nvram_readhit_ratio = 0;
	nvram_writehit_ratio = 0;
	writemiss_ratio = 0;
	readmiss_ratio = 0;
	
	if(u1_statistics->ReadReq_PerPeriod != 0){
		dram_readhit_ratio   = (double)u1_statistics->DRAMReadHit_PerPeriod / u1_statistics->ReadReq_PerPeriod;
		nvram_readhit_ratio  = (double)u1_statistics->NVRAMReadHit_PerPeriod / u1_statistics->ReadReq_PerPeriod;
	}
	readmiss_ratio       = 1.0 - dram_readhit_ratio - nvram_readhit_ratio;
	if(u1_statistics->WriteReq_PerPeriod != 0){
		dram_writehit_ratio  = (double)u1_statistics->DRAMWriteHit_PerPeriod / u1_statistics->WriteReq_PerPeriod;
		nvram_writehit_ratio = (double)u1_statistics->NVRAMWriteHit_PerPeriod / u1_statistics->WriteReq_PerPeriod;
	}
	writemiss_ratio      = 1.0 - dram_writehit_ratio - nvram_writehit_ratio;
	u1_executiontime = u1_statistics->ReadReq_Predict * dram_readhit_ratio * DRAM_PAGE_ACCESS_TIME +
					   u1_statistics->ReadReq_Predict * nvram_readhit_ratio * NVRAM_PAGE_READ_TIME  + 
					   u1_statistics->ReadReq_Predict * readmiss_ratio * (SSD_READ_PRE_CHAREG_VALUE + NVRAM_PAGE_WRITE_TIME + NVRAM_PAGE_READ_TIME + u1_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE)) + 
					   u1_statistics->WriteReq_Predict * dram_writehit_ratio * DRAM_PAGE_ACCESS_TIME + 
					   u1_statistics->WriteReq_Predict * nvram_writehit_ratio * (2*DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_READ_TIME + NVRAM_PAGE_WRITE_TIME)+
					   u1_statistics->WriteReq_Predict * writemiss_ratio * (DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + u1_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE));


*/

	//user 2 w/o adjustment
/*	dram_readhit_ratio = 0;
	dram_writehit_ratio = 0;
	nvram_readhit_ratio = 0;
	nvram_writehit_ratio = 0;
	writemiss_ratio = 0;
	readmiss_ratio = 0;	

	if(u2_statistics->ReadReq_PerPeriod != 0){
		dram_readhit_ratio   = (double)u2_statistics->DRAMReadHit_PerPeriod / u2_statistics->ReadReq_PerPeriod;
		nvram_readhit_ratio  = (double)u2_statistics->NVRAMReadHit_PerPeriod / u2_statistics->ReadReq_PerPeriod;
	}
	readmiss_ratio       = 1.0 - dram_readhit_ratio - nvram_readhit_ratio;
	if(u2_statistics->WriteReq_PerPeriod != 0){
		dram_writehit_ratio  = (double)u2_statistics->DRAMWriteHit_PerPeriod / u2_statistics->WriteReq_PerPeriod;
		nvram_writehit_ratio = (double)u2_statistics->NVRAMWriteHit_PerPeriod / u2_statistics->WriteReq_PerPeriod;
	}
	writemiss_ratio      = 1.0 - dram_writehit_ratio - nvram_writehit_ratio;

	u2_executiontime = u2_statistics->ReadReq_Predict * dram_readhit_ratio * DRAM_PAGE_ACCESS_TIME +
					   u2_statistics->ReadReq_Predict * nvram_readhit_ratio * NVRAM_PAGE_READ_TIME  + 
					   u2_statistics->ReadReq_Predict * readmiss_ratio * (SSD_READ_PRE_CHAREG_VALUE + NVRAM_PAGE_WRITE_TIME + NVRAM_PAGE_READ_TIME + u2_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE)) + 
					   u2_statistics->WriteReq_Predict * dram_writehit_ratio * DRAM_PAGE_ACCESS_TIME + 
					   u2_statistics->WriteReq_Predict * nvram_writehit_ratio * (2*DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_READ_TIME + NVRAM_PAGE_WRITE_TIME)+
					   u2_statistics->WriteReq_Predict * writemiss_ratio * (DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + u2_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE));

	if(thr_cal_method == 0){
		u1_throughput = (u1_statistics->ReadReq_Predict + u1_statistics->WriteReq_Predict) / (u1_executiontime + u2_executiontime);
		u2_throughput = (u2_statistics->ReadReq_Predict + u2_statistics->WriteReq_Predict) / (u1_executiontime + u2_executiontime);
	}
	else{
		u1_throughput = (u1_statistics->ReadReq_Predict + u1_statistics->WriteReq_Predict) / u1_executiontime;
		u2_throughput = (u2_statistics->ReadReq_Predict + u2_statistics->WriteReq_Predict) / u2_executiontime;
	}
	
	u1_degradation = u1_runalone_throughput / u1_throughput;
	u2_degradation = u2_runalone_throughput / u2_throughput;
	//u1_degradation = u1_executiontime / u1_runalone_executiontime;
	//u2_degradation = u2_executiontime / u2_runalone_executiontime;

	if(u1_degradation>=u2_degradation){
		min_degradation_diff = u1_degradation - u2_degradation;
	}
	else{
		min_degradation_diff = u2_degradation - u1_degradation;
	}
*/
	//printf("u1 throughput:%lf   degra:%lf\n", u1_throughput,u1_degradation);
	//printf("u2 throughput:%lf   degra:%lf\n", u2_throughput,u2_degradation);
	//printf("min_degradation_diff:%lf\n\n", min_degradation_diff);

	for(i = 1 ; i < u1_ghoststatitics->DRAMPartitionNumber ; i++){
		for(j = 1 ; j < u1_ghoststatitics->NVRAMPartitionNumber ; j++){
			//printf("i : %d , j : %d\n",i,j);
			dram_readhit_ratio = u1_ghoststatitics->HitRatioArray[i-1][j-1][0];
			dram_writehit_ratio = u1_ghoststatitics->HitRatioArray[i-1][j-1][1];
			nvram_readhit_ratio = u1_ghoststatitics->HitRatioArray[i-1][j-1][2];
			nvram_writehit_ratio = u1_ghoststatitics->HitRatioArray[i-1][j-1][3];
			writemiss_ratio = 1.0 - dram_writehit_ratio - nvram_writehit_ratio;
			readmiss_ratio = 1.0 - dram_readhit_ratio - nvram_readhit_ratio;

			u1_executiontime = u1_statistics->ReadReq_Predict * dram_readhit_ratio * DRAM_PAGE_ACCESS_TIME +
					   u1_statistics->ReadReq_Predict * nvram_readhit_ratio * NVRAM_PAGE_READ_TIME  + 
					   u1_statistics->ReadReq_Predict * readmiss_ratio * (SSD_READ_PRE_CHAREG_VALUE + NVRAM_PAGE_WRITE_TIME + NVRAM_PAGE_READ_TIME + u1_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE)) + 
					   u1_statistics->WriteReq_Predict * dram_writehit_ratio * DRAM_PAGE_ACCESS_TIME + 
					   u1_statistics->WriteReq_Predict * nvram_writehit_ratio * (2*DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_READ_TIME + NVRAM_PAGE_WRITE_TIME)+
					   u1_statistics->WriteReq_Predict * writemiss_ratio * (DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + u1_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE));
			

			dram_readhit_ratio = u2_ghoststatitics->HitRatioArray[u2_ghoststatitics->DRAMPartitionNumber-i-1][u2_ghoststatitics->NVRAMPartitionNumber-j-1][0];
			dram_writehit_ratio = u2_ghoststatitics->HitRatioArray[u2_ghoststatitics->DRAMPartitionNumber-i-1][u2_ghoststatitics->NVRAMPartitionNumber-j-1][1];
			nvram_readhit_ratio = u2_ghoststatitics->HitRatioArray[u2_ghoststatitics->DRAMPartitionNumber-i-1][u2_ghoststatitics->NVRAMPartitionNumber-j-1][2];
			nvram_writehit_ratio = u2_ghoststatitics->HitRatioArray[u2_ghoststatitics->DRAMPartitionNumber-i-1][u2_ghoststatitics->NVRAMPartitionNumber-j-1][3];
			writemiss_ratio = 1.0 - dram_writehit_ratio - nvram_writehit_ratio;
			readmiss_ratio = 1.0 - dram_readhit_ratio - nvram_readhit_ratio;

			u2_executiontime = u2_statistics->ReadReq_Predict * dram_readhit_ratio * DRAM_PAGE_ACCESS_TIME +
					   u2_statistics->ReadReq_Predict * nvram_readhit_ratio * NVRAM_PAGE_READ_TIME  + 
					   u2_statistics->ReadReq_Predict * readmiss_ratio * (SSD_READ_PRE_CHAREG_VALUE + NVRAM_PAGE_WRITE_TIME + NVRAM_PAGE_READ_TIME + u2_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE)) + 
					   u2_statistics->WriteReq_Predict * dram_writehit_ratio * DRAM_PAGE_ACCESS_TIME + 
					   u2_statistics->WriteReq_Predict * nvram_writehit_ratio * (2*DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_READ_TIME + NVRAM_PAGE_WRITE_TIME)+
					   u2_statistics->WriteReq_Predict * writemiss_ratio * (DRAM_PAGE_ACCESS_TIME + DRAM_PAGE_ACCESS_TIME + NVRAM_PAGE_WRITE_TIME + u2_dirty_ratio * (NVRAM_PAGE_READ_TIME + SSD_WRITE_PRE_CHAREG_VALUE));


			if(thr_cal_method == 0){
				u1_throughput = (u1_statistics->ReadReq_Predict + u1_statistics->WriteReq_Predict) / (u1_executiontime + u2_executiontime);
				u2_throughput = (u2_statistics->ReadReq_Predict + u2_statistics->WriteReq_Predict) / (u1_executiontime + u2_executiontime);
			}
			else{
				u1_throughput = (u1_statistics->ReadReq_Predict + u1_statistics->WriteReq_Predict) / u1_executiontime;
				u2_throughput = (u2_statistics->ReadReq_Predict + u2_statistics->WriteReq_Predict) / u2_executiontime;
			}

			u1_degradation = u1_runalone_throughput / u1_throughput;
			u2_degradation = u2_runalone_throughput / u2_throughput;

			//u1_degradation = u1_executiontime / u1_runalone_executiontime;
			//u2_degradation = u2_executiontime / u2_runalone_executiontime;
			if(u1_degradation >= u2_degradation){
				degradation_diff = u1_degradation - u2_degradation;
			}
			else{
				degradation_diff = u2_degradation - u1_degradation;
			}

			//printf("u1 throughput:%lf   degra:%lf\n", u1_throughput,u1_degradation);
			//printf("u2 throughput:%lf   degra:%lf\n", u2_throughput,u2_degradation);
			

			dram_pos[k] = i;
			nvram_pos[k] = j;
			//u1_degra[k] = u1_degradation;
			//u2_degra[k] = u2_degradation;
			degra_diff[k] = degradation_diff;
			u1_thr[k] = u1_throughput;
			u2_thr[k] = u2_throughput;
			k++;

			/*if(min_degradation_diff > degradation_diff){				
				min_degradation_diff = degradation_diff;
				u1_dram_space_allocate = i;
				u1_nvram_space_allocate = j;
				flag = 1;				
			}*/
			//printf("min_degradation_diff:%lf\n\n", min_degradation_diff);
			/*else if(flag == 1 && min_degradation_diff == degradation_diff){
				int d1_u1_dram_alloc = u1_dram_space_allocate;
				int d1_u1_nvram_alloc = u1_nvram_space_allocate;
				int d1_u2_dram_alloc = u2_ghoststatitics->DRAMPartitionNumber - u1_dram_space_allocate;
				int d1_u2_nvram_alloc = u2_ghoststatitics->NVRAMPartitionNumber - u1_nvram_space_allocate;
				double d1 = (double)((d1_u1_dram_alloc-d1_u2_dram_alloc) * (d1_u1_dram_alloc-d1_u2_dram_alloc) + (d1_u1_nvram_alloc-d1_u2_nvram_alloc) * (d1_u1_nvram_alloc-d1_u2_nvram_alloc));
				double distance1 = sqrt(d1);

				int d2_u1_dram_alloc = i;
				int d2_u1_nvram_alloc = j;
				int d2_u2_dram_alloc = u2_ghoststatitics->DRAMPartitionNumber - i;
				int d2_u2_nvram_alloc = u2_ghoststatitics->NVRAMPartitionNumber - j;
				double d2 = (double)((d2_u1_dram_alloc - d2_u2_dram_alloc)*(d2_u1_dram_alloc - d2_u2_dram_alloc) + (d2_u1_nvram_alloc - d2_u2_nvram_alloc) * (d2_u1_nvram_alloc - d2_u2_nvram_alloc));
				double distance2 = sqrt(d2);

				if(distance2 < distance1){
					u1_dram_space_allocate = i;
					u1_nvram_space_allocate = j;
				}
			}*/
			
		}
	}

	//printf("u1_dram_space_allocate:%d ,u1_nvram_space_allocate:%d \n", u1_dram_space_allocate,u1_nvram_space_allocate);

	int min_dram_pos = dram_pos[0];
	int min_nvram_pos = nvram_pos[0];
	double min_degra_diff = degra_diff[0];
	//double min_u1_degra = u1_degra[0];
	//double min_u2_degra = u2_degra[0];
	double min_u1_thr = u1_thr[0];
	double min_u2_thr = u2_thr[0];
	for (i = 1; i < length; i++)
	{
		if(degra_diff[i] < min_degra_diff){
			min_dram_pos = dram_pos[i];
			min_nvram_pos = nvram_pos[i];
			min_degra_diff = degra_diff[i];
			//min_u1_degra = u1_degra[i];
			//min_u2_degra = u2_degra[i];
			min_u1_thr = u1_thr[i];
			min_u2_thr = u2_thr[i];
		}
	}

	
	for (i = 0; i < length; i++){
		if((degra_diff[i] - min_degra_diff)<=0.1 && (u1_thr[i] + u2_thr[i]) > (min_u1_thr + min_u2_thr)){
			min_dram_pos = dram_pos[i];
			min_nvram_pos = nvram_pos[i];
			//min_u1_degra = u1_degra[i];
			//min_u2_degra = u2_degra[i];
			min_u1_thr = u1_thr[i];
			min_u2_thr = u2_thr[i];
		}
		/*else if((degra_diff[i] - min_degra_diff)<=0.1 && (u1_thr[i] + u2_thr[i]) == (min_u1_thr + min_u2_thr) && dram_pos[i] > min_dram_pos){
			min_dram_pos = dram_pos[i];
			min_nvram_pos = nvram_pos[i];
			//min_u1_degra = u1_degra[i];
			//min_u2_degra = u2_degra[i];
			min_u1_thr = u1_thr[i];
			min_u2_thr = u2_thr[i];
		}*/
	}

	
	Queue *u1_DRAMQ = queuearray->DRAMQueue[0];
  	Queue *u1_NVRAMQ = queuearray->NVRAMQueue[0];
  	Queue *u2_DRAMQ = queuearray->DRAMQueue[1];
  	Queue *u2_NVRAMQ = queuearray->NVRAMQueue[1];
	//if(u1_dram_space_allocate != -1 && u1_nvram_space_allocate != -1){
  		u1_DRAMQ->NumOfFrames = (long long int)((u1_ghoststatitics->DRAMSize/u1_ghoststatitics->DRAMPartitionNumber) * min_dram_pos);
  		u1_NVRAMQ->NumOfFrames = (long long int)((u1_ghoststatitics->NVRAMSize/u1_ghoststatitics->NVRAMPartitionNumber) * min_nvram_pos);
  		u2_DRAMQ->NumOfFrames = (long long int)((u1_ghoststatitics->DRAMSize/u1_ghoststatitics->DRAMPartitionNumber) * (u1_ghoststatitics->DRAMPartitionNumber - min_dram_pos));
  		u2_NVRAMQ->NumOfFrames = (long long int)((u1_ghoststatitics->NVRAMSize/u1_ghoststatitics->NVRAMPartitionNumber) * (u1_ghoststatitics->NVRAMPartitionNumber - min_nvram_pos));
	//}


  	free(dram_pos);
  	free(nvram_pos);
  	free(degra_diff);
  	free(u1_thr);
  	free(u2_thr);
}


int main(int argc,char *argv[]){
	if(argc!=6){
		fprintf(stderr,"usage: %s <DRAM Size> <NVRAM Size> <user1 trace file> <user2 trace file> <0(static partition) or 1(dynamic partition)>\n",argv[0]);
		exit(1);		
	}
		

	DRAM_size = atoll(argv[1]);
	NVRAM_size = atoll(argv[2]);
	int partitionscheme = atoi(argv[5]);
	int DRAMPartitionNumber = 32;
	int NVRAMPartitionNumber = 32;
	double latancy;
	double t1,t2,t3,min;
	int decision;
	int initFlag = 0;
	int u1_tot_req , u2_tot_req;
	FILE *IOPSperperiod;
	int periodcount = 0;
	int stopperiod = -1; // -1 = disable
 
	GhostStatistics *u1_ghoststatitics = initGhostStatistics(DRAM_size,DRAMPartitionNumber,NVRAM_size,NVRAMPartitionNumber);
	RANodeQueue *u1_ranqueue = createRANodeQueue(DRAM_size + NVRAM_size);
	RWNodeQueue *u1_rwnqueue = createRWNodeQueue(DRAM_size);
	RANodeHash *u1_ranhash = createRANodeHash(100000000);
	RWNodeHash *u1_rwnhash = createRWNodeHash(100000000);

	GhostStatistics *u2_ghoststatitics = initGhostStatistics(DRAM_size,DRAMPartitionNumber,NVRAM_size,NVRAMPartitionNumber);
	RANodeQueue *u2_ranqueue = createRANodeQueue(DRAM_size + NVRAM_size);
	RWNodeQueue *u2_rwnqueue = createRWNodeQueue(DRAM_size);
	RANodeHash *u2_ranhash = createRANodeHash(100000000);
	RWNodeHash *u2_rwnhash = createRWNodeHash(100000000);

	UserStatistics *u1_statistics = initUserStatistics();
	UserStatistics *u2_statistics = initUserStatistics();

	Queue *u1_DRAMQ = createQueue(DRAM_size/2);	
	Queue *u1_NVRAMQ = createQueue(NVRAM_size/2);	
	Queue *u2_DRAMQ = createQueue(DRAM_size/2);	
	Queue *u2_NVRAMQ = createQueue(NVRAM_size/2);	
	
	Hash *u1_hash = createHash(100000000);
	Hash *u2_hash = createHash(100000000);

	UserQueueArray *queuearr = createUserQueueArray(2);

  	queuearr->DRAMQueue[0] = u1_DRAMQ;
  	queuearr->NVRAMQueue[0] = u1_NVRAMQ;
  	queuearr->DRAMQueue[1] = u2_DRAMQ;
  	queuearr->NVRAMQueue[1] = u2_NVRAMQ;

  	UserHashArray *hasharr = createUserHashArray(2);
  	hasharr->array[0] = u1_hash;
  	hasharr->array[1] = u2_hash;
  
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
	
	if(partitionscheme == 0){
		IOPSperperiod = fopen("throughput_share_static.csv","w");
	}
	else{
		IOPSperperiod = fopen("throughput_share_dynamic.csv","w");
	}
	//FILE *u1_reqlog = fopen("u1_share_reqlog.txt","w");
	//FILE *u2_reqlog = fopen("u2_share_reqlog.txt","w");

	REQ r1;
	r1.userno = 0;
	r1.responseTime = 0;

	REQ r2;
	r2.userno = 1;
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

	printf("stage 1\n");

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
				u1_sign = ReferencePage(user1,queuearr,hasharr,u1_statistics,&u1_hostque->tail->r,ssd_que,&systemtime);
				
				//fprintf(u1_reqlog, "%lf %u %lu %u %u\n", r1.arrivalTime/1000, r1.devno, r1.diskBlkno, r1.reqSize, r1.reqFlag);
				GhostReferencePage(u1_ranqueue,u1_ranhash,u1_rwnqueue,u1_rwnhash,u1_hostque->tail->r.diskBlkno/8,u1_hostque->tail->r.reqFlag,u1_ghoststatitics);
				evictUSRQUEUE(u1_hostque);
			}
			else if(decision == 2){
				if(systemtime < min){
					systemtime = min;
				}
				if(testUSRQUEUE(ssd_que) == 0 && ssdReqCompleteTime < min){
					ssdReqCompleteTime = min;
				}
				u2_sign = ReferencePage(user2,queuearr,hasharr,u2_statistics,&u2_hostque->tail->r,ssd_que,&systemtime);
				//fprintf(u2_reqlog, "%lf %u %lu %u %u\n", r2.arrivalTime/1000, r2.devno, r2.diskBlkno, r2.reqSize, r2.reqFlag);
				GhostReferencePage(u2_ranqueue,u2_ranhash,u2_rwnqueue,u2_rwnhash,u2_hostque->tail->r.diskBlkno/8,u2_hostque->tail->r.reqFlag,u2_ghoststatitics);
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
		fprintf(IOPSperperiod, "%d , %lld, %lld, %d, %lld, %lld, %lf\n" , 
			u1_statistics->ReadReq_PerPeriod + u1_statistics->WriteReq_PerPeriod ,
			u1_DRAMQ->NumOfFrames ,
			u1_NVRAMQ->NumOfFrames, 
			u2_statistics->ReadReq_PerPeriod + u2_statistics->WriteReq_PerPeriod ,
			u2_DRAMQ->NumOfFrames ,
			u2_NVRAMQ->NumOfFrames , 
			currentperiod);		

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

		if(partitionscheme == 1){
			UpdateUserStatistics(u1_statistics,initFlag);
			UpdateUserStatistics(u2_statistics,initFlag);
			updateGhostStatistics(u1_ghoststatitics,initFlag);
			updateGhostStatistics(u2_ghoststatitics,initFlag);
			initFlag = 1;
			
			u1_tot_req = u1_statistics->ReadReq_Predict + u1_statistics->WriteReq_Predict;
		    u2_tot_req = u2_statistics->ReadReq_Predict + u2_statistics->WriteReq_Predict;

		    if(u1_tot_req > 0 && u2_tot_req > 0){
			    adjustcachespace(queuearr,u1_statistics,u2_statistics,u1_ghoststatitics,u2_ghoststatitics);
		    }
		     
		}

		currentperiod += periodlength;	
		cleanUserStatistics(u1_statistics);
		cleanUserStatistics(u2_statistics);
		InitialHitCountArray(u1_ghoststatitics);
		InitialHitCountArray(u2_ghoststatitics);

	}


	//fprintf(IOPSperperiod, "%d , %lld, %lld, %d, %lld, %lld, %lf\n" , u1_statistics->ReadReq_PerPeriod + u1_statistics->WriteReq_PerPeriod ,u1_DRAMQ->NumOfFrames ,u1_NVRAMQ->NumOfFrames, u2_statistics->ReadReq_PerPeriod + u2_statistics->WriteReq_PerPeriod,u2_DRAMQ->NumOfFrames ,u2_NVRAMQ->NumOfFrames , currentperiod);		
								
	PrintUserStatistics(user1,u1_DRAMQ,u1_NVRAMQ);
	PrintUserStatistics(user2,u2_DRAMQ,u2_NVRAMQ);
	fclose(u1_trace);
	fclose(u2_trace);
	fclose(IOPSperperiod);
	FreeQueue(u1_DRAMQ);
	FreeQueue(u1_NVRAMQ);
	FreeHash(u1_hash);
	FreeQueue(u2_DRAMQ);
	FreeQueue(u2_NVRAMQ);
	FreeHash(u2_hash);
	freeUSRQUEUE(ssd_que);
	freeUSRQUEUE(u1_hostque);
	freeUSRQUEUE(u2_hostque);
	FreeQueueArray(queuearr);
  	FreeHashArray(hasharr);
  	FreeUserStatistics(u1_statistics);
  	FreeUserStatistics(u2_statistics);
  	FreeRWNodeQueue(u1_rwnqueue);
	FreeRANodeQueue(u1_ranqueue);
	FreeRWNodeHash(u1_rwnhash);
	FreeRANodeHash(u1_ranhash);
	FreeGhostStatistics(u1_ghoststatitics);
	FreeRWNodeQueue(u2_rwnqueue);
	FreeRANodeQueue(u2_ranqueue);
	FreeRWNodeHash(u2_rwnhash);
	FreeRANodeHash(u2_ranhash);
	FreeGhostStatistics(u2_ghoststatitics);
	return 0;
}