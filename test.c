#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef struct write_buffer{
	struct write_buffer *a[10000];
	struct write_buffer *b[10000];
	int p;
}buf;
buf *test(buf *wb){
	wb=malloc(sizeof(buf));
	wb->a[0]=malloc(sizeof(buf));
	return wb;
}
int main(){
	int i;
	for(i=0;i<10;i++){
		buf *wb;
		wb=test(wb);
		printf("wb:%p a[0]:%p\n",wb,wb->a[0]);	
}
	/*
	float benefit;
	benefit=(float)5/3040;
	benefit/=4500;
	printf("%.19f\n",benefit);*/
	return 0;
}
