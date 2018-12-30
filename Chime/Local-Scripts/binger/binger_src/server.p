#include "server.h"
#include "artdata.h"
#include "hash.h"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#ifdef USE_SSL
#include <openssl/ssl.h>
#include <openssl/rand.h>
#else
#define SSL void
#endif

#define BUFLEN 510
#define INBUFSIZE 16000
#define MAXFIELDS 10

enum { CF_Inuse = 1 };
struct S_CONNECTION {
	int sckt, status, timeout, k_bytes_sec;
	int class, stats, id, priority;
	P_GROUP p_group;
	long long numbytes;
	ushort port, flags;
	struct timeval start_time;
	P_SERVER p_server;
	time_t closetime;
	SSL *handle;
	char *inbuf, *inbufend, *nextinbuf;
	char outbuf[BUFLEN+2], inbuffer[INBUFSIZE+2];
};

enum {
	SF_Init = 1, SF_Auth = 2, SF_Overview = 4, SF_Ssl = 8, SF_No_TLS = 16
};

typedef struct S_PRIDATA PRIDATA, *P_PRIDATA;
struct S_PRIDATA {
	int waiters;
	pthread_mutex_t conn_sem;
	pthread_cond_t conn_cond;
};
struct S_SERVER {
	pthread_mutex_t server_sem;
	ushort port, flags;
	int conn_max, conn_used, max_pri;
	int timeout, servernum, idletime;
	int prefetch, groupcache;
	int numfields, fields[MAXFIELDS]; /* overview fields */
	int field_off[MAXFIELDS]; /* 0 or size of header name */
	PRIDATA pri_data[PRI_MAX];
	P_CONNECTION *connects;
	char name[128], dispname[12];
	char ssl_info[48], user[32], pass[32];
	struct hostent hostinfo;
	char hostbuf[2048];
};

typedef struct S_QUEUEDATA QUEUEDATA, *P_QUEUEDATA;
struct S_QUEUEDATA {
	P_QUEUEDATA p_next;
	short server, task_id;
	int priority, arg;
	int *ret_status;
	void *task_data, *task_data1;
};

typedef struct S_QUEUE QUEUE, *P_QUEUE;
struct S_QUEUE {
	P_QUEUEDATA p_first, p_last;
	int count, queue_id;
};

P_SERVER GetServer( int num );
