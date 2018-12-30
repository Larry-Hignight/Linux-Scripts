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
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

#define TOP 0
#define MAXSERVERS 4
#define MAXTHREAD 20
#define CAPA 65
#define CAPZ 90
/* workers in config cannot be > MAXTHREAD-4 */

/* RFC 977 defines these, so don't change them */
#define NNTP_CLASS_TIMEOUT 0
#define	NNTP_CLASS_INF 	1
#define NNTP_CLASS_OK 	2
#define	NNTP_CLASS_CONT 	3
#define	NNTP_CLASS_ERR 	4
#define	NNTP_CLASS_FATAL	5
#define	NNTP_CLASS_UNKNOWN	9 /* mine for bad response */

#define	NNTP_POSTOK 	200	/* Hello -- you can post */
#define	NNTP_NOPOSTOK	201	/* Hello -- you can't post */
#define NNTP_GROUP_FOLLOWS	211	/* There's a group a-comin' next */
#define NNTP_LIST_FOLLOWS	215	/* There's a list a-comin' next */

#define NNTP_GOODBYE	400	/* Have to hang up for some reason */
#define	NNTP_NOSUCHGROUP	411	/* No such newsgroup */
#define NNTP_NONEXT		421	/* No next article */
#define NNTP_NOPREV		422	/* No previous article */
#define	NNTP_POSTFAIL	441	/* Posting failed */

#define	NNTP_AUTH_NEEDED 	480	/* Authorization Failed */
#define	NNTP_AUTH_REJECT	482	/* Authorization data rejected */

#define	NNTP_BAD_COMMAND	500	/* Command not recognized */
#define	NNTP_SYNTAX		501	/* Command syntax error */
#define	NNTP_ACCESS 	502	/* Access to server denied */
#define	NNTP_TMPERR 	503	/* Program fault, command not performed */
#define	NNTP_AUTH_BAD 	580	/* Authorization Failed */
#define	NNTP_NOSTAT 	999	/* buffer has no status code */

typedef unsigned char uchar;
typedef unsigned char* puchar;
typedef unsigned long int ARTNUM;
typedef unsigned int uint;
typedef struct S_SERVER SERVER, *P_SERVER;
typedef struct S_CONNECTION CONNECTION, *P_CONNECTION;

enum { /* task_ids */
 TASK_Overview = 0, TASK_BG_Overview, TASK_Overview1,
 TASK_Cancel, TASK_GroupSave, TASK_GroupUpdate, TASK_FetchArt
};
int GetKey( int timeout );
char *GetPattern( char *in, char *patt );
char *LastPattern( void );
char *GetBuff( char *in, int line, int key );
char *MatchPattern( char *in, puchar p_in );
int PatternInfo( int doclear );
char *Trim( char *str );
time_t ParseDate( char *indate );
time_t GetCurrDate( int offset);
void InitDate( void );
char *MakeFileName( char *buf, int buflen, char *dir, char *name, char *ext );
int InitQueues( void );
void FreeQueues( void );
char * NextToken( char *in, char *key, char **value );
int IsWord( puchar in, char *match );
puchar FindLast( puchar in, int what );
char *MakeDir( char *name, char *dir );
char *GetHomeDir( );
unsigned int GetInt( uchar **in );
uchar *GetInts( unsigned int *ptr, int count, uchar *in );
uchar *GetShorts( ushort *ptr, int count, uchar *in );
uchar *PutInts( unsigned int *ptr, int count, uchar *in );
uchar *PutShorts( unsigned short *ptr, int count, uchar *in );
int PrintDownloads( int lnum );
int PrintStatus( char *msg, int line );
void Cleanup( int status );
int SetEditor( char *name );
int SetPoster( char *name );
int SetPar2( char *name );
int RunPar2( char *name );
void EnableWin( void );
void DisableWin( void );
