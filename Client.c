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
  
	while(1)  
 	{ 
 		if(wOk == 0) { 
 			sleep(1); 
 		} 
 		 fgets(msg, BUF_SIZE, stdin); 
 		  
 		if(!strcmp(msg, "/menu\n")) { // 메뉴 
 			 
 			printf("\n"); 
 			printf("[MENU]\n\n"); 
 			printf("1. /menu -> 메뉴를 출력합니다. \n"); 
 			printf("2. /whisper -> 원하는 사용자에게 귓속말을 보냅니다.\n"); 
			printf("3. /sendfile -> 1:1로 파일을 전송합니다. \n");
			printf("4. /sendfile all -> 1:N으로 파일을 전송합니다. \n");
 			printf("\n[END MENU] \n\n"); 
 
 
 		}

		else if(!strcmp(msg, "/sendfile\n")) // 파일전송
		{
			char location[BUF_SIZE];
			char who[NAME_SIZE];
			FILE *fp;
			FILE *size;

			printf("파일 위치를 입력하시오 : ");
			// 파일 위치를 입력하시오 출력
			scanf("%s", location);
			// 파일 위치를 입력받는다.

			size = fopen(location, "rb");
			if(size == NULL) {
				printf("파일이 존재하지 않습니다.\n");
				continue;
			}
			// 보낼 파일이 존재하는지 확인

			printf("파일을 전송받을 ID를 입력하시오 : ");
			// 파일을 전송할 사용자 입력 출력
			scanf("%s", who);
			// ID를 입력받는다.

			write(sock, "file : cl->sr", BUF_SIZE);
			// 파일을 보낸다는 신호를 서버쪽에 보냄.

			write(sock, who, NAME_SIZE);
			// 보내고싶은 사용자 아이디를 보낸다.

			while(cli_exist == NOTSET) { // 재운다
				sleep(1);
			}

			if(cli_exist == NOTEXIST) { // 클라이언트가 없는 경우
				printf("클라이언트가 존재 하지 않습니다. \n");
				cli_exist = NOTSET;
				continue;
			} 
			// 클라이언트가 없으면 빠져나옴

			while(1) { // 파일 버퍼값을 file_size에 저장한다.
				fEnd = fread(noUse, 1 , BUF_SIZE, size);
				file_size += fEnd;

				if(fEnd != BUF_SIZE)
					break;
			}
			fclose(size);

			printf(" 파일 전송을 시작합니다 \n(File Size : %d Byte)\n", file_size); 
			write(sock, &file_size, sizeof(int)); // 파일 크기정보 먼저 보냄.
			file_size = 0; // 파일크기 초기화
			
			fp = fopen(location, "rb"); // 경로값을 불러온다.
			
			while(1) {
				
				Flength = fread(t_msg, 1 , BUF_SIZE, fp);

				if(Flength != BUF_SIZE) {
					for(i=0; i<Flength; i++) {
						last_msg[i] = t_msg[i];
					} 
					//fread를 사용해서 이전 데이터와 MERGE상태가 될수있으므로 방지하였음.

					write(sock, last_msg, BUF_SIZE); // 소켓에 클라이언트의 메시지를 저장한다.

					write(sock, "FileEnd_cl->sr", BUF_SIZE); // 서버로 파일의 끝을 알린다.
					break;
				}
				write(sock, t_msg, BUF_SIZE); // 버퍼값이 최대값이 되면 잘라서 전송한다.

			}		
			
			fclose(fp);
			printf(" 파일 전송이 완료되었습니다. \n");
			cli_exist = NOTSET;
				
		}
		else if(!strcmp(msg, "/sendfile all\n")) { // 파일전송 1:N

			char location[BUF_SIZE];
			FILE *fp;
			FILE *size;

			printf("파일 위치를 입력하시오 : ");
			scanf("%s", location);

			size = fopen(location, "rb"); // 경로값 size에 저장
			if(size == NULL) {
				printf("파일이 존재하지 않습니다. \n");
				continue;
			}
			// 보낼 파일이 유효한지를 확인한다.

			write(sock, "file : cl->sr_all", BUF_SIZE);
			// 파일을 보낸다는 신호를 서버에 보낸다.

			while(1) { // 버퍼값을 불러온다
				fEnd = fread(noUse, 1 , BUF_SIZE, size);
				file_size += fEnd;

				if(fEnd != BUF_SIZE)
					break;
			}
			fclose(size);

			printf("파일 전송을 시작합니다 \n(File Size : %d Byte)\n", file_size); 
			write(sock, &file_size, sizeof(int)); // 파일 크기 정보를 보낸다
			file_size = 0;
			
			fp = fopen(location, "rb");
			

			while(1) {
				
				Flength = fread(t_msg, 1 , BUF_SIZE, fp);

				if(Flength != BUF_SIZE) {
					for(i=0; i<Flength; i++) {
						last_msg[i] = t_msg[i];
					} 
					//fread를 사용해서 이전 데이터와 MERGE상태가 될수있으므로 방지하였음.

					write(sock, last_msg, BUF_SIZE); // 소켓에 클라이언트의 메시지를 저장한다.
					write(sock, "FileEnd_cl->sr", BUF_SIZE); // 서버로 파일의 끝을 알린다.	
					break;
				}
				write(sock, t_msg, BUF_SIZE); 

			}
			// 서버에 파일의 내용을 보낸다.		

			fclose(fp);
			printf("파일 전송이 완료되었습니다.n");		
		} 

  
 		else if(!strcmp(msg, "/whisper\n")) { // 귓속말 기능 
 			char who[NAME_SIZE]; 
 			char wmsg[BUF_SIZE] = {NULL}; 
 
 
 			 
 			printf("<귓속말> (유저이름) (메시지) : "); 
 			scanf("%s %[^\n]", who, wmsg); 
 
 
 			write(sock, "whisper : cl->sr", BUF_SIZE); 
 			// 서버에 귓속말사용 신호를 보낸다.		 
 
 
 			write(sock, who, NAME_SIZE); 
 			// 사용자 아이디를 보낸다.			 
 
 
 			strcpy(t_msg, "\n"); 
 			sprintf(t_name_msg,"[(귓속말)%s] %s", name, t_msg); // 이름 , 내 메시지 연결 
 			sprintf(name_msg,"[(귓속말)%s] %s", name, wmsg); // 이름 , 보내는 사람 id 연결 
 
 
 			name_msg[strlen(name_msg)] = '\n'; 
 
 
 			if(strcmp(name_msg, t_name_msg) != 0)  
 			   write(sock, name_msg, BUF_SIZE); 
 			// 아무것도 입력받지 않았을때는 출력 X 
 			// 메시지 보내기 
 
 
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
	char file_msg[BUF_SIZE] = {NULL};
	const char signal[BUF_SIZE] = {"file : sr->cl"};
	const char end_msg[BUF_SIZE] = {"FileEnd_sr->cl"}; 
 	const char noConnect[BUF_SIZE] = {"사용자가 너무 많습니다"}; 
	const char yescl_msg[BUF_SIZE] = {"[파일 전송을 진행합니다.]"};
 	int str_len = 0; 
 	int file_size = 0;
 
 	while(1) 
 	{ 
 		str_len=read(sock, name_msg, BUF_SIZE); // str_len에 이름과 메시지 저장 
 
 
 		if(!strcmp(name_msg, signal)) { 
 			 
 			setFName = 1; 
 			wOk = 0;  
			printf("파일전송을 요청합니다 ");

			read(sock, &file_size, sizeof(int));
			printf("(File size : %d Byte)\n [Enter키를 눌러주세요]", file_size);
			//파일 사이즈 받아 출력하기.

			printf("파일명을 입력하세요 : ");
			
			wOk = 1; 
			while(setFName == 1) {
				sleep(1);
			}

			msg[strlen(msg)-1] = '\0';
			
			FILE *fp;
			fp = fopen(msg, "wb"); 
		
			while(1)
			{		
				read(sock, file_msg, BUF_SIZE);
				if(!strcmp(file_msg, end_msg)) 
				   break;

				fwrite(file_msg, 1, BUF_SIZE, fp);
			}

			fclose(fp);
			
			printf("파일 전송이 완료 되었습니다. \n");
			// send_msg 쓰레드의 활동을 재개한다.
			
 			 
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

