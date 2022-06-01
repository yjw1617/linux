#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <signal.h>
#include "mychar.h"

int fd = -1;
char buf[8] = "";
void signal_handler(int sig_id){
	if(sig_id == SIGIO){
		read(fd, buf, 8);
		printf("read data is %s\n", buf);
	}
	// if(sig_id == SIGINT){
	// 	printf("sigint sig\n");
	// }
}
int main(int argc, char** argv){
	if(argc < 2){
		printf("argument is too few\n");
		return 1;
	}
	signal(SIGIO,signal_handler);
	fd = open(argv[1],O_RDWR | O_NONBLOCK);
	if(fd < 0){
		printf("open %s failed\n", argv[1]);
		return 2;
	}


	fcntl(fd, F_SETOWN, getpid());

	int oflags = fcntl(fd, F_GETFL);

	fcntl(fd, F_SETFL, oflags | FASYNC);

	

	// fd_set rset;
	// int ret = 0;
	while(1){
    }
	//wait to read
	read(fd, buf, 8);
	printf("read data is %s\n", buf);
	
	close(fd);
	fd = -1;
	return 0;

}
