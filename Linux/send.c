#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/msg.h>
#define name "/quque"
#define MSG_STOP "/0"
int main(int argc,char **argv){
        mqd_t mq1;
        char buffer[256];
        mq1=mq_open(name,O_WRONLY|O_CREAT,0644,NULL);
        if(mq1==-1){
                perror("mq1 error:\n");
                exit(1);
        }
        printf("send to receiver(enter\"exit\"to stop it):\n");
        do{
                for(int i=0;i<sizeof(buffer);i++){
                        buffer[i]='\0';}
                printf(">");
                fgets(buffer,256,stdin);
                int send=mq_send(mq1,buffer,256,0);
                if(send<0){
			perror("message:\n");
			exit(1);
		}
		if(buffer[0]=='e'){
		//	printf("e\n");
			if(buffer[1]=='x'){
		//		 printf("x\n");
				if(buffer[2]=='i'){
		//			 printf("i\n");
					if(buffer[3]=='t'){
		//				 printf("t\n");
						exit(1);
					}
				}
			}
		}
	}while(1);
	return 0;}
