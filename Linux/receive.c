#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <mqueue.h>
#define name "/quque"
#define MSG_STOP "/0"
int main(int argc,char **argv){
        mqd_t mq1;
        char buffer[257];
        struct mq_attr attr;
	int must_stop=0;
	attr.mq_flags=0;
	attr.mq_maxmsg=10;
	attr.mq_msgsize=256;
	attr.mq_curmsgs=0;
	mq1=mq_open(name,O_RDONLY|O_CREAT,0644,&attr);
	if(mq1==-1){
		perror("mq error:\n");
		exit(1);
	}
	do{
		ssize_t bytes_read;
		bytes_read=mq_receive(mq1,buffer,257,NULL);
		if(bytes_read<0){
			perror("bytes_read error\n");
			exit(1);
		}
		buffer[bytes_read]='\0';
		if(!strncmp(buffer,MSG_STOP,strlen(MSG_STOP))){
			must_stop=1;
		}
		else{
			printf("received:%s\n",buffer);
		}
	}while(!must_stop);
	int s_close=mq_close(mq1);
	if(s_close==-1){
		perror("s_close error:\n");
		exit(1);
	}
	int tmp;
	tmp=mq_unlink(name);
	if(-1==tmp){
		exit(0);}
	return 0;
}


	
