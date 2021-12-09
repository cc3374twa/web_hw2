#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

int sockfd;
int fds[1000];
int size = 1000;
char *IP = "168.143.1.0";
short PORT = 8888;
typedef struct sockaddr SA;
char *account[1000];

void initialize();
void *server_thread(void *p);
void alluser(int fd);
int authen(char buf[]);
void ALL_Massage(char *msg);

int main(){
    initialize();
    printf("[Server Message] Initialize OK!\n");
    while(1){
        struct sockaddr_in from;
        socklen_t len = sizeof(from);
        int fd = accept(sockfd, (SA*)&from, &len);
        if(fd==-1){
            printf("[Server Message] Client error\n");
            continue;
        }
        for (int i = 0; i < size;i++){
            if(fds[i]==0){  //新的thread加入
                fds[i] = fd;
                pthread_t tid;
                pthread_create(&tid, 0, server_thread, &fd);
                break;
            }if (size == i){
				char* str = "Sorry, the room is full!";
				send(fd,str,strlen(str),0);
				close(fd);
			}
        }
    }
}
void initialize(){
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1){
		perror("[Server Message] Socket Create failed!");
		exit(1);
	}

    struct sockaddr_in address;
    address.sin_family = PF_INET;
	address.sin_port = htons(PORT);
	address.sin_addr.s_addr=htonl(INADDR_ANY);
    
    int reuseaddr = 1;
    socklen_t reuseaddr_len;
    reuseaddr_len = sizeof(reuseaddr);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, reuseaddr_len);

    if(bind(sockfd,(SA*)&address,sizeof(address))==-1){
        perror("[Server Message] Binding failed!");
        exit(1);
    }
    if(listen(sockfd,1000)==-1){
        perror("[Server Message] Listening failed!");
        exit(1);
    }

    for (int i = 0; i < size;i++){
        account[i] = (char *)malloc(sizeof(char) * 1000);
    }
}

void *server_thread(void *p){
    //printf("server_thread\n");
    int fd = *(int *)p;
    char buffer[1000]={};
    char *ptr,tmp[1000];
    while(1){
        send(fd, "account check!", strlen("account check!"), 0);  //check account
        recv(fd, buffer, sizeof(buffer), 0);    //receive account
        printf("[Server Message] receive from client = / %s /\n", buffer);
        
        ptr = strstr(buffer, "_");
        *ptr = '\0';
        //printf("strstr\n");

        strcpy(account[fd], buffer);
        account[fd][strlen(account[fd])] = '\0';
        *ptr = '_';
        ptr = strstr(buffer, "@");
        *ptr = '\0';
        //printf("sw\n");
        int sw = authen(buffer);
        if(sw==1){
            ptr = strstr(buffer, "_");
            *ptr = '\0';
            printf("[Server Message] New Account Connect: %s\n", account[fd]);
            ALL_Massage(account[fd]);
            break;
        }if(sw==0){
            printf("[Server Message] login failed\n");
            pthread_kill(fd,SIGALRM);
        }
    }

    while(1){
        char buffer2[1000]={};
        if(recv(fd,buffer2,sizeof(buffer2),0)<=0){
            int i;
            for (i = 0; i < size;i++){
                if(fd==fds[i]){
                    fds[i] = 0;
                    break;
                }
            }
            printf("[Server Message] fd= %d is logout\n", fd);
            pthread_exit((void*)i);
        }

        if(strcmp(buffer2,"ls")==0){
            alluser(fd);
        }else if(buffer2[0]=='@'){
            int oppofd = atoi(&buffer2[1]);
            char *msg=(char*)malloc( 256*sizeof(char) );
            char tmp[1000];
            
            strcpy(msg,"CONNECT ");
			strcat(msg,account[fd]);
			sprintf(tmp," %d",fd);
			strcat(msg,tmp);

			send(oppofd,msg,strlen(msg),0);
			send(fd,"Waiting for response...\n\0",strlen("Waiting for response...\n\0"),0);
        }else if(strncmp(buffer2,"AGREE ",6)==0)
		{
			int oppofd=atoi(&buffer2[6]);
			char *msg = (char*)malloc( 256*sizeof(char) );
			strcpy(msg,"AGREE ");
			strcat(msg,account[fd]);
			sprintf(tmp," %d",fd);
			strcat(msg,tmp);
			send(oppofd,msg,strlen(msg),0);
		}
        else if(buffer2[0]=='!')
		{
            int n = atoi(&buffer2[1]);
            int oppofd;
			
            char *ptr,tmp[1000];
			ptr=strstr(buffer2," ");
			ptr++;
			
            oppofd=atoi(ptr);
			
            sprintf(tmp,"!%d",n);
			
            printf("[%d] buf=%s\n",fd,tmp);
			
            send(oppofd,tmp,strlen(tmp),0);
		}else{
			ALL_Massage(buffer2);
		}
		memset(buffer2,0,sizeof(buffer2));
    }
}
int authen(char buf[])
{
    //printf("authen\n");
    FILE *fp;
	char tmp1[1000];
	fp=fopen("account","r");
	while(fscanf(fp,"%s",tmp1)!=EOF)
	{
		if(strcmp(tmp1,buf)==0)
            return 1;
	}
	return 0;
}

void alluser(int fd){
	
    char bar[100]="\n***********************\n";
	printf("[Server Message] %d print list (ls) .\n", fd);
	send(fd,bar,strlen(bar),0);
	
    send(fd,"[Online Player]\n",strlen("[Online Player]\n"),0);
	
    for (int i = 0;i < size;i++){
		if (fds[i] != 0){
			char buf[1000] = {};
			//if(fds[i]!=fd){
				sprintf(buf, "Player: [%s] fd:%d\n" ,account[fds[i]] ,fds[i]);
				send(fd,buf,strlen(buf),0);
			//}
		}
	}
	send(fd,bar,strlen(bar),0);
	char buf[1000] = {};
	
    strcpy(buf,"[Server Message] Choose your opponent by @fd");
	send(fd,buf,strlen(buf),0);
}

void ALL_Massage(char* msg){
	for (int i = 0;i < size;i++){
		if (fds[i] != 0){
			printf("[Server Message] sending message to %s\n",account[fds[i]]);
			send(fds[i],msg,strlen(msg),0);
		}
	}
}
