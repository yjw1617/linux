#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include "mychar.h"

int main(int argc, char** argv){
	int fd = -1;
	char buf[8] = "";
	int max = 0;
	int cur = 0;
	if(argc < 2){
		printf("argument is too few\n");
		return 1;
	}
	fd = open(argv[1],O_RDWR | O_NONBLOCK);
	if(fd < 0){
		printf("open %s failed\n", argv[1]);
		return 2;
	}

	fd_set rset;
	int ret = 0;
	while(1){
        FD_ZERO(&rset);
        // FD_SET(0, &rset);//标准输入
        FD_SET(fd, &rset);//新用户到达

        // maxfd = fd;

        // tout.tv_sec = 5;
        // tout. tv_sec = 0;
        select(fd + 1, &rset, NULL, NULL, NULL);
        // if(FD_ISSET(0, &rset)){
        //     //读取键盘输入  发送到网络套接子
        //     bzero(buf, 50);
        //     if(fgets(buf, 50, stdin) < 0){
        //         continue;
        //     }
        //     if(write(fd, buf, strlen(buf)) < 0){
        //         perror("write to spcket");
        //         return 0;
        //     }
        // }
        if(FD_ISSET(fd, &rset)){//服务器发送过来数据  读取套芥子数据 处理
            bzero(buf, 8);
            ret = read(fd, buf, 8);
            if(ret < 0){
                perror("read");
            }
            if(ret == 0){
                break;
            }
            printf("read msg: %s\n", buf);
        }
    }
	//wait to read
	read(fd, buf, 8);
	printf("read data is %s\n", buf);
	
	close(fd);
	fd = -1;
	return 0;

}
