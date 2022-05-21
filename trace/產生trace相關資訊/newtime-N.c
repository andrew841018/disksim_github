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
    FILE *input, *output;
    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w");
    
    REQ *tmp;
    tmp = calloc(1, sizeof(REQ));
    int i=0,j=0;
    char buf[25];
    double newtime = 0.0;
    int changeflag=0;
    int crab=1503596;
    int add;
    printf("Analyzing.......\n");
    while (!feof(input)) {
    //for(add=0;add<crab;add++)
    //{
        
        fscanf(input, "%lf%u%lu%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag);
        newtime += 0.0001;
        if(tmp->reqFlag==3)
        {
            changeflag=1;
        }
        else
        {
            changeflag=0;
        }
        fprintf(output, "%lf\t%u\t%lu\t%u\t%u\n", tmp->arrivalTime, tmp->devno, tmp->diskBlkno, tmp->reqSize, changeflag);
        //fprintf(output, "%lf\t%u\t%lu\t%u\t%u\n", newtime, tmp->devno, tmp->diskBlkno, tmp->reqSize, tmp->reqFlag);
        //fprintf(output, "%lf\t%u\t%lu\t%u\t%u\n", tmp->arrivalTime, tmp->devno, tmp->diskBlkno, tmp->reqSize, tmp->reqFlag);
        //add++;
    }
    // while (!feof(input)) {
    // //while(i<10){
    // 	fscanf(input, "%lf%u%lu%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag);
    // 	//newtime += 0.00001;
    // 	fprintf(output, "%lf\t%u\t%lu\t%u\t%u\n", tmp->arrivalTime, tmp->devno, tmp->diskBlkno, tmp->reqSize, tmp->reqFlag);
    //     fprintf(output, "%lf\t%u\t%lu\t%u\t%u\n", tmp->arrivalTime, tmp->devno, tmp->diskBlkno, tmp->reqSize, tmp->reqFlag);
    //     i++;
    // }
    // while (j<i) {
    // //while(i<10){
    //     fscanf(input, "%lf%u%lu%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag);
    //     //newtime += 0.00001;
    //     fprintf(output, "%lf\t%u\t%lu\t%u\t%u\n", tmp->arrivalTime, tmp->devno, tmp->diskBlkno, tmp->reqSize, tmp->reqFlag);
    //     j++;
    // }
    // while (i<1) {
    // 	//fscanf(input, "%lf\n%u\n%lu\n%u\n%u\n", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag);
    //     fgets(buf,sizeof(buf),input);
    //     for(j=0;j<sizeof(buf);j++)
    //     {
    //       // if(buf[j]==' ')
    //       // {
    //       //   buf[j]=5;
    //       // }
    //        printf("%c",buf[j]);
    //     }

    //     //puts(buf);
    // 	//newtime += 0.00001;
    // 	//fprintf(output, "%lf\t%u\t%lu\t%u\t%u\n", &tmp->arrivalTime, tmp->devno, tmp->diskBlkno, tmp->reqSize, tmp->reqFlag);
    //     i++;
    // }

    printf("[Finish]\n");
    fclose(input);
    fclose(output);

    return 0;
}
