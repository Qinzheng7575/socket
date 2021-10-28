#include <iostream>
#include<winsock.h>
#pragma comment(lib,"wsock32.lib")

int main(int argc, int* argv[]) {
	SOCKET s, newsock;
	sockaddr_in ser_addr;
	sockaddr_in remote_addr;
	int len;
	char buffer[128];
	int retval;


	WSAData wsa;
	WSAStartup(0x101, &wsa);
	inet_addr("192.168.51.2");
	s = socket(AF_INET, SOCK_STREAM, 0);
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.S_un.S_addr = inet_addr("192.168.51.2");//htonl转化为网络字节序

	ser_addr.sin_port = htons(0x1234);

	bind(s, (sockaddr*)&ser_addr, sizeof(ser_addr));

	listen(s,0);
	while (true)
	{
		len = sizeof(remote_addr);
		if (s == SOCKET_ERROR) retval = WSAGetLastError();

		newsock = accept(s, (sockaddr*)&remote_addr, &len);//卡在这里
		recv(newsock, buffer, sizeof(buffer), 0);
		send(newsock, "OK!", 3, 0);
		printf("receive");
		closesocket(newsock);
	}
	closesocket(s);
	WSACleanup();
	return(0);
}