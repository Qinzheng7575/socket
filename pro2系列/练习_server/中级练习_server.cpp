/*
标准服务器流程：
listen(s,5)//s前面已经定义好，自己的端口啥的
while(1){
newsock=accept(s,...)//用刚刚在listen的s来和对端建立连接
recv接收
send发送
}
*/

#include "winsock.h"
#include "stdio.h"
#pragma comment (lib,"wsock32.lib")
struct socket_list {
	//套接字管理队列结构
	SOCKET MainSock;
	int num;
	SOCKET socket_array[256];
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
void insert_list(SOCKET s, socket_list* list)
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
void delete_list(SOCKET s, socket_list* list)
{
	//从套接字管理队列中删除s
	int i;
	for (i=0;i<64;i++) {
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
	FD_SET(list->MainSock,fd_list);
	for (i = 0; i < 64; i++) {
		if (list->socket_array[i]>0) {
			FD_SET(list->socket_array[i], fd_list);
		}
	}

};
int main(int argc, char* argv[])
{
	SOCKET s, sock;
	struct sockaddr_in ser_addr, remote_addr;
	int len;
	char buf[128];
	WSAData wsa;
	int retval;
	struct socket_list sock_list;
	fd_set readfds, writefds, exceptfds;
	timeval timeout;
	int i;
	unsigned long arg;

	WSAStartup(0x101, &wsa);
	s = socket(AF_INET, SOCK_STREAM, 0);
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(0x1234);
	bind(s, (sockaddr*)&ser_addr, sizeof(ser_addr));

	listen(s, 5);

	//与select相关的变量、套接字队列等初始化
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	init_list(&sock_list);
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	sock_list.MainSock = s;
	arg = 1;
	ioctlsocket(sock_list.MainSock, FIONBIO, &arg);

	while (1) {
		//建立那三个状态队列
		make_fdlist(&sock_list, &readfds);
		make_fdlist(&sock_list,&writefds);
		make_fdlist(&sock_list,&exceptfds);
		retval = select(0, &readfds, &writefds, &exceptfds, &timeout);
		if (retval == SOCKET_ERROR) {
			retval = WSAGetLastError();
			break;
		}
		if (FD_ISSET(sock_list.MainSock, &readfds)) {
			//主套接字上的read事件
			len = sizeof(remote_addr);
			sock = accept(sock_list.MainSock, (sockaddr*)&remote_addr, &len);
			if (sock == SOCKET_ERROR)continue;
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
				recv(sock, buf, 128, 0);
				printf("接收到：%s", buf);
			}


			if (FD_ISSET(sock, &writefds)) {
				//是不是该send了？？？？？
				send(sock, "ACK by server.", 14, 0);
			}
			if (FD_ISSET(sock, &exceptfds)) {
				closesocket(sock);
			}
		}
	}
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

closesocket(sock_list.MainSock);
WSACleanup();
return 0;
}