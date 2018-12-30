/*
 *
 * Copyright (C) 2016 David Borowski.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * this file is part of the binger news reader, a multithreaded text-based
 * news reading program for the visually impaired and others who wish they were.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	02111-1307	USA
 *
 */
#define I_AM_SERVER
#define USE_SSL
#include "common.h"
#include "server.p"
#include "artdata.p"
#include <signal.h>
#include <sys/file.h>
#include <ctype.h>
#ifdef USE_SSL
SSL_CTX * ssl_ctx = NULL; 

void
free_ssl_ctx( void ) {
	if ( ssl_ctx != NULL )
		SSL_CTX_free( ssl_ctx );
}

void
DoInitssl( void ) {
	time_t t=time( NULL );
	pid_t pid=getpid( );
	long l,seed;
	SSL_library_init( );
	ssl_ctx = SSL_CTX_new( SSLv23_client_method( ) );
	SSL_CTX_set_options( ssl_ctx, SSL_OP_ALL );
	SSL_CTX_set_default_verify_paths( ssl_ctx );
	atexit( free_ssl_ctx );
	/* Seed in time ( mod_ssl does this ) */
	RAND_seed( ( unsigned char * )&t,sizeof( time_t ) );
	/* Seed in pid ( mod_ssl does this ) */
	RAND_seed( ( unsigned char * )&pid,sizeof( pid_t ) );
	/* Initialize system's random number generator */
	RAND_bytes( ( unsigned char * )&seed,sizeof( long ) );
	srand48( seed );
	while( RAND_status( )==0 ) {
		l=lrand48( );
		RAND_seed( ( unsigned char * )&l,sizeof( long ) );
	}
}
#endif

#define ClearBuffer p_conn->inbufend = p_conn->nextinbuf = p_conn->inbuffer
pthread_attr_t default_attr;
static pthread_t janitor_tid;
extern int max_workers;
P_SERVER p_servers[MAXSERVERS+1];
int nntp_debug = 0, shutdown_flag = 0, use_colors = 0;
char savedir[512] = ".", infodir[512] = ".";
char help_dir[512] = ".",	statname[512];
int numservers = 0, store_msgids = 0, binary_save = 0;
	long long num_bytes[MAXSERVERS][3];
	static time_t last_time = 0, last_update = 0, stats_time = 0;
static pthread_mutex_t log_sem = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t stats_sem = PTHREAD_MUTEX_INITIALIZER;
static int refresh_interval = 600, logsync = 3, logcount = 0;
int loglevel = 3, mingrpsize = 0;
FILE *logfile = NULL, *f_cfg = NULL;
static char statsdate[32];

int DoAuth( P_CONNECTION p_conn );

void
SetDebug( int level ) {
	if ( level >= 0 && level <= 127 )
		nntp_debug = level;
}

struct hostent *
InitHostInfo( P_SERVER p_serve ) {
	struct hostent *p_hostinfo = &p_serve->hostinfo, *pServer;
	char *buf = p_serve->hostbuf;
	int my_errno, status;
	status = gethostbyname_r( p_serve->name, p_hostinfo, buf, 2048, 
	&pServer, &my_errno );
	if ( status ) return NULL;
	return p_hostinfo;
}

void
PutLog( char *fmt, ... ) {
	va_list arg;
	if ( logfile == NULL ) return;
	pthread_mutex_lock( &log_sem );
	va_start( arg, fmt );
	vfprintf( logfile, fmt, arg );
	va_end( arg );
	putc( '\n', logfile );
	if ( ++logcount >= logsync ) {
		logcount = 0;
		fflush( logfile );
	}
	pthread_mutex_unlock( &log_sem );
}

void ParseFields( P_SERVER p_serve, char *buf ) {
	char *cp = buf;
	int l, n = 0;
	do {
		l = sscanf( cp, "\t%d.%d", &p_serve->fields[n], &p_serve->field_off[n] );
		if ( l < 2 ) return;
		n++;
		cp = strchr( ++cp, '\t' );
	} while ( cp && n < MAXFIELDS );
	if ( n > 1 ) p_serve->numfields = n;
}

int
ReadStats( ) {
	int i;
	long long *nb;
	char buf[128], *cp;
	P_SERVER p_serve = NULL;
	FILE *statfile;
	pthread_mutex_lock( &stats_sem );
	MakeFileName( statname, 512, infodir, "server", "stats" );
	statfile = fopen( statname, "r" );
	if ( statfile == NULL ) {
		LOG_Error ( "cannot read stats" );
		strcpy( statsdate, ctime( &last_time ) );
		pthread_mutex_unlock( &stats_sem );
		return -1;
	}
	if ( !fgets( statsdate, 28, statfile ) ) {
		LOG_Error ( "cannot read date from stats" );
		strcpy( statsdate, ctime( &last_time ) );
		pthread_mutex_unlock( &stats_sem );
		return -1;
	}
	while ( ( cp = fgets( buf, 125, statfile ) ) ) {
		if ( p_serve != NULL && *cp == '\t' ) {
			ParseFields( p_serve, cp );
			continue;
		}
		if ( ( cp = strchr( cp, ' ' ) ) == NULL ) continue;
		*cp++ = '\0';
		for ( i = 0; i < MAXSERVERS && ( p_serve = p_servers[i] ) != NULL; i++ ) {
			if ( strcmp( p_serve->dispname, buf ) == 0 ) break;
		}
		if ( p_serve == NULL ) continue;
		nb = num_bytes[p_serve->servernum];
		sscanf( cp, "%lld %lld %lld", nb, &nb[1], &nb[2] );
	}
	fclose( statfile );
	stats_time = time( NULL );
	pthread_mutex_unlock( &stats_sem );
	return 0;
}

int
SaveStats( int force_save ) {
	P_SERVER p_serve = NULL;
	int i, n;
	long long *nb;
	FILE *statfile;
	pthread_mutex_lock( &stats_sem );
	if ( !force_save && stats_time+900 > time( NULL ) ) {
		pthread_mutex_unlock( &stats_sem );
		return 0;
	}
	statfile = fopen( statname, "w" );
	if ( statfile == NULL ) {
		LOG_Error ( "cannot save stats" );
		pthread_mutex_unlock( &stats_sem );
		return -1;
	}
	fprintf( statfile, statsdate );
	for ( i = 0; i < MAXSERVERS && ( p_serve = p_servers[i] ) != NULL; i++ ) {
		nb = num_bytes[i];
		fprintf( statfile, "%s %lld %lld %lld\n", p_serve->dispname,
			nb[0], nb[1], nb[2] );
		for ( n = 0; n < p_serve->numfields; n++ ) {
			fprintf( statfile, "\t%d.%d", p_serve->fields[n], p_serve->field_off[n] );
		}
		putc( '\n', statfile );
	}
	fclose( statfile );
	stats_time = time( NULL );
	pthread_mutex_unlock( &stats_sem );
	return 0;
}

int
DoSend( P_CONNECTION p_conn, char *buf, int len ) {
	int n;
#ifdef USE_SSL
	if ( p_conn->handle != NULL )
		n = SSL_write( p_conn->handle, buf, len );
	else
#endif
		n = send( p_conn->sckt, buf, len, MSG_NOSIGNAL );
	if ( !(nntp_debug&4) ) return n;
	fprintf( stderr, "> %s", buf );
		return n;
}

void
DoClose( P_CONNECTION p_conn, int quit_delay ) {
	if( p_conn->sckt == -1 ) return;
	if ( quit_delay > 0 ) {
		DoSend( p_conn, "quit\r\n", 6 );
		usleep( quit_delay );
	}
	shutdown( p_conn->sckt, 2 );
	close( p_conn->sckt );
	p_conn->sckt = -1;
#ifdef USE_SSL
	if ( p_conn->handle != NULL ) SSL_free( p_conn->handle );
#endif
	p_conn->handle = NULL;
}

void *JanitorThread( void *arg ) {
	int i, j, count;
	P_SERVER p_server;
	P_CONNECTION p_conn;
	time_t now;
	sigset_t sigs;
	sigemptyset( &sigs );
	sigaddset( &sigs, SIGHUP );
	sigaddset( &sigs, SIGINT );
	sigaddset( &sigs, SIGQUIT );
	sigaddset( &sigs, SIGTERM );
	pthread_sigmask( SIG_BLOCK, &sigs, NULL );
	while ( !shutdown_flag ) {
		now = time( NULL );
		if ( refresh_interval > 0 && now-refresh_interval > last_update ) {
			for ( i = 0; p_servers[i] != NULL; i++ ) {
				p_server = p_servers[i];
				if ( p_server->conn_used > 0 ) continue;
				QueueWork(	i, TASK_GroupUpdate, NULL, NULL, PRI_Low, 0, NULL );
				LOG_Debug ( "refreshing %s",p_server->dispname );
			}
			last_update = now;
		}
		for ( i = 0; p_servers[i] != NULL; i++ ) {
			count = 0;
			p_server = p_servers[i];
			pthread_mutex_lock( &p_server->server_sem );
			for ( j = 0; j < p_server->conn_max; j++ ) {
				p_conn = p_server->connects[j];
				if ( ( p_conn->flags&CF_Inuse ) || p_conn->sckt == -1 ) continue;
				if ( p_conn->closetime > now ) continue;
				DoClose( p_conn, 100000 );
				count++;
			}
			pthread_mutex_unlock( &p_server->server_sem );
			if ( count > 0 ) {
				LOG_Debug ( "closed %d connects for %s", count,
					p_server->dispname );
			}
		}
		SaveStats( 0 );
		sleep(15);
	}
	return 0;
}

int
DoOpen( P_CONNECTION p_conn ) {
	struct sockaddr_in clntAddr, servAddr;
	P_SERVER p_server = p_conn->p_server;
	struct hostent *p_hostinfo = &p_server->hostinfo;
	int					 status, sckt, i;
	p_conn->p_group = NULL;
try_again:
	p_conn->status = 0;
	sckt = socket( PF_INET, SOCK_STREAM, 0 );
	if ( -1 == sckt ) goto bad;
	bzero( &clntAddr, sizeof( clntAddr ) );
	clntAddr.sin_family			= AF_INET;
	clntAddr.sin_addr.s_addr = htonl( INADDR_ANY );
	clntAddr.sin_port = 0;
	bzero( &servAddr, sizeof( servAddr ) );
	memcpy( &servAddr.sin_addr, p_hostinfo->h_addr_list[0],
		p_hostinfo->h_length );
	servAddr.sin_family = p_hostinfo->h_addrtype;
	servAddr.sin_port	 = htons(p_conn->port);
	status = connect( sckt, (const struct sockaddr *)&servAddr,
										 sizeof( servAddr ) );
	if ( status == -1 ) goto bad;
	p_conn->sckt = sckt;
#ifdef USE_SSL
	if ( (p_server->flags&SF_Ssl) ) {
		SSL *handle = SSL_new( ssl_ctx );
		long l=lrand48( );
		char info_buf[48];
		if ( handle == NULL ) {
			strcpy( p_conn->inbuf, "null ssl handle" );
			p_conn->status = 505;
			goto bad1;
		}
		RAND_seed( ( unsigned char * )&l,sizeof( long ) );
		p_conn->handle = handle;
		SSL_set_fd( handle, sckt );
		if ( (p_server->flags&SF_No_TLS) ) {
			unsigned long opts = SSL_get_options( handle ) ;
			opts |= SSL_OP_NO_TLSv1;
			SSL_set_options( handle, opts );
		}
		status = SSL_connect( handle );
		if ( status < 0 ) {
			if ( !(p_server->flags&SF_No_TLS) ) {
				p_server->flags |= SF_No_TLS;
				DoClose( p_conn, 0 );
				goto try_again;
			}
		}
		sprintf( info_buf, "%d-bit %s (%s)", SSL_get_cipher_bits( handle, NULL ),
			SSL_get_cipher_version( handle ), SSL_get_cipher( handle ) );
		LOG_Debug ( "ssl connection %d, %s",
			p_conn->id, info_buf );
		pthread_mutex_lock( &p_server->server_sem );
		if ( *(p_server->ssl_info) == '\0' ) strcpy( p_server->ssl_info, info_buf );
		pthread_mutex_unlock( &p_server->server_sem );
	}
#endif
	ClearBuffer;
	GetResponse( p_conn, p_conn->timeout/3 );
	if ( (p_server->flags&SF_Auth) && DoAuth( p_conn ) != 0 )
		goto bad1;
	i = DoSend( p_conn, "MODE READER\r\n", 13 );
	if ( i != 13 ) goto bad;
	i = GetResponse( p_conn, p_conn->timeout );
	ClearBuffer;
	if ( p_conn->status == NNTP_AUTH_NEEDED && DoAuth( p_conn ) != 0 ) goto bad1;
	num_bytes[p_server->servernum][0] += (long long)p_conn->numbytes;
	p_conn->numbytes = 0;
	gettimeofday( &p_conn->start_time, NULL );
	return 0;
bad:
	perror( "connect error" );
	p_conn->status = errno;
	goto bad2;
bad1:
	fprintf( stderr, "connect error %s\n", p_conn->inbuf );
bad2:
	if ( sckt == -1 ) return -1;
	DoClose( p_conn, 0 );
	sleep( 1 );
	return p_conn->status;
}

int
BounceConnection( P_CONNECTION p_conn, int interval ) {
	int i;
	P_GROUP p_group = p_conn->p_group;
	if ( p_conn->sckt != -1 )
		DoClose( p_conn, 0 );
	for ( i = 0; i <= interval; i++ ) {
		sleep( 1 );
		if ( shutdown_flag ) return NNTP_CLASS_FATAL;
	}
	i = DoOpen( p_conn );
	if ( i != 0 || p_group == NULL ) return i;
	if ( SelectGroup( p_conn, p_group, UP_Change ) == 0 ) return 0;
	DoClose( p_conn, 0 );
	return -1;
}

P_CONNECTION
OpenConnect( int serve, int priority, int stats ) {
	int					 i, conn_type = priority&0xf0;
	P_SERVER p_server = p_servers[serve];
	P_PRIDATA p_pridata;
	P_CONNECTION p_conn;
	if ( shutdown_flag || p_server == NULL ) return NULL;
	priority &= 7;
	p_pridata = &p_server->pri_data[priority];
	pthread_mutex_lock( &p_server->server_sem );
	if ( priority > p_server->max_pri ) p_server->max_pri = priority;
	else if ( (conn_type&PRI_NW) && p_server->conn_used >= p_server->conn_max ) {
		pthread_mutex_unlock( &p_server->server_sem );
		return NULL;
	}
	while ( !shutdown_flag && p_server->conn_used >= p_server->conn_max ) {
		pthread_mutex_lock( &p_pridata->conn_sem );
		pthread_mutex_unlock( &p_server->server_sem );
		p_pridata->waiters++;
		LOG_Debug ( "connect wait %d pri %d for %d",
			p_pridata->waiters, priority, serve );
		do {
			pthread_cond_wait( &p_pridata->conn_cond, &p_pridata->conn_sem );
		} while ( !shutdown_flag && p_server->max_pri > priority );
		p_pridata->waiters--;
		pthread_mutex_lock( &p_server->server_sem );
		pthread_mutex_unlock( &p_pridata->conn_sem );
	}
	if ( shutdown_flag ) {
		pthread_mutex_unlock( &p_server->server_sem );
		return NULL;
	}
	for ( i = 0; i < p_server->conn_max; i++ ) {
		p_conn = p_server->connects[i];
		if ( !( p_conn->flags&CF_Inuse ) ) break;
	}
	if ( i >= p_server->conn_max ) {
		LOG_Error ( "connection not found for %d", serve );
		pthread_mutex_unlock( &p_server->server_sem );
		return NULL;
	}
	p_server->conn_used++;
	p_conn->flags |= CF_Inuse;
	pthread_mutex_unlock( &p_server->server_sem );
	LOG_Debug ( "connect %d opened, pri %d", p_conn->id, priority );
	p_conn->stats = stats;
	p_conn->priority = priority;
	p_conn->numbytes = 0;
	if ( p_conn->sckt != -1 ) {
		gettimeofday( &p_conn->start_time, NULL );
		return p_conn;
	}
	if ( DoOpen( p_conn ) == 0 ) return p_conn;
	pthread_mutex_lock( &p_server->server_sem );
	p_conn->flags &= ~CF_Inuse;
	p_server->conn_used--;
	pthread_mutex_unlock( &p_server->server_sem );
	return NULL;
}

void
ConnSetStats( P_CONNECTION p_conn, int stats ) {
	p_conn->stats = stats;
}

int
CalcConnectSpeed( P_CONNECTION p_conn, int closing ) {
	struct timeval curr_time;
	P_SERVER p_server = p_conn->p_server;
	int sn = p_server->servernum, milisecs, av_k;
	gettimeofday( &curr_time, NULL );
	milisecs = (curr_time.tv_usec-p_conn->start_time.tv_usec)/1000;
	milisecs += ((curr_time.tv_sec-p_conn->start_time.tv_sec)*1000);
	if ( closing == 0 && milisecs < 100 ) return -1;
	if ( milisecs < 10 )
		av_k = p_conn->k_bytes_sec;
	else {
		av_k = p_conn->k_bytes_sec*9;
		av_k = ((p_conn->numbytes/milisecs)+av_k)/10;
	}
	pthread_mutex_lock( &p_server->server_sem );
	p_conn->k_bytes_sec = av_k;
	num_bytes[sn][p_conn->stats] += (long long)p_conn->numbytes;
	p_conn->numbytes = 0;
	p_conn->start_time = curr_time;
	pthread_mutex_unlock( &p_server->server_sem );
	return av_k;
}

int
CloseConnect( P_CONNECTION p_conn, int doClose ) {
	P_SERVER p_server;
	int i;
	P_PRIDATA p_pridata;
	if ( p_conn == NULL ) return 0;
	p_server = p_conn->p_server;
	LOG_Debug ( "closing connection %d, close %d",
		p_conn->id, doClose );
	CalcConnectSpeed( p_conn, 1 );
	p_conn->closetime = p_conn->start_time.tv_sec+p_server->idletime;
	if ( doClose ) {
		DoClose( p_conn, 20000 );
		sleep( 2 );
	}
	pthread_mutex_lock( &p_server->server_sem );
	p_server->conn_used--;
	p_conn->flags &= ~CF_Inuse;
	for ( i = p_server->max_pri; i >= 0; i-- ) {
		p_pridata = &p_server->pri_data[i];
		if ( p_pridata->waiters > 0 ) break;
	}
	if ( i < 0 ) {
		p_server->max_pri = 0;
		pthread_mutex_unlock( &p_server->server_sem );
		return 0;
	}
	p_server->max_pri = i;
	pthread_mutex_lock( &p_pridata->conn_sem );
	pthread_mutex_unlock( &p_server->server_sem );
	pthread_cond_signal( &p_pridata->conn_cond );
	pthread_mutex_unlock( &p_pridata->conn_sem );
	return 0;
}

P_CONNECTION
SurrenderConnect( P_CONNECTION p_conn ) {
	P_SERVER p_server = p_conn->p_server;
	P_GROUP p_group;
	int stats, pri = p_conn->priority;
	if ( p_server->max_pri <= pri ) return p_conn;
	stats = p_conn->stats;
	p_group = p_conn->p_group;
	CloseConnect( p_conn, 0 );
	usleep( 50000 );
	if ( ( p_conn = OpenConnect( p_server->servernum, pri, stats ) ) == NULL ) return NULL;
	if ( SelectGroup( p_conn, p_group, UP_Change ) == 0 ) return p_conn;
	CloseConnect( p_conn, 0 );
	return NULL;
}

int
TestConnect( P_CONNECTION p_conn ) {
	P_SERVER p_server;
	if ( p_conn == NULL ) return 0;
	p_server = p_conn->p_server;
	if ( p_server->max_pri > p_conn->priority ) return p_server->max_pri;
	return 0;
}

void
FreeServer( P_SERVER p_server ) {
	P_PRIDATA p_pridata = NULL;
	int i, conn_max = p_server->conn_max;
	P_CONNECTION p_conn;
	if ( p_server->flags&SF_Init ) {
		for ( i = p_server->max_pri; i >= 0; i-- ) {
			p_pridata = &p_server->pri_data[i];
			if ( p_pridata->waiters == 0 ) continue;
			pthread_mutex_lock( &p_pridata->conn_sem );
			pthread_cond_broadcast( &p_pridata->conn_cond );
			pthread_mutex_unlock( &p_pridata->conn_sem );
			usleep( 100000 );
		}
		for ( i = 0; i < 4; i++ ) {
			usleep( 100000 );
			if ( p_server->conn_used == 0 ) break;
		}
	}
	for ( i = 0; i < conn_max; i++ ) {
		p_conn = p_server->connects[i];
		if ( p_conn->sckt != -1 ) {
			DoClose( p_conn, 20000 );
		}
		free( p_conn );
	}
	if ( p_server->connects != NULL ) free( p_server->connects );
	if ( p_server->flags&SF_Init ) {
		pthread_mutex_destroy( &p_server->server_sem );
		for ( i = 0; i < PRI_MAX; i++ ) {
			p_pridata = &p_server->pri_data[i];
			pthread_mutex_destroy( &p_pridata->conn_sem );
			pthread_cond_destroy( &p_pridata->conn_cond );
		}
	}
	free( p_server );
}

void
FreeServers( ) {
	int i;
	shutdown_flag = 1;
	SaveStats( 1 );
	for ( i = 0; p_servers[i] != NULL; i++ )
		FreeServer( p_servers[i] );
	if ( logfile != NULL ) fclose( logfile );
	flock( fileno( f_cfg ), LOCK_UN );
	fclose( f_cfg );
}

int YesNo( char *in, int *val ) {
	static char *is_yes[] = { "yes", "true", "1" };
	static char *is_no[] = { "no", "false", "0" };
	int i;
	char buf[8], ch;
	if ( in == NULL || *in == '\0' ) return -1;
	for ( i = 0; i < 7 && ( ch = in[i] ) != '\0'; i++ )
		buf[i] = tolower( ch );
	buf[i] = '\0';
	for ( i = 0; i < 3; i++ ) {
	if ( !strcmp( buf, is_yes[i] ) ) {
			*val = 1;
			return 0;
		}
	if ( !strcmp( buf, is_no[i] ) ) {
			*val = 0;
			return 0;
		}
	}
	return -1;
}
	
#define P_String 16
#define P_Bool 32
#define P_Misc 48
enum {
	S_Port = 0, S_Connects, S_Timeout, S_Priority, S_Idletime,
	S_Forceauth, S_Headers, S_Prefetch, S_Groupcache, S_Ssl,
	S_User = P_String, S_Pass, S_Dispname
};

typedef struct S_CFGPARM CFGPARM, *P_CFGPARM;
struct S_CFGPARM {
	char *key;
	int arg_num, min, max, def_val;
};

static int ParmCmp( const void *p1, const void *p2 ) {
 P_CFGPARM s1 = (P_CFGPARM)p1, s2 = (P_CFGPARM)p2;
	return strcmp( s1->key, s2->key );
}

CFGPARM svrparms[] = {
	{ "connects", S_Connects, 1, 20, 1 }, { "dispname", S_Dispname, 0, 10, 0 },
	{ "forceauth", S_Forceauth, 0, 1, 0 },
	{ "groupcache", S_Groupcache, 2, 480, 120 },
	{ "headers", S_Headers, 0, 2, 1 }, {	 "idletime", S_Idletime, 1, 60, 3 },
	{ "pass", S_Pass, 0, 15, 0 }, { "port", S_Port, 80, 65535, 0 },
	{ "prefetch", S_Prefetch, 0, 999, 500 }, { "priority", S_Priority, 0, 9, 0 },
	{ "ssl", S_Ssl, 0, 1, 0 },
	{ "timeout", S_Timeout, 5, 120, 30 }, { "user", S_User, 0, 31, 0 }
};

P_SERVER
InitServer( char *name, int intArgs[], char stringArgs[6][32], int num ) {
	int i;
	P_PRIDATA p_pridata = NULL;
	char *buf, *cp;
	P_SERVER p_serve = calloc( 1, sizeof( SERVER ) );
	P_CONNECTION p_conn;
	if ( p_serve == NULL ) return NULL;
	p_serve->numfields = -1;
	strcpy( p_serve->name, name );
	cp = stringArgs[S_Dispname-P_String];
	if ( cp && *cp ) strcpy( p_serve->dispname, cp );
	cp = stringArgs[S_User-P_String];
	if ( cp && *cp ) 
		strcpy( p_serve->user, cp );
	cp = stringArgs[S_Pass-P_String];
	if ( cp && *cp ) strcpy( p_serve->pass, cp );
	if ( InitHostInfo( p_serve ) == NULL ) {
		FreeServer( p_serve );
		return NULL;
	}
	p_serve->servernum = num;
	p_serve->conn_max = intArgs[S_Connects];
	if ( intArgs[S_Port] > 0 ) p_serve->port = intArgs[S_Port];
	else {
		p_serve->port = (intArgs[S_Ssl] != 0) ? 563 : 119;
	}
	p_serve->prefetch = intArgs[S_Prefetch];
	p_serve->timeout= intArgs[S_Timeout];
	p_serve->idletime = intArgs[S_Idletime]*60;
	p_serve->groupcache = intArgs[S_Groupcache]*3600;
	p_serve->connects = calloc( p_serve->conn_max, sizeof( P_CONNECTION ) );
	for ( i = 0; i < p_serve->conn_max; i++ ) {
		p_conn = calloc( 1, sizeof( CONNECTION ) );
		if ( p_conn == NULL ) break;
		p_conn->sckt = -1;
		p_conn->id = num*10+i;
		p_conn->port = p_serve->port;
		p_conn->timeout = p_serve->timeout;
		p_conn->p_server = p_serve;
		buf = p_conn->inbuffer;
		p_conn->inbuf = buf;
		p_conn->inbufend = buf;
		p_conn->nextinbuf = buf;
		p_serve->connects[i] = p_conn;
	}
	if ( i < p_serve->conn_max ) {
		FreeServer( p_serve );
		return NULL;
	}
	pthread_mutex_init( &p_serve->server_sem, NULL );
	for ( i = 0; i < PRI_MAX; i++ ) {
		p_pridata = &p_serve->pri_data[i];
		pthread_mutex_init( &p_pridata->conn_sem, NULL );
		pthread_cond_init( &p_pridata->conn_cond, NULL );
	}
	p_serve->flags = SF_Init;
	if ( intArgs[S_Forceauth] ) p_serve->flags |= SF_Auth;
	if ( intArgs[S_Headers] == 1 ) p_serve->flags |= SF_Overview;
	if ( intArgs[S_Ssl] == 1 ) p_serve->flags |= SF_Ssl;
	return p_serve;
}

int
ParseServerInfo( FILE *f_cfg, char *server, char *cp ) {
	int intArgs[16];
	int i, arg, l, state = 1;
	int nparms = sizeof(svrparms)/sizeof(CFGPARM);
	P_SERVER p_server;
	char *cp1, stringArgs[6][32], key[16], buf[128];
	CFGPARM cfg_in = { key, 0, 0, 0, 0 };
	P_CFGPARM p_parm;
	if ( numservers >= MAXSERVERS ) return -1;
	bzero( stringArgs, sizeof( stringArgs ) );
	bzero( intArgs, sizeof( intArgs ) );
	for ( i = 0; i < nparms; i++ ) {
		arg = svrparms[i].arg_num;
		if ( arg >= P_String ) continue;
		intArgs[arg] = svrparms[i].def_val;
	}
	while ( 1 ) {
		while ( !( cp = NextToken( cp, key, &cp1 ) ) ) {
			if ( !( cp = fgets( buf, 127, f_cfg ) ) ) {
				fprintf( stderr, "eof while looking for }\n" );
				return -1;
			}
		}
		if ( *key == '{' ) {
			state = 2;
			continue;
		} else if ( *key == '}' ) break;
		if ( state < 2 ) {
			fprintf( stderr, "expected { before %s\n", key );
			return -1;
		}
		p_parm = bsearch( &cfg_in, svrparms, nparms, sizeof(CFGPARM),	ParmCmp );
		if ( p_parm == NULL ) {
			fprintf( stderr, "unknown configuration option %s\n", key );
			return -1;
		}
		arg = p_parm->arg_num;
		if ( arg >= P_String ) {
			arg -= P_String;
			l = strlen( cp1 );
			if ( l < p_parm->max ) strcpy( stringArgs[arg], cp1 );
			else {
				fprintf( stderr, "value too long for %s\n", key );
				return -1;
			}
		} else if ( p_parm->min == 0 && p_parm->max == 1 ) {
			l = YesNo( cp1, &intArgs[arg] );
			if ( l < 0 ) {
				fprintf( stderr, "expected boolean for %s\n", key );
				return -1;
			}
		} else {
			l = atoi( cp1 );
			if ( l < p_parm->min || l > p_parm->max ) {
				fprintf( stderr, "value out of range for %s, range %d-%d\n", key,
					p_parm->min, p_parm->max );
				return -1;
			}
			intArgs[arg] = l;
		}
	}
	p_server = InitServer( server, intArgs, stringArgs, numservers );
	if ( p_server == NULL ) {
		fprintf( stderr, "can't init server %s\n", server );
		return -1;
	}
	p_servers[numservers++] = p_server;
	return 0;
}

enum {
	C_Debug = 0, C_Flush, C_Level, C_Refresh, C_Retain, C_Worker, C_MinGroup,
		C_Color = P_Bool, C_Store, C_BinSave, C_Log = P_String, C_HelpDir,
	C_Dir = P_Misc, C_Info, C_Server, C_Editor, C_Poster, C_Par2
};

CFGPARM cfgparms[] = {
	{ "color", C_Color, 0, 1, 0 }, { "debug", C_Debug, 0, 127, 0 },
	{ "directory", C_Dir, 0, 0, 0 }, { "downloaddir", C_Dir, 0, 0, 0 },
	{ "editor", C_Editor, 0, 0, 0 }, { "helpdir", C_HelpDir, 0, 0, 0 },
	{ "infodir", C_Info, 0, 0, 0 }, { "log", C_Log, 0, 0, 0 },
	{ "logflush", C_Flush, 0, 9, 0 }, { "loglevel", C_Level, 1, 5, 1 },
	{ "mingroupsize", C_MinGroup, 0, 1000, 1 },
	{ "par2", C_Par2, 0, 0, 0 },
	{ "poster", C_Poster, 0, 0, 0 },
	{ "refresh", C_Refresh, 5, 120, 15 }, { "retain",	C_Retain, 1, 100, 7 },
	{ "save_binary", C_BinSave, 0, 1, 0 }, { "server", C_Server, 0, 0, 0 },
	{ "store_msgids", C_Store, 0, 1, 0 },
	{ "workers", C_Worker, 1, MAXTHREAD-4, 3 }
};

int
SetIntParm( P_CFGPARM parm, char *val ) {
	int l = atoi( val );
	if ( l < parm->min || l > parm->max ) {
		fprintf( stderr, "value out of range for %s, range %d-%d\n",
			parm->key, parm->min, parm->max );
		return -1;
	}
	switch( parm->arg_num ) {
		case C_Debug:
			nntp_debug = l;
			break;
		case C_Flush:
			logsync = l;
			break;
		case C_Level:
			loglevel = l;
			break;
		case C_MinGroup:
			mingrpsize = l;
			break;
		case C_Refresh:
			refresh_interval = l*60;
			break;
		case C_Retain:
			SetDefaultRetain( l );
			break;
		case C_Worker:
			max_workers = l;
			break;
		default: return -1;
	}
	return 0;
}

int
ReadConfig( char *filename ) {
	int i, arg, status = 0;
	char *cp, *cp1;
	char buf[512],key[16], logname[512];
	int nparms = sizeof(cfgparms)/sizeof(CFGPARM);
	CFGPARM cfg_in = { key, 0, 0, 0, 0 };
	P_CFGPARM p_parm;
	bzero( p_servers, sizeof( p_servers ) );
	if ( filename && strlen( filename ) < 510 )
		strcpy( buf, filename );
	else MakeFileName( buf, 512, GetHomeDir( ), ".binger.cfg", NULL );
	if ( (f_cfg = fopen( buf, "r" ) ) == NULL ) {
		perror( buf );
		exit( 1 );
	}
	if ( flock( fileno( f_cfg ), LOCK_EX|LOCK_NB ) != 0 ) {
		perror( buf );
		printf( "another binger is running\n" );
		exit( 1 );
	}
	while ( ( cp = fgets( buf, 127, f_cfg ) ) && status == 0 ) {
		cp = NextToken( cp, key, &cp1 );
		p_parm = bsearch( &cfg_in, cfgparms, nparms, sizeof(CFGPARM),	ParmCmp );
		if ( p_parm == NULL ) {
			status = -1;
			break;
		}
		arg = p_parm->arg_num;
		if ( arg < P_String )
			status = SetIntParm( p_parm, cp1 );
		else if ( arg == C_Server )
			ParseServerInfo( f_cfg, cp1, cp );
		else if ( arg == C_Dir ) {
			if ( MakeDir( cp1, savedir ) == NULL ) exit( 1 );
		} else if ( arg == C_Info ) {
			if ( MakeDir( cp1, infodir ) == NULL ) exit( 1 );
		} else if ( arg == C_HelpDir ) strcpy( help_dir, cp1 );
		else if ( arg == C_Log ) {
			strncpy( logname, cp1, 510 );
			logname[510] = '\0';
		} else if ( arg == C_Editor ) status = SetEditor( cp1 );
		else if ( arg == C_Poster ) status = SetPoster( cp1 );
		else if ( arg == C_Par2 ) status = SetPar2( cp1 );
		else if ( arg == C_Color )
			status = YesNo( cp1, &use_colors );
		else if ( arg == C_Store )
			status = YesNo( cp1, &store_msgids );
		else if ( arg == C_BinSave )
			status = YesNo( cp1, &binary_save );
		else status = -1;
	}
	if ( numservers == 0 ) status = -1;
	if ( status ) {
		fprintf( stderr, "error in config\n" );
	return status;
	}
	last_time = time( NULL );
	last_update = last_time;
	if ( *logname ) {
		MakeFileName( buf, 512, infodir, logname, NULL );
		logfile = fopen( buf, "a" );
		if ( logfile == NULL ) {
			fprintf( stderr, "cannot open log %s\n", buf );
			return -1;
		}
		fprintf( logfile, "binger %d started %s", getpid(), 
			ctime( &last_time ) );
	} else loglevel = 0;
	ReadStats( );
#ifdef USE_SSL
	for ( i = 0; i < numservers; i++ ) {
		if ( (p_servers[i]->flags&SF_Ssl) ) break;
	}
	if ( i < numservers ) DoInitssl( );
#endif
	pthread_attr_init( &default_attr );
	pthread_attr_setdetachstate( &default_attr, PTHREAD_CREATE_DETACHED );
	status = pthread_create(&janitor_tid, &default_attr, JanitorThread, (void *)NULL);
		if ( status ) return -2;
	return status;
}
	
void
UngetLine( P_CONNECTION p_conn ) {
	p_conn->nextinbuf = p_conn->inbuf;
}

char *
GetLine( P_CONNECTION p_conn, int timeout, int *p_len ) {
	char *cp = p_conn->nextinbuf, *cp1;
	int len, status = 0, sckt = p_conn->sckt;
	if ( shutdown_flag ) return NULL;
	if ( cp < p_conn->inbufend ) {
		if ( ( cp1 = strchr( cp, '\n' ) ) ) goto have_line;
		cp1 = p_conn->inbuffer;
		while ( cp < p_conn->inbufend ) *cp1++ = *cp++;
		p_conn->inbufend = cp1;
	} else p_conn->inbufend = p_conn->inbuffer;
	p_conn->nextinbuf = p_conn->inbuffer;
	do {
		if ( sckt == -1 ) {
			p_conn->class = NNTP_CLASS_FATAL;
			p_conn->status = NNTP_TMPERR;
			*p_len = -1;
			return NULL;
		}
#ifdef USE_SSL
		if ( p_conn->handle == NULL && timeout > 0 ) {
#else
		if ( timeout > 0 ) {
#endif
		fd_set readset;
		struct timeval tm = { timeout, 0 };
			FD_ZERO( &readset );
			FD_SET( sckt, &readset );
			status = select( sckt+1, &readset, 0, 0, &tm );
			if ( status < 1 ) {
				p_conn->class = NNTP_CLASS_TIMEOUT;
				p_conn->status = NNTP_TMPERR;
				p_conn->inbuf =strcpy( p_conn->nextinbuf, "timeout" );
				*p_len = -2;
				return NULL;
			}
		}
		cp = p_conn->inbufend;
		len = INBUFSIZE-(cp-p_conn->inbuffer);
#ifdef USE_SSL
		if ( p_conn->handle != NULL )
			len = SSL_read( p_conn->handle, cp, len );
		else len = recv( sckt, cp, len, MSG_NOSIGNAL );
#else
		len = recv( sckt, cp, len, MSG_NOSIGNAL );
#endif
		if ( nntp_debug&1 ) fprintf( stderr, "GetLine got %d\n", len );
		if ( len < 1 ) {
			p_conn->class = NNTP_CLASS_FATAL;
			p_conn->status = NNTP_TMPERR;
			*p_len = -1;
			return NULL;
		}
		p_conn->numbytes += len;
		cp1 = cp+len;
		p_conn->inbufend = cp1;
		*cp1 = '\0';
	} while ( ( cp1 = strchr( cp, '\n' ) ) == NULL );
	cp = p_conn->nextinbuf;
	CalcConnectSpeed( p_conn, 0 );
have_line:
	p_conn->nextinbuf = cp1+1;
	if ( cp1[-1] == '\r' ) cp1--;
	*cp1 = '\0';
	len = cp1-cp;
	p_conn->inbuf = cp;
	if ( nntp_debug&1 ) fprintf( stderr, "< %s\n", cp );
	*p_len = len;
	return cp;
}

int
GetResponse( P_CONNECTION p_conn, int timeout ) {
	int len;
	char *cp = GetLine( p_conn, timeout, &len );
	if ( len == -1 ) {
		LOG_Error ( "read error on connection %d", p_conn->id );
		return NNTP_CLASS_FATAL;
	}
	if ( len == -2 ) {
		LOG_Error ( "time out on connection %d", p_conn->id );
		return NNTP_CLASS_TIMEOUT;
	}
	if ( *cp < '0' || *cp > '9' ) {
		len = NNTP_CLASS_UNKNOWN;
		p_conn->status = NNTP_NOSTAT;
	} else {
		p_conn->status = atoi( cp );
		len = (int)(*cp&15);
		if ( (nntp_debug&3) == 2 ) fprintf( stderr, "< %s\n", cp );
	}
	p_conn->class = len;
	if ( len >= NNTP_CLASS_ERR )
		LOG_Error ( "connection %d %s", p_conn->id, cp );
	return len;
}

int
DoAuth( P_CONNECTION p_conn ) {
		char auth_buf[64];
	int i, len;
	P_SERVER p_serve = p_conn->p_server;
	char* auth_user = p_serve->user, * auth_pass = p_serve->pass;
	if ( !*auth_user ) return -2;
	sprintf( auth_buf, "AUTHINFO USER %s\r\n", auth_user );
	len = strlen( auth_buf );
	i = DoSend( p_conn, auth_buf, len );
	ClearBuffer;
	if ( i != len ) return -1;
	len = GetResponse( p_conn, p_conn->timeout/3 );
	if ( len == NNTP_CLASS_OK ) return 0;
	if ( len != NNTP_CLASS_CONT || !*auth_pass ) return -2;
	sprintf( auth_buf, "AUTHINFO PASS %s\r\n", auth_pass );
	len = strlen( auth_buf );
	i = DoSend( p_conn, auth_buf, len );
	if ( i != len ) return -1;
	len = GetResponse( p_conn, p_conn->timeout/3 );
	if ( len != NNTP_CLASS_OK ) return -2;
	p_serve->flags |= SF_Auth;
		return 0;
}

int
SendCmd( P_CONNECTION p_conn, char *command, int no_wait ) {
	char *sp = command, *dp = p_conn->outbuf, *p_end = dp+BUFLEN-2;
	int len, i;
	if ( shutdown_flag ) return -1;
	while ( *sp && dp < p_end ) *dp++ = *sp++;
	*dp++ = '\r';
	*dp++ = '\n';
	*dp = '\0';
	len = dp-p_conn->outbuf;
resend:
	i = DoSend( p_conn, p_conn->outbuf, len );
	if ( no_wait )
		return ( i != len ) ? -1 : 0;
	ClearBuffer;
	if ( i != len ) i = NNTP_CLASS_UNKNOWN;
	else i = GetResponse( p_conn, p_conn->timeout );
	if ( i == NNTP_CLASS_UNKNOWN ) {
		for ( i = 10; BounceConnection( p_conn, i ) != 0; i *= 2 ) {
			if ( i > 80 || shutdown_flag ) return NNTP_CLASS_FATAL;
			LOG_Status ( "bounced connection %d interval %d",
				p_conn->id, i );
		}
		goto resend;
	}
	if ( p_conn->status != NNTP_AUTH_NEEDED ) return i;
	if ( DoAuth( p_conn ) == 0 ) goto resend;
	return p_conn->class;
}

int
NntpStatus( P_CONNECTION p_conn, char *lastbuf ) {
	if ( lastbuf ) lastbuf = p_conn->inbuf;
	return p_conn->status;
}

P_SERVER
GetServer( int num ) {
	return p_servers[num];
}

int
GetServerStats( int num, int *stats, char *ssl_info ) {
	int i, r;
	long long *nb = num_bytes[num];
	P_SERVER p_server = p_servers[num];
	if ( p_server == NULL ) return -1;
	for ( i = 0; i < 3; i++ ) {
		stats[i] = (int)(nb[i]/1000l);
		r = (int)(nb[i]%1000l);
		if ( r >= 500 ) stats[i]++;
	}
	strcpy( ssl_info, p_server->ssl_info );
	return p_server->conn_used;
}

char *
GetDispName( int num ) {
	char *cp = "no name";
	P_SERVER p_server = p_servers[num];
	if ( p_server && p_server->dispname ) cp = p_server->dispname;
	return cp;
}
