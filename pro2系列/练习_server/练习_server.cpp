#include <winsock.h>
#include <stdio.h>
#pragma comment (lib,"wsock32.lib")
struct socket_list{
    SOCKET MainSock;
    int num;//在列表中的sock数目
    SOCKET socket_array[64];
    //因为SOCKET本质是整数型的标识符，所以可以定义数组
};

void init_list(socket_list* list) {//“引用”了数组
    int i;
    list->MainSock = 0;
    list->num = 0;
    for (i = 0; i < 64; i++) {
        list->socket_array[i] = 0;
    }
}//初始化置零，防止野指针

void insert_list(SOCKET s,socket_list * list) {
    int i;
    for (i = 0; i < 64; i++) {
        if (list->socket_array[i] == 0) {
            list->socket_array[i] = s;
            list->num += 1;//用num来限制边界，64来规定空间
            break;
        }
    }
}

void delete_list(SOCKET s, socket_list* list) {
    int i;
    for (i = 0; i < 64; i++) {
        if (list->socket_array[i] == s) {
            list->socket_array[i] = 0;
            list->num -= 1;
            break;
        }
    }
}

void make_fdlist(socket_list* list, fd_set* fd_list) {
    int i;
    FD_SET(list->MainSock, fd_list);
    /*
    * from the global name space
    * 确定一个或多个套接口的状态。对每一个套接口，
    * 调用者可查询它的可读性、可写性及错误状态信息。
    * 用fd_set结构来表示一组等待检查的套接口。
    * 在调用返回时，这个结构存有满足一定条件的套接口组的子集，
    * 并且select()返回满足条件的套接口的数目。
    */
    //将管理队列中的套接字逐个添加到指定的状态队列fd_list中，
    for (i = 0; i < 64; i++) {
        if (list->socket_array[i] > 0) {
            FD_SET(list->socket_array[i], fd_list);//向集合添加描述字s。
        }
    }
}

int main()
{
    SOCKET s, sock;
    struct sockaddr_in ser_addr, remote_addr;//本地和远端套接字
    int len;
    char buf[128];

    WSAData wsa;
    int retval;

    struct socket_list sock_list;//套接字标识符数组
    fd_set readfds, writefds, exceptfds;//三个状态队列
    timeval timeout;
    /*
    tv_sec 秒
    tv_usec 微秒
    */
    int i;
    unsigned long arg;

    WSAStartup(0x101, &wsa);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == SOCKET_ERROR)return(0);
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(0x1234);

    bind(s, (sockaddr*)&ser_addr, sizeof(ser_addr));

    listen(s, 5);
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    init_list(&sock_list);
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);//将set初始化为空集NULL。
    sock_list.MainSock = s;
    arg = 1;//允许非阻塞

    ioctlsocket(sock_list.MainSock, FIONBIO, &arg);
    /*
    s：一个标识套接口的描述字。 
    cmd：对套接口s的操作命令。 
    argp：指向cmd命令所带参数的指针。
    此函数可以方便的控制阻塞模式
    */

    while (true)
    {
        make_fdlist(&sock_list, &readfds);//把咱们定义的套接字管理队列加到fd_list里去
        //make_fdlist(&sock_list,&writefds);
        //make_fdlist(&sock_list,&exceptfds);
        retval = select(0, &readfds, &writefds, &exceptfds, NULL);//select将三个队列还有对应状态的套接字保留
        if (retval == SOCKET_ERROR) {
            retval = WSAGetLastError();
            break;
        }

        if (FD_ISSET(sock_list.MainSock, &readfds)) {//用于测试指定的文件描述符是否在该集合中。
            len = sizeof(remote_addr);
            sock = accept(sock_list.MainSock, (sockaddr*)&remote_addr, &len);//检测到了有听状态的，下一步就是accept建立连接
            //Mainsock是为了和可能到来的建立连接
            if (sock == SOCKET_ERROR)continue;
            printf("accept a connection\n");
            insert_list(sock, &sock_list);//temp变量sock，加入到所有列表去
        }

        for (i = 0; i < 64; i++) {
            if (sock_list.socket_array[i] == 0)continue;
            sock = sock_list.socket_array[i];
            if (FD_ISSET(sock, &readfds)) {
                retval = recv(sock, buf, 128, 0);//收东西，存buf
                if (retval == 0) {
                    closesocket(sock);
                    printf("close a socket\n");
                    delete_list(sock, &sock_list);
                    continue;
                }
                else if (retval == -1) {//收东西失败，有可能是缓存区已满或套接字已经关闭
                    retval = WSAGetLastError();
                    if (retval == WSAEWOULDBLOCK)continue;
                    closesocket(sock);
                    printf("close a socket\n");
                    delete_list(sock, &sock_list);
                    continue;
                }
                buf[retval] = 0;
                printf("->%s\n", buf);
                send(sock, "ACK by server", 13, 0);//已经成功收到，回复对面
            }
            //if(FD_ISSET(sock,&writefds)){
            //}
            //if(FD_ISSET(sock,&exceptfds)){
            //}
        }
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);
    }
    closesocket(sock_list.MainSock);
    WSACleanup();
    return(0);
}
