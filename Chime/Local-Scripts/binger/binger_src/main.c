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
#include "nzbparse.h"
#include <unistd.h>
#include "artdata.p"
#include "server.h"
#include <curses.h>
#include <term.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

char progname[] = "( binger 1.6 )";
char *nzb_name = NULL;
extern char savedir[], infodir[];
int lastline = 24, bottomline = 22, topline = 1, num_scroll = 20;
static int groupnum = 0, in_win = 0, lnum = 1;
static int min, max;
extern P_GROUP *p_groups;
extern int num_groups, ignore_threading, use_colors;
static P_GROUP p_group = NULL;

void
DisableWin( ) {
	if ( in_win == 0 ) return;
	clear();
	keypad(stdscr,0);
	refresh();
	delwin( stdscr );
	endwin();
	in_win = 0;
}


void
HandleSigPipe( int status ) {
}

void
Cleanup( int status ) {
	int err;
	if ( !in_win ) return;
	DisableWin( );
	FreeServers( );
	err = SaveGroup( NULL, GR_Save );
	if ( err == 0 ) fprintf( stderr, "save ok\n" );
	else if ( err == -1 )fprintf( stderr, "error saving current group\n" );
	SaveGroups( );
	if ( status >= 0 ) exit( status );
}

void
EnableWin( ) {
	signal( SIGHUP, Cleanup );
	signal( SIGQUIT, Cleanup );
	signal( SIGINT, HandleSigInt );
	signal(SIGPIPE, HandleSigPipe);
	signal( SIGTERM, Cleanup );
	if ( in_win != 0 ) return;
	initscr();
	cbreak();
	noecho();
	keypad(stdscr,1);
	if ( !has_colors() ) use_colors = 0;
	in_win = 1;
	lastline = LINES-1;
	bottomline = LINES-3;
	num_scroll = bottomline-topline;
}

static int
PrintGroup( int group, int ln, int ctl ) {
	ARTNUM arts;
	char ch;
	P_SRVRDATA p_srvrdata;
	if ( group < 0 || group >= num_groups ) {
		if ( num_groups == 0 ) mvprintw( 0, 64, "no groups	 " );
		return -1;
	}
	p_group = p_groups[group];
	p_srvrdata = p_group->srvrdata;
	if ( ln < topline ) {
		ln = topline;
		move( bottomline-1, 0 );
		clrtobot( );
		move( topline, 0 );
		insertln( );
		ctl |= 2;
	} else if ( ln >= bottomline ) {
		ln = bottomline-1;
		move( topline, 0 );
		deleteln( );
		move( ln, 0 );
		clrtobot( );
		ctl |= 2;
	} else if ( !(ctl&8) ) {
		move( bottomline, 0 );
		clrtobot( );
	}
	if ( ctl&1 ) {
		mvprintw( 0, 64, "%5d of %5d", group+1, num_groups );
	}
	if ( ctl&4 ) {
		move( ln, 0 );
		clrtoeol( );
	}
	if ( ctl&2 ) {
		arts = p_srvrdata->first;
		if ( p_srvrdata->highest > arts )
			arts = p_srvrdata->highest;
		arts = p_srvrdata->last-arts;
		ch = ( (p_group->flags&GF_Subscribed)	) ? '*' : ' ';
			mvprintw( ln, 0, "%-8u %c %s", arts, ch, p_group->name );
	}
	move( ln, 0 );
	if ( !(ctl&16) ) {
		lnum = ln;
		groupnum = group;
	}
	return ln;
}

static int
PageGroup( int group, char *pInfo ) {
int line;
	ARTNUM arts;
	char ch;
	P_SRVRDATA p_srvrdata;
	clear( );
	refresh( );
	mvprintw( 0, 0, "%s group selection menu", progname );
	if ( num_groups == 0 ) {
		mvprintw( 0, 64, "No Groups	 " );
		return -1;
	}
	if ( group < 0 ) {
		group = 0;
		lnum = topline;
	} else if ( group >= num_groups ) {
		group = num_groups-1;
		lnum = bottomline-1;
	}
	groupnum = group;
	group -= (lnum-topline);
	if ( group <= 0 ) group = 0;
	mvprintw( 0, 64, "%5d of %5d", groupnum+1, num_groups );
	lnum = topline;
	for ( line = topline; line < bottomline; line++ ) {
		if ( groupnum == group ) lnum = line;
		p_group = p_groups[group++];
		p_srvrdata = p_group->srvrdata;
		arts = p_srvrdata->first;
		if ( p_srvrdata->highest > arts )
			arts = p_srvrdata->highest;
		arts = p_srvrdata->last-arts;
		ch = ( (p_group->flags&GF_Subscribed)	) ? '*' : ' ';
			mvprintw( line, 0, "%-8u %c %s", arts, ch, p_group->name );
		if ( group >= num_groups ) break;
	}
	p_group = p_groups[groupnum];
	move( lastline, 0 );
	clrtoeol( );
	if ( pInfo ) mvprintw( lastline, 0, pInfo );
	move( lnum, 0 );
	return lnum;
}

static int
GotoGroup( int key ) {
	int n;
	char buf[80], *cp;
	cp = GetBuff( buf, lastline, key );
	move( lnum, 0 );
	if ( !cp ) return -1;
	n = atoi( buf )-1;
	if ( n < 0 || n >= num_groups ) return -1;
	if ( n >= min && n <= max ) {
		int i = min + (lnum-topline);
		PrintGroup( n, i, 3 );
			} else PageGroup( n, NULL );
	return 0;
}

int
PrintStatus( char *msg, int line ) {
	move( lastline, 0 );
	clrtoeol( );
	mvprintw( lastline, 0, msg );
	move( line, 0 );
	refresh( );
	return -1;
}

void
PrintTransferStats( ) {
	char ssl_info[48], *cp;
	int i= 0, n, nb[3];
	clear( );
	mvprintw( 0, 0, "transfer stats in k bytes" );
	mvprintw( 1, 4, "name				 misc overview	article" );
	while ( ( n = GetServerStats( i, nb, ssl_info ) ) >= 0 ) {
		cp = GetDispName( i );
		mvprintw( i+2, 0, cp );
		mvprintw( i+2, 12, " %7d %7d %7d", nb[0], nb[1], nb[2] );
		if ( *ssl_info != '\0' ) mvprintw( i+2, 40, ssl_info );
		i++;
	}
	mvprintw( lastline, 0, "press any key to return to group menu" );
	move( 2, 0 );
	refresh( );
	GetKey( 30 );
}

static int
GroupStatus( char *msg, int count ) {
	char buf[80];
	sprintf( buf, msg, count );
	return PageGroup( groupnum, buf );
}

static int
GroupCompare( p1, p2 )
P_GROUP *p1, *p2;
{
	char *s1 = (*p1)->name, *s2 = (*p2)->name;
	return strcmp( s1, s2 );
}

static int
GetGroups( P_GROUP this_group, int server ) {
	int i = 0, lc = 0;
	ARTNUM first, last;
	FILE *in;
	char buf[256], group[200], *cp;
	char posting[4];
	in = LoadGroups( server, buf );
	if ( in == NULL ) {
		clear( );
		mvprintw( 0, 0, "constructing group list" );
		refresh( );
		if ( BuildGroups( server, buf ) == 0 )
			in = LoadGroups( server, buf );
	}
	if ( in == NULL ) {
		PrintStatus( buf, lnum );
		return -1;
	}
	while ( fgets( buf, 250, in ) ) {
		lc++;
		i = (int)(*buf)-32;
		if ( i < 0 || i > 127 ) break;
		cp = group+i;
		if ( sscanf( buf+1, "%s%lu%lu%s\n", cp, &first, &last, posting ) < 4 )
			continue;
		last += first;
		if ( MatchPattern( group, NULL ) )
			p_group = AddGroup( group, first, last, server );
	}
	fclose( in );
	if ( num_groups > 1 )
		qsort( (void *)p_groups, num_groups, sizeof( P_GROUP ), GroupCompare );
	if ( this_group != NULL ) {
		for ( i = 0; i < num_groups; i++ )
			if ( this_group == p_groups[i] ) break;
		if ( i == num_groups ) i = 0;
	} else i = 0;
	return PageGroup( i, NULL );
}

static int
MatchGroups( P_GROUP this_group ) {
	int i, count = 0;
	char *cp, cmd = ' ';
	move( lnum, 0 );
	if ( ( cp = GetPattern( NULL, NULL ) ) == NULL ) return 0;
	if ( *cp == ':' ) cp++;
	cmd = *cp;
	if ( cmd == '\0' ) {
		for ( i = 0; i < num_groups; i++ ) {
			p_group = p_groups[i];
			if ( MatchPattern( p_group->name, NULL ) ) break;
		}
		if ( i == num_groups ) return -1;
		return PageGroup( i, NULL );
	}
	if ( cmd == 'y' )
		return GetGroups( p_group, 0 );
	if ( cmd == 'k' ) {
		for ( i = 0; i < num_groups; i++ ) {
			p_group = p_groups[i];
			if ( !MatchPattern( p_group->name, NULL ) ) continue;
			if ( DelGroup( p_group, i ) == 0 )
				count++;
		}
		if ( count == 0 ) return PageGroup( groupnum, "no groups found" );
		groupnum = -1;
		for ( i = 0; p_groups[i] != NULL; i++ )
			if ( this_group == p_groups[i] ) groupnum = i;
		for ( count = i++; i < num_groups; i++ ) {
			if ( p_groups[i] == NULL ) continue;
			p_groups[count] = p_groups[i];
			if ( this_group == p_groups[count] ) groupnum = count;
			count++;
		}
		p_groups[count] = NULL;
		if ( groupnum == -1 ) groupnum = 0;
		num_groups = count;
		return GroupStatus( "%d groups deleted", count );
	} else if ( cmd == 's' || cmd == 'u' ) {
		for ( i = 0; i < num_groups; i++ ) {
			p_group = p_groups[i];
			if ( !MatchPattern( p_group->name, NULL ) ) continue;
			count++;
			if ( cmd == 's' ) p_group->flags |= GF_Subscribed;
			else p_group->flags &= ~GF_Subscribed;
		}
		cp = ( cmd == 's' ) ?	"%d subscribed" : "%d unsubscribed";
		return GroupStatus( cp, count );
	}
	return -1;
}

void
DoBatch( ) {
	int i;
	for ( i = 0; i < num_groups; i++ )
		UpdateOverview( p_groups[i], OV_Bg|OV_Batch );
	do {
		sleep( 5 );
		pthread_mutex_lock( &group_sem );
		for ( i = 0; i < num_groups; i++ ) {
			p_group = p_groups[i];
			if ( p_group->ov_req != 0 ) break;
		}
		pthread_mutex_unlock( &group_sem );
	} while ( i < num_groups );
	SaveGroups( );
	FreeServers( );
	exit( 0 );
}

void
DoNzb( void ) {
	EnableWin( );
				int stat = ProcessNZBFile( nzb_name );
	while ( ( stat = GetNumDownloads( ) ) > 0 ) {
		PrintStatus( "waiting for downloads", lastline );
		sleep( 9 );
		move( lastline, 0 );
		clrtoeol( );
	refresh( );
		sleep( 1 );
	}
	DisableWin( );
	FreeQueues( );
	FreeServers( );
	FreeArticles( );
	exit( 0 );
}

int
CheckBackground( ) {
	int i, first = -1, done = 1, count = GetNumDownloads( );
	P_GROUP p_group;
	char buf[80];
	if ( count > 0 ) {
		if ( count == 1 ) strcpy( buf, "1 download" );
		else sprintf( buf, "%d downloads", count );
		PrintStatus( buf, lnum );
		done = 0;
	}
	for ( count = 0, i = 0; i < num_groups; i++ ) {
		p_group = p_groups[i];
		if ( (p_group->flags&GF_IO) || p_group->ov_req != 0 ) {
			if ( first == -1 ) first = i;
			count++;
		}
	}
	if ( count > 0 ) {
		p_group = p_groups[first];
		if ( done == 0 ) sleep( 1 );
		sprintf( buf, "background update for: %s", p_group->name );
		PrintStatus( buf, lnum );
		done = 0;
	}
	return done;
}

int
main( int argc, char *argv[] ) {
	int done = 0, stat = 0, no_sync = 0;
	int key, i, n;
	int batch_mode = 0;
	char *cp;
	struct rlimit rlim;
	char *configname = NULL;
	if ( getrlimit( RLIMIT_CORE, &rlim) == 0) {
		rlim.rlim_cur = rlim.rlim_max;
		setrlimit(RLIMIT_CORE, &rlim);
	}
	while ( --argc > 0 ) {
		argv++;
	if ( !strcmp( *argv, "-s" ) ) no_sync = 1;
	else if ( !strcmp( *argv, "-b" ) ) batch_mode = 1;
	else if ( !strcmp( *argv, "-i" ) ) ignore_threading = 1;
	else if ( !strcmp( *argv, "-d" ) ) {
			argv++;
			if ( --argc > 0 ) SetDebug(atoi(*argv) );
	} else if ( !strcmp( *argv, "-z" ) ) {
			argv++;
			if ( --argc > 0 ) nzb_name = *argv;
		} else if ( *argv[0] != '-' ) configname = *argv;
	}
	printf( "%s initializing\n", progname );
	InitDate( );
	if ( ReadConfig( configname ) != 0 ) {
		fprintf( stderr, "config error\n" );
		exit( 1 );
	}
	if ( *savedir && chdir( savedir ) ) {
		fprintf( stderr, "can't cd to %s\n", savedir );
		FreeServers( );
		exit( 3 );
	}
	InitQueues( );
	InitArticles( );
	if ( nzb_name ) DoNzb( ); // will exit when done
	if ( InitGroups( no_sync ) != 0 ) {
		fprintf( stderr, "sorry love, no news is bad news\n" );
		FreeServers( );
		exit( 4 );
	}
	if (num_groups == 0 ) {
		char buf[80];
		BuildGroups( 0, buf );
	}
	if ( batch_mode ) DoBatch( ); // will exit when done
	EnableWin( );
	PageGroup( 0, NULL );
	while ( !done ) {
		if ( stat == -1 ) beep( );
		refresh( );
		stat = 0;
		min = groupnum - (lnum-topline);
		max = groupnum + ( bottomline-lnum );
		key = GetKey( 0 );
		if ( key >= '0' && key <= '9' ) {
			stat = GotoGroup( key );
			if ( stat != 0 ) continue;
			key = 10;
		}
		switch (key) {
		case 0:
			beep( );
			break;
		case EOF:
		case 4:
		case 'q':
			done = CheckBackground( );
			break;
		case KEY_F(1):
		case 'h':
			stat = ShowHelp( "binger_groups" );
			if ( stat == 0 )
				PageGroup( groupnum, NULL );
			break;
		case '/':
			MatchGroups( p_group );
			break;
		case KEY_UP:
			stat = PrintGroup( groupnum-1, lnum-1, 3 );
			break;
		case KEY_PPAGE:
			if ( groupnum <= 0 ) stat = -1;
			else {
				groupnum -= num_scroll;
				PageGroup( groupnum, NULL );
			}
			break;
		case KEY_HOME:
			if ( groupnum < num_scroll )
				PrintGroup( 0, topline, 3 );
			else PageGroup( 0, NULL );
			break;
		case KEY_DOWN:
			stat = PrintGroup( groupnum+1, lnum+1, 3 );
			break;
		case 'u':
			if ( groupnum >= num_groups ) {
				stat = -1;
				break;
			}
			p_group = p_groups[groupnum];
			stat = DelGroup( p_group, groupnum );
			if ( stat == -1 ) {
				PrintStatus( "group is currently updating", lnum );
				break;
			}
			num_groups--;
			for ( i = groupnum; i < num_groups; i++ )
				p_groups[i] = p_groups[i+1];
			p_groups[i] = NULL;
			clrtobot( );
			if ( groupnum == num_groups ) {
				PrintGroup( --groupnum, --lnum, 3 );
				break;
			}
			n = lnum;
			i = groupnum;
		for	( ; groupnum < num_groups; groupnum++ ) {
				PrintGroup( groupnum, lnum, 3 );
				if ( ++lnum >= bottomline ) break;
			}
			PrintGroup( i, n, 3 );
			break;
		case 32:
		case KEY_NPAGE:
			if ( max >= num_groups ) stat = -1;
			else {
				groupnum += num_scroll;
				stat = PageGroup( max, NULL );
			}
			break;
		case KEY_END:
			n = num_scroll;
			if ( n > num_groups ) n = num_groups;
			if ( groupnum >= num_groups-n )
				PrintGroup( num_groups-1, n-1+topline, 3 );
			else {
				lnum = bottomline-1;
				stat = PageGroup( num_groups-1, NULL );
			}
			break;
		case KEY_LEFT:
			stat = -1;
			break;
		case 'e':
		case 10:
		case KEY_RIGHT:
			if (!p_group) {
				PrintStatus( "No Group", lnum );
				stat = -1;
				break;
			}
			if ( (p_group->flags&GF_IO) || p_group->ov_req != 0 ) {
				PrintStatus( "group is currently updating", lnum );
				stat = -1;
				break;
			}
			if ( !(p_group->flags&GF_HaveData) ) GetRetain( p_group, lnum );
			if (	ReadGroup( p_group, GR_Current ) < 0 ) {
				stat = -1;
				break;
			}
			p_group->flags |= GF_Subscribed;
			if ( key != 'e' ) {
				UpdateOverview( p_group, 0 );
				stat = ArticleHandler( p_group, 0, 0 );
			} else stat = ArticleHandler( p_group, 1, 0 );
			SaveGroup( p_group, GR_Current );
			if ( !stat ) stat = PageGroup( groupnum, NULL );
			break;
		case 'p':
			n = lnum;
			for ( i = groupnum-1; i >= 0; i-- ) {
				n--;
				if ( p_groups[i]->flags&GF_Subscribed ) break;
			}
			if ( i < 0 ) stat = -1;
			else if ( i >= min ) PrintGroup( i, n, 3 );
			else stat = PageGroup( i, NULL );
			break;
		case 'n':
			n = lnum;
			for ( i = groupnum+1; i < num_groups; i++ ) {
			 n++;
				if ( p_groups[i]->flags&GF_Subscribed ) break;
			}
			if ( i >= num_groups ) stat = -1;
			else if ( i < max ) PrintGroup( i, n, 3 );
			else stat = PageGroup( i, NULL );
			break;
		case 's':
			if (!p_group) {
				PrintStatus( "No Group", lnum );
				stat = -1;
				break;
			}
			pthread_mutex_lock( &group_sem );
			if ( !(p_group->flags&GF_IO) && p_group->ov_req == 0 ) {
				p_group->flags |= GF_Subscribed;
			} else stat = -1;
			pthread_mutex_unlock( &group_sem );
			if ( stat == -1 ) PrintStatus( "group is currently updating", lnum );
			else PrintGroup( groupnum, lnum, 3 );
			break;
		case 'r':
			if (!p_group) {
				PrintStatus( "No Group", lnum );
				stat = -1;
				break;
			}
			stat = UpdateOverview( p_group, OV_Bg );
			if ( stat < 0 )
				stat = PrintGroup( groupnum, lnum, 3 );
			else stat = PrintGroup( groupnum+1, lnum+1, 3 );
			break;
		case 'Z':
			if (!p_group) {
				PrintStatus( "No Group", lnum );
				stat = -1;
				break;
			}
			pthread_mutex_lock( &group_sem );
			if ( !(p_group->flags&GF_IO) && p_group->ov_req == 0 ) {
				P_SRVRDATA p_srvrdata = p_group->srvrdata;
				for ( i = 0; i < numservers; i++ )
					p_srvrdata[i].highest = p_srvrdata[i].first = p_srvrdata[i].last;
			} else stat = -1;
			pthread_mutex_unlock( &group_sem );
			if ( stat == -1 ) PrintStatus( "group is currently updating", lnum );
				else PrintGroup( groupnum, lnum, 2 );
			break;
		case 'b':
			if ( PrintDownloads( lnum ) == 1 )
			stat = PageGroup( groupnum, NULL );
			break;
		case 'C':
			i = CancelDownloads( );
			if ( i == -1 )
				PrintStatus( "cancel aborted", lnum );
			if ( i == 0 )
				PrintStatus( "no background downloads", lnum );
			else {
				PrintStatus( "downloads canceled", lnum );
			stat = PageGroup( groupnum, NULL );
			}
			break;
		case 'x':
			PrintTransferStats( );
			stat = PageGroup( groupnum, NULL );
			break;
		case 'z':
			mvprintw( lastline-1, 0, "enter filename" );
			cp = GetBuff( NULL, lastline, 0 );
			move( lnum, 0 );
			if ( !*cp ) stat = -1;
			else {
				stat = ProcessNZBFile( cp );
				stat = PageGroup( groupnum, NULL );
			}
			break;
		default: stat = -1;
		}
	}
	DisableWin( );
	SaveGroups( );
	FreeQueues( );
	FreeServers( );
	FreeGroups();
	FreeArticles();
	return 0;
}
