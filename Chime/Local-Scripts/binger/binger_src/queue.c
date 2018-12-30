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
#include "common.h"
#include "server.p"
#include "artdata.p"
#include <signal.h>
#include <sys/file.h>
#include <ctype.h>

pthread_attr_t default_attr;
static pthread_t worker_tid[MAXTHREAD];
static int thread_status[MAXTHREAD];
static pthread_mutex_t queue_sem = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
static int queue_ctr = 0, queue_max_pri = 0, queue_waiters = 0;
QUEUE queue[PRI_MAX];
static int num_workers = 0;
int max_workers = 1, bg_downloads = 0;
static char *task_names[] = {
	"overview", "bg overview", "overview back", "cancel",
	"group save", "group update", "art fetch",
	"unknown", "unknown"
};

P_QUEUEDATA WaitForWork( int id );

int
GetNumDownloads( ) {
	return bg_downloads;
}

void *WorkerThread( void *arg ) {
	P_QUEUEDATA p_qd;
	int id = (int)arg, status = 0;
	short task,	server;
	P_ARTICLE p_art;
	P_ART_DIR p_dir;
	P_GROUP p_group;
	sigset_t sigs;
	sigemptyset( &sigs );
	sigaddset( &sigs, SIGHUP );
	sigaddset( &sigs, SIGINT );
	sigaddset( &sigs, SIGQUIT );
	sigaddset( &sigs, SIGTERM );
	pthread_sigmask( SIG_BLOCK, &sigs, NULL );
	LOG_Debug ( "thread %d starting", id );
	pthread_mutex_lock( &queue_sem );
	thread_status[id] = 2;
	pthread_mutex_unlock( &queue_sem );
	while ( ( p_qd = WaitForWork( id ) ) ) {
		status = 0;
		server = p_qd->server;
		task = p_qd->task_id;
		LOG_Debug ( "thread %d server %hd task %s", id, server, 
			task_names[task] );
		switch( task ) {
			case TASK_Overview:
			case TASK_BG_Overview:
				p_group = (P_GROUP)p_qd->task_data;
				status = GetGroupInfo( p_group, server, p_qd->arg );
				break;
			case TASK_GroupUpdate:
				status = UpdateGroupInfo( server );
				break;
			case TASK_GroupSave:
				p_group = (P_GROUP)p_qd->task_data;
				SaveGroup( p_group, 0 );
				break;
			case TASK_FetchArt:
				p_art = (P_ARTICLE)p_qd->task_data;
				p_dir = (P_ART_DIR)p_qd->task_data1;
				status = SaveArticle( p_art, 0, p_qd->arg, id, p_dir );
				break;
			default:
				status = -1;
		}
		LOG_Debug ( "thread %d server %hd done %s, status %d", id,
			server, task_names[task], status );
		if ( p_qd->ret_status != NULL ) *p_qd->ret_status = status;
		free( p_qd );
	}
	pthread_mutex_lock( &queue_sem );
	num_workers--;
	worker_tid[id] = 0;
	thread_status[id] = 0;
	pthread_mutex_unlock( &queue_sem );
	LOG_Debug ( "thread %d done", id );
	return 0;
}

int
InitQueues( void ) {
	int status = 0;
	bzero( queue, sizeof( queue ) );
	pthread_attr_init( &default_attr );
	pthread_attr_setdetachstate( &default_attr, PTHREAD_CREATE_DETACHED );
	bzero( worker_tid, sizeof(worker_tid) );
	worker_tid[0] = pthread_self( );
	bzero( thread_status, sizeof(thread_status) );
	return status;
}

void
FreeQueues( ) {
	int i;
	shutdown_flag = 1;
	pthread_mutex_lock( &queue_sem );
	pthread_cond_broadcast( &queue_cond );
	pthread_mutex_unlock( &queue_sem );
	usleep( 100000 );
	for ( i = 0; i < 4 && num_workers > 0; i++ ) {
		usleep( 100000 );
	}
}

int
StartWorker( void ) {
	int thr_id = 0, i, wait;
	pthread_mutex_lock( &queue_sem );
	for ( i = 1; i < MAXTHREAD && thr_id == 0; i++ )
		if ( thread_status[i] == 0 ) thr_id = i;
	if ( thr_id > 0 ) {
		num_workers++;
		thread_status[thr_id] = 1;
	}
	pthread_mutex_unlock( &queue_sem );
	if ( thr_id == 0 ) return 0;
	LOG_Debug ( "creating thread %d, num %d", thr_id, num_workers );
	pthread_create(&worker_tid[thr_id], &default_attr, WorkerThread,
		(void *)thr_id);
	for ( i = 0; i < 50; i++ ) {
		usleep( 50000 );
		pthread_mutex_lock( &queue_sem );
		wait = thread_status[thr_id];
		pthread_mutex_unlock( &queue_sem );
		if ( wait != 1 ) break;
	}
	return ( wait != 1 ) ? 0 : -1;
}

int
QueueWork( short server, short task_id, void *task_data, void *task_data1,
					 int priority, int arg, int *ret_status ) {
	P_QUEUE p_queue = &queue[priority];
	P_QUEUEDATA p_qd;
	int wait, count;
	if ( shutdown_flag ) return -1;
	if ( ( p_qd = calloc( 1, sizeof( QUEUEDATA ) ) ) == NULL ) return -1;
	pthread_mutex_lock( &queue_sem );
	if ( queue_max_pri < priority ) queue_max_pri = priority;
	if ( p_queue->count == 0 ) p_queue->p_first = p_qd;
	else p_queue->p_last->p_next = p_qd;
	p_queue->p_last = p_qd;
	p_queue->count++;
	queue_ctr++;
	p_qd->server = server;
	p_qd->priority = priority;
	p_qd->task_id = task_id;
	p_qd->task_data = task_data;
	p_qd->task_data1 = task_data1;
	p_qd->arg = arg;
	p_qd->ret_status = ret_status;
	count = p_queue->count;
	if ( (wait = queue_waiters) > 0 )
		pthread_cond_signal( &queue_cond );
	pthread_mutex_unlock( &queue_sem );
	LOG_Debug ( "adding %hd %s to queue %d at %d", server,
		task_names[task_id], priority,	count );
	if ( num_workers >= max_workers ) return count;
	if ( wait > 0 ) usleep( 100000 );
	else StartWorker( );
	return count;
}

P_QUEUEDATA
WaitForWork( int id ) {
	P_QUEUE p_queue;
	P_QUEUEDATA p_qd;
	int i, status = 0, waits = 0;
	struct timespec timeout = { 0, 0 };
	if ( shutdown_flag ) return NULL;
	pthread_mutex_lock( &queue_sem );
	queue_waiters++;
wait_again:
	if ( ++waits > 1 || queue_ctr < 1 ) {
		time( &timeout.tv_sec );
		timeout.tv_sec += 60;
		status = pthread_cond_timedwait( &queue_cond, &queue_sem, &timeout );
		LOG_Debug ( "queue timedwait, thread %d, return %d", id,
			status );
	} else usleep(100000 );
	if ( status != 0 || shutdown_flag || queue_ctr < 1 ) {
		queue_waiters--;
		pthread_mutex_unlock( &queue_sem );
		return NULL;
	}
	p_queue = NULL;
	for ( i = queue_max_pri; i >= 0; i-- ) {
		p_queue = &queue[i];
		if ( p_queue->count > 0 ) break;
	}
	if ( p_queue == NULL || ( p_qd = p_queue->p_first ) == NULL ) {
		queue_waiters--;
		pthread_mutex_unlock( &queue_sem );
		return NULL;
	}
	if ( p_qd->server != -1 ) {
		P_SERVER p_s = p_servers[p_qd->server];
		pthread_mutex_lock( &p_s->server_sem );
		i = ( ( p_qd->priority > p_s->max_pri || p_s->conn_used < p_s->conn_max ) );
		pthread_mutex_unlock( &p_s->server_sem );
		if ( i == 0) goto wait_again;
	}
	queue_ctr--;
	queue_waiters--;
	p_queue->p_first = p_qd->p_next;
	p_qd->p_next = NULL;
	if ( --(p_queue->count) == 0 ) {
		p_queue->p_first = p_queue->p_last = NULL;
		while ( queue_max_pri > 0 ) {
			p_queue = &queue[--queue_max_pri];
			if ( p_queue->count > 0 ) break;
		}
	}
	pthread_mutex_unlock( &queue_sem );
	return p_qd;
}

int
CancelQueuedDownloads( ) {
	P_QUEUE p_queue = &queue[PRI_BG];
	P_QUEUEDATA p_qd, p_next, p_cancel = NULL;
	int count = 0;
	P_ARTICLE p_article;
	P_GROUP p_group;
	P_ART_DIR p_dir;
	pthread_mutex_lock( &queue_sem );
	if ( bg_downloads == 0 || p_queue->count <= 0 ) {
		pthread_mutex_unlock( &queue_sem );
		return 0;
	}
	p_qd = p_queue->p_first;
	p_queue->p_first = p_queue->p_last = NULL;
	for ( ; p_qd != NULL; p_qd = p_next ) {
		p_next = p_qd->p_next;
		p_qd->p_next = NULL;
		if ( p_qd->task_id != TASK_FetchArt ) {
			if ( p_queue->p_first == NULL ) p_queue->p_first = p_qd;
			else p_queue->p_last->p_next = p_qd;
			p_queue->p_last = p_qd;
		} else {
			queue_ctr--;
			p_queue->count--;
			count++;
			if ( p_cancel ) p_qd->p_next = p_cancel;
			p_cancel = p_qd;
		}
	}
	pthread_mutex_unlock( &queue_sem );
	pthread_mutex_lock( &article_sem );
	for ( p_qd = p_cancel; p_qd != NULL; p_qd = p_next ) {
		p_next = p_qd->p_next;
		p_article = (P_ARTICLE)p_qd->task_data;
		p_group = p_article->p_group;
		p_dir = (P_ART_DIR)p_qd->task_data1;
		p_article->flags &= ~AF_Loads;
		if ( p_dir != NULL && --p_dir->nref <= 0 ) {
			free( p_dir->dir );
			free( p_dir );
		}
		bg_downloads--;
		if ( --p_group->bg_downloads == 0 ) {
			if ( p_group->flags&GF_SaveRequest )
				QueueWork	( -1, TASK_GroupSave, p_group, NULL, PRI_High, 0, NULL );
			if ( p_group->flags&GF_NZB )
				FreeGroup( p_group );
		}
		free( p_qd );
	}
	pthread_mutex_unlock( &article_sem );
	return count;
}

extern int topline, bottomline, num_scroll, lastline, COLS;

int QueueFromFile( P_GROUP p_group ) {
	int cmd = AF_B_Get|AF_Kill, in = -1;
	off_t filelen;
	P_ARTICLE p_next;
	static char pattbuf[80];
	char *buff, *p_end, *fname;
	struct stat statbuf;
	PrintStatus( "enter file name", lastline );
	fname = GetBuff( NULL, lastline, 0 );
	if ( fname == NULL ) return -1;
	if ( -1 == stat( fname, &statbuf ) ) {
		PrintStatus( "can't stat file", lastline );
		return -1;
	}
	filelen = statbuf.st_size;
	if ( filelen > 200000 ) {
		PrintStatus( "file too large >200kb", lastline );
		return -1;
	}
	in = open( fname, O_RDONLY );
	if ( -1 == in ) {
		PrintStatus( "can't open file", lastline );
		return -1;
	}
	buff = malloc( filelen+1 );
	if ( NULL == buff ) {
		PrintStatus( "can't allocate buffer", lastline );
		close( in );
		return -1;
	}
	if ( read( in, buff, filelen ) != filelen ) {
		PrintStatus( "can't read file", lastline );
		free( buff );
		close( in );
		return -1;
	}
	close( in );
	p_end = buff+filelen;
	*p_end = 0;
}
