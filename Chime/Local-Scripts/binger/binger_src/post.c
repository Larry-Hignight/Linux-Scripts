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
#include <sys/utsname.h>
#include "common.h"
#include "server.p"
#include "artdata.p"
#include <curses.h>

extern int topline, bottomline, num_scroll, lastline, COLS;

static char editor[256] = "";
static char poster[80]= "", sender[60] = "";

int
SetEditor( char *name ) {
	struct stat statbuf;
	if ( stat( name, &statbuf ) != 0 || !(statbuf.st_mode&S_IXUSR) ) {
		fprintf( stderr, "the etitor %s is not executeable\n", name );
		return -1;
	}
	strcpy( editor, name );
	return 0;
}

int
SetPoster( char *in ) {
	char	*paren, *angle, *email, *p_at, *errbuf = NULL;
	char buf[80];
	Trim( in );
	if ( strlen( in ) > 70 ) return -1;
	strcpy( buf, in );
	paren = strchr( buf, '(' );
	angle = strchr( buf, '<' );
	// First form: No parens, no angle brackets
	if ( !paren && !angle ) {
		email = buf;
check_email:
		Trim( email );
		p_at = strchr( email, '@' );
		if ( p_at && strlen( email ) < 50 ) {
			strcpy( poster, in );
			strcpy( sender, email );
			return 0;
		}
		errbuf = "invalid email address";
		goto oops;
	} else if ( paren ) { // Second form: Comment in parentheses
		email = buf;
		*( paren++ ) = '\0';
		if ( ( paren = strchr( paren, ')' ) ) ) {
			*paren = '\0';
			goto check_email;
		}
		errbuf = "missing )";
		goto oops;
	} else if ( angle ) { // Third form: Email in angle brackets
		*( angle++ ) = '\0';
		email = angle;
		if ( ( angle = strchr( email, '>' ) ) ) {
			*angle = '\0';
			goto check_email;
		}
		errbuf = "missing >";
	} else errbuf = "unknown format";
oops:
	fprintf( stderr, "invalid posting id: %s\n", errbuf );
	return -1;
}

static void
PostingDate( FILE *out ) {
	char datebuf[80];
	time_t now;
	const struct tm *tm;
	time( &now );
	tm = gmtime( &now );
	strftime( datebuf, sizeof( datebuf )-1, "%a, %d %b %Y %H:%M:%S %Z", tm );
	fprintf( out, "Date: %s\n", datebuf );
}

static void
PostMsgID( FILE *out ) {
	struct utsname uts;
	char *cp;
	if ( uname( &uts ) ) cp = "localhost";
	else {
		cp = uts.nodename;
		while ( strlen( cp ) > 80 ) {
			char *cp1 = strrchr( cp, '.' );
			if ( !cp1 ) break;
			*cp1 = '\0';
		}
	}
	fprintf( out, "Message-ID: <%lx%x@%s>\n", time( NULL ), getpid( ), cp );
}

int
MakeHeader( char *name, char *groupname, char *subj ) {
	int lc = 8;
	FILE *out = fopen( name, "w" );
	if ( !out ) return -1;
	fprintf( out, "From: %s\n", poster );
	if ( *sender ) fprintf( out, "Sender: %s\n", sender );
	else lc--;
	fprintf( out, "User-Agent: binger v1.6\n" );
	PostingDate( out );
	fprintf( out, "Newsgroups: %s\n", groupname );
	fprintf( out, "Subject: %s\n", subj );
		PostMsgID( out );
	fprintf( out, "\n\n" );
	fclose( out );
	return lc;
}

int
PostArt( char *groupname, P_ARTICLE p_article ) {
	int i, pid, status, lc = 1, hl;
	FILE *in = NULL;
	P_CONNECTION p_conn;
	struct stat statbuf;
	char buf[160], subj[160], argv1[16];
	char *cp, *cp1, *artbuf = NULL, *argv[4];
	char *name = "post.0";
	clear( );
	refresh( );
	if ( *editor == '\0' ) {
		strcpy( buf, "no editor is defined" );
		goto oops;
	}
	if ( *poster == '\0' ) {
		strcpy( buf, "poster not set" );
		goto oops;
	}
	if ( p_article != NULL ) {
		cp = subj;
		if ( !((p_article->flags)&AF_HasRe) ) {
			strcpy( subj, "Re: " );
			cp += 4;
		}
		strncpy( cp, (char *)p_article->subject, 100 );
		cp[100] = '\0';
		Trim( cp );
		cp = subj;
		mvprintw( 0, 0, "posting follow-up to:" );
		mvprintw( 1, 0, cp );
		mvprintw( 2, 0, "press enter to continue, anything else to cancel" );
		refresh( );
		i = GetKey( 30 );
		if ( i != 10 ) return 0;
	} else {
		mvprintw( 0, 0, "enter subject or return to abort" );
		refresh( );
		GetBuff( subj, 1, 0 );
		if ( strlen( Trim( subj ) ) < 3 ) return 0;
	}
	hl = MakeHeader( name, groupname, subj );
	if ( hl <= 1 ) {
		sprintf( buf, "make header error %d", hl );
		goto oops;
	}
	clear( );
	refresh( );
	DisableWin( );
	if ( ( pid = fork( ) ) == 0 ) {
		cp = strrchr( editor, '/' );
		if ( cp ) cp++;
		else cp = editor;
		argv[0] = cp;
		sprintf( argv1, "+%d", hl+1 );
		argv[1] = argv1;
		argv[2] = name;
		argv[3] = NULL;
		return execv( editor, argv );
	}
	waitpid( pid, &status, 0 );
	EnableWin( );
	clear( );
	refresh( );
	if ( WIFEXITED( status ) ) {
		int stat = WEXITSTATUS( status );
		if ( stat != 0 ) {
			sprintf( buf, "editor exited with error %d", status );
			goto oops;
		}
	}
	if ( stat( name, &statbuf ) != 0 ) {
		sprintf( buf, "can't stat %s", name );
		goto oops;
	}
	if ( statbuf.st_size > 100000 ) {
		sprintf( buf, "article of %ld bytes too large", statbuf.st_size );
		goto oops;
	}
	if ( !( in	= fopen( name, "r" ) ) ) {
		sprintf( buf, "can't open %s", name );
		goto oops;
	}
	for ( lc = 1; ( cp = fgets( buf, 156, in ) ); lc++ )
		if ( *cp == '\n' ) break;
	if ( lc != hl ) {
		strcpy( buf, "header invalid" );
		goto oops;
	}
	i = lc = 0;
	while ( ( cp = fgets( buf, 156, in ) ) ) {
		if ( *cp == '.' ) i++;
		if ( cp[strlen( cp )-1] == '\n' ) lc++;
	}
	rewind( in );
	if ( lc == 0 ) {
		strcpy( buf, "empty post" );
		fclose( in );
		goto oops;
	}
	i += (int)(statbuf.st_size)+lc+200;
	if ( !( artbuf = malloc( i ) ) ) {
		sprintf( buf, "can't alloc %d bytes for article", i );
		fclose( in );
		goto oops;
	}
	sprintf( argv1, "Lines: %d\r\n", lc );
	cp1 = artbuf;
	i = 0;
	while ( ( cp = fgets( buf, 156, in ) ) ) {
		i++;
		if ( i == hl ) {
			strcpy( cp1, argv1 );
			cp1 += strlen( argv1 );
		}
		if ( *cp == '.' ) *cp1++ = '.';
		while ( *cp ) *cp1++ = *cp++;
		if ( cp1[-1] != '\n' ) continue;
		cp1[-1] = '\r';
		*cp1++ = '\n';
	}
	fclose( in );
	if ( cp1[-1] != '\n' ) {
		strcpy( cp1, "\r\n" );
		cp1 += 2;
	}
	*cp1 = '\0';
	mvprintw( 0, 0, "post %d lines to %s", lc, groupname );
	mvprintw( 2, 0, "post article? (y/n)" );
	refresh( );
	do {
		beep( );
		i = GetKey( 60 );
		if ( i == 'n' ) goto no_post;
	} while ( i != 'y' );
	LOG_Status ( "post %d lines to %s", lc, groupname );
	i = cp1-artbuf;
	if ( ( p_conn = OpenConnect( 0, PRI_FG, STAT_Article ) ) == NULL ) {
		free( artbuf );
		strcpy( buf, "can't get connection for post" );
		goto oops;
	}
	status = SendCommand( p_conn, "POST" );
		if ( status != NNTP_CLASS_CONT ) goto post_err;
	status = send( p_conn->sckt, artbuf, i, MSG_NOSIGNAL );
	if ( status != i ) {
		sprintf( buf, "send failed, returned %d", i );
		goto post_err1;
	}
	status = SendCommand( p_conn, "." );
		if ( status != NNTP_CLASS_OK ) {
post_err:
		sprintf( buf, "posting rejected, error %d", NntpStatus( p_conn, NULL ) );
post_err1:
		free( artbuf );
		CloseConnect( p_conn, 1 );
		LOG_Error ( buf );
		goto oops;
	}
	CloseConnect( p_conn, 0 );
	LOG_Status ( "posted %d lines to %s", lc, groupname );
	mvprintw( 0, 0, "posting successful");
	refresh( );
	sleep( 2 );
no_post:
	free( artbuf );
	unlink( name );
	return 0;
oops:
	mvprintw( bottomline, 0, buf );
	mvprintw( lastline, 0, "press a key to return to article list" );
	refresh( );
	i = GetKey( 30 );
	return -1;
}
