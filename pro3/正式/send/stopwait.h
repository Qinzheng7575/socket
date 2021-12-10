
//----------------------------------
// Í·ÎÄ¼þstopwait.h

#ifndef __STOPWAIT_H__
#define __STOPWAIT_H__

#include <netinet/ip.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>

// common
#define TIMEOUT 5

#define MAXN 128
typedef struct sw_frame {
    uint8_t   flag : 2;
    uint8_t   seq : 2;
    uint16_t  len : 12;
    uint16_t  check : 16;
    char data[MAXN];
}SWFrame;

// sender
enum { EVENT_TIMEOUT, EVENT_ACK, };
enum { STATE_SEND0, STATE_SEND1, STATE_WAIT0, STATE_WAIT1, };

// receiver
enum { EVENT_FRAME0, EVENT_FRAME1, };
enum { STATE_RECV0, STATE_RECV1, };
#define DELAY 5

int sw_send(const char* data, size_t len);
int sw_recv(char* buf, size_t* len);
int sw_init(uint32_t ipaddr, uint16_t port);

#endif

