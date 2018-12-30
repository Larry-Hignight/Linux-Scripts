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
#include <ctype.h>
#include <sys/wait.h>
#include "common.h"
#include "artdata.p"
#include "server.h"
#include "nzbparse.h"
#include <curses.h>

extern int topline, bottomline, num_scroll, lastline, COLS;

#define MAXPARTS 2048
static char *nzb_filter = "nzb";
static char *file_attr[4] = { "poster", "date", "subject", NULL };
static char *seg_attr[3] = { "bytes", "number", NULL };
static char *nzb_attr[2] = { "xmlns", NULL };

typedef struct S_TAGTBL TAGTBL, *P_TAGTBL;
struct S_TAGTBL {
	char *name;
	int prev, state, type;
	char **attributes;
};
static TAGTBL tags[] = {
	{ "nzb", NZ_Init, NZ_Nzb, NZ_WithAttr, nzb_attr },
	{ "file", NZ_Nzb, NZ_File, NZ_WithAttr, file_attr },
	{ "groups", NZ_File, NZ_Groups, 0, NULL },
	{ "group", NZ_Groups, NZ_Group, NZ_Text, NULL },
	{ "segments", NZ_File, NZ_Segs, 0, NULL },
	{ "segment", NZ_Segs, NZ_Seg, NZ_WithAttr+NZ_Text, seg_attr },
};

#define TAGSIZE (sizeof(tags)/sizeof(TAGTBL))
P_TAGTBL thisTag;

int	FindTag( char **in, char **attribVals, int *tagState ) {
	static int state = NZ_Init;
	char *tag = *in;
	char tagBuff[16], c1;
	int tagType = NZ_End, i = 0;
	char **attributes;
	thisTag = NULL;
	if ( *tag == '/' ) tag++;
	else tagType = NZ_Start;
	if ( (*tag&0xc0) != 0x40 ) {
		if ( '!' == *tag ) return NZ_Comment;
		return NZ_Unknown;
	}
	do {
		tagBuff[i++] = (*tag|0x20);
		tag++;
	} while ( i < 15 && (*tag&0xc0) == 0x40 );
	tagBuff[i] = 0;
	c1 = *tag++;
	*in = tag;
	for ( i = 0; i < TAGSIZE; i++ )
		if ( strcmp(tags[i].name, tagBuff ) == 0 ) break;
	if ( i >= TAGSIZE ) return NZ_BadTag;
	thisTag = &tags[i];
	if ( tagType == NZ_End ) {
		if ( c1 != '>' ) return NZ_Unknown;
		if ( state != thisTag->state ) return NZ_Unknown;
		state = thisTag->prev;
		*tagState = thisTag->state;
		return tagType;
	}
	if ( state != thisTag->prev ) return NZ_Unknown;
	*tagState = state = thisTag->state;
	for ( i = 0; i < 8; i++ ) attribVals[i] = NULL;
	tagType = thisTag->type;
	if ( c1 == '>' ) {
		if ( (tagType&NZ_WithAttr) ) return NZ_Unknown;
		return tagType;
	}
	if ( c1 != ' ' ) return NZ_Unknown;
	if ( !(tagType&NZ_WithAttr) ) return tagType;
	attributes = thisTag->attributes;
	while ( *tag && *tag != '>' ) {
		while ( *tag && *tag < '>' ) tag++;
		if ( *tag == '>' ) break;
		for ( i = 0; i < 15 && *tag > '>'; i++ ) {
			tagBuff[i] = tolower( *tag );
			tag++;
		}
		tagBuff[i] = '\0';
		if ( *tag++ != '=' ) continue;
		if ( *tag == '"' )
			c1 = *tag++;
		else c1 = ' ';
		for ( i = 0; attributes[i] != NULL; i++ )
			if ( strcmp( tagBuff, attributes[i] ) == 0 ) break;
		if ( attributes[i] != NULL && attribVals[i] == NULL )
			attribVals[i] = tag;
		else tag++;
		while ( *tag && *tag != c1 ) tag++;
		if ( *tag ) *tag++ = '\0';
	}
	if ( *tag == '>') tag++;
	*in = tag;
	return tagType;
}

char *GetText( char *in, char *buf, int size, int *p_chars ) {
	char *cp = buf;
	while ( *in && *in != '<' ) {
		if ( cp-buf < size ) *cp++ = *in;
		in++;
	}
	*cp = '\0';
	if ( p_chars != NULL ) *p_chars = cp-buf;
	return in;
}

static char par2cmd[256];

int
SetPar2( char *name ) {
	struct stat statbuf;
	if ( stat( name, &statbuf ) != 0 || !(statbuf.st_mode&S_IXUSR) ) {
		fprintf( stderr, "the par2 command %s is not executeable\n", name );
		return -1;
	}
	strcpy( par2cmd, name );
	return 0;
}

int
RunPar2( char *name ) {
	int pid, status;
	char *cp, *argv[6];
	if ( *par2cmd == '\0' ) return -1;
	if ( ( pid = fork( ) ) == 0 ) {
		cp = strrchr( par2cmd, '/' );
		if ( cp ) cp++;
		else cp = par2cmd;
		argv[0] = cp;
		argv[1] = "-q";
		argv[2] = "-q";
		argv[3] = name;
		argv[4] = NULL;
		return execv( par2cmd, argv );
	}
	waitpid( pid, &status, 0 );
	if ( WIFEXITED( status ) )
		return WEXITSTATUS( status );
	return 0;
}

static char *entitylist[] = { "'apos", "\"quot", "&amp", "<lt", ">gt" };

void
CopyBuf ( char *out, char *in, int len ) {
	char *ps = in, *pd = out, *pe, *pe1,	ch;
	int i;
	while ( ( ch = *ps++ )	) {
		if ( pd-out >= len ) break;
		if ( ch != '&' ) {
			*pd++ = ch;
			continue;
		}
		pe = strchr( ps, ';' );
		if ( pe == NULL || pe-ps > 6 ) {
			*pd++ = ch;
			continue;
		}
		ch = '\0';
		*pe = '\0';
		if ( *ps == '#' ) {
			ps++;
			if ( *ps >= '0' && *ps <= '9' ) i = atoi( ps );
			else if ( *ps == 'x' || *ps == 'X' ) 
				i = strtol( ++ps, NULL, 16 );
			else i = 0;
			if ( i > 0 ) ch = (i&0xff);
		} else {
			for ( i = 0; ( pe1 = entitylist[i] ) != NULL; i++ ) {
				if ( !strcmp( ps, pe1+1 ) ) {
					ch = *pe1;
					break;
				}
			}
		}
		if ( ch == '\0' )
			*pe = ';';
		else {
			*pd++ = ch;
			ps = pe+1;
		}
	}
	*pd = '\0';
}
		
static P_GROUP p_group = NULL;
static char groupName[256], msg_ids[MAXPARTS][72];
static char authbuf[128], subjbuf[512];
static int parts, bytes[MAXPARTS];
static time_t a_date;

int
GenArticle( ) {
	int i;
	P_ARTICLE p_article = NULL;
	ART_INFO info;
	bzero( &info, sizeof( info ) );
	info.author = (uchar *)authbuf;
	info.authlen = strlen( authbuf );
	info.subject = (uchar *)strdup( subjbuf );
	info.subjlen = strlen( subjbuf );
	info.artdate = a_date;
	info.parts = parts;
	p_article = AllocArticle( &info, p_group, NULL, info.subject, NULL );
	for ( i = 0; i < parts; i++ ) {
		if ( msg_ids[i][0] == '\0' ) {
			LOG_Error( "part %d is missing" );
			continue;
		}
		info.msg_id = (uchar *)msg_ids[i];
		info.msglen = strlen( msg_ids[i] );
		info.bytes = bytes[i];
		info.part = i+1;
		AddPart( &info, p_article );
	}
	return 0;
}

int ProcessNZB( char *in ) {
	char *attr[8];
	char buf[128];
	int tt, i;
	static int state = 0;
	while ( *in ) {
		if ( *in++ != '<' ) continue;
		tt = FindTag( &in, attr, &state );
		if ( tt == NZ_Comment ) break;
		if ( tt < NZ_End ) continue;
		if ( state == NZ_File ) {
			if ( tt ==NZ_End )
				GenArticle( );
			else {
				*groupName = '\0';
				for ( i = 0; i < MAXPARTS; i++ ) {
					msg_ids[i][0] = '\0';
					bytes[i] = 0;
				}
				parts = 0;
				if ( attr[1] != NULL )
					a_date = atoi( attr[1] );
				if ( attr[0] != NULL )
					CopyBuf( authbuf, attr[0], 127 );
				if ( attr[2] != NULL )
					CopyBuf( subjbuf, attr[2], 500 );
			}
		}
		if ( !(tt&NZ_Text) ) continue;
		if ( state == NZ_Group && p_group == NULL ) {
			in = GetText( in, groupName, 250, NULL );
			p_group = AllocGroup( strdup( groupName ) );
			p_group->flags = GF_Temp|GF_MsgIds|GF_NZB;
		} else if ( state == NZ_Seg && attr[1] != NULL	) {
			i = atoi( attr[1] );
			if ( i < 1 || i > MAXPARTS-1 ) return -1;
			if ( parts < i ) parts = i;
			in = GetText( in, buf, 70, NULL );
			CopyBuf( msg_ids[--i], buf, 70 );
			if ( attr[0] != NULL )
				bytes[i] = atoi( attr[0] );
		}
	}
	return state;
}

int
ProcessNZBFile( char *name ) {
	char buf[256];
	u_char *cp = (u_char*)buf;
	int n, lc = 0, is_gz = 0;
	p_group = NULL;
	FILE *infile = fopen( name, "r" );
	if ( infile == NULL )	{
		return E_FileOpen;
	}
	bzero( buf, 256 );
	fread( buf, 1, 2, infile );
	if ( *cp == 0x1f && cp[1] == 0x8b ) { // gzip file
		fclose( infile );
		sprintf( buf, "/bin/zcat \"%s\"", name );
		infile = popen( buf, "r" );
		if ( infile == NULL ) 
			return E_FileOpen;
		is_gz = 1;
	} else rewind( infile );
	while ( fgets( buf, 255, infile ) ) {
		lc++;
		n = ProcessNZB( buf );
		if ( n < 0 )
			break;
	}
	if ( is_gz ) pclose( infile );
	else fclose( infile );
	if ( n != NZ_Nzb ) {
		mvprintw( lastline-2, 0, "nzb file error %d at line %d", n, lc );
		move( lastline-2, 0 );
		refresh( );
		FreeGroup( p_group );
		return -1;
	}
	if ( !p_group || p_group->num_arts < 1 ) {
		mvprintw( lastline-2, 0, "nzb file contains no articles" );
		move( lastline-2, 0 );
		refresh( );
		FreeGroup( p_group );
		return -1;
	}
	SortArticles( p_group, 4 );
	ReadFilters( p_group, nzb_filter );
	p_group->bg_downloads++; /* to prevent freeing by downloader */
	ArticleHandler( p_group, 1, 0 );
	if ( (p_group->flags&GF_FilterMod) )
		SaveFilters( p_group, nzb_filter );
	FreeFilters( p_group );
	pthread_mutex_lock( &article_sem );
	n = --p_group->bg_downloads;
	if ( p_group->bg_downloads == 0 )
		FreeGroup( p_group );
	pthread_mutex_unlock( &article_sem );
	return n;
}
