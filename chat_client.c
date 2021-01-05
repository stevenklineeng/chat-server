/* Steven Kline : chat_client.c
 * Works in conjunction with chat_server.c, used as a chat server that has support for
 * up to 30 clients at once; has built in commands (type "HELP")
 *
 * Compile :: gcc chat_client.c -lpthread -o chat_client
 * Run :: ./chat_client <IP_address> <port_number> <username>
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>

void * reader(void *);
void * writer(void *); 

int sockfd; 
char * username;
char * ip_addy;
fd_set fds;
 
int main(int argc, char * argv[]) {
	/* declare all needed variables */
	int port_number, maxfd;
	struct hostent *server;
	pthread_t read_tid, write_tid;
	struct sockaddr_in svr_addr;
	
	/* error checks the command line args */
	if(argc != 4) { 
		printf("Please provide IP address, port number, and username.\n"); 
		exit(1); 
	}
	ip_addy = strdup(argv[1]);
	port_number = atoi(argv[2]);
	username = strdup(argv[3]);
	
	/* connects to the server */
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		perror("socket"); 
		exit(EXIT_FAILURE); 
	}

	if( (server = gethostbyname(argv[1])) == NULL) { 
		perror("host"); 
		exit(EXIT_FAILURE); 
	}

	bzero((char *) &svr_addr, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&svr_addr.sin_addr.s_addr, server->h_length);
	svr_addr.sin_port = htons(port_number);
	
	if (connect(sockfd, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) < 0) { 
		perror("connect"); 
		exit(EXIT_FAILURE); 
	}

	printf("Welcome to the chat room!\n");
	maxfd = sockfd + 1;
	
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
	FD_SET(0, &fds);
	
	/* splits into two threads for reading and writing */
	pthread_create(&read_tid, NULL, reader, NULL);
	pthread_create(&write_tid, NULL, writer, NULL);
	
	pthread_join(read_tid, NULL);
	pthread_join(write_tid, NULL);
	
	return 0; 
}

/* function responsible for reading info from server */
void * reader(void * passed_arg) {
	char svr_reply[300];
	char * temp;
	char * tokenstr = malloc(300*sizeof(char));
	
	while(1) {
		if (FD_ISSET(sockfd, &fds)) {
			bzero(svr_reply, 300);

			if (recv(sockfd, svr_reply, 2000, 0) < 0) { 
				perror("recv"); 
				exit(EXIT_FAILURE); 
			}
			
			printf("%s\n\n", svr_reply);

			bzero(svr_reply, 300);
			bzero(tokenstr, 300);
		}
	}
	return 0;
}

/* function responsible for writing info to the server */
void * writer(void * passed_arg) {
	char * input = malloc(256*sizeof(char));
	char * temp = malloc(256*sizeof(char));
	char * temp2, x;
	char * cat_temp = malloc(256*sizeof(char));
	int del = 0;
	
	// sends euid and other client info to server
	if(send(sockfd, username, strlen(username), 0) < 0) { 
		perror("send"); 
		exit(EXIT_FAILURE); 
	}
	usleep(1000);
	if(send(sockfd, ip_addy, strlen(ip_addy), 0) < 0) { 
		perror("send"); 
		exit(EXIT_FAILURE); 
	}
	
	
	/* reads in input and performs the corresponding action */
	while( 1 ) {
		del = 0;
		for(int z = 0; (x = getchar()); z++) { // reads in input
			if(x == '\n' && z == 0) { del = 1; break; }
			else if(x == '\n') { break; }
			else { input[z] = x; }
		}
		if(del == 1) { 
			continue; 
		}

		temp2 = strdup(input);
		printf("\n");
		
		if(strcmp(input, "QUIT") == 0) { // if QUIT command
			break; 
		}
		else if(strcmp(input, "LIST") == 0) { // if LIST command
			if(send(sockfd, input, strlen(input), 0) < 0) { 
				perror("send"); 
				exit(EXIT_FAILURE); 
			}
		}
		else if(strcmp(input, "HELP") == 0) { // if HELP command
			printf("Valid commands are:\n");
			printf("LIST\nQUIT\n");
			printf("SEND:(<euid>|*):<string>\n");
		}
		else {	// SEND command
			if(send(sockfd, temp2, strlen(temp2), 0) < 0) { 
				perror("send"); 
				exit(EXIT_FAILURE); 
			}
		}
			
		bzero(input, 256); 
		bzero(temp, 256);
	}
	exit(0);
	return 0;
}