// 练习文件io.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include "unistd.h"
#include <stdlib.h>
#include <winsock.h>
#pragma comment (lib,"wsock32.lib")


#define STDIN 0//键盘文件描述符


int main()
{
	int fd = STDIN;
    fd_set iofds;
	int retval;
	struct timeval timeout;
	char buff[128] = {0};
	WSAData wsa;
	WSAStartup(0x101, &wsa);
	
	while (true)
	{
		FD_ZERO(&iofds);
		FD_SET(STDIN, &iofds);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		retval = select(STDIN+1, &iofds, NULL, NULL, &timeout);

		if (retval <0) {
			printf("select error\n");
			retval = WSAGetLastError();
			continue;
		}
		else if (retval == 0) {
			printf("time out\n");
			continue;
		}
		else{
			if (FD_ISSET(fd, &iofds)) {
				read(STDIN, buff, 127);
				printf("have read:%s\n", buff);
			}
		}
	}
}

