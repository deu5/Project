#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 128
#define NAME_SIZE 20
#define NOTSET 0
#define EXIST 1
#define NOTEXIST 2
	
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
	
char name[NAME_SIZE]= {NULL};
char msg[BUF_SIZE] = {NULL};
int cli_exist = NOTSET;
int setFName = 0;
int wOk = 1;
	
int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in server_address;
	pthread_t send_thread, receive_thread;
	void * thread_return;
	if(argc!=4) {
		printf("%s <IP> <port> <name> 로 입력하시오.\n", argv[0]);
		exit(1);
	}	

	sprintf(name, "%s", argv[3]);
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&server_address, 0, sizeof(server_address));
	// server_address 를 모두 0으로 초기화
	server_address.sin_family=AF_INET;
	// IPv4 프로토콜 사용
	server_address.sin_addr.s_addr=inet_addr(argv[1]);
	// 32bit IPv4 주소
	server_address.sin_port=htons(atoi(argv[2]));
	// 포트설정
	  
	if(connect(sock, (struct sockaddr*)&server_address, sizeof(server_address))==-1)// 연결실패시
	error_handling("connect() error");

	write(sock, name, NAME_SIZE);
	// 이름을 서버로 전송

	printf("\n\n");
	printf("서버에 연결되었습니다.\n");

	pthread_create(&send_thread, NULL, send_msg, (void*)&sock); // 쓰레드(send_thread) 생성
	pthread_create(&receive_thread, NULL, recv_msg, (void*)&sock); // 쓰레드(receive_thread) 생성
	pthread_join(send_thread, &thread_return); 
	pthread_join(receive_thread, &thread_return); 
	close(sock); // 소켓 종료 
	return 0;
}
	
void *send_msg(void * arg) 
{
	int sock=*((int*)arg);
	int Flength = 0;
	int i=0;
	int file_size = 0;
	int fEnd = 0;
	char name_msg[NAME_SIZE+BUF_SIZE] = {NULL};
	char t_msg[BUF_SIZE] = {NULL};
	char last_msg[BUF_SIZE] = {NULL};
	char t_name_msg[BUF_SIZE] = {NULL};
	char noUse[BUF_SIZE] = {NULL};
	const char enter[BUF_SIZE] = {"\n"};
	// 혹시 모르니 const char whisper[BUF_SIZE] = {"/whisper\n"};
	int where=0;

	
	while(1) 
	{
		if(wOk == 0) {
			sleep(1);
		}

		fgets(msg, BUF_SIZE, stdin);
		
	
		else if(setFName == 1) { // 파일 수신시 파일 이름을 설정할때
			if(strcmp(msg, enter)) {
				setFName = 0;
			}

		}        
		else 
		{	
			
			strcpy(t_msg, "\n");
			sprintf(t_name_msg,"[%s] %s", name, t_msg); // 메시지만 출력
			sprintf(name_msg,"[%s] %s", name, msg); // 이름과 메시지 합쳐서 출력

			if(strcmp(name_msg, t_name_msg) != 0) 
			{
				write(sock, name_msg, BUF_SIZE); 
				write(sock, msg, BUF_SIZE);
				write(sock, name, NAME_SIZE);
				
			}
			// 아무것도 입력받지 않았을때는 출력 X
			// 메시지 보내기
			
		}
		
	}
	return NULL;
}
	
void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	char name_msg[BUF_SIZE] = {NULL};
	const char nocl_msg[BUF_SIZE] = {"[사용자가 없습니다.]"};
	const char noConnect[BUF_SIZE] = {"사용자가 너무 많습니다"};
	int str_len = 0;
	int file_size = 0;

	while(1)
	{
		str_len=read(sock, name_msg, BUF_SIZE); // str_len에 이름과 메시지 저장

		if(!strcmp(name_msg, signal)) {
			
			setFName = 1;
			wOk = 0; 
			
		}
		else if(strcmp(name_msg, yescl_msg) == 0) {

			cli_exist = EXIST;

		}
		else if(strcmp(name_msg, nocl_msg) == 0) {

			cli_exist = NOTEXIST; 
		}
		else if(!strcmp(name_msg, noConnect)) {
			printf("사용자가 너무 많습니다.\n");
			exit(0);
		}
		else {
			fputs(name_msg, stdout);
		}
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

