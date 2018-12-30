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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <sys/file.h>
#include <time.h>

#define BUFFSIZE 8192
static SSL_CTX * ssl_ctx = NULL; 
static SSL *ssl_handle = NULL;
static struct hostent hostinfo;
static char ssl_info[48];
int ssl_port = 563, nntp_port = 1119, flags = 0;
int ssl_sckt = -1, sckt = -1, accept_sckt = -1;
char rx_buf[BUFFSIZE], tx_buf[BUFFSIZE];
long long rx_bytes, tx_bytes;
time_t t_start;

void
free_ssl( void ) {
	if ( ssl_ctx != NULL )
		SSL_CTX_free( ssl_ctx );
	ssl_ctx = NULL;
}

void
init_ssl( void ) {
	time_t t=time( NULL );
	pid_t pid=getpid( );
	long l,seed;
	SSL_library_init( );
	ssl_ctx = SSL_CTX_new( SSLv23_client_method( ) );
	SSL_CTX_set_options( ssl_ctx, SSL_OP_ALL );
	SSL_CTX_set_default_verify_paths( ssl_ctx );
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

struct hostent *
InitHostInfo( char *server_name ) {
	struct hostent *pServer;
	int my_errno, status;
	status = gethostbyname_r( server_name, &hostinfo, rx_buf, 2048, 
	&pServer, &my_errno );
	if ( status ) return NULL;
	return &hostinfo;
}

int
DoOpen( void ) {
	struct sockaddr_in clntAddr, servAddr;
	int					 status, i;
	long l=lrand48( );
	char info_buf[48];
try_again:
	ssl_sckt = socket( PF_INET, SOCK_STREAM, 0 );
	if ( -1 == ssl_sckt ) goto bad;
	bzero( &clntAddr, sizeof( clntAddr ) );
	clntAddr.sin_family			= AF_INET;
	clntAddr.sin_addr.s_addr = htonl( INADDR_ANY );
	clntAddr.sin_port = 0;
	bzero( &servAddr, sizeof( servAddr ) );
	memcpy( &servAddr.sin_addr, hostinfo.h_addr_list[0],
		hostinfo.h_length );
	servAddr.sin_family = hostinfo.h_addrtype;
	servAddr.sin_port	 = htons(ssl_port );
	status = connect( ssl_sckt, (const struct sockaddr *)&servAddr,
										 sizeof( servAddr ) );
	if ( status == -1 ) goto bad;
	ssl_handle = SSL_new( ssl_ctx );
	if ( ssl_handle == NULL ) {
		goto bad1;
	}
	RAND_seed( ( unsigned char * )&l,sizeof( long ) );
	SSL_set_fd( ssl_handle, ssl_sckt );
	if ( (flags&1) )
		ssl_handle->options |= SSL_OP_NO_TLSv1;
	status = SSL_connect( ssl_handle );
	if ( status < 0 ) {
		if ( !(flags&1) ) {
			flags |= 1;
			shutdown( ssl_sckt, 2 );
			close( ssl_sckt );
			goto try_again;
		}
	}
	fprintf( stderr, "%d-bit %s (%s)\n", SSL_get_cipher_bits( ssl_handle, NULL ),
		SSL_get_cipher_version( ssl_handle ), SSL_get_cipher( ssl_handle ) );
	return 0;
bad:
	perror( "connect error" );
	goto bad2;
bad1:
	fprintf( stderr, "null ssl\n" );
bad2:
	if ( ssl_sckt == -1 ) return -1;
	sleep( 1 );
	shutdown( ssl_sckt, 2 );
	close( ssl_sckt );
	ssl_sckt = -1;
	if ( ssl_handle ) SSL_free( ssl_handle );
	ssl_handle = NULL;
	return status;
}

int OpenServer( char *in_address, int port ) {
	struct sockaddr_in sa;
	int s_sock, one = 1;
	bzero( &sa, sizeof(sa) );
	sa.sin_port = htons((unsigned short)port);
	inet_pton( AF_INET, in_address, &sa.sin_addr.s_addr );
	sa.sin_family = PF_INET;
	if ( ( s_sock = socket ( AF_INET, SOCK_STREAM, 0 ) ) <0 ) {
		fprintf( stderr, "cannot get socket\n" );
		return -1;
	}
	if ( setsockopt( s_sock, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(int) ) ) {
		fprintf( stderr, "cannot set keepalive\n" );
			shutdown( s_sock, 2 );
	close( s_sock );
			return -2;
	 }
	if ( setsockopt( s_sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int) ) ) {
		fprintf( stderr, "cannot set keepalive\n" );
			shutdown( s_sock, 2 );
	close( s_sock );
			return -3;
	 }
	if ( bind ( s_sock, (struct sockaddr *)&sa, sizeof(sa) ) <0 ) {
		fprintf( stderr, "cannot bind socket\n" );
			shutdown( s_sock, 2 );
	close( s_sock );
		return -5;
	}
	listen ( s_sock, 5 );
	return s_sock;
}

int RunServer( void ) {
	int i, l, s_local;
	fd_set accept_set;
	struct sockaddr_in isa;
	char buff[64];
	s_local = OpenServer( "127.0.0.1", nntp_port );
	if ( s_local < 0 )
		return s_local;
	fprintf( stderr, "listening on port %d\n", nntp_port );
	while ( 1 ) {
		struct timeval wait = { 1, 0 };
		FD_ZERO( &accept_set );
		FD_SET( s_local, &accept_set );
		l = select( s_local+1, &accept_set, NULL, NULL, &wait );
		if ( l < 1 ) continue;
		if ( FD_ISSET( s_local, &accept_set ) ) break;
	}
	i = sizeof(isa);
	sckt = accept( s_local, (struct sockaddr *)&isa, &i );
	shutdown( s_local, 2 );
	close( s_local );
	return sckt;
}

void DoTransfer( void ) {
	int len, status = 0;
	int max = sckt;
	rx_bytes = 0l;
	tx_bytes = 0l;
	t_start = time( NULL );
	if ( ssl_sckt > max ) max = ssl_sckt;
	fd_set readset;
	DoOpen( );
	while ( 1 ) {
		struct timeval tm = { 1, 0 };
		FD_ZERO( &readset );
		FD_SET( sckt, &readset );
		FD_SET( ssl_sckt, &readset );
		status = select( max+1, &readset, 0, 0, &tm );
		if ( status == 0 ) continue;
		if ( status < 0 ) break;
		len = BUFFSIZE;
		if ( FD_ISSET( ssl_sckt, &readset ) ) {
			len = SSL_read( ssl_handle, rx_buf, len );
			if ( len < 1 ) break;
			rx_bytes += len;
			status =	send( sckt, rx_buf, len, MSG_NOSIGNAL );
			if ( status != len ) break;
		}
		if ( FD_ISSET( sckt, &readset ) ) {
			len = recv( sckt, tx_buf, len, MSG_NOSIGNAL );
			if ( len < 1 ) break;
			tx_bytes += len;
			status =	SSL_write( ssl_handle, tx_buf, len );
			if ( status != len ) break;
		}
	}
	usleep( 100000 );
	shutdown( ssl_sckt, 2 );
	close( ssl_sckt );
	ssl_sckt = -1;
	SSL_free( ssl_handle );
	ssl_handle = NULL;
	shutdown( sckt, 2 );
	close( sckt );
	sckt = -1;
	len = time( NULL )-t_start;
	fprintf( stderr, "%lld read, %lld written, time %d seconds\n",
		rx_bytes, tx_bytes, len );
}

int main( int argc, char *argv[] ) {
	if ( argc < 2 ) {
		fprintf( stderr, "Usage: %s server [connects]\n", argv[0] );
		exit ( 1 );
	}
	if ( !InitHostInfo( argv[1] ) ) {
		fprintf( stderr, "server init for %s failed\n", argv[1] );
		exit ( 2 );
		}
	init_ssl( );
	if ( RunServer( ) > 0 )
		DoTransfer( );
	free_ssl( );
	exit( 0 );
}
