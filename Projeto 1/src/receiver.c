#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "DataLink.h"

#define RECEIVER 0
#define TRANSMITTER 1

int main(int argc, char** argv)
{
    
    	if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS4", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0\n");
      exit(1);
	}
	
	const char * path = argv[1];
	int fd = startConnection(path);

    	if(fd == -1){
    		printf("Cant find a connection.");
    		return 0;
    	}
    	llopen(fd, RECEIVER);
	llclose(fd, RECEIVER);

	return 0;
}
