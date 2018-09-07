int connect_to_server(char * ip, int port);
void receive_msg(int sockfd, char * msg);
void send_msg(int sockfd, char * msg);
void login(int sockfd, char * user, char * password);
int pasv(int sockfd);
void cwd_path(int sockfd, char * path);
void retr(int sockfd, char * file);
void download(int datafd, char * filename);
void disconnect(int sockfd);
