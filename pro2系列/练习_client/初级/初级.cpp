#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#define SERVER_PORT 0x1234
#pragma comment (lib, "wsock32.lib")
int main()
{
    SOCKET sock;
	struct sockaddr_in server;
	char* buf;
	int len;
	WSAData wsa;
	int val;

	WSAStartup(0x101, &wsa);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("creating sicket error.\n");
		return 0;
	}

	server.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	server.sin_port = htons(SERVER_PORT);
	server.sin_family = AF_INET;

	if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
		//连接失败
		val = WSAGetLastError();
		printf("Connect to server error.\n");
		return 0;
	}

	printf("connect to server.\n");

	buf = (char*)malloc(2048);
	//scan the keboard until input string is "exit"
	while (true)
	{
		/*
		我们再来回忆一下一个socket标准流程
		Client：
		connect(s,...)//已经设置好了socket的参数，发起连接
		send(s,buf)//发送数据
		recv(s,buf)//用buf接收送回来的数据
		*/
		printf("要发送的是=");
		scanf("%s", buf);
		if (strcmp(buf, "exit") == 0)break;

		send(sock, buf, strlen(buf), 0);

		val = recv(sock, buf, 100, 0);
		if (val > 0) {//成功接收
			buf[val] = 0;
			printf("%s\n", buf);
		}
		else break;
	}
	closesocket(sock);
	WSACleanup();
	free(buf);
	printf("over\n");
	return 0;
}
