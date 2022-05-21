#include <stdlib.h>
#include <stdio.h>

typedef struct req {
		double arrivalTime;		//抵達時間
		unsigned devno;			//裝置編號(預設為0)
		unsigned long diskBlkno;//Block編號(根據Disksim格式)
		unsigned reqSize;		//Block連續數量(至少為1)
		unsigned reqFlag;		//讀:1;寫:0
		unsigned userno;		//使用者編號(1~n)       
		double responseTime;	//反應時間(初始為0)
	} REQ;

int main(int argc, char *argv[]){
	if (argc != 3) {
    	fprintf(stderr, "usage: %s <Input trace file> <Output trace file>\n", argv[0]);
    	exit(1);
    }
    printf("Analyzing.......1");

    FILE *input, *output;
    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w");
    REQ *tmp;
    tmp = calloc(1, sizeof(REQ));

    double write_count=0;
    double read_count=0;
    double all_reqSize=0;
    double all_RreqSize=0;
    double all_WreqSize=0;
    double reqcount=0;
    double all_inter=0;
    double pre_arrivalTime=0;
    int seqcount=0;
    int seqRcount=0;
    int seqWcount=0;
    int lastseq=-9;
    int lastRseq=-9;
    int lastWseq=-9;

    printf("Analyzing.......");
    while (!feof(input)) {
    	fscanf(input, "%lf %u %lu %u %u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag);
    	reqcount++;
        if(tmp->reqFlag == 0)
        {
            write_count ++;
            all_WreqSize += tmp->reqSize/8;
        }
        else if(tmp->reqFlag == 1)
        {
            read_count++;
            all_RreqSize += tmp->reqSize/8;
        }
        all_reqSize += tmp->reqSize/8;
        all_inter += tmp->arrivalTime-pre_arrivalTime;
        pre_arrivalTime = tmp->arrivalTime;

        if(tmp->diskBlkno == lastseq+8 )
        {
            seqcount++;
            lastseq = tmp->diskBlkno;
        }
        else
        {
            lastseq = tmp->diskBlkno;
        }

        if(tmp->reqFlag==0)
        {
            if(tmp->diskBlkno == lastWseq+8 )
            {
                seqWcount++;
                lastWseq = tmp->diskBlkno;
            }
            else
            {
                lastWseq = tmp->diskBlkno;
            }
        }
        else if(tmp->reqFlag==1)
        {
            if(tmp->diskBlkno == lastRseq+8 )
            {
                seqRcount++;
                lastRseq = tmp->diskBlkno;
            }
            else
            {
                lastRseq = tmp->diskBlkno;
            }
        }
    }
    fprintf(output, "req_count=%lf\n", reqcount);
    fprintf(output, "-----------------------------\n");
    fprintf(output, "write_count=%lf\n", write_count);
    fprintf(output, "read_count=%lf\n", read_count);
    fprintf(output, "write_ratio=%lf\n", write_count/(read_count+write_count));
    fprintf(output, "read_ratio=%lf\n", read_count/(read_count+write_count));
    fprintf(output, "-----------------------------\n");
    fprintf(output, "avg_reqSize=%lf\n", all_reqSize/reqcount);
    fprintf(output, "avg_Read_reqSize=%lf\n", all_RreqSize/read_count);
    fprintf(output, "avg_Write_reqSize=%lf\n", all_WreqSize/write_count);
    fprintf(output, "-----------------------------\n");
    fprintf(output, "avg_inter_arrival_time=%lf\n", all_inter/reqcount);

    fprintf(output, "-----------------------------\n");

    fprintf(output, "-----------------------------\n");
    fprintf(output, "seqcount = %d \n", seqcount);
    fprintf(output, "seq_ratio = %lf \n", (double)seqcount/reqcount);

    fprintf(output, "seqRcount = %d \n", seqRcount);
    fprintf(output, "seqR_ratio = %lf \n", (double)seqRcount/read_count);

    fprintf(output, "seqWcount = %d \n", seqWcount);
    fprintf(output, "seqW_ratio = %lf \n", (double)seqWcount/write_count);
    fprintf(output, "-----------------------------\n");


    printf("[Finish]\n");
    fclose(input);
    fclose(output);

    return 0;
}
