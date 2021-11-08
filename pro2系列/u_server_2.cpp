#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h> 

#define STDIN 0//���������ļ�������

struct socket_list {
	//�׽��ֹ�����нṹ
	int MainSock;
	int num;
	int socket_array[256];
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

void insert_list(int s, socket_list* list)
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

void delete_list(int s, socket_list* list)
{
	//���׽��ֹ��������ɾ��s
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
	//����������е��׽��������ӵ�ָ����״̬����fd_list�У�
	//ע������FD_SET()
	int i;
	//there is no mainsocket!
	for (i = 0; i < 64; i++) {
		if (list->socket_array[i] > 0) {
			//printf("i=%d is%d\n",i,list->socket_array[i]);
			FD_SET(list->socket_array[i], fd_list);
		}
	}
};



int main()
{
	int s, sock;//int instead SOCKET in Linux
	struct sockaddr_in ser_addr, remote;
	unsigned int len;
	char buf[128];
	struct socket_list sock_list;//���ǽ�����socket�����б�
	fd_set readfds, writefds, exceptfds;//����socket�б���Ӧ����д������

	int i;
	unsigned long arg;
	struct timeval timeout;
	int retval;//�н�select��recv�Ⱥ����ķ���ֵ

	s = socket(AF_INET, SOCK_DGRAM, 0);//UDP
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);//���ص�ַfor����
	ser_addr.sin_port = htons(0x4321);

	bind(s, (struct sockaddr*)&ser_addr, sizeof(ser_addr));

	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	remote.sin_port = htons(0x1234);

	timeout.tv_sec = 2;
	timeout.tv_usec = 0;//���õȴ�ʱ��
	init_list(&sock_list);
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	arg = 1;
	int fd = STDIN;
	//ioctlsocket(sock_list.MainSock, FIONBIO, &arg);//����������for win
	ioctl(s, FIONBIO, &arg);//����������for linux
	ioctl(fd, FIONBIO, &arg);

	while (1) {
		//����������״̬����
		FD_ZERO(&readfds);

		insert_list(s, &sock_list);//it's important!!
		make_fdlist(&sock_list, &readfds);
		make_fdlist(&sock_list, &writefds);
		make_fdlist(&sock_list, &exceptfds);

		FD_SET(fd, &readfds);
		retval = select(1024, &readfds, &writefds, &exceptfds, &timeout);
		//printf("%d\n",retval);
				//���׽����ϵĸ����¼�����
				//ע��˴�Ӧ���Ϊ���ϴ��׽��ֹ�����������ȡ��sock��ѭ��ģʽ

		for (i = 0; i < 64; i++) {
			if (sock_list.socket_array[i] == 0)continue;
			sock = sock_list.socket_array[i];
			//printf("%d,%d\n",i,sock);
						//��ʱ�Ѿ����뵽�׽��ֹ��������ˣ�����Ҫ�ٹ�������ɶ����
			if (FD_ISSET(STDIN, &readfds)) {
				char in_buf[128];
				read(STDIN, in_buf, 127);
				len = sizeof(remote);
				if (strlen(in_buf) > 0) {
					int a;
					printf("have read from keyboard:%s\n", in_buf);
					a = sendto(s, in_buf, strlen(in_buf), 0, (sockaddr*)&remote, len);
					memset(in_buf, 0x00, sizeof(char) * 128);
					printf("send ok %d\n", a);
				}
			}
			if (FD_ISSET(sock, &readfds)) {
				//len = sizeof(sock);
				len = sizeof(remote);
				int postion = 0;
				postion = recvfrom(s, buf, 127, 0, (struct sockaddr*)&remote, &len);
				buf[postion] = '\0';
				if (strlen(buf) > 0) {
					printf("���յ���%s\n", buf);
					memset(buf, 0x00, sizeof(char) * 128);
					//sendto(s, "ACK", 3, 0, (sockaddr*)&remote, len);
				}
			}
			if (FD_ISSET(sock, &writefds)) {

				//�ǲ��Ǹ�send�ˣ���������
				len = sizeof(remote);
				//sendto(s, "ACK", 3, 0, (sockaddr*)&remote, len);
			}
			if (FD_ISSET(sock, &exceptfds)) {

				printf("excepted error.\n");

				close(sock);
			}
		}
	}
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	close(s);
	return 0;

}



