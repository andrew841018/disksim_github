#include <stdlib.h>
#include <stdio.h>
#include <math.h>
typedef struct req {
		double arrivalTime;		//抵達時間
		unsigned devno;			//裝置編號(預設為0)
		unsigned long diskBlkno;//Block編號(根據Disksim格式)
		unsigned reqSize;		//Block連續數量(至少為1)
		unsigned reqFlag;		//讀:1;寫:0
		unsigned userno;		//使用者編號(1~n)
		double responseTime;	//反應時間(初始為0)
	} REQ;
int table[10000000];
int ctable[10000000];
int rst=0;
int tablesize=0;
int fsize=0;
int rssize=0;
int i=0,j=0,k=0,sw=0;
int over=0,unique=0,seq=0;
int total_req=0;
double avg_over=0.0,avg_seq=0.0,avg_size=0.0;
// int test[]={1,6,3,1,3,6,6};
int compare1 (const void * a, const void * b)
{ 
  return ( *(int*)a - *(int*)b );
}
// void sort_FB(int point,int* arr)
// {
//   int i;
//   qsort (arr, point+1, sizeof(int), compare1);
// }


void findrp(int arr[],int n)
{
    int i=0,j=0;
    for(i=0;i<n;i++)
    {
        int index = arr[i] % n;
        arr[index]+= n;
    }

    for(j=0;j<n;j++)
    {
        if((arr[j]/n) >= 2)
        {
            over++;
            //printf("%d\n",j);
        }
    }
}
// int finduni(int arr[],int n)
// {
//     qsort (arr, sizeof(arr)+n, sizeof(int), compare1);
//     int res=0;
//     int i = 0;
//     for(i=0;i<n;i++)
//     {
//       while(i<n-1 && arr[i]==arr[i+1])
//       {
//           i++;
//       }
//       res++;
//     }
//     return res;
// }

void findseq(int arr[],int n)
{
    int i=0,j=0,sw=0;
    for(i=0;i<n;i++)
    {
        // for(j=i+1;j<64;j++)
        // {
        //     compare=arr[i]+8*j;
        //     if(compare==arr[j])
        //     {
        //      seq++;
        //      //i=i+1;
        //     }
        //     else
        //     {
        //         break;
        //     }
        // }
        if(arr[i+1]==arr[i]+8)
        {
            sw=1;
            seq++;
        }
        else
        {
            // if(sw==1)
            // {
            //   seq++;
            // }
            // sw=0;
        }
        // if(sw==1)
        // {
        //     seq++;
        // }
    }

}


int main(int argc, char *argv[]){
	if (argc != 2) {
    	fprintf(stderr, "usage: %s <Input trace file> <Output trace file>\n", argv[0]);
    	exit(1);
    }
    FILE *input, *output;
    input = fopen(argv[1], "r");
    //output = fopen(argv[2], "w");

    REQ *tmp;
    tmp = calloc(1, sizeof(REQ));

    double newtime = 0.0;
    // printf("Analyzing.......\n");
    printf("--------------------------------------------------\n");
    while (!feof(input)) {
    	fscanf(input, "%lf%u%lu%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag);
  
    	//newtime += 0.00001;
        table[tablesize]=tmp->diskBlkno;
        ctable[tablesize]=tmp->diskBlkno;
        rst+=tmp->reqSize;
        tablesize++;
        fsize++;
        total_req++;
    	//fprintf(output, "%lf\t%u\t%lu\t%u\t%u\n", tmp->arrivalTime, tmp->devno, tmp->diskBlkno, tmp->reqSize, tmp->reqFlag);
    }
    // int test[]={1,6,3,3,6,6,7,7,8,9,1};
    // int arrsize = sizeof(test)/sizeof(test[0]);
    // findrp(test,arrsize);
    

    findrp(table,tablesize);
    findseq(ctable,fsize);
    //unique=finduni(table,tablesize);
    //findseq(table,tablesize);
    //printf("over:%d,unique:%d,seq2:%d\n",over,unique,seq);
    avg_size = (rst*1.0)/(total_req*1.0);
    //avg_over = (over*1.0)/(total_req*1.0);
    avg_over = (double)over/total_req;
    avg_seq  = (seq*1.0)/(total_req*1.0);
    printf("File name:%s\n",argv[1]);
    printf("total_req:%d\n",total_req);
    printf("--------------------------------------------------\n");
    printf("over:%d,,seq:%d,request size amount:%d\n",over,seq,rst);
    printf("avg_over:%lf,avg_seq:%lf,avg_req_size:%lf\n",avg_over,avg_seq,avg_size);
    printf("--------------------------------------------------\n");
    printf("[Finish]\n");
    fclose(input);
    //fclose(output);

    return 0;
}
