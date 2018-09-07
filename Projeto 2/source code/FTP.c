#include "FTP.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> 
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>

#define MAX_SIZE 256

int connect_to_server(char * ip, int port) {
	int	sockfd;
	struct	sockaddr_in server_addr;
	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		exit(1);
	}

	/*connect to the server*/
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("connect()");
		exit(1);
	}

	return sockfd;
}

void receive_msg(int sockfd, char * msg){
	int bytes_read;

	bytes_read = recv(sockfd, msg, MAX_SIZE, 0);
	msg[bytes_read] = '\0';

	if (bytes_read <= 0){
		printf("Reading from socket wasn't successful\n");
		exit(1);
	}
}

void send_msg(int sockfd, char * msg) {
	if (write(sockfd, msg, strlen(msg)) < 0){
		printf("Error writing to TCP socket");
		exit(1);
	}
}

void login(int sockfd, char * user, char * password) {
	char user_input[MAX_SIZE], password_input[MAX_SIZE];
	char * response = malloc(MAX_SIZE * sizeof(char));

	sprintf(user_input, "user %s\n", user);
	printf("%s", user_input);

	// send username
	send_msg(sockfd, user_input);

	// receive response from socket
	receive_msg(sockfd, response);
	printf("%s", response);

	sprintf(password_input, "pass %s\n", password);
	printf("%s", password_input);

	// send password
	send_msg(sockfd, password_input);
	
	// receive response from socket (tells us whether the login was successful or not)
	receive_msg(sockfd, response);
	printf("%s", response);
	if(strncmp("230", response,3)){
		printf("Incorrect password.\n");
		exit(1);
	}

	free(response);
}

int pasv(int sockfd) {
	int datafd;
	char * msg = malloc(MAX_SIZE * sizeof(char));
	char * pasv = "pasv\n";
	printf("%s", pasv);

	send_msg(sockfd, pasv);
	receive_msg(sockfd, msg);
	printf("%s\n", msg);
	if(strncmp("227", msg,3)){
		printf("pasv mode not established.\n");
		exit(1);
	}

	char * token = malloc(strlen(msg));
	char ip[MAX_SIZE] = "";

	memcpy(token, msg, strlen(msg));
	token = strtok(token, "(");
	token = strtok(NULL, ",");

	strcat(ip, token);
	strcat(ip, ".");

    int cnt = 3;

	while(cnt > 0) {
		token = strtok(NULL, ",");

		strcat(ip, token);

		if(cnt > 1){
			strcat(ip, ".");
		}
		cnt--;
	}

	token = strtok(NULL, ",");
	char tmp1[MAX_SIZE];
	strcpy(tmp1, token);

	token = strtok(NULL, ")");
	char tmp2[MAX_SIZE];	
	strcpy(tmp2, token);

	int upper_port = atoi(tmp1);
	int lower_port = atoi(tmp2);

	int port = 256*upper_port + lower_port;

	datafd = connect_to_server(ip, port);
	
	free(msg);

	return datafd;
}

void cwd_path(int sockfd, char * path) {
	char * response = malloc(MAX_SIZE * sizeof(char));
	char cwd[MAX_SIZE];

	sprintf(cwd, "cwd %s\n", path);
	printf("%s", cwd);

	send_msg(sockfd, cwd);
	receive_msg(sockfd, response);
	printf("%s", response);
	
	free(response);

}

void retr(int sockfd, char * file) {
	char * response = malloc(MAX_SIZE * sizeof(char));
	char retr[MAX_SIZE];

	sprintf(retr, "retr %s\n", file);
	printf("%s", retr);

	send_msg(sockfd, retr);
	receive_msg(sockfd, response);
	printf("%s", response);

	if(strncmp("150", response,3)){
		printf("Incorrect file.\n");
		exit(1);
	}

	free(response);
}

void download(int datafd, char * filename) {
	int filefd;

    if ((filefd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0) {
        printf("Unable to open file.\n");
        exit(1);
	}

	char buffer[MAX_SIZE];
	int bytes_read;

	while( (bytes_read = read(datafd, buffer, MAX_SIZE)) > 0){
		if(bytes_read == -1){
			printf("Error reading file from server.\n");
			exit(-1);
		}

		if(write(filefd, buffer, bytes_read) < 0){
			perror("Error writing to the destination file.\n");
			exit(-1);
		}
	}

	close(filefd);
}

void disconnect(int sockfd) {

    char * msg = malloc(MAX_SIZE * sizeof(char));

	receive_msg(sockfd, msg);
	printf("%s", msg);

    sprintf(msg, "QUIT\n");
    send_msg(sockfd, msg);

	free(msg);
}
