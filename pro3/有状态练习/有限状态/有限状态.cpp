// client.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <winsock.h>
#include<stdio.h>
#define SERVER_PORT 0x1234
#pragma comment (lib, "wsock32.lib")

int main(int argc, char* argv[])
{
	SOCKET sock;
	struct sockaddr_in server;
	int len;
	WSAData wsa;
	int val;

	WSAStartup(0x101, &wsa);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("Creating socket error\n");
		return 0;
	}


	server.sin_family = AF_INET;
	server.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);//inet_addr("202.115.12.35");
	server.sin_port = htons(SERVER_PORT);

	if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
		val = WSAGetLastError();
		printf("Conneting to server error\n");
		return 0;
	}

	printf("connect to server\n");

	while (1) {
		//CODES NEEDS HERE

	}

	//close the socket immediately
	closesocket(sock);
	WSACleanup();
	free(buf);
	printf("passed\n");
	return 0;
}
