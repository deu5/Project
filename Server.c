#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define BUF_SIZE 128
#define MAX_CLNT 4
#define NAME_SIZE 20

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);
void save_msg(char *msg);
char *between_msg(char *msg);
char *replace_string(char *in, char *from, char *to);

int client_count=0;
int client_socks[MAX_CLNT];
char client_name[NAME_SIZE]= {NULL};
char client_names[MAX_CLNT][NAME_SIZE]= {NULL};
char game_text[BUF_SIZE]={"text\n"};

pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
        int i;
	int server_sock, client_sock;        
	struct sockaddr_in server_address,client_address;
        int client_address_sz;
        
	pthread_t thread_id;
        
	if(argc!=2) {
                printf("%s <port> 로 입력하시오.\n", argv[0]);
                exit(1);
        }

        pthread_mutex_init(&mutx, NULL);
        // 쓰레드 초기화

        server_sock=socket(PF_INET, SOCK_STREAM, 0);
        //(IPv4,TCP/IP,0)

        memset(&server_address, 0, sizeof(server_address));
        // 서버IP 를 0으로 서버IP SIZE 만큼 초기화
        server_address.sin_family=AF_INET;
        // IPv4 설정
        server_address.sin_addr.s_addr=htonl(INADDR_ANY);
        //
        server_address.sin_port=htons(atoi(argv[1]));
        // 포트 설정

        if(bind(server_sock, (struct sockaddr*) &server_address, sizeof(server_address))==-1)
                error_handling("bind() error");

        // -1 이외 소켓식별자  -1 실패

        if(listen(server_sock, 5)==-1)
                error_handling("listen() error");

        // -1 이외 소켓식별자  -1 실패

        while(1)
        {
                client_address_sz=sizeof(client_address);
                // 클라이언트 IP SIZE

                client_sock=accept(server_sock, (struct sockaddr*)&client_address,&client_address_sz);
                // 서버소켓에 클라이언트IP의 접속요청을 받아들인다.

                if(client_count >= MAX_CLNT) {
                        printf("CONNECT FAIL : %d \n", client_sock);
                        write(client_sock, "too many users. sorry", BUF_SIZE);
                        continue;
                }
                //클라이언트 수가 최대 클라이언트 보다 많으면 표시

                pthread_mutex_lock(&mutx);
                // 다른 쓰레드 접근 제한

                client_socks[client_count]=client_sock;
                // 클라이언트로 받은 접속자 이름 입력

                read(client_sock, client_name, NAME_SIZE);
                // read ( 파일디스크립터,읽어들일 버퍼,버퍼크기);

                strcpy(client_names[client_count++], client_name);
                // 다음 클라이언트로부터 받은 접속자 이름입력

                pthread_mutex_unlock(&mutx);
                // 함수에 의하여 다른 쓰레드의 접근을 허락


                pthread_create(&thread_id, NULL, handle_clnt, (void*)&client_sock);
                //thread_id : 쓰레드의 ID
                //NULL : 쓰레드 특성(보통 0)
                //handle_clnt : 쓰레드가 실행할 함수
                //(void*)&client_sock : 쓰레드가 실행할 함수에 들어갈 인자값

                pthread_detach(thread_id);
                // 쓰레드 종료시 자원 반환
                printf("Connected client IP: %s \n", inet_ntoa(client_address.sin_addr));
                // 연결되면 출력
        }
        close(server_sock);
        // 서버소켓 종료
        return 0;
}

void * handle_clnt(void * arg)
{
        int i;
	int client_sock=*((int*)arg);
        int str_len=0;
        int file_size = 0;
        int where=0;
	
	const char single_file[BUF_SIZE] = {"file : cl->sr"}; // 1:1 파일전송
	const char single_file_all[BUF_SIZE] = {"file : cl->sr_all"}; // 1:N 파일전송
	const char Fmsg_end[BUF_SIZE] = {"FileEnd_cl->sr"}; // 파일의 끝
	const char sig_whisper[BUF_SIZE] = {"whisper : cl->sr"}; // 귓속말
        const char game[BUF_SIZE] = {"game : cl->sr"}; // 게임
	const char exit[BUF_SIZE] = { "exit : cl->sr" }; // 클라이언트 종료 확인 메세지
	
        char msg[BUF_SIZE] = {NULL}; // 메세지 변수
        char file_msg[BUF_SIZE] = {NULL}; // 파일메세지 변수
	

	char game_text_end[BUF_SIZE]="";	
	
        while((str_len=read(client_sock, msg, BUF_SIZE))!=0)
        // read ( 파일디스크립터,읽어들일 버퍼,버퍼크기);
        // client_sock 읽는다.
        {	
        	if(!strcmp(msg, sig_whisper)) { // 귓속말 일때
                        int j=0;
                        int noCli = 0;
                        int mGo = 0;
                        char tmpName[NAME_SIZE]= {NULL};
                        char msg[NAME_SIZE]= {NULL};

                        read(client_sock, tmpName, NAME_SIZE);
			// 귓속말 대상의 이름을 읽어온다.

                        pthread_mutex_lock(&mutx);
			// 다른 쓰레드 접근 제한

                        for(j=0; j<client_count; j++) {
                                if(!strcmp(tmpName, client_names[j]) ) {
                                        noCli = 0;
                                        mGo = j; // 보낼 소켓 번호를 저장
                                        break;
                                }
                                else if(j == client_count - 1) {
                                        noCli = 1; // 그런 사용자가 없을 때 표시
                                        break;
                                }
                        }

                        pthread_mutex_unlock(&mutx);
			// 다른 쓰레드 접근 제한 해제 

                        read(client_sock, msg, BUF_SIZE);
			// 귓속말할 메세지를 읽어온다.

                        if(noCli == 1) {
                                write(client_sock, "귓속말 대상 사용자가 존재하지 않습니다.", BUF_SIZE);
                        } // 귓속말 대상 사용자가 없을 때
                        else {                                
				char whisper_save[BUF_SIZE]="";
				write(client_socks[j], msg, BUF_SIZE);
				strcpy(whisper_save,tmpName);
				strcat(whisper_save,"에게 ");
				strcat(whisper_save,msg);
				save_msg(whisper_save);
                        } // 귓속말 대상 사용자가 존재할 때

                }
                
                else if(!strcmp(msg, single_file)) // 1:1 파일전송 일때
                {
                        int j;
                        int noCli = 0; // 클라이언트 유무 확인 변수
                        int fileGo = NULL; // 소켓 번호 변수
                        char tmpName[NAME_SIZE]= {NULL}; // 클라이언트 이름을 담을 변수

                        read(client_sock, tmpName, NAME_SIZE);
                        // read ( 파일디스크립터,읽어들일 버퍼,버퍼크기);

                        pthread_mutex_lock(&mutx);
                        // 다른 쓰레드 접근 제한


			//해당 사용자가 존재하는지 찾기
                        for(j=0; j<client_count; j++) {

                                if(!strcmp(tmpName, client_names[j]) ) {
                                        noCli = 0;
                                        fileGo = j; // 보낼 소켓 번호를 저장
                                        break;
                                }
                                else if(j == client_count - 1) {
                                        noCli = 1; // 그런 사용자가 없을 때 표시
                                        break;
                                }
                        }

                        if(noCli == 1) {
                                write(client_sock, "[사용자가 없습니다.]", BUF_SIZE);
                                pthread_mutex_unlock(&mutx);
                                // 다른 쓰레드 접근 가능
                                continue;
                        }


                        else if(noCli == 0) {
                                write(client_sock, "[파일 전송을 진행합니다.]", BUF_SIZE);
                        }
                        //해당 사용자가 존재하는지 찾기

                        write(client_socks[fileGo], "file : sr->cl", BUF_SIZE);
                        // 데이터를 보낸다는 신호를 보낸다.

                        read(client_sock, &file_size, sizeof(int));
			// 클라이언트로 부터 파일정보를 읽는다.
                        printf("File size %d Byte\n", file_size);
			// 파일 사이즈 출력
                        write(client_socks[fileGo], &file_size, sizeof(int));
                        // 파일 정보를 파일을 받을 클라이언트에게 보낸다.

                        while(1) {
                                read(client_sock, file_msg, BUF_SIZE);
                                if(!strcmp(file_msg, Fmsg_end))
                                break;
                                write(client_socks[fileGo], file_msg, BUF_SIZE);
                        } // 파일 전송

                        write(client_socks[fileGo], "FileEnd_sr->cl", BUF_SIZE);
			// 파일의 끝을 알려준다.

                        pthread_mutex_unlock(&mutx);
			// 다른 쓰레드 접근 제한 해제
                        printf("파일 전송 완료\n");
			// 파일 전송 완료 메세지 출력
                }

                else if(!strcmp(msg, single_file_all)) { // 1:N 파일 전송 일때

                        pthread_mutex_lock(&mutx); 
                        for(i=0; i<client_count; i++) {
                                if(client_sock == client_socks[i])
                                        continue;
                                write(client_socks[i], "file : sr->cl", BUF_SIZE);                        
                        }// 데이터를 보낸다는 신호를 보낸다

                        read(client_sock, &file_size, sizeof(int));
			// 파일의 정보를 읽어온다.
                        printf("File size %d Byte\n", file_size);
			// 파일의 정보 출력

                        for(i=0; i<client_count; i++) {
                                if(client_sock == client_socks[i])
                                        continue;
                                write(client_socks[i], &file_size, sizeof(int));
                        }
                        // 파일 크기 정보를 보낸다

                        while(1) {
                                read(client_sock, file_msg, BUF_SIZE);
                                if(!strcmp(file_msg, Fmsg_end))
                                        break;

                                for(i=0; i<client_count; i++) {
                                        if(client_sock == client_socks[i])
                                                continue;
                                        write(client_socks[i], file_msg, BUF_SIZE);
                                }
                        } // 파일을 읽어오고 파일을 클라이언트에게 전송한다.

                        for(i=0; i<client_count; i++) {
                                if(client_sock == client_socks[i])
                                        continue;
                                write(client_socks[i], "FileEnd_sr->cl", BUF_SIZE);
                        } // 파일의 끝을 알려준다.

                        pthread_mutex_unlock(&mutx);
			// 다른 쓰레드 접근 제한 해제 
                        printf("파일 전송 완료\n");
                }
                
		else if(!strcmp(msg, game)) // 게임 일때
                {			
			char game_start[BUF_SIZE]="게임시작\n";
			                        
			send_msg(game_start,BUF_SIZE);
			// 게임시작 메세지를 클라이언트에게 보낸다.

			read(client_sock, msg, BUF_SIZE);
			// 게임 텍스트를 클라이언트에게 받는다.

			strcpy(game_text, msg);
			strcat(game_text,"\n");

			int b = strlen(game_text);
			// 게임 텍스트 길이 값

			send_msg(game_text, b);
			// 게임 텍스트를 클라이언트에게 전송
						
                }

		else if (!strcmp(msg, exit)) // 클라이언트가 접속을 종료했을 때
		{
			char msg_exit[BUF_SIZE]="";
			
			read(client_sock, msg, BUF_SIZE);
			// 클라이언트 아이디 값을 읽는다.

 			pthread_mutex_lock(&mutx);
			// 다른 쓰레드 접근 제한                        
			sprintf(msg_exit,"%s 님이 퇴장하셨습니다.\n", msg);
			pthread_mutex_unlock(&mutx); 
			//다른 쓰레드 접근 제한 해제
 
                        send_msg(msg_exit, str_len);
			// 클라이언트에게 사용자 퇴장 메세지 전송
			
			printf("%s",msg_exit);
			// 퇴장 메세지 출력

		} 

                else
                {
			int j=0;
                        int noCli = 0;
                        int mGo = 0;
                        char tmpName[NAME_SIZE]= {NULL};
                        char tmpMsg[NAME_SIZE]= {NULL};

			send_msg(msg, str_len); // 메세지 전송 (이름+메세지)

                        read(client_sock, msg, BUF_SIZE); //클라이언트 메세지 읽는다.
			strcpy(tmpMsg, msg);
			
                        read(client_sock, msg, BUF_SIZE); //클라이언트 이름을 읽는다.
			strcpy(tmpName, msg);			

			
                        if(strcmp(tmpMsg,game_text)==0) // 누가 1등 게임종료
			{

			sprintf(game_text_end,"%s가 1등하셨습니다.\n", tmpName);
			send_msg(game_text_end,BUF_SIZE);  			
			}
			
                }
        }

        pthread_mutex_lock(&mutx);
	// 다른 쓰레드 접근 제한 

        for(i=0; i<client_count; i++)   // 클라이언트 종료시 종료된 클라이언트 변수 초기화 
        {
                if(client_sock==client_socks[i])
                {
                        while(i++<client_count-1) {
                                client_socks[i]=client_socks[i+1];
                                strcpy(client_names[i], client_names[i+1]);
                        }
                        break;
                }
        }
        client_count--;
        
	pthread_mutex_unlock(&mutx);
	// 다른 쓰레드 접근 제한 해제
        close(client_sock);
	// 소켓 종료
        return NULL;
}
void save_msg(char *msg)
{
        int month,day,hour,min;
       
  	char filepath[100]="/ftp/";
        char filename[100];

        struct tm *today;
        time_t the_time;
        time(&the_time);
        today = localtime(&the_time);

        month=today->tm_mon+1;
        day=today->tm_mday;
  	hour=today->tm_hour;
   	min=today->tm_min;

        sprintf(filename,"%d월%d일.txt",month,day);   
   	strcat(filepath,filename);
          
        FILE *out;
        out = fopen(filepath, "a");
        fprintf(out, "[%d시 %d분] : %s",hour,min,msg);
        fclose(out);
}
void error_handling(char * msg)
{
        fputs(msg, stderr);
        fputc('\n', stderr);
        exit(1);
}
char *between_msg(char *msg)
{
   char *output;
        
   output = replace_string(msg, "병신", "**");
   output = replace_string(msg, "븅신", "**");
   output = replace_string(msg, "호구", "**");
   output = replace_string(msg, "시발", "**");
   output = replace_string(msg, "니미", "**");
   output = replace_string(msg, "엠창", "**");
   output = replace_string(msg, "뻐큐", "**");
   output = replace_string(msg, "지랄", "**");
   output = replace_string(msg, "염병", "**");
   output = replace_string(msg, "좆까", "**");
   output = replace_string(msg, "조까", "**");
   output = replace_string(msg, "새끼", "**");
   output = replace_string(msg, "시발놈", "***");
   output = replace_string(msg, "장애", "**");   
   
   return output;
}
char *replace_string(char *in, char *from, char *to)
{ 
        char *rp;
        char *fp;
        char *sp;
        char *wp;
        int from_len = 0;

        wp = in;
        from_len = strlen(from);

        if (!from_len)
	return in;

        if (from_len < strlen(to))
	return in;

        for (rp = in; *rp; rp++)
	{
                sp = rp;
                for (fp = from; *fp; fp++, rp++)
		{
                        if (*rp!=*fp)
			break;
                }
                if(*fp)
		{
		 rp = sp;
		 *wp++ = *rp;
		}
                else
		{
                 --rp;
                 for (fp = to; *fp; fp++) *wp++ = *fp;
                }
        }
        *wp = '\0';
        return in;
}
void send_msg(char *msg, int len)   // send to all
{
        int i,j;

        pthread_mutex_lock(&mutx);
	// 다른 쓰레드 접근 제한
        save_msg(msg);
	// 메세지 저장 함수
        msg = between_msg(msg);
	// 메세지 필터링 함수

        for(i=0; i<client_count; i++)
        write(client_socks[i], msg, BUF_SIZE);
	// 클라이언트에게 메세지 전송

        pthread_mutex_unlock(&mutx);
	// 다른 쓰레드 접근 제한 해제
}
