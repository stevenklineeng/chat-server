/* Steven Kline : chat_server.c
 * Works in conjunction with chat_client.c, used as a chat server that has support for
 * up to 30 clients at once; has built in commands
 * 
 * Compile :: gcc chat_server.c -lpthread -o chat_server
 * Run :: ./chat_server <port_number>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/tcp.h>

void * connector(void *);

int port_number;
char * euid_list[30];
int sockfd_list[30];
int num_clients;

int main(int argc, char * argv[]) {
	/* declare all needed variables */
	int svr_sockfd, cli_sockfd, *new_sockfd;
	struct sockaddr_in svr_addr, cli_addr;
	socklen_t clilen;
	
	/* error checks the command line, and copies values over if no error */
	if(argc != 2) { 
		printf("Please provide a port number to use.\n"); 
		exit(1); 
	}
	port_number = atoi(argv[1]);
	
	/* establishes connection */
	if( (svr_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		perror("socket"); 
		exit(EXIT_FAILURE); 
	}

    bzero((char *) &svr_addr, sizeof(svr_addr)); 
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(port_number);	
	
	if (bind(svr_sockfd, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) < 0) { 
		perror("bind"); 
		exit(EXIT_FAILURE); 
	}

	listen(svr_sockfd, 30);
	printf("Listening on port %d \n", port_number);
	
	/* creates new thread for every connection */
	clilen = sizeof(cli_addr);
	while ((cli_sockfd = accept(svr_sockfd, (struct sockaddr *) &cli_addr, &clilen))) {
		pthread_t client_thread;
		new_sockfd = malloc(sizeof(int));
		*new_sockfd = cli_sockfd;
		
		if (pthread_create(&client_thread, NULL, connector, (void *) new_sockfd) < 0) { 
			perror("pthread_create"); 
			exit(EXIT_FAILURE); 
		}		
	}
	
	if (cli_sockfd < 0) { 
		perror("accept"); 
		exit(EXIT_FAILURE); 
	}
	return 0;
}

/* function responsible for connecting to clients */
void * connector(void * passed_arg) {
	int cli_sockfd = *(int *) passed_arg;
	char * cli_username = malloc(20*sizeof(char));
	char * ip_address = malloc(20*sizeof(char));
	char * temp = malloc(256*sizeof(char));
	char * send_message = malloc(256*sizeof(char));
	char * cat_euid = malloc(300*sizeof(char));
	char client_message[256];
	int nread;
	bzero(client_message, 256);
	
	// recieves info about the client
	nread = recv(cli_sockfd, cli_username , 20, 0);
	euid_list[num_clients] = strdup(cli_username);
	sockfd_list[num_clients] = cli_sockfd;
	nread = recv(cli_sockfd, ip_address , 20, 0);
	num_clients++;
	
	printf("New connection, socket fd is %d, ip is : %s, port: %d \n", cli_sockfd, ip_address, port_number);
	printf("User %s has logged in\n\n", cli_username);
	
	while ((nread = recv(cli_sockfd, client_message, 256, 0)) > 0) {
		// deal with all actions from client
		temp = strtok(client_message, ":");
		if(strcmp(temp, "SEND") == 0) { // if SEND command
			temp = strtok(NULL, ":");
			if(strcmp(temp, "*") == 0) { // SEND:*
				send_message = strtok(NULL, ":");
				strcpy(cat_euid, "From: ");
				strcat(cat_euid, cli_username);
				strcat(cat_euid, "\n");
				strcat(cat_euid, send_message);
				for(int i = 0;  i < num_clients; i++) {
					write(sockfd_list[i], cat_euid, strlen(cat_euid));
				}
			}
			else { // SEND:<euid>
				send_message = strtok(NULL, ":");
				strcpy(cat_euid, "From: ");
				strcat(cat_euid, cli_username);
				strcat(cat_euid, "\n");
				strcat(cat_euid, send_message);
				for(int i = 0; i < num_clients; i++) {
					if(strcmp(euid_list[i], temp) == 0) {
						write(sockfd_list[i], cat_euid, strlen(cat_euid));
					}
				}
			}
		}
		else if(strcmp(temp, "LIST") == 0) {  // if LIST command
			for(int i = 0; i < num_clients; i++) {
				strcat(cat_euid, euid_list[i]);
				strcat(cat_euid, "\n");
			}
			write(cli_sockfd, cat_euid, strlen(cat_euid));
		}
		
		fflush(stdout);
		bzero(cat_euid, 300);
		bzero(client_message, 256);
		bzero(send_message, 256);
		bzero(temp, 256);
	}
	
	/* if client disconnects */
	if (nread <= 0) {
		printf("Host disconnected, ip %s, port %d \n", ip_address, port_number);
		printf("User %s has logged out\n\n", cli_username);
	}
	free(passed_arg);
	
	return 0;
}
