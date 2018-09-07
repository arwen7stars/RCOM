#include "FTP.h"
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_SIZE 256
#define PORT 21

typedef struct URL {
	char user[MAX_SIZE];
	char password[MAX_SIZE];
	char host[MAX_SIZE];
	char url_path[MAX_SIZE];
	char filename[MAX_SIZE];
	char ip[MAX_SIZE];
} url;

void decodify_arg(char * str, url * url_info){
	char * token = malloc(strlen(str));
	char * tmp_url = malloc(strlen(str));

	char path[MAX_SIZE]; 
	int index;

	if (strncmp("ftp://[", str, 7)){
		printf("Incorrect format.\n");
		exit(1);
	}
	
	memcpy(token, str, strlen(str));

	if( (strchr(token+7, ':') == NULL) || (strchr(token+7, '@') == NULL) || (strchr(token+7, '/') == NULL)){
		printf("Incorrect format.\n");
		exit(1);
	}

	token = strtok(token+7, ":");
	memcpy(url_info->user, token, strlen(token));			// username

	token = strtok(NULL, "@");
	memcpy(url_info->password, token, strlen(token));		// password

	token = strtok(NULL, "/");

	memcpy(url_info->host, token+1, strlen(token)-1);		// host

	tmp_url = strchr(str+7, '/');

	token = strrchr(tmp_url, '/');
	memcpy(url_info->filename, token+1, strlen(token));		// filename

	index = (int)(token - tmp_url) + 1;

	memcpy(path, &tmp_url[0], index);
	path[index] = '\0';

	memcpy(url_info->url_path, path, strlen(path));			// path
		
}




int main(int argc, char** argv){
	
	if(argc != 2){
		printf("ERROR: wrong number of arguments!\n");
		printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
		printf("Example: %s ftp://[anonymous:abcd@]ftp.up.pt/pub/robots.txt\n", argv[0]);
		exit(1);
	}

	struct hostent *h;
	int	sockfd, datafd;
	char * response = malloc(MAX_SIZE * sizeof(char));
	int port = PORT;

	// get info from the given argument
	url * url_info = malloc(sizeof(url));
	char * arg = argv[1];
	decodify_arg(arg, url_info);

	// get ip address - NOTE: shouldn't we be using getaddrinfo because gethostbyname is "obsolete"?
	if((h=gethostbyname(url_info->host)) == NULL) {
		herror("gethostbyname(): ");
        exit(1);
    	}

	char * ip = inet_ntoa(*((struct in_addr *)h->h_addr));
	memcpy(url_info->ip, ip, MAX_SIZE);

	printf("Information given:\n");
	printf("User name: %s\n", url_info->user);
	printf("User password: %s\n", url_info->password);
	printf("Host: %s\n", url_info->host);					// NOTE: it is tom.fe.up.pt
	printf("URL path: %s\n", url_info->url_path);
	printf("Filename: %s\n", url_info->filename);
	printf("IP: %s\n", url_info->ip);

	// open an TCP socket and connect it to server
	sockfd = connect_to_server(url_info->ip, port);

	// receive response from TCP socket
	receive_msg(sockfd, response);		// we need to do this, otherwise 'response' will have undesirable content
	printf("\n%s", response);
	
	if(strncmp("220", response,3)){
		printf("Connection not established.\n");
		exit(1);
	}

	login(sockfd, url_info->user, url_info->password);

	datafd = pasv(sockfd);

	// change directory path
	cwd_path(sockfd, url_info->url_path);

	// retrieve file
	retr(sockfd, url_info->filename);

	// download file
	download(datafd, url_info->filename);

	// disconnect socket
	disconnect(sockfd);
	
	close(datafd);
	close(sockfd);

	free(response);
	free(url_info);

	exit(0);
}
