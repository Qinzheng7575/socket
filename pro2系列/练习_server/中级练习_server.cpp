/*
��׼���������̣�
listen(s,5)//sǰ���Ѿ�����ã��Լ��Ķ˿�ɶ��
while(1){
newsock=accept(s,...)//�øո���listen��s���ͶԶ˽�������
recv����
send����
}
*/

#include "winsock.h"
#include "stdio.h"
#pragma comment (lib,"wsock32.lib")
struct socket_list {
	//�׽��ֹ�����нṹ
	SOCKET MainSock;
	int num;
	SOCKET socket_array[256];
};
void init_list(socket_list* list)
{
	//��ʼ���׽���
	int i = 0;
	list->num = 0;
	list->MainSock = 0;
	for (i = 0; i < 64; i++) {
		list->socket_array[i] = 0;
	}
};
void insert_list(SOCKET s, socket_list* list)
{
	//��s���뵽�׽��ֹ��������
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
	//���׽��ֹ��������ɾ��s
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
	//����������е��׽��������ӵ�ָ����״̬����fd_list�У�
	//ע������FD_SET()
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

	//��select��صı������׽��ֶ��еȳ�ʼ��
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
		//����������״̬����
		make_fdlist(&sock_list, &readfds);
		make_fdlist(&sock_list,&writefds);
		make_fdlist(&sock_list,&exceptfds);
		retval = select(0, &readfds, &writefds, &exceptfds, &timeout);
		if (retval == SOCKET_ERROR) {
			retval = WSAGetLastError();
			break;
		}
		if (FD_ISSET(sock_list.MainSock, &readfds)) {
			//���׽����ϵ�read�¼�
			len = sizeof(remote_addr);
			sock = accept(sock_list.MainSock, (sockaddr*)&remote_addr, &len);
			if (sock == SOCKET_ERROR)continue;
			printf("�ͶԷ���������\n");
			insert_list(sock, &sock_list);
		}
		//���׽����ϵĸ����¼�����
		//ע��˴�Ӧ���Ϊ���ϴ��׽��ֹ�����������ȡ��sock��ѭ��ģʽ

		for (i = 0; i < 64; i++) {
			if (sock_list.socket_array[i] == 0)continue;
			sock = sock_list.socket_array[i];
			//��ʱ�Ѿ����뵽�׽��ֹ��������ˣ�����Ҫ�ٹ�������ɶ����
			if (FD_ISSET(sock, &readfds)) {
				recv(sock, buf, 128, 0);
				printf("���յ���%s", buf);
			}


			if (FD_ISSET(sock, &writefds)) {
				//�ǲ��Ǹ�send�ˣ���������
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