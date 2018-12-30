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
#ifndef SERVER_H

#define SERVER_H

#include <stdarg.h>

#define SendCommand( con, cmd ) SendCmd( con, cmd, 0 )
#define SendData( con, cmd ) SendCmd( con, cmd, 1 )
enum { /* for transfer stats classification */
	STAT_Misc = 0, STAT_Overview, STAT_Article
};
enum { /* connection/queue priorities */
	PRI_Low = 0, PRI_BG, PRI_BGOV, PRI_FG, PRI_MAX, PRI_NW = 16, PRI_Sem = 32
};
#define PRI_High PRI_FG
#ifndef I_AM_SERVER
extern P_SERVER p_servers[MAXSERVERS+1];
extern int numservers, store_msgids, mingrpsize;
extern int loglevel, shutdown_flag;
#endif
#define LOG_Fatal if (loglevel >= 1) PutLog
#define LOG_Error if (loglevel >= 2) PutLog
#define LOG_ArtGet if (loglevel >= 3) PutLog
#define LOG_Status if (loglevel >= 4) PutLog
#define LOG_Info if (loglevel >= 5) PutLog
#define LOG_Debug if (loglevel >= 6) PutLog

int ReadConfig( char *name);
void SetDebug( int n );
int GetNumDownloads( );
int CancelQueuedDownloads( );
void PutLog( char *fmt, ... );
P_CONNECTION OpenConnect( int serve, int priority, int stats );
void ConnSetStats( P_CONNECTION p_conn, int stats );
int CloseConnect( P_CONNECTION p_conn, int doClose );
P_CONNECTION SurrenderConnect( P_CONNECTION p_conn );
int TestConnect( P_CONNECTION p_conn );
int NntpStatus( P_CONNECTION p_conn, char *lastbuf );
void FreeServers( );
char *GetLine( P_CONNECTION p_conn, int timeout, int *len );
void UngetLine( P_CONNECTION p_conn );
int GetResponse( P_CONNECTION p_conn, int timeout );
int SendCmd( P_CONNECTION p_conn, char *command, int no_wait );
int WeShutdown( void );
char *GetDispName( int num );
int GetServerStats( int num, int *stats, char *ssl_info );

int QueueWork( short server, short task_id, void *task_data, void *task_data1,
							 int pri, int arg, int *status) ;

#endif
