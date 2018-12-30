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
#define GROUP_SEM
#include "common.h"
#include "server.p"
#include "artdata.p"
#include "hash.h"

	pthread_mutex_t group_sem = PTHREAD_MUTEX_INITIALIZER;
P_HASHTABLE group_hash = NULL;
extern char infodir[];
extern int topline, bottomline, num_scroll, lastline, COLS;
extern int binary_save, bg_downloads;
extern P_ARTICLE p_download[MAXTHREAD+1];
int num_groups = 0, max_groups = 0;
int ignore_threading = 0, bg_errors = 0;
P_GROUP *p_groups = NULL;
static P_GROUP p_currentgroup = NULL;
#define VERSION 5
static char *filt = "filt";
static char *filt_tmp = "filt.tmp";

void Cleanup( int status );

FILE *
DoCreate( char *name ) {
	FILE *out	= fopen( name, "w" );
	if ( out ) return out;
	Cleanup( -1 );
	fprintf( stderr, "can't create %s error %d\n", name, errno );
	exit( 1 );
	return NULL; /* never get here */
}

void
DoRename( char *name ) {
	char dest[512];
	int l = strlen( name )-4;
	if ( strcmp( name+l, ".tmp" ) ) {
		LOG_Error( "bad parm to rename, %s", name );
		return;
	}
	strncpy( dest, name, l );
	dest[l] = '\0';
	if ( rename( name, dest ) == 0 ) return;
	Cleanup( -1 );
	fprintf( stderr, "can't rename %s to	%s error %d\n", name, dest, errno );
	exit( 1 );
}

void
Oops( char *msg, int lc ) {
	Cleanup( -1 );
	if ( lc > 0 ) fprintf( stderr, "error: %s at %d\n", msg, lc );
	else fprintf( stderr, "error: %s %d\n", msg, lc );
	exit( 1 );
}

int
SaveVer5( FILE *out, P_GROUP p_group, ARTNUM *artbase, P_ARTICLE p_in, 
						uchar **lastauth ) {
	int i, server, count = 0;
	short next, flags;
	P_ARTICLE p_art;
	P_PARTDATA p_part;
	P_EXTRAPART p_extra;
	ARTNUM artnum;
	uchar *auth;
	for ( p_art = p_in; p_art; p_art = p_art->p_next ) {
		flags = p_art->flags&AF_IOMask;
		if ( p_art->p_next == p_in ) {
			p_art->p_next = NULL;
		}
		if ( (flags&AF_Kill) ) {
			if ( p_art->artdate <= p_group->base_time ) continue;
		}
		count++;
		p_part = p_art->p_parts;
		fprintf( out, "\t%hx %hd %u %hx %hx %hx\n",
			flags, p_art->parts, (uint)(p_art->artdate-p_group->base_time),
			p_art->file_ext, p_art->name_pos, p_art->ext_end );
		fprintf( out, "%s\n", p_art->subject );
		auth = p_art->author;
		if ( *lastauth != auth )
			fprintf( out, "@%s\n", auth );
		*lastauth = auth;
		for ( i = 0; i < p_art->parts; i++, p_part++ ) {
			server = p_part->server;
			artnum = p_part->artnum;
			if ( artnum == 0 || artnum <= artbase[server] ) {
				fprintf( out, "0\n" );
				continue;
			}
			artnum -= artbase[server];
			fprintf( out, "%lu %d %d", artnum, server, p_part->bytes );
			if ( (p_group->flags&GF_MsgIds) && p_art->msg_id != NULL && p_art->msg_id[i] != NULL )
				fprintf( out, "\t%s\n", p_art->msg_id[i] );
			else fputc( '\n', out );
			for ( next = p_part->next; next > 0; next = p_extra->next ) {
				p_extra = &p_art->p_extras[next-1];
				server = p_extra->server;
				artnum = p_extra->artnum;
				if ( artnum == 0 || artnum <= artbase[server] )
					continue;
				artnum -= artbase[server];
				fprintf( out, "+%lu %d\n", artnum, server );
			}
		}
	}
	return count;
}

int
SaveFilters( P_GROUP p_group, char *name ) {
	P_FILTER p_filter = p_group->p_filter;
	FILE *out;
	char buf[512];
	if ( p_filter == NULL ) return 0;
	MakeFileName( buf, 512, infodir, p_group->name, filt_tmp );
	out	= DoCreate( buf );
	for ( ; p_filter != NULL; p_filter = p_filter->p_next )
		fprintf( out, "%s\n", (char *)p_filter->pattern );
	fclose( out );
	DoRename( buf );
	p_group->flags &= ~GF_FilterMod;
return 0;
}

void FreeFilters( P_GROUP p_group ) {
	P_FILTER p_filter =p_group->p_filter, p_next;
	p_group->flags &= ~GF_FilterMod;
	p_group->p_filter = NULL;
	while ( p_filter != NULL ) {
		p_next = p_filter->p_next;
		free( p_filter );
		p_filter = p_next;
	}
}

int
SaveGroup( P_GROUP p_group, int reading ) {
	char name[512];
	P_ARTICLE *p_base;
	ARTNUM artbase[MAXSERVERS];
	int i, arts = 0, dels = 0;
	FILE *out;
	uchar *lastauth = NULL;
	if ( (reading&GR_Save) ) {
		if ( pthread_mutex_trylock( &group_sem ) != 0 ) return -1;
		p_group = p_currentgroup;
		if ( p_group == NULL || (p_group->flags&GF_Temp) ) {
			pthread_mutex_unlock( &group_sem );
			return 1;
		}
	} else if ( !(reading&GR_HaveSem) ) {
		pthread_mutex_lock( &group_sem );
		if ( ( p_group->flags&(GF_Temp|GF_IO)) || p_group->ov_req ) {
			pthread_mutex_unlock( &group_sem );
			return -1;
		}
	}
	if ( reading&GR_Current ) p_currentgroup = NULL;
	pthread_mutex_lock( &article_sem );
	if ( p_group->delete_ctr > 0 )
		RemoveDeletes( p_group, 0 );
	pthread_mutex_unlock( &article_sem );
	p_group->flags |= GF_Saving;
	if ( p_group->bg_downloads > 0 ) p_group->flags |= GF_SaveRequest;
	else p_group->flags &= ~GF_SaveRequest;
	pthread_mutex_unlock( &group_sem );
	MakeFileName( name, 512, infodir, p_group->name, "tmp" );
	out	= DoCreate( name );
	fprintf( out, "binger_%d %hd %u %hd %hd %d\n", VERSION, p_group->retain,
			(uint)p_group->base_time, p_group->subjtype, p_group->mode, numservers );
	for ( i = 0; i < numservers; i++ ) {
		artbase[i] = p_group->srvrdata[i].lowest;
		if ( artbase[i] == 0xffffffff ) artbase[i] = 0;
		else if ( artbase[i] > 0 ) artbase[i]--;
		fprintf( out, "%lu %lu\n", artbase[i], p_group->srvrdata[i].highest );
	}
	if ( p_group->num_arts > 0 ) {
		for ( p_base = p_group->p_articles; *p_base; p_base++ ) {
			arts += SaveVer5( out, p_group, artbase, *p_base, &lastauth );
		}
	}
	if ( p_group->p_dels ) {
		lastauth = NULL;
		dels = SaveVer5( out, p_group, artbase, p_group->p_dels, &lastauth );
	}
	fclose( out );
	DoRename( name );
	ctime_r( &p_group->base_time, name );
	name[strlen( name ) -1] = '\0';
	LOG_Info( "saved group %s articles %d deletes %d time %s", p_group->name,
		arts, dels, name );
	if ( (p_group->flags&GF_FilterMod) )
		SaveFilters( p_group, p_group->name );
	pthread_mutex_lock( &group_sem );
	p_group->flags &= ~GF_Saving;
	if ( !(reading&GR_HaveSem) )
		pthread_mutex_unlock( &group_sem );
	return 0;
}

int
ReadAscii( P_GROUP p_group, FILE *in, ARTNUM *artbase, int version ) {
	int i, is_extra, lc = 1;
	int server, flags, arts = 0, dels = 0;
	time_t base_time = p_group->base_time, art_date;
	unsigned int t_in;
	char *cp, *cp1;
	P_ARTICLE p_article = NULL, p_prev = NULL;
	P_SRVRDATA p_srvrdata = p_group->srvrdata;
	ART_INFO info;
	ARTNUM artnum;
	char buf[512], subj[512], auth_msg[512];
	bzero( &info, sizeof( info ) );
	art_date = GetCurrDate( p_group->retain );
	if ( base_time+7200 < art_date )
		p_group->base_time = art_date;
	bzero( &subj, sizeof( subj ) );
	bzero( &auth_msg, sizeof( auth_msg ) );
	info.subject = (uchar *)subj;
	info.author = (uchar *)auth_msg;
	while ( ( cp = fgets( buf, 510, in ) ) ) {
		lc++;
		if ( *cp == '#' ) continue;
done_skip:
		p_article = NULL;
		if ( *cp != '\t' )
			Oops( "flags sync error", lc );
		if ( version == 5 )
			sscanf( cp, "%x %hd %u %hx %hx %hx", &flags, &info.parts, &t_in,
				&info.file_ext, &info.name_pos, &info.ext_end );
		else sscanf( cp, "%x %x %hx %hx %hx %hx", &flags, &t_in, &info.parts,
			&info.file_ext, &info.name_pos, &info.ext_end );
		art_date = t_in + base_time;
		if ( ( cp = fgets( subj, 510, in ) ) == NULL )
			Oops( "subject eof", lc );
		lc++;
		cp1 = strchr( cp, '\t' );
		if ( !cp1 ) cp1 = strchr( cp, '\n' );
		info.subjlen = cp1-cp;
		*cp1 = '\0';
		if ( version == 5 ) i = getc( in );
		else if ( (flags&AF_Author) ) i = '@';
		if ( i == '@' ) {
			if ( ( cp = fgets( auth_msg, 250, in ) ) == NULL )
				Oops( "author eof", lc );
			lc++;
			info.authlen = strlen( cp )-1;
		cp[info.authlen] = '\0';
		} else if ( version == 5 ) ungetc( i, in );
		if ( art_date <= p_group->base_time && (flags&AF_Kill) ) {
			/* skip old deleted articles */
			while ( ( cp = fgets( buf, 510, in ) ) ) {
				if ( *cp == '\t' ) goto done_skip;
			}
			break; /* done reading */
		}
		if ( (flags&AF_Kill) ) dels++;
		else arts++;
		info.flags = (flags&AF_IOMask);
		info.artdate = art_date;
		if ( ignore_threading ) bzero( &info.file_ext, SUBJINFOLEN );
		for ( i = 1; i <= info.parts; i++ ) {
			if ( ( cp = fgets( buf, 250, in ) ) == NULL )
				Oops( "part eof", lc );
			lc++;
			if ( *cp == '0' ) continue;
			info.part = i;
			sscanf( buf, "%lu %d %d", &artnum, &server, &info.bytes );
			if ( server < 0 || server >= numservers )
				Oops( "part server range", lc );
			info.artnum = artnum+artbase[server];
			info.server = server;
			if ( (p_group->flags&GF_MsgIds) )
				cp = strchr( cp, '\t' );
			else cp = NULL;
			info.msg_id = (uchar *)cp;
			if ( cp != NULL ) {
				if ( ( cp = strchr( cp, '\n' ) ) != NULL )
					*cp = '\0';
			}
			if ( p_srvrdata[info.server].first <= info.artnum ) {
				if ( !p_article ) p_article = AddArticle( &info, p_group );
				AddPart( &info, p_article );
			}
			while ( '+' ==	( is_extra = getc(in ) ) ) {
				if ( ( cp = fgets( buf, 250, in ) ) == NULL )
					Oops( "extra eof", lc );
				lc++;
				sscanf( buf, "%lu %hd", &info.artnum, &info.server );
				if ( info.server < 0 || info.server >= numservers )
					Oops( "extra server range", lc );
				info.artnum += artbase[info.server];
				if ( p_group->srvrdata[info.server].first <= info.artnum ) {
					AddPart( &info, p_article );
				}
			}
			ungetc( is_extra, in );
		}
		i = p_group->num_arts-1;
		if ( p_article && i >= 0 && p_article == p_group->p_articles[i] ) {
			if ( p_prev && ArtCmp( p_prev, p_article, 0 ) == 0 ) {
				p_prev->p_next = p_article;
				p_group->num_arts--;
			}
			p_prev = p_article;
		}
	}
	ctime_r( &p_group->base_time, buf );
	buf[strlen( buf ) -1] = '\0';
	LOG_Info( "read group %s articles %d deletes %d time %s", p_group->name,
		arts, dels, buf );
	return arts;
}

void
AddFilter( P_GROUP p_group, char *in_pattern, int author ) {
	static char pattern[128];
	int i = 1, l;
	P_FILTER p_filter =p_group->p_filter, p_prev = NULL;
	char *p_pattern = pattern;
	if ( strlen(in_pattern) > 124 ) return;
	if ( author ) *p_pattern++ = 9;
	strcpy( p_pattern, in_pattern );
	for ( ; p_filter; p_filter = p_filter->p_next ) {
		p_pattern = (char *)p_filter->pattern;
		if ( ( i = strcmp( p_pattern, pattern ) ) == 0 ) return;
		if ( i > 0 ) break;
		p_prev = p_filter;
	}
	l = (sizeof( FILTER)-3) + strlen( pattern );
	p_filter = malloc( l );
	if ( p_filter == NULL ) return;
	p_pattern = (char *)p_filter->pattern;
	strcpy( p_pattern, pattern );
	pthread_mutex_lock( &group_sem );
	if ( p_prev == NULL ) {
		p_filter->p_next =p_group->p_filter;
		p_group->p_filter = p_filter;
	} else {
		p_filter->p_next =p_prev->p_next;
		p_prev->p_next = p_filter;
	}
	p_group->flags |= GF_FilterMod;
	pthread_mutex_unlock( &group_sem );
}

int
ReadFilters( P_GROUP p_group, char *name ) {
	P_FILTER p_filter, p_prev = NULL;
	FILE *in;
	int i, count = 0;
	char buf[512], *cp;
	MakeFileName( buf, 512, infodir, name, filt );
	in	= fopen( buf, "r" );
	if ( !in ) return -1;
	while ( ( cp = fgets( buf, 250, in ) ) ) {
		i = strlen( cp );
		if ( i < 3 || *cp == '#' ) continue;
		cp[--i] = '\0';
		i += (sizeof( FILTER)-3);
		if ( ( p_filter = malloc( i ) ) == NULL ) break;
		strcpy( (char *)p_filter->pattern, cp );
		if ( p_prev == NULL ) p_group->p_filter = p_filter;
		else p_prev->p_next =p_filter;
		p_prev = p_filter;
		count++;
	}
	p_filter->p_next = NULL;
	return count;
}

int
ReadGroup( P_GROUP p_group, int reading ) {
	int i, version, count = 0;
	unsigned int t_in;
	char *cp;
	FILE *in = NULL;
	ARTNUM artbase[MAXSERVERS];
	char buf[512];
	if ( !(reading&GR_HaveSem) ) {
		pthread_mutex_lock( &group_sem );
		if ( ( p_group->flags&GF_IO) || p_group->ov_req ) {
			pthread_mutex_unlock( &group_sem );
			return -1;
		}
	}
	if ( ( p_group->flags&GF_Read) ) {
		count = p_group->num_arts;
		if ( reading&GR_Current ) p_currentgroup = p_group;
		if ( !(reading&GR_HaveSem) )
			pthread_mutex_unlock( &group_sem );
		return count;
	}
	p_group->flags |= GF_Reading;
	pthread_mutex_unlock( &group_sem );
	MakeFileName( buf, 512, infodir, p_group->name, NULL );
	in	= fopen( buf, "r" );
	if ( !in ) goto done;
	LOG_Info( "reading group %s", p_group->name );
	p_group->base_time = 0;
	bzero( artbase, sizeof( artbase ) );
	if ( ( cp = fgets( buf, 250, in ) ) == NULL ) Oops( "no data", 0 );
	if ( strncmp( cp, "binger_", 7 ) ) Oops( "not a binger file", 0 );
	i = sscanf( cp+7, "%d %hd %u %hd %hd %d", &version, &p_group->retain,
		&t_in, &p_group->subjtype, &p_group->mode, &count );
	if ( i < 6 || count < 1 ) Oops( "invalid header", i );
	if ( version < 5 || version > 6 )
		Oops( "unsupported version", version );
	for ( i = 0; i < count; i++ ) {
		if ( ( cp = fgets( buf, 250, in ) ) == NULL ) break;
		artbase[i] = A2Art( cp );
		if ( ( cp = strchr( cp, ' ' ) ) == NULL ) break;
	}
	if ( i < count ) Oops( "insufficient server data", i+1 );
	p_group->base_time = t_in;
	if ( version == 6 ) {
		MakeFileName( buf, 512, infodir, p_group->name, "_parts" );
		p_group->p_stream	= fopen( buf, "r" );
		if ( !p_group->p_stream ) Oops( "no parts file", 0 );
	}
	ReadAscii( p_group, in, artbase, version );
	ReadFilters( p_group, p_group->name );
done:
	if ( in != NULL ) fclose( in );
	pthread_mutex_lock( &article_sem );
	if ( p_group->delete_ctr > 0 )
		RemoveDeletes( p_group, 0 );
	pthread_mutex_unlock( &article_sem );
	pthread_mutex_lock( &group_sem );
	p_group->flags &= ~GF_Reading;
	p_group->flags |= GF_Read|GF_HaveData;
	count = p_group->num_arts;
	if ( reading&GR_Current ) p_currentgroup = p_group;
	if ( !(reading&GR_HaveSem) )
	pthread_mutex_unlock( &group_sem );
	return count;
}

P_GROUP
AllocGroup( char *name ) {
	int i = sizeof( GROUP )+( sizeof(SRVRDATA)*(numservers-1));
	P_GROUP p_group = calloc( 1, i );
	p_group->name = name;
	p_group->max_arts = -1;
	for ( i = 0; i < numservers; i++ ) p_group->srvrdata[i].lowest = 0xffffffff;
	return p_group;
}

void
FreeGroup( P_GROUP p_group ) {
	int i, ctl = 0;
	if ( NULL == p_group ) return;
	if ( (p_group->flags&GF_NZB) ) {
		free( p_group->name );
		ctl = 1;
	}
	if ( p_group->p_articles != NULL ) {
		for ( i = 0; i < p_group->num_arts; i++ ) {
			FreeArticle( p_group->p_articles[i], ctl );
		}
		free( p_group->p_articles );
	}
	FreeArticle( p_group->p_dels, ctl );
	if ( !(p_group->flags&GF_Temp) )
		FreeFilters( p_group );
	free( p_group );
}

P_GROUP
AddGroup( char *name, ARTNUM first, ARTNUM last, int server ) {
	P_GROUP p_group;
	P_SRVRDATA p_srvrdata;
	int i;
	P_HASHENT p_hash = FindHash( group_hash, (puchar)name, strlen( name ), 0 );
	if ( p_hash->data.ptr == NULL ) {
		p_group = AllocGroup( (char *)p_hash->key );
		p_hash->data.ptr = (void *)p_group;
		if ( num_groups >= max_groups ) {
			max_groups += 1024;
			i = sizeof( P_GROUP )*(max_groups+1);
			p_groups = realloc( p_groups, i );
		}
		p_groups[num_groups++] = p_group;
	if ( store_msgids ) p_group->flags = GF_MsgIds;
	} else
		p_group = (P_GROUP)p_hash->data.ptr;
	p_srvrdata = &p_group->srvrdata[server];
	if ( p_srvrdata->first < first ) p_srvrdata->first = first;
	if ( p_srvrdata->last < last ) p_srvrdata->last = last;
	return p_group;
}

int
DelGroup( P_GROUP p_group, int pos ) {
	char buf[512], *name;
	pthread_mutex_lock( &group_sem );
	if ( ( p_group->flags&GF_IO) || p_group->ov_req ) {
		pthread_mutex_unlock( &group_sem );
		return -1;
	}
	name = p_group->name;
		MakeFileName( buf, 512, infodir, name, NULL );
	unlink( buf );
		MakeFileName( buf, 512, infodir, name, filt );
	unlink( buf );
	DelHash( group_hash, (puchar)name, strlen( name ), 1 );
	if ( pos >= 0 && pos < num_groups && p_group == p_groups[pos] )
		p_groups[pos] = NULL;
	free( p_group );
		pthread_mutex_unlock( &group_sem );
	return 0;
}

int
SelectGroup( P_CONNECTION p_conn, P_GROUP p_group, int upd_ctl ) {
	char buf[256], *cp;
	ARTNUM first, last;
	int server = p_conn->p_server->servernum, len;
	P_SRVRDATA p_srvrdata = &p_group->srvrdata[server];
	if ( upd_ctl == UP_Change && p_conn->p_group == p_group ) return 0;
	sprintf( buf, "GROUP %s", p_group->name );
	if ( SendCommand( p_conn, buf ) != NNTP_CLASS_OK ) return -1;
	cp = p_conn->inbuf+4;
	sscanf( cp, "%d%lu%lu%s", &len, &first, &last, buf );
	p_conn->p_group = p_group;
	LOG_Debug ( "%s %s %u %u", buf, GetDispName( server ),
		first, last );
	if ( first +2 > last ) return -1;
	pthread_mutex_lock( &group_sem );
	if ( upd_ctl == UP_Force || (p_group->ov_req) == 0 ) {
		if ( first > p_srvrdata->first )
			p_srvrdata->first = first;
		if ( last > p_srvrdata->last )
			p_srvrdata->last = last;
		if ( p_srvrdata->highest > last ) p_srvrdata->highest = last;
	}
	pthread_mutex_unlock( &group_sem );
	return 0;
}

int
UpdateGroupInfo( int server) {
	int j, status = 0;
	P_GROUP p_group;
	P_CONNECTION p_conn = OpenConnect( server, PRI_Low, STAT_Misc );
	if ( p_conn == NULL ) return -1;
	for ( j = 0; j < num_groups && status == 0; j++ ) {
		p_group = p_groups[j];
		if ( p_group == p_currentgroup || p_group->ov_req != 0 ) continue;
		status = SelectGroup( p_conn, p_group, UP_Check );
	}
	if ( p_conn->p_server->numfields == -1 ) {
		status = GetFields( p_conn );
		if ( status != 0 ) return status;
	}
	CloseConnect( p_conn, 0 );
	return status;
}

int
InitGroups( int no_sync) {
	FILE *in = NULL;
	char name[512], *buf, *cp, *prompt = ". ";
	struct stat statbuf;
	P_GROUP p_group;
	int i, j, len = strlen( infodir )+1;
	int status[MAXSERVERS];
	bzero( p_download, sizeof( p_download ) );
	group_hash = NewHashTable( "group", 717, 0 );
	MakeFileName( name, 512, infodir, "groups", NULL );
	in	= fopen( name, "r" );
	if ( !in ) return 0;
	buf = name+len;
	len = 510-len;
	while ( fgets( buf, len, in ) ) {
		cp = strchr( buf, ' ' );
		if ( cp ) *cp = '\0';
		else buf[strlen( buf )-1] = '\0';
		p_group = AddGroup( buf, 0, 0, 0 );
		p_group->flags = GF_Subscribed;
		if ( stat( name, &statbuf ) == 0 ) {
			if ( statbuf.st_size > 24 )
				p_group->flags |= GF_HaveData;
			for ( j = 0; j < numservers && cp; j++ ) {
				p_group->srvrdata[j].highest = A2Art( ++cp );
				cp = strchr( cp, ' ' );
			}
		}
	}
	fclose( in );
	if ( no_sync ) return 0;
	printf( "getting group information\n" );
	for ( i = 0; i < numservers; i++ ) {
		status[i] = 99;
		QueueWork(	i, TASK_GroupUpdate, NULL, NULL, 0, 0, &status[i] );
	}
	do {
		sleep(1);
		write( 1, prompt, 2 );
			for ( i = 0, j = 0; i < numservers; i++ )
			if ( status[i] != 99 ) j++;
	} while ( j < numservers );
			for ( i = 0, j = 0; i < numservers; i++ ) {
		if ( status[i] == 0 ) continue;
		fprintf( stderr, "init for %s failed, status %d\n",
			GetDispName( i ), status[i] );
		j = status[i];
	}
	return j;
}

int
SaveGroups( ) {
	FILE *out;
	P_GROUP p_group;
	int i, j;
	ARTNUM n;
	char name[512];
	if ( num_groups == 0 ) return 0;
	MakeFileName( name, 512, infodir, "groups", "tmp" );
	out	= DoCreate( name );
	for ( i = 0; i < num_groups; i++ ) {
		p_group = p_groups[i];
		if ( !(p_group->flags&GF_Subscribed) ) continue;
		fputs( p_group->name, out );
		for ( j = 0; j < numservers; j++ ) {
			n = p_group->srvrdata[j].highest;
			if ( n == 0 ) break;
			fprintf( out, " %lu", n );
		}
		fputc( '\n', out );
	}
	fclose( out );
	DoRename( name );
	return 0;
}

void
FreeGroups( ) {
	P_GROUP p_group;
	int i;
	if ( num_groups == 0 ) return;
	for ( i = 0; i < num_groups; i++ ) {
		p_group = p_groups[i];
		FreeGroup( p_group );
	}
	free( p_groups );
	FreeHashTable( group_hash );
}

int
QueueArt( P_ARTICLE p_article, P_ART_DIR p_dir, int cmd ) {
	P_PARTDATA p_part = p_article->p_parts;
	int i;
	short server = -1;
	if ( p_article->msg_id != NULL ) server = 0;
	else {
		for ( i = 0; i < p_article->parts && server == -1; i++, p_part++ ) {
			if ( p_part->artnum > 0 )
				server = p_part->server;
		}
	}
	pthread_mutex_lock( &article_sem );
	if ( bg_downloads++ == 0 ) bg_errors = 0;
	p_article->p_group->bg_downloads++;
	p_article->flags |= AF_B_Get;
	if ( p_dir != NULL ) p_dir->nref++;
	pthread_mutex_unlock( &article_sem );
	return QueueWork( server, TASK_FetchArt, p_article, p_dir, PRI_BG, cmd,
		NULL );
}

void
HandleSigInt( int sig ) {
	pthread_mutex_lock( &group_sem );
	if ( p_currentgroup ) {
		if ( p_currentgroup->ov_req != 0 )
			p_currentgroup->flags |= GF_Cancel;
	}
	pthread_mutex_unlock( &group_sem );
	pthread_mutex_lock( &article_sem );
	if ( p_download[0] )
		p_download[0]->flags |= AF_Cancel;
	pthread_mutex_unlock( &article_sem );
}

struct TG {
	char *name;
	ARTNUM first, arts;
	char posting;
};

static int
TGCompare( p1, p2 )
struct TG *p1, *p2;
{
	char *s1 = p1->name, *s2 = p2->name;
	return strcmp( s1, s2 );
}

static int
WriteTemps( struct TG *p_temps, int n_temps, FILE *out ) {
	char *cp1, *cp2, *last_name;
	int i, nchars;
	struct TG *p_temp;
	if ( n_temps > 1 )
		qsort( (void *)p_temps, n_temps, sizeof( struct TG ), TGCompare );
	p_temp = p_temps;
	fprintf( out, " %s %lu %lu %c\n", p_temp->name, p_temp->first, p_temp->arts,
		p_temp->posting	);
	last_name = p_temp->name;
	for ( i = 1; i < n_temps; i++ ) {
		p_temp++;
		cp2 = p_temp->name;
		for ( cp1 = last_name; *cp1 == *cp2; cp1++ ) cp2++;
		nchars = (cp1-last_name)+32;
		fprintf( out, "%c%s %lu %lu %c\n", nchars, cp2, p_temp->first, p_temp->arts,
			p_temp->posting	);
		last_name = p_temp->name;
	}
	p_temp = p_temps;
	for ( i = 0; i < n_temps; i++ ) {
		free( p_temp->name );
		p_temp->name = NULL;
		p_temp++;
	}
	return 0;
}

FILE *
LoadGroups( int server, char *msg ) {
	P_SERVER p_server = p_servers[server];
	FILE *in;
	time_t now;
	struct stat statbuf;
char name[512];
	MakeFileName( name, 512, infodir, p_server->dispname, "grp" );
	if ( stat( name, &statbuf ) != 0 ) return NULL;
	now = time( NULL );
	if ( statbuf.st_mtime < now-p_server->groupcache ) {
		unlink( name );
		return NULL;
	}
	if ( ( in	= fopen( name, "r" ) ) != NULL ) return in;
	strcpy( msg, "can't read group list file" );
	return NULL;
}

#define MAXTEMPS 2048

int
BuildGroups( int server, char *msg ) {
	int i, n_temps = 0;
	ARTNUM first, last;
	P_SERVER p_server = p_servers[server];
	struct TG *p_temp, *p_temps = NULL;
	FILE *out = NULL;
	P_CONNECTION p_conn = NULL;
char *cp, *cp1, name[512], posting;
	MakeFileName( name, 512, infodir, p_server->dispname, "grp" );
	if ( ( p_conn = OpenConnect( server, PRI_Low, STAT_Misc ) ) == NULL ) {
		strcpy( msg, "connection busy, try later" );
		goto load_error;
	}
	if ( ( out	= fopen( name, "w" ) ) == NULL ) {
		strcpy( msg, "can't create group list file" );
		goto load_error;
	}
	i = SendCommand( p_conn, "LIST ACTIVE" );
	if ( i != NNTP_CLASS_OK ) {
		strcpy( msg, "can't get group list" );
		goto load_error;
	}
	p_temps = calloc( MAXTEMPS, sizeof( struct TG ) );
	while ( 1 ) {
		cp = GetLine( p_conn, 5, &i );
		if ( i < 3 ) break;
		cp1 = strchr( cp, ' ' );
		if ( !cp1 || cp1-cp > 80 ) continue;
		*cp1++ = '\0';
		i = sscanf( cp1, "%lu %lu %c", &last, &first, &posting );
		if ( i < 3 ) continue;
		if ( last == 0 || last-mingrpsize <= first ) continue;
		p_temp = p_temps+n_temps;
		p_temp->name = strdup( cp );
		p_temp->first = first;
		p_temp->arts = last-first;
		p_temp->posting = posting;
		if ( ++n_temps < MAXTEMPS ) continue;
		n_temps = WriteTemps( p_temps, n_temps, out );
		if ( n_temps < 0 ) break;
	}
	CloseConnect( p_conn, 0 );
	if ( n_temps > 0 ) {
		n_temps = WriteTemps( p_temps, n_temps, out );
		fflush( out );
	}
	free( p_temps );
	if ( fclose( out ) == 0 && n_temps == 0 ) return 0;
	unlink( name );
	strcpy( msg, "error writing group list file" );
	return -1;
load_error:
	if ( p_conn != NULL ) CloseConnect( p_conn, 1 );
	if ( out != NULL ) fclose( out );
	if ( p_temps != NULL ) free( p_temps );
	return -1;
}

int
ApplyFilters( P_GROUP p_group, int *p_first ) {
	int count = 0, first = -1;
	int i, matched = 0;
	P_FILTER p_filter, p_start = p_group->p_filter;
	puchar p_pattern, cp;
	P_ARTICLE p_article, p_next;
	if ( p_start == NULL ) return -1;
	for ( i = 0; i < p_group->num_arts; i++ ) {
		p_article = p_group->p_articles[i];
		for ( p_next = p_article; p_next; p_next = p_next->p_next ) {
			for ( p_filter = p_start; p_filter; p_filter = p_filter->p_next ) {
				p_pattern = p_filter->pattern;
				if ( *p_pattern == 9 ) {
					cp = p_next->author;
				p_pattern++;
				} else cp = p_next->subject;
				if ( MatchPattern( (char *)cp, p_pattern ) ) break;
			}
			if ( p_filter != NULL ) {
				pthread_mutex_lock( &article_sem );
				p_next->flags |= AF_Kill;
				pthread_mutex_unlock( &article_sem );
				matched++;
			}
		}
		if ( matched > 0 ) {
			count += matched;
			matched = 0;
			if ( first == -1 ) first = i;
		}
	}
	if ( p_first ) *p_first = first;
	return count;
}
