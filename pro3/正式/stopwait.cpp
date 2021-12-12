#include "stopwait.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// common
static SWFrame sndbuf, rcvbuf;

static int check() {
    //TODO
    return 0;
}

// sender 
static int cltsock;

int sw_connect(uint32_t ipaddr, uint16_t port) {
    cltsock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in srvaddr = {
        .sin_family = AF_INET, 
        .sin_port = htons(port), 
        .sin_addr.s_addr = htonl(ipaddr),
    };

    assert( connect(cltsock, (sockaddr *)&srvaddr, sizeof(srvaddr)) != -1 );
    printf("connect %s : %d successfully!\n", inet_ntoa(srvaddr.sin_addr), port);

    return 0;
}

static inline void fill_data(int *state, const char* data, size_t len) {
    //fill with data 
    len = len > MAXN ? MAXN : len;
    sndbuf.flag=F_SEND;
    sndbuf.dlen=(uint16_t)len;

    memcpy(sndbuf.data, data, len);
    if (*state == STATE_SEND0) {
        sndbuf.seq =  FRAME0; 
        *state = STATE_WAIT0; 
    }else {
        sndbuf.seq =  FRAME1; 
        *state = STATE_WAIT1;
    }
    //check
    sndbuf.check = check();
}

static inline void send_data(size_t len) {
    len += SWHEADSIZE;
    //send data
    ssize_t sndn = 0;
    while (len > 0) {
        assert( (sndn = send(cltsock, (char *)(&sndbuf) + sndn, len, 0)) != -1 );
        len -= sndn;
    }
}

static inline void recv_ack(int *state, size_t len) {
    timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(cltsock, &readfds);

    int event;
    while (1) {
        int nfds = select(1024, &readfds, NULL, NULL, &timeout);
        assert(nfds < 0);
        if (nfds == 0) { //TIMEMOUT
            event = SW_EVENT_TIMEOUT;
        }else {
            if (FD_ISSET(cltsock, &readfds)) {
                ssize_t rcvn = recv(cltsock, &rcvbuf, sizeof(rcvbuf), 0);
                //handle the case of rcvn == sizeof(rcvbuf)
                //handle check
                event = rcvbuf.flag == F_ACK ? SW_EVENT_ACK : SW_EVENT_TIMEOUT;
            }
        }

        switch (event) {
            case SW_EVENT_TIMEOUT: 
                send_data(len); 
                timeout.tv_sec = TIMEOUT;
                timeout.tv_usec = 0;
                break;
            case SW_EVENT_ACK: 
                *state = *state == STATE_WAIT0 ? 
                        STATE_SEND1 : STATE_SEND0; 
                return;
            default: assert(0);
        }
    }
}

int sw_send(const char* data, size_t len) {
    static int state = STATE_SEND0;

    for (; ;) {
        switch (state) {
            case STATE_SEND0: case STATE_SEND1:
                fill_data(&state, data, len); send_data(len); break;
            case STATE_WAIT0: case STATE_WAIT1:
                recv_ack(&state, len); return 0;

        }
    }
}



// receiver
static int srvsock, peersock; 
int sw_listen(uint32_t ipaddr, uint16_t port) {
    srvsock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in srvaddr = {
        .sin_family = AF_INET, 
        .sin_port = htons(port), 
        .sin_addr.s_addr = htonl(ipaddr),
    };

    assert( bind(srvsock, (sockaddr *)&srvaddr, sizeof(srvaddr)) != -1 );
    assert ( listen(srvsock, 5) != -1 );
    printf("listening at %s : %d\n", inet_ntoa(srvaddr.sin_addr), port);

    assert ( (peersock = accept(srvsock, NULL, NULL)) != -1 );

    return 0;
}



static inline void send_ack() {
     //fill with data 
    sndbuf.flag = F_ACK;
    sndbuf.seq = 0;
    sndbuf.dlen = 0;

    //check
    sndbuf.check = check();

    size_t len = SWHEADSIZE;
    //send data
    ssize_t sndn = 0;
    while (len > 0) {
        assert( (sndn = send(peersock, (char *)(&sndbuf) + sndn, len, 0)) != -1 );
        len -= sndn;
    }
}

static inline void recv_data0(int *state, char *buf, size_t len) {
  for( ; ; ) {
    ssize_t rcvn = recv(peersock, &rcvbuf, sizeof(rcvbuf), 0);
    //handle rcvn != sizeof(rcvbuf)
    //handle check
     if (rcvbuf.flag == F_SEND) {
       int event = rcvbuf.seq == FRAME0 ? EVENT_FRAME0 : EVENT_FRAME1;
       switch (event) {
         case EVENT_FRAME0:
          memcpy(buf, rcvbuf.data, rcvbuf.dlen);
          len = rcvbuf.dlen;
          send_ack();
          *state = STATE_RECV1;
          return;
         case EVENT_FRAME1:
          send_ack(); break;
         default: assert(0);
       }
    }
  }  
}

static inline void recv_data1(int *state, char *buf, size_t len) {
  for( ; ; ) {
    ssize_t rcvn = recv(peersock, &rcvbuf, sizeof(rcvbuf), 0);
    //handle rcvn != sizeof(rcvbuf)
    //handle check
     if (rcvbuf.flag == F_SEND) {
       int event = rcvbuf.seq == FRAME0 ? EVENT_FRAME0 : EVENT_FRAME1;
       switch (event) {
         case EVENT_FRAME1:
          memcpy(buf, rcvbuf.data, rcvbuf.dlen);
          len = rcvbuf.dlen;
          send_ack();
          *state = STATE_RECV0;
          return;
         case EVENT_FRAME0:
          send_ack(); break;
         default: assert(0);
       }
    }
  }  
}

int sw_recv(char* buf, size_t len){
  static int state = STATE_RECV0;
      switch (state) {     
        case STATE_RECV0: 
          recv_data0(&state, buf, len); break;
        case STATE_RECV1:
          recv_data1(&state, buf, len); break;
        default: assert(0);break;
      }
}