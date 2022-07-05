/*simulate SSD only*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#include "syssim_driver.h"
#include "disksim_interface.h"
#include "disksim_rand48.h"
#include "my_ipc.h"
#include "cpff_parameter.h"

#define	BLOCK	4096
#define	SECTOR	512
#define	BLOCK2SECTOR	(BLOCK/SECTOR)

typedef	struct	{
  int n;
  double sum;
  double sqr;
} Stat;


static SysTime now = 0;		/* current time */
static SysTime next_event = -1;	/* next event */
static int completed = 0;	/* last request was completed */
static Stat st;


void
panic(const char *s)
{
  perror(s);
  exit(1);
}


void
add_statistics(Stat *s, double x)
{
  s->n++;
  s->sum = x;
  s->sqr += x*x;
}


void
print_statistics(Stat *s, const char *title)
{
  double avg, std;

  avg = s->sum/s->n;
  std = sqrt((s->sqr - 2*avg*s->sum + s->n*avg*avg) / s->n);
  printf("%s: n=%d average=%f std. deviation=%f\n", title, s->n, avg, std);
}


/*
 * Schedule next callback at time t.
 * Note that there is only *one* outstanding callback at any given time.
 * The callback is for the earliest event.
 */
void
syssim_schedule_callback(disksim_interface_callback_t fn, 
			 SysTime t, 
			 void *ctx)
{
  next_event = t;
}


/*
 * de-scehdule a callback.
 */
void
syssim_deschedule_callback(double t, void *ctx)
{
  next_event = -1;
}


void
syssim_report_completion(SysTime t, struct disksim_request *r, void *ctx)
{
  completed = 1;
  now = t;
  add_statistics(&st, t - r->start);
}



double IssueRequest(unsigned long diskblkno , unsigned reqflag , struct disksim_interface *disksim){
  struct disksim_request *r = malloc(sizeof(struct disksim_request));
  r->start = now;
  r->flags = reqflag;
  r->devno = 0;
  r->blkno = diskblkno;
  r->bytecount = BLOCK;
  completed = 0;
  disksim_interface_request_arrive(disksim, now, r);
  while(next_event >= 0) {
    now = next_event;
    next_event = -1;
    disksim_interface_internal_event(disksim, now, 0);
  }
  if (!completed) {
    fprintf(stderr,"internal error. Last event not completed\n");
    exit(1);
  }
  free(r);
  return st.sum;
}

int
main(int argc, char *argv[])
{
  int i;
  struct stat buf;
  struct disksim_request r;
  struct disksim_interface *disksim;
  struct msqid_ds ds;
  double latancy;


  if (argc != 3) {
    fprintf(stderr, "usage: %s <param file> <output file>\n",
	    argv[0]);
    exit(1);
  }

  if (stat(argv[1], &buf) < 0)
    panic(argv[1]);

  disksim = disksim_interface_initialize(argv[1], 
					 argv[2],
					 syssim_report_completion,
					 syssim_schedule_callback,
					 syssim_deschedule_callback,
					 0,
					 0,
					 0);

  if(create_message_queue(KEY_MSQ_DISKSIM_1, IPC_CREAT) == -1) {
       printf("MSQ create error in create_SSD_message_queue():\n");
  }
  if(remove_message_queue(KEY_MSQ_DISKSIM_1, &ds) == -1) {
      printf("Not remove message queue:(key)\n");
  }

    
  if(create_message_queue(KEY_MSQ_DISKSIM_1, IPC_CREAT) == -1) {
       printf("MSQ create error in create_SSD_message_queue():\n");
  }


  REQ *request;
  request = calloc(1, sizeof(REQ));

  while(1){
    if(recv_request_by_MSQ(KEY_MSQ_DISKSIM_1, request, MSG_TYPE_DISKSIM_1) == 0){
       //receive a request
      if(request->reqFlag == 2){
        break;
      }
    }

    latancy = IssueRequest(request->diskBlkno,request->reqFlag,disksim);
    request->responseTime = latancy;

    if(send_request_by_MSQ(KEY_MSQ_DISKSIM_1, request, MSG_TYPE_DISKSIM_1_SERVED) == -1) {
      printf("A request not sent to MSQ in send_request_by_MSQ()");  
    }

  }
  free(request);

  if(remove_message_queue(KEY_MSQ_DISKSIM_1, &ds) == -1) {
      printf("Not remove message queue:(key)\n");
  }

  
  disksim_interface_shutdown(disksim, now);


  exit(0);
}

