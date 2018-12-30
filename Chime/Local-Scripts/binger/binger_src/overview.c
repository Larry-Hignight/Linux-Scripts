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
#define ARTICLE_SEM
#include "common.h"
#include "artdata.p"
#include "hash.h"
#include "decoders.h"
#include "server.p"
#include <curses.h>
#include <ctype.h>

extern int topline, bottomline, num_scroll, lastline, COLS;
	pthread_mutex_t article_sem = PTHREAD_MUTEX_INITIALIZER;
static char *names[] = {
	"article", "subject", "from", "date",
	"message-id", "references", "bytes", "lines", "xref", NULL
};
extern int nntp_debug;

struct S_OVDATA {
	P_GROUP p_group;
	P_CONNECTION p_conn;
	int status;
	ARTNUM first, last;
};

int
RemoveDeletes( P_GROUP p_group, int start ) {
	int i, n = start, num_arts = p_group->num_arts;
	P_ARTICLE *p_articles = p_group->p_articles;
	P_ARTICLE p_prev, p_this, p_next, p_xpost;
	P_ARTICLE p_dels = NULL, p_firstdel = NULL;
	if ( (p_group->flags&GF_Saving) ) return num_arts;
	p_group->delete_ctr = 0;
	for ( i = start; i < num_arts; i++ ) {
		p_this = p_articles[i];
		p_prev = NULL;
		do {
			p_next = p_this->p_next;
			if ( (p_this->flags&AF_Kill) ) {
				if ( p_dels != NULL ) p_dels->p_next = p_this;
				else p_firstdel = p_this;
				p_dels = p_this;
				p_xpost = p_this->p_xpost;
				for ( ;p_xpost != p_this; p_xpost = p_xpost->p_xpost ) {
					if ( (p_xpost->flags&AF_Kill ) ) continue;
					p_xpost->p_group->delete_ctr++;
					p_xpost->flags |= AF_Kill;
				}
				if ( p_prev ) p_prev->p_next = p_next;
				else p_articles[i] = p_next;
			} else p_prev = p_this;
		} while ( (p_this = p_next ) );
	}
	if ( p_firstdel == NULL ) return num_arts;
	p_dels->p_next = p_group->p_dels;
	p_group->p_dels = p_firstdel;
	if ( !p_group->p_lastdel )
		p_group->p_lastdel = p_dels;
	for ( n = start; n < num_arts; n++ )
		if ( ! p_articles[n] ) break;
	for ( i = n; i < num_arts; i++ ) {
		p_this = p_articles[i];
		if ( p_this )
			p_articles[n++] = p_this;
	}
	p_group->num_arts = n;
	p_articles[n] = NULL;
	return n;
}

int
GetOverview( P_CONNECTION p_conn, ARTNUM first, ARTNUM last, int inc,
						 P_ART_INFO p_info ) {
	P_SERVER p_server = p_conn->p_server;
	P_GROUP p_group = p_conn->p_group;
	P_ARTICLE p_article;
	int *fields = p_server->fields, field, got_fields;
	int num = p_server->numfields, i, len;
	int *offsets = p_server->field_off, offset;
	char buf[32];
	char *p_fields[MAXFIELDS], *cp, *cp1;
	int f_lens[MAXFIELDS], count = 0;
	time_t artdate;
	p_info->last_art = 0;
	if ( first == last ) sprintf( buf, "XOVER %lu", first );
	else sprintf( buf, "XOVER %lu-%lu", first, last );
	i = SendCommand( p_conn, buf );
	if ( i != NNTP_CLASS_OK ) return -1;
	while ( ( cp = GetLine( p_conn, p_conn->timeout, &len ) ) ) {
		if ( len < 2 ) {
			if ( len < 0 ) break;
			if ( len == 1 && *cp == '.' ) break;
			continue;
		}
		bzero( p_fields, sizeof( p_fields ) );
		bzero( f_lens, sizeof( f_lens ) );
		got_fields = 0;
		for ( i = 0; cp && i < num; cp = cp1 ) {
			for ( cp1 = cp; *cp1; cp1++ ) if ( *cp1 == '\t' ) break;
			len = cp1-cp;
			if ( *cp1 == '\t' ) *cp1++ = '\0';
			else cp1 = NULL;
			field = fields[i++];
			if ( field == -1 ) continue;
			offset = offsets[field];
			cp += offset;
			len -= offset;
			if ( field ==	F_Msg ) {
				cp[--len] = '\0';
				cp++;
				len--;
			} else if ( field == F_Subj && len > 500 ) {
				len = 500;
				cp[len] = '\0';
			}
			p_fields[field] = cp;
			f_lens[field] = len;
			got_fields |= 1<<field;
		}
		if ( got_fields < 31 ) continue;
		cp = p_fields[F_Date];
		if ( cp ) {
			artdate = ParseDate( cp );
			if ( artdate > p_info->last_art ) p_info->last_art = artdate;
			if ( artdate < p_group->base_time ) continue;
		}
		bzero( &p_info->file_ext, SUBJINFOLEN );
		p_info->flags = 0;
		p_info->artdate = artdate;
		p_info->artnum = A2Art( p_fields[F_Article] );
		p_info->subject = (puchar)p_fields[F_Subj];
		p_info->subjlen = f_lens[F_Subj];
		p_info->author = (puchar)p_fields[F_From];
		p_info->authlen = f_lens[F_From];
		p_info->msg_id = (puchar)p_fields[F_Msg];
		p_info->msglen = f_lens[F_Msg];
		cp = p_fields[F_Bytes];
		if ( cp ) p_info->bytes = atoi( cp );
		ParseSubject( p_info );
		p_article = AddArticle( p_info, p_group );
		if ( AddPart( p_info, p_article ) == 0 )
			count++;
	}
	if ( --len < 0 ) return len;
	return count;
}

ARTNUM
AdjustStart( P_CONNECTION p_conn, P_ART_INFO p_info, int ctl ) {
	int server = p_info->server;
	P_GROUP p_group = p_conn->p_group;
	P_SRVRDATA p_srvrdata = &p_group->srvrdata[server];
	ARTNUM first = p_srvrdata->first, last = p_srvrdata->last, high;
	time_t now = time(NULL), base_hold = p_group->base_time;
	int retain=p_group->retain*24, ctr = 0, arts, time_diff;
	float artsperhour;
	if ( (ctl&OV_Goback) ) {
		p_srvrdata->start_adjust = 0;
	} else {
		if ( p_srvrdata->highest > first	|| p_srvrdata->last_ov > base_hold )
			return p_srvrdata->highest;
	}
	if ( last-first < 5000 ) return first;
	p_group->base_time = now;
	while ( ++ctr < 7 && first < last ) {
		if (			 ( p_group->flags&GF_Cancel) ) goto oops;
		high = first+24;
		if ( high > last ) break;
		if ( GetOverview(	p_conn, first, high, 0, p_info ) < 0 )
			goto oops;
		if ( p_info->last_art >= p_group->base_time ) break;
		arts = last-high;
		time_diff = (int)(now-p_info->last_art)/3600;
		LOG_Debug( "first %d last %d arts %d timediff %d", first, last,
arts, time_diff );
		artsperhour = (float)arts/(float)time_diff;
		artsperhour *= (float)retain;
		first = last-(int)artsperhour;
		if ( first < p_srvrdata->first )
			first = p_srvrdata->first;
		LOG_Info ( "connection %d adjusted to %u", p_conn->id, first );
		if ( first == p_srvrdata->first ) break;
		if ( time_diff == retain ) break;
	}
	if ( (ctl&OV_Goback) )
		p_srvrdata->start_adjust = first;
	p_group->base_time = base_hold;
	return first;
oops:
	p_group->base_time = base_hold;
	return 0;
}

int
GetGroupInfo( P_GROUP p_group, int server, int ctl ) {
	int i, count;
	int status = 0, big_group = 0;
	short flags = 1<<server;
	ARTNUM first, last, high, low;
	P_CONNECTION p_conn = NULL;
	P_SRVRDATA p_srvrdata = &p_group->srvrdata[server];
	int pri = (ctl&OV_Bg) ? PRI_BGOV : PRI_FG;
	ART_INFO info;
	bzero( &info, sizeof( info ) );
	info.server = server;
	pthread_mutex_lock( &group_sem );
	if ( !(p_group->flags&GF_Read) ) {
		if ( p_group->ov_running != 0 ) { /* wait for first thread to read group */
			for ( i = 0; !(p_group->flags&GF_Read); i++ ) {
				pthread_mutex_unlock( &group_sem );
				if ( i > 80 ) return -1;
				usleep( 200000 );
				pthread_mutex_lock( &group_sem );
			}
		} else {
			i = ReadGroup( p_group, GR_HaveSem );
			if ( i < 0 ) {
				pthread_mutex_unlock( &group_sem );
				return -1;
			}
			p_group->base_time = GetCurrDate( p_group->retain );
		}
	}
	if ( (p_group->flags&GF_IO) || (p_group->ov_running&flags) ) {
		pthread_mutex_unlock( &group_sem );
		return -1;
	}
	p_group->ov_running |= flags;
	if ( p_group->base_time == 0 )
		p_group->base_time = GetCurrDate( p_group->retain );
	pthread_mutex_unlock( &group_sem );
	if ( ( p_conn = OpenConnect( server, pri, STAT_Overview ) ) == NULL )
		goto bad;
	if ( SelectGroup( p_conn, p_group, UP_Force ) != 0 ) goto bad;
	last = p_srvrdata->last;
	if ( (ctl&OV_Goback) ) {
		first = AdjustStart( p_conn, &info, ctl );
		if ( first == 0 ) goto bad;
		low = p_srvrdata->lowest;
		if ( low < p_srvrdata->highest && low > first ) last = low;
		LOG_Debug ( "goback for %d %u %u", server, first, last );
		while ( first < last && !			( p_group->flags&GF_Cancel) ) {
			low = last-999;
			if ( low < first ) low = first;
			i = GetOverview(	p_conn, low, last, -1000, &info );
			if ( i < 0 ) {
				status = i;
				break;
			}
			last -= 1000;
			CheckHash( );
			if ( (p_conn = SurrenderConnect( p_conn ) ) == NULL ) goto bad;
		}
	} else if ( p_srvrdata->highest != p_srvrdata->last ) {
		first = AdjustStart( p_conn, &info, ctl );
		if ( first == 0 ) goto bad;
		LOG_Debug ( "overview for %d %u %u", server, first, last );
		big_group = ( first+25000 < last ) ? 1 : 0;
		while ( first <= last && !			( p_group->flags&GF_Cancel) ) {
			high = first+999;
			if ( high > last ) high = last;
			i = GetOverview(	p_conn, first, high, 1000, &info );
			LOG_Debug ( "ovm from %u to %u stat=%d", first, high, i );
			if ( i < 0 ) {
				status = i;
				break;
			}
			first += 1000;
			CheckHash( );
			if ( big_group && first+2000 > last ) {
				if ( SelectGroup( p_conn, p_group, UP_Force ) == 0 )
					last = p_srvrdata->last;
			big_group = 0;
			}
			if ( (p_conn = SurrenderConnect( p_conn ) ) == NULL ) goto bad;
		}
	} else {
		big_group = -1;
	}
	if ( ( p_group->flags&GF_Cancel ) ) {
		status = 1;
	}
	CloseConnect( p_conn, 0 );
	p_srvrdata->last_ov = time(NULL);
	pthread_mutex_lock( &group_sem );
	if ( p_group->ov_req != flags ) { /* not the last one running */
		p_group->ov_running &= ~flags;
		p_group->ov_req &= ~flags;
		pthread_mutex_unlock( &group_sem );
		return status;
	}
	count = p_group->num_arts;
	p_group->flags &= ~GF_Cancel;
	if ( count > 0 ) p_group->p_articles[count] = NULL;
	if ( big_group != -1 ) {
		if ( count > 1 ) ThreadArticles( p_group );
		SaveGroup( p_group, GR_HaveSem );
	}
	p_group->ov_running = 0;
	p_group->ov_req = 0;
	pthread_mutex_unlock( &group_sem );
	beep( );
	return status;
bad:
	pthread_mutex_lock( &group_sem );
	p_group->ov_running &= ~flags;
	p_group->ov_req &= ~flags;
	flags = p_group->ov_req;
	if ( flags == 0 ) p_group->flags &= ~GF_Cancel;
	pthread_mutex_unlock( &group_sem );
	if ( p_conn ) CloseConnect( p_conn, 1 );
	if ( !flags ) beep( );
	return -1;
}

static int default_retain = 7;

void
SetDefaultRetain( int val ) {
	if ( val >= 1 && val <= 200 ) default_retain = val;
}

int
GetRetain( P_GROUP p_group, int lnum ) {
	char *cp;
	int status = 0, days = p_group->retain;
		mvprintw( lastline-1, 0, "retention days for %s, currently %d",
		p_group->name, days );
	cp = GetBuff( NULL, lastline, 0 );
	move( lnum, 0 );
	if ( *cp == '\0' ) {
		if ( days > 0 ) return 0;
		days = default_retain;
			move( lastline-1, 0 );
			clrtoeol( );
			mvprintw( lastline-1, 0, "using default of %d", days );
			move( lnum, 0 );
	} else {
		days = atoi( cp );
		if ( days < 1 || days > 999 ) return -1;
	}
	pthread_mutex_lock( &group_sem );
	if ( p_group->retain == 0 || p_group->retain > days ) status = 1;
	else {
		p_group->flags |= GF_BackDated;
		status = 2;
	}
	p_group->retain = days;
	p_group->base_time = GetCurrDate( p_group->retain );
	pthread_mutex_unlock( &group_sem );
	return status;
}

int
GetOverList( P_CONNECTION p_conn, P_GROUP p_group, ARTNUM *arts, int count ) {
	ARTNUM art, high;
	int i, n = 0, num = 0;
	ART_INFO info;
	bzero( &info, sizeof( info ) );
	info.server = p_conn->p_server->servernum;
	while ( n < count ) {
		if (			 ( p_group->flags&GF_Cancel) ) return 0;
		art = arts[n++];
		high = art;
		while ( n < count ) {
			if ( arts[n] != high+1 ) break;
			high++; n++;
		}
		i = GetOverview(	p_conn, art, high, 0, &info );
		fprintf( stderr, "ov for %lu %lu got %d\n", art, high, i );
		if ( i > 0 ) num += i;
	}
	return num;
}

#define BITMASK 31
#define BITSHIFT 5

static int
ScanArts( P_ARTICLE p_art, ARTNUM *artbits, int sn, ARTNUM low, ARTNUM high ) {
	int part, parts, count = 0;
	ARTNUM art, artbit = 0;
	P_PARTDATA p_part;
P_EXTRAPART p_extra, p_next;
	short next;
	do {
		parts = p_art->parts;
		p_part = p_art->p_parts;
		p_extra = p_art->p_extras;
		for ( part = 0; part < parts; part++ ) {
			if ( p_part->server == sn ) {
				art = p_part->artnum;
			} else {
				next = p_part->next;
				while ( next > 0 ) {
					p_next =	p_extra+(next-1);
					if ( p_next->server == sn ) break;
					next = p_next->next;
				}
				if ( next == 0 ) continue;
				art = p_next->artnum;
			}
			if ( art >= low && art < high ) {
				art -= low;
				artbit = 1<<(art&BITMASK);
				art >>= BITSHIFT;
				artbits[art] |= artbit;
				count++;
			}
			p_part++;
		}
	} while ( ( p_art = p_art->p_next ) );
	return count;
}

int
FindMissing( P_GROUP p_group, int server ) {
	P_CONNECTION p_conn = NULL;
	int i, count = 0, n, j, total = 0;
	P_SRVRDATA p_srvrdata = &p_group->srvrdata[server];
	short flags = 1<<server;
	ARTNUM low= p_srvrdata->lowest, high = p_srvrdata->highest, art, art_test;
	P_ARTICLE p_article;
	ARTNUM *artbits, arts[128];
	pthread_mutex_lock( &group_sem );
	if ( (p_group->ov_req&flags) ) {
		pthread_mutex_unlock( &group_sem );
		return -1;
	}
	p_group->ov_req |= flags;
	p_group->ov_running |= flags;
	pthread_mutex_unlock( &group_sem );
	art = (high-low)&~BITMASK;
	low = high-art;
	n = art>>BITSHIFT;
	artbits = calloc( sizeof(ARTNUM), n+1 );
	for ( i = 0; i < p_group->num_arts; i++ ) {
		p_article = p_group->p_articles[i];
		count += ScanArts( p_article, artbits, server, low, high );
	}
	if( p_group->p_dels != NULL )
		count += ScanArts( p_group->p_dels, artbits, server, low, high );
	PrintCount( art-count, "missing, ctl-c to cancel" );
	sleep( 2 );
		if ( p_group->flags&GF_Cancel ) goto bad;
	if ( ( p_conn = OpenConnect( server, PRI_FG, STAT_Overview ) ) == NULL ) goto bad;
	if ( SelectGroup( p_conn, p_group, UP_Change ) != 0 ) goto bad;
	count = 0;
	for ( i = 0; i <= n; i++ ) {
		art_test = artbits[i];
		if ( art_test == 0xffffffff ) continue;
		art = (i<<BITSHIFT)+low;
		for ( j = 0; j <= BITMASK; j++ ) {
			if ( (art_test&(1<<j)) == 0 ) arts[count++] = art+j;
		}
		if ( p_group->flags&GF_Cancel ) break;
		if ( count <= 90 ) continue;
		beep( );
		count = GetOverList( p_conn, p_group, arts, count );
		if ( count > 0 ) total += count;
		count = 0;
	}
	if ( !(p_group->flags&GF_Cancel) && count > 0 ) {
		beep( );
		count = GetOverList( p_conn, p_group, arts, count );
		if ( count > 0 ) total += count;
	}
	pthread_mutex_lock( &group_sem );
	p_group->ov_running &= ~flags;
	p_group->ov_req &= ~flags;
	pthread_mutex_unlock( &group_sem );
	if ( p_conn != NULL ) CloseConnect( p_conn, 0 );
	free( artbits );
	return total;
bad:
	pthread_mutex_lock( &group_sem );
	p_group->ov_running &= ~flags;
	p_group->ov_req &= ~flags;
	pthread_mutex_unlock( &group_sem );
	if ( p_conn != NULL ) CloseConnect( p_conn, 0 );
	free( artbits );
	return -1;
}

int
GetFields( P_CONNECTION p_conn ) {
	P_SERVER p_server = p_conn->p_server;
	int i, len, num = 1;
	int	*fields = p_server->fields, *offsets = p_server->field_off;
	char *cp, *cp1;
	if ( p_server->numfields > 0 ) {
		return 0;
	}
	i = SendCommand( p_conn, "LIST OVERVIEW.FMT" );
	if ( i != NNTP_CLASS_OK ) {
		return -1;
	}
	bzero( offsets, sizeof( p_server->field_off ) );
	for ( i = 1; i < MAXFIELDS; i++ ) fields[i] = -1;
	fields[0] = 0; /* article num */
	while ( 1 ) {
		cp = GetLine( p_conn, p_conn->timeout, &len );
		if ( !cp || len <= 1 ) break;
		for ( cp1 = cp; *cp1 && *cp1 != ':'; cp1++ )
			if ( *cp1 >= 'A' ) *cp1 |= 32; /* downshift */
		*cp1++ = 0;
		for ( i = 1; names[i]; i++ ) {
			if ( strcmp( cp, names[i] ) ) continue;
			fields[num] = i;
			if ( strcasecmp( cp1, "full" ) == 0 )
				offsets[num] = strlen( names[i] )+1;
/* :full means hdr name is sent with field, adjust to drop it */
			if ( nntp_debug&16 ) fprintf( stderr,
				"field %d is %s offset %d\n", num, names[i], offsets[i] );
			break;
		}
		num++;
	}
	if ( num < 2 ) len = -1;
	else p_server->numfields = num;
	return --len;
/* 0 ok, -1 premature list end, -2 no fields; */
}
