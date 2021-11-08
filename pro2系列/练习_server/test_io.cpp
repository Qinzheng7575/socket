/*
标准服务器流程：
listen(s,5)//s前面已经定义好，自己的端口啥的
while(1){
newsock=accept(s,...)//用刚刚在listen的s来和对端建立连接
recv接收
send发送
}
*/
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
//#include <winsock.h>//此行在Linux下似乎要注释掉
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h> 
//#pragma comment (lib,"wsock32.lib")


#define STDIN 0//键盘输入文件描述符

struct socket_list {
	//套接字管理队列结构
	int MainSock;
	int num;
	int socket_array[256];
};
void init_list(socket_list* list)
{
	//初始化套接字
	int i = 0;
	list->num = 0;
	list->MainSock = 0;
	for (i = 0; i < 64; i++) {
		list->socket_array[i] = 0;
	}
};
void insert_list(int s, socket_list* list)
{
	//将s插入到套接字管理队列中
	int i;
	for (i = 0; i < 64; i++) {
		if (list->socket_array[i] == 0) {
			list->socket_array[i] = s;
			list->num++;
			break;
		}
	}
};
void delete_list(int s, socket_list* list)
{
	//从套接字管理队列中删除s
	int i;
	for (i = 0; i < 64; i++) {
		if (list->socket_array[i] = s) {
			list->socket_array[i] = 0;
			list->num--;
			break;
		}
	}
};
void make_fdlist(socket_list* list, fd_set* fd_list)
{
	//将管理队列中的套接字逐个添加到指定的状态队列fd_list中，
	//注意利用FD_SET()
	int i;
	FD_SET(list->MainSock, fd_list);
	for (i = 0; i < 64; i++) {
		if (list->socket_array[i] > 0) {
			FD_SET(list->socket_array[i], fd_list);
		}
	}

};

int main()
{
	int s, sock;//int instead SOCKET in Linux
	struct sockaddr_in ser_addr, remote_addr;
	int fd = STDIN;
	unsigned int len;
	char buf[128];

	struct socket_list sock_list;//我们建立的socket管理列表
	fd_set readfds, writefds, exceptfds;//三种socket列表，对应读、写、意外

    //fd_set iofds;
	int i;
	unsigned long arg;
	struct timeval timeout;
	char in_buf[128] = {0};//
	int retval;//承接select、recv等函数的返回值

	s = socket(AF_INET, SOCK_STREAM, 0);//TCP
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//环回地址for测试
	ser_addr.sin_port = htons(0x1234);
	bind(s, (sockaddr*)&ser_addr, sizeof(ser_addr));

	listen(s, 5);
        printf("listening...\n");

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;//设置等待时间
	init_list(&sock_list);
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	sock_list.MainSock = s;
	arg = 1;
	//ioctlsocket(sock_list.MainSock, FIONBIO, &arg);//开启非阻塞for win
        ioctl(sock_list.MainSock,FIONBIO,1);
		insert_list(fd, socket_list);//插入键盘文件描述符到监听队列


	while (1) {
		//建立那三个状态队列
		make_fdlist(&sock_list, &readfds);
		make_fdlist(&sock_list, &writefds);
		make_fdlist(&sock_list, &exceptfds);

		retval = select(0, &readfds, &writefds, &exceptfds, &timeout);
                /*
		if (retval == SOCKET_ERROR) {
			//retval = WSAGetLastError();
			break;
		}
                */
		if (FD_ISSET(sock_list.MainSock, &readfds)) {
			//主套接字上的read事件
			len = sizeof(remote_addr);
			sock = accept(sock_list.MainSock, (sockaddr*)&remote_addr, &len);
			//if (sock == SOCKET_ERROR)continue;
			printf("和对方建立连接\n");
			insert_list(sock, &sock_list);
		}
		//从套接字上的各种事件处理
		//注意此处应设计为不断从套接字管理队列中逐个取出sock的循环模式

		for (i = 0; i < 64; i++) {
			if (sock_list.socket_array[i] == 0)continue;
			sock = sock_list.socket_array[i];
			//此时已经进入到套接字管理流程了，不需要再管新来的啥的了
			if (FD_ISSET(sock, &readfds)) {
				if (sock == STDIN) {
					read(STDIN, in_buff, 127);
					printf("have read:%s\n", buff);
				}

				recv(sock, buf, 128, 0);
				printf("接收到：%s", buf);
			}


			if (FD_ISSET(sock, &writefds)) {
				//是不是该send了？？？？？
				send(sock, in_buf, strlen(in_buf), 0);//发送从in_buf接收到的输入文字
			}
			if (FD_ISSET(sock, &exceptfds)) {
				printf("excepted error.\n");
				//closesocket(sock);
                                close(sock);
			}
		}
	}
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	//closesocket(sock_list.MainSock);
        close(sock_list.MainSock);
	return 0;



	/*
	while (true)
	{
		FD_ZERO(&iofds);
		FD_SET(STDIN, &iofds);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		retval = select(STDIN+1, &iofds, NULL, NULL, &timeout);

		if (retval <0) {
			printf("select error\n");
			//retval = WSAGetLastError();
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
	*/
}


