

#include <iostream>
#include <winsock.h>
#pragma comment(lib,"wsock32.lib")
int main(int argc,int* argv[])
{
	SOCKET s;
	sockaddr_in local, server;
	WSAData wsa;

	char *buffer = "This is client";

	char recvbuffer[128];

	WSAStartup(0x101, &wsa);

	s = socket(AF_INET, SOCK_STREAM, 0);
	local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	local.sin_family = AF_INET;
	local.sin_port = htons(0x3412);

	bind(s, (sockaddr*)&local, sizeof(local));

	server.sin_family = AF_INET;
	//server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	server.sin_port = htons(0x1234);
	
	connect(s, (sockaddr*)&server, sizeof(server));

	send(s, buffer, strlen(buffer), 0);
	printf("have sent");
	recv(s, recvbuffer, sizeof(recvbuffer), 0);
	closesocket(s);
	WSACleanup();
	return(0);
}

