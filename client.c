#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

int sockfd;
int filefd,Game=0,MyTurn=0;

char *IP = "168.143.1.0";
short PORT = 8888;

typedef struct sockaddr SA;

char name[1000];
char passwd[1000];

int oppofd;
char opponame[1000],x[9],p1='O',p2='X';

void initialize();
void start();
void *recv_thread(void *p);

void board(){
	printf(" %c * %c * %c\n",x[0],x[1],x[2]);
	printf("**************\n");
	printf(" %c * %c * %c\n",x[3],x[4],x[5]);
	printf("**************\n");
	printf(" %c * %c * %c\n",x[6],x[7],x[8]);
}
int iswin(char my){
	if(x[0]==my&&x[1]==my&&x[2]==my)return 1;
	if(x[3]==my&&x[4]==my&&x[5]==my)return 1;
	if(x[6]==my&&x[7]==my&&x[8]==my)return 1;
	if(x[0]==my&&x[3]==my&&x[6]==my)return 1;
	if(x[1]==my&&x[4]==my&&x[7]==my)return 1;
	if(x[2]==my&&x[5]==my&&x[8]==my)return 1;
	if(x[0]==my&&x[4]==my&&x[8]==my)return 1;
	if(x[2]==my&&x[4]==my&&x[6]==my)return 1;
	
    return 0;
}
int isfair(){
	int i;
	for(i=0;i<9;i++)
	{
		if(x[i]==' ')
            return 0;
	}
	return 1;
}

int main(){
	initialize();
	printf("[Client Message] login your account :");
	scanf("%s",name);
	printf("[Client Message] login your password :");
	
    scanf("%s",passwd);
	strcat(name,"_");
	
    strcat(name,passwd);
	strcat(name,"@");
	
    start();
	return 0;
}

void initialize(){
    sockfd = socket(PF_INET,SOCK_STREAM,0);
	struct sockaddr_in address;
	
    address.sin_family = PF_INET;
	address.sin_port = htons(PORT);
	address.sin_addr.s_addr=htonl(INADDR_ANY);

	if (connect(sockfd,(SA*)&address,sizeof(address)) == -1){
		perror("[Client Message] Connect failed");
		exit(1);
	}
	printf("[Client Message] Client start\n");
}

void start(){

	char buffer2[1000] = {};
	
    while(1){
		recv(sockfd,buffer2,sizeof(buffer2),0);
		if (strcmp(buffer2,"account check!") == 0){
			send(sockfd,name,strlen(name),0);
			break;
		}
	}
	
    pthread_t id;
	void* recv_thread(void*);
	pthread_create(&id,0,recv_thread,0);
	
    char *ptr;
	ptr=strstr(name,"_");
	*ptr='\0';
	
    int f=1;
	
    while(1){
		char buf[1000] = {};
		fgets(buf, sizeof(buf), stdin);
		
        if(Game==1&&MyTurn==1&&f==1)
		{
			board();
			printf("[Client Message] Please enter chess with  #(0-8)\n");
			f=0;
		}
		if(Game==1){
			f=0;
		}
		
        char *ptr = strstr(buf, "\n");
		*ptr = '\0';
		char msg[1000] = {};
		
        if (strcmp(buf,"logout") == 0){
			memset(buffer2,0,sizeof(buffer2));
			sprintf(buffer2,"[Client Message] %s is quit",name);
			send(sockfd,buffer2,strlen(buffer2),0);
            break; //jump to close(fd)
        }
		if (strcmp(buf,"ls") == 0){
			memset(buffer2,0,sizeof(buffer2));
			sprintf(buffer2,"ls");
			send(sockfd,buffer2,strlen(buffer2),0);
		}
		else if(buf[0]=='@')
		{
			sprintf(msg,"%s",buf);
			send(sockfd,msg,strlen(msg),0);
		}
		else if(buf[0]=='#')
		{
			if(Game==0)
                printf("[Client Message] No Game is playing.\n");
			else if(MyTurn==0)
                printf("[Client Message] It is not your turn now.\n");
			else{   //playing and myturn (put the chess)
				int n=atoi(&buf[1]);
				if(x[n]!=' '||n>9||n<0)
				{
					printf("[Client Message] This place is not empty ,choose another one.\n");
				}
				else
				{
					x[n]=p1;
					printf("***************\n");
					board();
					
                    if(iswin(p1))
					{
						printf("[Client Message] You Win.\n\n");
						Game=0;
					}
					else if(isfair())
					{
						printf("[Client Message] Fair , Good Game.\n\n");
						Game=0;
					}
					else{
                        printf("\n[Client Message] Waiting for your opponent....\n");
                    }
					MyTurn=0;   
					sprintf(msg,"#%d %d",n,oppofd);
					send(sockfd,msg,strlen(msg),0);
				}
			}
		}
		else if(strcmp(buf,"yes")==0)
		{
			printf("[Client Message]connect sucessful\n");
			
            sprintf(buf,"AGREE %d",oppofd);
			send(sockfd,buf,strlen(buf),0);
			
            printf("[Client Message]Game Start!\n");
			
            int i;
			for(i=0;i<9;i++){
                x[i]=' ';                
            }
			
            p1='O';p2='X';
			Game=1;
			
            if(p1=='O'){
                MyTurn=1;
            }
			else{
                MyTurn=0;                   
            }
			if(MyTurn){
                printf("[Client Message] Press Enter to continue...\n");
            }
			else{
                printf("[Client Message] Waiting for your opponent....\n");
            }
		}
		else if(buf[0]=='#')
		{
			x[atoi(&buf[1])]=p2;
			if(iswin(p2))
			{
				printf("[Client Message] You lose.\n\n");
				Game=0;
			}
			else if(isfair())
			{
				printf("[Client Message] Fair , Good Game.\n\n");
				Game=0;
			}
			MyTurn=1;
		}
		else if(strcmp(buf,"print")==0)
		{
			board();
		}
		else if(buf[0]!='\0'){
			sprintf(msg,"[%s] is saying : %s",name,buf);
			send(sockfd,msg,strlen(msg),0);
		}
	}
	close(sockfd);
}

void* recv_thread(void* p){
	char *ptr,*qtr;
	while(1){
		char buf[1000] = {};
		if (recv(sockfd,buf,sizeof(buf),0) <= 0){
			return;
		}
		//printf("buf=*%s*\n",buf);
		if (strcmp(buf,"account check!") == 0){
			send(sockfd,name,strlen(name),0);
		}
		else if(strncmp(buf,"CONNECT ",8)==0)
		{
			ptr=strstr(buf," ");
			ptr++;
			qtr=strstr(ptr," ");
			*qtr='\0';
			
            strcpy(opponame,ptr);
			qtr++;
			oppofd=atoi(qtr);
            printf("[Client Message] Do you agree playing with (%s) ?\n",opponame);
		}
		else if(strncmp(buf,"AGREE ",6)==0)
		{
			ptr=strstr(buf," ");
			ptr++;
			qtr=strstr(ptr," ");
			*qtr='\0';
			
            strcpy(opponame,ptr);
			qtr++;
			oppofd=atoi(qtr);
			
            printf("[Client Message] %s agree playing with you.\n",opponame);
			printf("[Client Message]Game Start!\n");
			
            int i;
			for(i=0;i<9;i++){
                x[i]=' ';
            }
			
            p1='X';p2='O';
			Game=1;
			
            if(p1=='O'){
                MyTurn=1;
            }
			else{
                MyTurn=0;
            }
			
            if(MyTurn)
                printf("[Client Message] Press Enter to continue...\n");
			else 
                printf("[Client Message] Waiting for your opponent....\n");
		}
		else if(buf[0]=='#')
		{
			x[atoi(&buf[1])]=p2;
			if(iswin(p2))
			{
				printf("[Client Message]You lose!\n\n");
				Game=0;
			}
			else if(isfair())
			{
				printf("[Client Message] Fair , Good Game.\n\n");
			}
			else{
				MyTurn=1;
				board();
				printf("[Client Message] Please enter #(0-8)\n");
			}
		}
		else{
			printf("%s\n",buf);
		}
	}
}