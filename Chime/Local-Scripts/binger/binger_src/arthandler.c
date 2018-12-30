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
#include <unistd.h>
#include "artdata.p"
#include "server.h"
#include <curses.h>

enum {
	SA_Clear = 1, SA_Chain = 2, SA_Print = 4, SA_PC = 7
};

extern int topline, bottomline, num_scroll, lastline, COLS;
static int artnum = 0, num_arts = 0;
static int last_cmd = 0, lnum = 1;
static P_ART_DIR p_dir = NULL;
static P_GROUP p_group = NULL;
static P_ARTICLE *p_articles = NULL;
static P_ARTICLE p_article = NULL;
static char subjbuf[512];
static char *p_end = NULL;
static char *stypes[] = { "left", "middle", "right" };
static char *mtypes[] = { "subject", "author", "date" };

static char *
SubjLine( char *in ) {
	int len;
	char *sp = in, *sp1 = subjbuf, *last_space = NULL;
	*sp1 = '\0';
	while ( *sp && *sp != '\t' && sp1 < p_end ) {
		if ( *sp == ' ' ) last_space = sp;
		*sp1++ = *sp++;
	}
	*sp1 = '\0';
	if ( sp1 == p_end && last_space != NULL ) {
		len = last_space-in;
		if ( len > COLS-6 ) {
			subjbuf[len] = '\0';
			sp = last_space+1;
		}
	}
	return sp;
}

static char *
MakeSubj( P_ARTICLE p_article, int subjtype ) {
	static char *indicators[] = { "   ", "@  ", "%% ", "%@ ", "-  ", "%  " };
	char *sp1 = subjbuf, *sp = (char *)p_article->subject;
	P_ARTICLE p_next;
	short len, cols = COLS-1, flags = 0;
	len = p_article->subjlen;
	if ( p_end == NULL ) p_end = subjbuf+COLS;
	if ( subjtype >= 4 ) goto longsubj;
	if ( p_article->p_next ) {
		for ( p_next = p_article; p_next; p_next = p_next->p_next ) {
			if (	!(p_next->flags&AF_Complete) ) flags |= 1;
			else flags |= 2;
		}
	} else if ( p_article->parts > 1 ) {
		flags = ( (p_article->flags&AF_Complete) ) ? 5 : 4;
	}
	strcpy( sp1, indicators[flags] );
	sp1 += 3;
	cols -= 4;
	if ( subjtype > 0 && len > cols ) {
		if ( subjtype == 1 ) len = (len-cols)/2;
		else len -= cols;
		sp += len;
	}
	while ( *sp && *sp != '\t' && sp1 < p_end ) *sp1++ = *sp++;
	*sp1 = '\0';
	return subjbuf;
longsubj:
	if ( subjtype == 5 ) {
		mvprintw( lastline-1, 0, "%s", p_article->author );
		ctime_r( &p_article->artdate, subjbuf );
		mvprintw( lastline, 0, "%s", subjbuf );
		move( lastline-1, 0 );
	} else if ( len <= cols ) {
		beep( );
	} else {
		sp = SubjLine( sp );
		mvprintw( lastline-2, 0, "%s", subjbuf );
		sp = SubjLine( sp );
		mvprintw( lastline-1, 0, "%s", subjbuf );
		sp = SubjLine( sp );
		if ( *subjbuf ) mvprintw( lastline, 0, "%s", subjbuf );
		move( lastline-2, 0 );
	}
	return subjbuf;
}

static void
DirRef( int inc ) {
	pthread_mutex_lock( &article_sem );
	if ( p_dir != NULL ) {
		p_dir->nref += inc;
		if ( p_dir->nref <= 0 ) {
			free( p_dir->dir );
			free( p_dir );
			p_dir = NULL;
		}
	}
	pthread_mutex_unlock( &article_sem );
}

static void
StatArticle( P_ARTICLE p_article, int how ) {
	int i, diff, server;
	static int st_count, st_complete, st_parts, st_missing, st_loaded;
	static long long st_artbytes, st_servebytes[MAXSERVERS];
	long long artbytes;
	char k_m;
	if ( how&SA_Clear ) {
		st_artbytes = 0l;
		bzero( st_servebytes, sizeof( st_servebytes ) );
		st_count = st_missing = 0;
		st_parts = st_complete = 0;
		st_loaded = 0;
	}
	while ( p_article )	{
		st_count++;
		if ( p_article->flags&AF_Complete ) st_complete++;
		if ( p_article->flags&AF_Loaded ) st_loaded++;
		else {
			st_parts += p_article->parts;
			diff = p_article->parts-p_article->got_parts;
			st_missing += diff;
			for ( i = 0; i < numservers; i++ ) {
				if ( 0 == p_article->bytes[i] ) continue;
				st_artbytes += (long long)p_article->bytes[i];
				st_servebytes[i] += (long long)p_article->bytes[i];
				server = i;
			}
		}
		if ( !(how&SA_Chain) ) break;
		p_article = p_article->p_next;
	}
	if ( !(how&SA_Print) ) return;
	if ( st_parts == 1 ) {
		i = (int)(st_artbytes/500l);
		if ( i&1 ) i >>= 1;
		else i = (i>>1)+1;
		mvprintw( bottomline+1, 0, "%d k-byte%c from %s",
			i, (i != 1) ? 's' : ' ',GetDispName( server ) );
		move( lnum, 0 );
		return;
	}
	mvprintw( bottomline+1, 0, "%d article%c, %d complete, ",
		st_count, (st_count != 1 ) ? 's' : ' ', st_complete );
	if ( st_loaded > 0 ) printw( "%d loaded ", st_loaded );
	printw( "%d parts, %d missing, ", st_parts, st_missing);
	artbytes = st_artbytes/500l;
	if ( artbytes > 20000 ) {
		artbytes /= 1000l;
		k_m = 'm';
	} else k_m = 'k';
	if ( (artbytes&1) == 0 ) artbytes >>= 1;
	else artbytes = ( artbytes >> 1 )+1;
	printw( "%d %c-bytes", (int)artbytes, k_m );
	if ( numservers < 2 ) {
		move( lnum, 0 );
		return;
	}
	move( bottomline+2, 0 );
	for ( i = 0; i < numservers; i++ ) {
		if ( ( artbytes = st_servebytes[i] ) == 0 ) continue;
		artbytes /= 500l;
		if ( artbytes > 20000 ) {
			artbytes /= 1000l;
			k_m = 'm';
		} else k_m = 'k';
		if ( (artbytes&1) == 0 ) artbytes >>= 1;
		else artbytes = ( artbytes >> 1 )+1;
		printw( "%d %c-bytes from %s ",
			(int)artbytes+1, k_m, GetDispName( i ) );
	}
	move( lnum, 0 );
}

static int
PrintArticle( int art, int ln, int ctl ) {
	if ( art < 0 || art >= num_arts ) {
			if ( num_arts == 0 ) mvprintw( 0, 64, "No Articles	 " );
		refresh( );
		return -1;
	}
	p_article = p_articles[art];
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
	if ( ctl&1 )
		mvprintw( 0, 64, "%5d of %5d", art+1, num_arts );
	if ( ctl&4 ) {
		move( ln, 0 );
		clrtoeol( );
	}
	if ( ctl&2 ) {
		mvprintw( ln, 0, "%s", MakeSubj( p_article, p_group->subjtype ) );
	}
	artnum = art;
	lnum = ln;
	move( lnum, 0 );
	return lnum;
}

static int
PageArticles( int art, char *pInfo ) {
 int line;
	char *what = "	 group";
	clear( );
	if ( (p_group->flags&GF_Temp) ) what = "subgroup";
	else if ( (p_group->flags&GF_NZB) ) what = "n z b";
	mvprintw( 0, 0, "%s %s", what, p_group->name );
	if ( (num_arts = p_group->num_arts) == 0 ) {
		mvprintw( 0, 64, "No Articles	 " );
		refresh( );
		return -1;
	}
	refresh( );
	if ( art < 0 ) {
		art = 0;
		lnum = topline;
	} else if ( art >= num_arts ) {
		art = num_arts-1;
		lnum = bottomline-1;
	}
	artnum = art;
	art -= (lnum-topline);
	if ( art <= 0 ) art = 0;
	mvprintw( 0, 64, "%5d of %5d", artnum+1, num_arts );
	lnum = topline;
	for ( line = topline; line < bottomline; line++ ) {
		if ( artnum == art ) lnum = line;
		p_article = p_articles[art++];
		mvprintw( line, 0, "%s", MakeSubj( p_article, p_group->subjtype ) );
		if ( art >= num_arts ) break;
	}
	p_article = p_articles[artnum];
	if ( pInfo ) mvprintw( lastline, 0, pInfo );
	move( lnum, 0 );
	return lnum;
}

static int
PrintDeletes( int start, int count ) {
	int i;
	char buf[32];
	pthread_mutex_lock( &article_sem );
	num_arts = RemoveDeletes( p_group, start );
	if ( num_arts+8200 <= p_group->max_arts ) {
		p_group->max_arts -= 8192;
		i = sizeof( P_ARTICLE )*(p_group->max_arts+1);
		p_group->p_articles = realloc( p_articles, i );
		p_articles = p_group->p_articles;
	}
	pthread_mutex_unlock( &article_sem );
	if ( count == 0 ) return 0;
	sprintf( buf, "%d articles killed", count );
	return PageArticles( artnum, buf );
}

void
CleanGroup( ) {
	int i;
	P_ARTICLE p_this;
	pthread_mutex_lock( &article_sem );
	p_articles = p_group->p_articles;
	for ( i = 0; i < num_arts; i++ ) {
		for ( p_this = p_articles[i]; p_this; (p_this = p_this->p_next ) ) {
			if ( p_this->artdate < p_group->base_time ) p_this->flags |= AF_Kill;
		}
	}
	pthread_mutex_unlock( &article_sem );
	PrintDeletes( 0, 0 );
}

static int
ParseCmd( char *in ) {
	int cmds = 0, i;
	char *cp1, *cp2;
	static char cmd_chars[] = "+BIadfk:u<";
	static int cmd_flags[] = {
		AF_Add, AF_B_Get, AF_B_Get|AF_Incomplete,
		AF_AuthSearch, AF_Get, AF_Filter, AF_Kill, 0, AF_Undel, AF_Unload
	};
	last_cmd = 0;
	for ( cp1 = in; *cp1; cp1++ ) {
		cp2 = strchr( cmd_chars, *cp1 );
		if ( !cp2 ) break;
		i = cp2-cmd_chars;
		cmds |= cmd_flags[i];
	}
	if ( *cp1 == '>' ) {
		DirRef( -1 );
			p_dir = NULL;
		if ( strlen( cp1 ) > 2 ) {
			char dirname[200];
			if ( MakeDir( ++cp1, dirname ) == NULL ) return -1;
			p_dir = calloc( 1, sizeof( ART_DIR ) );
			p_dir->dir = strdup( dirname );
			p_dir->len = strlen( dirname );
			p_dir->nref = 1;
		}
	} else if ( *cp1 ) return PrintStatus( "command error", lnum );
		if ( (cmds&AF_Gets) && (cmds&AF_Cancel) )
		return PrintStatus( "conflicting download/cancel options", lnum );
		if ( (cmds&AF_Undel) && (cmds&AF_Mask) != 0 )
		return PrintStatus( "undelete can only be with a", lnum );
	last_cmd = cmds;
	return cmds;
}

static int
UndelSingle( void ) {
	int i, count = 0;
	P_ARTICLE p_this = p_group->p_dels, p_next, p_prev = NULL;
	short mask = AF_Kill|AF_Loaded;
	char buf[64];
	while ( p_this ) {
		p_next = p_this->p_next;
		if ( p_this->got_parts == 1 ) {
			count++;
			pthread_mutex_lock( &article_sem );
			p_this->flags &= ~mask;
			if ( p_group->num_arts >= p_group->max_arts ) {
				p_group->max_arts += 4096;
				i = sizeof( P_ARTICLE )*(p_group->max_arts+1);
				p_group->p_articles = realloc( p_group->p_articles, i );
			}
			p_group->p_articles[p_group->num_arts++] = p_this;
			p_this->p_next = NULL;
			if ( p_prev ) p_prev->p_next = p_next;
			else p_group->p_dels = p_next;
			pthread_mutex_unlock( &article_sem );
		} else p_prev = p_this;
		p_this = p_next;
	}
	if ( count == 0 )
		return PrintStatus( "no articles matched", lnum );
	pthread_mutex_lock( &article_sem );
	p_group->p_lastdel = p_prev;
	pthread_mutex_unlock( &article_sem );
	pthread_mutex_lock( &group_sem );
	p_group->p_articles[p_group->num_arts] = NULL;
	 ThreadArticles( p_group );
	pthread_mutex_unlock( &group_sem );
	p_articles = p_group->p_articles;
	sprintf( buf, "%d articles undeleted", count );
	return PageArticles( artnum, buf );
}

static int
Undelete( int cmd ) {
	int i, count = 0;
	int authsrch = (cmd&AF_AuthSearch) ? 1 : 0;
	P_ARTICLE p_this = p_group->p_dels, p_next, p_prev = NULL;
	short mask = ((cmd&AF_Unload)) ? AF_Kill|AF_Loaded : AF_Kill;
	char buf[64];
	puchar cp;
	if ( PatternInfo( 0 ) == 0 )
		return PrintStatus( "no pattern", lnum );
	while ( p_this ) {
		p_next = p_this->p_next;
		cp = (authsrch) ? p_this->author : p_this->subject;
		if ( MatchPattern( (char *)cp, NULL ) ) {
			count++;
			pthread_mutex_lock( &article_sem );
			p_this->flags &= ~mask;
			if ( p_group->num_arts >= p_group->max_arts ) {
				p_group->max_arts += 4096;
				i = sizeof( P_ARTICLE )*(p_group->max_arts+1);
				p_group->p_articles = realloc( p_group->p_articles, i );
			}
			p_group->p_articles[p_group->num_arts++] = p_this;
			p_this->p_next = NULL;
			if ( p_prev ) p_prev->p_next = p_next;
			else p_group->p_dels = p_next;
			pthread_mutex_unlock( &article_sem );
		} else p_prev = p_this;
		p_this = p_next;
	}
	if ( count == 0 )
		return PrintStatus( "no articles matched", lnum );
	pthread_mutex_lock( &article_sem );
	p_group->p_lastdel = p_prev;
	pthread_mutex_unlock( &article_sem );
	pthread_mutex_lock( &group_sem );
	p_group->p_articles[p_group->num_arts] = NULL;
	ThreadArticles( p_group );
	pthread_mutex_unlock( &group_sem );
	p_articles = p_group->p_articles;
	sprintf( buf, "%d articles undeleted", count );
	return PageArticles( artnum, buf );
}

static int
StatMatched( ) {
	int i, count = 0;
	P_ARTICLE p_next;
	StatArticle( NULL, SA_Clear );
	for ( i = 0; i < num_arts; i++ ) {
		p_next = p_articles[i];
		if ( !(p_next->flags&AF_BaseMatch) ) continue;
		for ( ; p_next; p_next = p_next->p_next ) {
			if ( !(p_next->flags&AF_Matched) ) continue;
			StatArticle( p_next, 0 );
			count++;
		}
	}
	if ( count == 0 ) return PrintStatus( "no articles", lnum );
	StatArticle( NULL, SA_Print );
	return 0;
}

static int
ExecArtCommand( int cmd) {
	int i, count = 0, first = -1;
	int cmd1 = 0, status = 0;
	short flags;
	P_ARTICLE p_next;
	time_t incomp_time = time( NULL )-(30*3600);
	char buf[64], *cp;
	cmd &= (AF_Mask|AF_Unload);
	if ( cmd == 0 ) return 0;
	if ( (cmd&AF_Gets) ) {
		if ( (cmd&AF_Get) )
			cmd1 = AS_Stat|AS_Err|AS_Foreground;
		if ( (cmd&AF_Incomplete) )
			cmd1 = AS_GetPartial;
		if ( (cmd&AF_Kill) ) {
			cmd1 |= AS_Kill;
			cmd &= ~AF_Kill;
		}
	}
	for ( i = 0; status == 0 && i < num_arts; i++ ) {
		p_next = p_articles[i];
		if ( !(p_next->flags&AF_BaseMatch) ) continue;
		if ( first == -1 ) first = i;
		for ( ; p_next; p_next = p_next->p_next ) {
			flags = p_next->flags;
			if ( !(flags&AF_Matched) ) continue;
			if ( (cmd&AF_Gets) ) {
				if ( (flags&AF_Loads) ) continue;
				if ( !(flags&AF_Complete) ) {
					if (	!(cmd&AF_Incomplete) || p_next->artdate > incomp_time )
						continue;
				}
				pthread_mutex_lock( &article_sem );
				p_next->flags |= cmd;
				if ( !p_article->p_parts && p_article->parts_pos > 0 )
					ReadParts( p_next );
				pthread_mutex_unlock( &article_sem );
				if ( (cmd&AF_Get) )
					status = SaveArticle( p_next, 0, cmd1, 0, p_dir );
				else if ( QueueArt( p_next, p_dir, cmd1) < 0 ) status = E_Full;
				if ( status != 0 ) break;
			} else if ( (cmd&AF_Unload) ) {
				pthread_mutex_lock( &article_sem );
				p_next->flags &= ~AF_Loaded;
				pthread_mutex_unlock( &article_sem );
			} else {
				pthread_mutex_lock( &article_sem );
				p_next->flags |= cmd;
				pthread_mutex_unlock( &article_sem );
			}
			count++;
		}
	}
	if ( count == 0 ) return PrintStatus( "no articles", lnum );
	if ( cmd == AF_Kill )
		return PrintDeletes( first, count );
	if ( status == E_Full )
		strcpy( buf, "queue is full!" );
	else if ( (cmd&AF_Gets) ) {
		cp = ( (cmd&AF_Get) ) ? "downloaded" : "queued";
		sprintf( buf, "%d articles %s", count, cp );
		if ( p_group->delete_ctr > 0 ) {
			pthread_mutex_lock( &article_sem );
			num_arts = RemoveDeletes( p_group, 0 );
			pthread_mutex_unlock( &article_sem );
		}
	} else if ( (cmd&AF_Unload) )
		sprintf( buf, "%d articles marked unloaded", count );
	else
		sprintf( buf, "%d articles matched", count );
	PrintStatus( buf, lnum );
	return status;
}

static int
DoArtCommand( int key ) {
	int cmd;
	char *cp;
	cp = GetBuff( NULL, lastline, key );
	move( lnum, 0 );
	if ( cp == NULL ) return -1;
	cmd = ParseCmd( cp );
	if ( cmd <= 0 ) return cmd;
	if ( (cmd&AF_Undel) ) return Undelete( cmd );
	cmd &= AF_Mask;
	return ExecArtCommand( cmd );
}

void
PrintCount( int count, char *what ) {
	char buf[64];
	sprintf( buf, "%d %s", count, what );
	PrintStatus( buf, lnum );
}

static int
DeleteAuthor( uchar *author ) {
	int i, count = 0, first = -1;
	P_ARTICLE p_next;
	for ( i = 0; i < num_arts; i++ ) {
		p_next = p_articles[i];
		if ( p_next->author != author ) continue;
		if ( first == -1 ) first = i;
		pthread_mutex_lock( &article_sem );
		for ( ; p_next; p_next = p_next->p_next ) {
			p_next->flags |= AF_Kill;
			count++;
		}
		pthread_mutex_unlock( &article_sem );
	}
	if ( count == 0 )
		return PrintStatus( "no articles matched", lnum );
	return PrintDeletes( first, count );
}

static int
MatchArticles( ) {
	int i, count = 0, first = -1, cmd;
	int	matched = 0, addto = 0, authsrch = 0, cmd1 = AF_Matched;
	char buf[64], *cp;
	puchar pp;
	P_ARTICLE p_next;
	cp = GetPattern( NULL, NULL );
	move( lnum, 0 );
	if ( cp == NULL ) return 0;
	cmd = ParseCmd( cp );
	if ( cmd < 0 ) return cmd;
	if ( (cmd&AF_Filter) && !(p_group->flags&GF_Temp) )
		AddFilter( p_group, LastPattern( ), (cmd&AF_AuthSearch) ? 1 : 0 );
	if ( (cmd&AF_Undel) ) return Undelete( cmd );
			if ( cmd&AF_AuthSearch) authsrch = 1;
			if ( cmd&AF_Add) addto = 1;
	cmd &= AF_Mask;
	if ( cmd == AF_Kill ) cmd1 = cmd;
	for ( i = 0; i < num_arts; i++ ) {
		p_article = p_articles[i];
		matched = 0;
		for ( p_next = p_article; p_next; p_next = p_next->p_next ) {
			pp = (authsrch) ? p_next->author : p_next->subject;
			if ( MatchPattern( (char *)pp, NULL ) ) {
				pthread_mutex_lock( &article_sem );
				p_next->flags |= cmd1;
				pthread_mutex_unlock( &article_sem );
				matched++;
			} else if ( !addto ) {
				pthread_mutex_lock( &article_sem );
				p_next->flags &= ~(AF_Matched|AF_BaseMatch);
				pthread_mutex_unlock( &article_sem );
			}
		}
		if ( matched ) {
			p_article->flags	|= AF_BaseMatch;
			count += matched;
			if ( first == -1 ) first = i;
		}
	}
	if ( count == 0 )
		return PrintStatus( "no articles matched", lnum );
	if ( cmd1 == AF_Kill )
		return PrintDeletes( first, count );
	artnum = first;
	sprintf( buf, "%d articles matched", count );
	PageArticles( artnum, buf );
	i = ExecArtCommand( cmd );
	if ( i == E_Full )
		PrintStatus( "queue is full!", lnum );
	return i;
}

static int
DelSingleArts( ) {
	int i, count = 0, first = -1;
	P_ARTICLE p_next;
	for ( i = 0; i < num_arts; i++ ) {
		p_next = p_articles[i];
		if ( p_next->p_next != NULL || p_next->got_parts > 1 ) continue;
		pthread_mutex_lock( &article_sem );
		p_next->flags |= AF_Kill;
		pthread_mutex_unlock( &article_sem );
		count ++;
		if ( first == -1 ) first = i;
	}
	if ( count == 0 )
		return PrintStatus( "no articles matched", lnum );
	return PrintDeletes( first, count );
}

int QueueFromFile( P_GROUP pGroup );

static void
SubArtHandle( ) {
	P_GROUP hold_group = p_group;
	int count = 0, artnumhold;
	int first = -1, matches = 0;
	P_ARTICLE p_base, p_this, p_next;
	pthread_mutex_lock( &article_sem );
			 hold_group->flags |= GF_Temp;
	p_group = AllocGroup( hold_group->name );
	p_group->subjtype = hold_group->subjtype;
	p_group->p_filter = hold_group->p_filter;
			 p_group->flags = GF_Temp;
	artnumhold = artnum;
	p_base = p_articles[artnum];
	p_this = p_base;
	for ( p_next = p_base; p_next; p_next = p_next->p_next ) count++;
	p_group->num_arts = count;
	p_group->max_arts = count;
	p_group->p_articles = calloc( count+1, sizeof( P_ARTICLE ) );
	count = 0;
p_this->flags &= ~AF_BaseMatch;
	for ( ; p_this; p_this = p_next ) {
		if ( (p_this->flags&AF_Matched) && first < 0 ) first = count;
		p_group->p_articles[count++] = p_this;
		p_next = p_this->p_next;
		p_this->p_next = NULL;
	}
	pthread_mutex_unlock( &article_sem );
	if ( first < 0 ) first = 0;
	ArticleHandler( p_group, 1, first );
	pthread_mutex_lock( &article_sem );
	if ( ( p_this = p_group->p_dels ) != NULL ) {
		p_group->p_lastdel->p_next = hold_group->p_dels;
		hold_group->p_dels = p_group->p_dels;
		if ( !hold_group->p_lastdel )
			hold_group->p_lastdel = p_group->p_lastdel;
		p_group->p_dels = NULL;
	}
	p_articles = hold_group->p_articles;
	artnum = artnumhold;
	if ( num_arts > 0 ) {
		p_this = p_group->p_articles[0];
		p_group->p_articles[0] = NULL;
		p_articles[artnum] = p_this;
		if ( (p_this->flags&AF_Matched) ) matches = 1;
		for ( count = 1; count < num_arts; count++ ) {
			p_next = p_group->p_articles[count];
			p_group->p_articles[count] = NULL;
			p_this->p_next = p_next;
			p_this = p_next;
			if ( (p_this->flags&AF_Matched) )
				matches++;
		}
		p_this->p_next = NULL;
		if ( num_arts > 1 ) {
			p_this = p_articles[artnum];
			if ( matches > 0 ) p_this->flags |= AF_BaseMatch;
		}
	} else {
		hold_group->num_arts--;
		for ( count = artnum; count < hold_group->num_arts; count++ )
			p_articles[count] = p_articles[count+1];
		p_articles[count] = NULL;
		if ( artnum > hold_group->num_arts ) artnum--;
	}
	FreeGroup( p_group );
	p_group = hold_group;
	p_group->flags &= ~GF_Temp;
	num_arts = p_group->num_arts;
	pthread_mutex_unlock( &article_sem );
}

int
UpdateOverview( P_GROUP p_group, int ctl ) {
	P_SRVRDATA p_data;
	int i, j, arts;
	int status[MAXSERVERS], done[MAXSERVERS], flags;
	if ( (ctl&OV_Bg) && !(p_group->flags&GF_HaveData) ) {
		if ( (ctl&OV_Batch) ) return -1;
		GetRetain( p_group, lnum );
	}
	pthread_mutex_lock( &group_sem );
	flags = p_group->flags;
	if ( (flags&GF_IO) || p_group->ov_req != 0 ) {
gr_upd:
		pthread_mutex_unlock( &group_sem );
		return PrintStatus( "group is already updating", lnum );
	}
	if ( !(flags&GF_Read) ) {
		if ( ReadGroup( p_group, GR_HaveSem ) < 0 ) goto gr_upd;
	}
	flags &= GF_BackDated;
	p_group->flags &= ~GF_BackDated;
	for ( i = 0, j = 0; i < numservers; i++ )
		j |= 1<<i;
	p_group->ov_req = j;
	pthread_mutex_unlock( &group_sem );
	if ( flags && (p_group->num_arts > 0 || p_group->p_dels) )
		ctl |= OV_Goback;
	if ( (ctl&OV_Bg) ) {
		for ( i = 0; i < numservers; i++ )
			QueueWork	( i, TASK_BG_Overview, p_group, NULL, PRI_BGOV, ctl, NULL );
		if ( !(ctl&OV_Batch) )
			PrintStatus( "background overview started", lnum );
		return 0;
	}
	for ( i = 0; i < numservers; i++ ) {
		status[i] = 0;
		done[i] = 0;
	QueueWork	( i, TASK_Overview, p_group, NULL, PRI_FG, ctl, &status[i] );
	}
	clear();
	mvprintw( 0, 0, "group %s", p_group->name );
	mvprintw( 1, 0, "getting overview information" );
	refresh( );
	j = 0;
	while ( 1 ) {
		usleep( 200000 );
		pthread_mutex_lock( &group_sem );
		flags = p_group->ov_req;
		pthread_mutex_unlock( &group_sem );
		if ( !flags ) break;
		if ( ++j < 25 ) continue;
		for ( i = 0; i < numservers; i++ ) {
			if ( done[i] ) continue;
		j = 1<<i;
		if ( !(flags&j) ) done[i] = 1;
		p_data = &p_group->srvrdata[i];
			if ( (ctl&OV_Goback) ) {
				if ( 0 == p_data->start_adjust ) continue;
				arts =	p_data->lowest - p_data->start_adjust;
			} else arts = p_data->last - p_data->highest;
		 move( i+3, 0 );
			clrtoeol( );
			mvprintw( i+3, 0, "%s %6d", GetDispName( i ), arts );
		}
		refresh( );
		j = 0;
	}
	pthread_mutex_lock( &group_sem );
	flags = p_group->flags;
	if ( flags&AF_Cancel ) i = EO_Cancel;
	else i = 0;
	pthread_mutex_unlock( &group_sem );
	p_articles = p_group->p_articles;
	num_arts = p_group->num_arts;
	if ( i == 0 )
		for ( j = 0; j < numservers; j++ ) if ( status[j] != 0 ) i = status[j];
	return i;
}

int
ArticleHandler( P_GROUP p_ingroup, int ctl, int start ) {
	int done = 0, stat = 0, key, i, n, min, max;
	P_ARTICLE p_next;
	p_group = p_ingroup;
	if ( 0 == ctl ) PatternInfo( 1 );
	clear();
	refresh();
		if ( p_group->delete_ctr > 0 ) {
			pthread_mutex_lock( &article_sem );
			RemoveDeletes( p_group, 0 );
			pthread_mutex_unlock( &article_sem );
		}
	num_arts = p_group->num_arts;
	p_articles = p_group->p_articles;
	PageArticles( start, NULL );
	while ( !done ) {
		if ( num_arts > 0 )
			p_next = p_articles[artnum];
		else {
			if ( ctl != 1 ) break;
			p_next = NULL;
		}
		min = artnum - (lnum-topline);
		max = artnum + ( bottomline-lnum );
		if ( max > num_arts ) max = num_arts;
		if ( stat == -1 ) beep( );
		else refresh( );
		stat = 0;
		key = GetKey( 0 );
		if ( p_next == NULL && key != KEY_LEFT ) {
			if ( key >= 127 || strchr( "/>:CMPRbrzq", (char)key ) == 0 ) {
				stat = -1;
				continue;
			}
		}
		switch (key) {
		case EOF:
		case KEY_LEFT:
		case 'q':
			done = 1;
			break;
		case KEY_F(1):
		case 'h':
			stat = ShowHelp( "binger_arts" );
			if ( stat == 0 )
				PageArticles( artnum, NULL );
			break;
		case '/':
			stat = MatchArticles( p_group );
			break;
		case KEY_UP:
			stat = PrintArticle( artnum-1, lnum-1, 1 );
			break;
		case KEY_PPAGE:
			if ( artnum <= 0 ) stat = -1;
			else PageArticles( artnum-num_scroll, NULL );
			break;
		case KEY_HOME:
			if ( artnum < num_scroll )
				PrintArticle( 0, topline, 1 );
			else PageArticles( 0, NULL );
			break;
		case KEY_DOWN:
			stat = PrintArticle( artnum+1, lnum+1, 1 );
			break;
		case 'P':
			stat = PostArt( p_group->name, NULL );
			PageArticles( i, NULL );
			break;
		case 'p':
			n = lnum;
			for ( i = artnum-1; i >= 0; i-- ) {
				n--;
				if ( p_articles[i]->flags&(AF_BaseMatch|AF_Matched) ) break;
			}
			if ( i < 0 ) stat = -1;
			else if ( i >= min ) PrintArticle( i, n, 1 );
			else PageArticles( i, NULL );
			break;
		case 'n':
			n = lnum;
			for ( i = artnum+1; i < num_arts; i++ ) {
				n++;
				if ( p_articles[i]->flags&(AF_BaseMatch|AF_Matched) ) break;
			}
			if ( i == num_arts ) stat = -1;
			else if ( i < max ) PrintArticle( i, n, 1 );
			else PageArticles( i, NULL );
			break;
		case 'R':
			if ( (p_group->flags&GF_Temp) ) {
				stat = -1;
				break;
			}
			stat = GetRetain( p_group, lnum );
			if ( stat <= 0 ) break;
			if ( stat == 2 ) UpdateOverview( p_group, OV_Goback );
			else CleanGroup( );
			num_arts = p_group->num_arts;
			p_articles = p_group->p_articles;
			PageArticles( 0, NULL );
			break;
		case 'r':
			if ( (p_group->flags&GF_Temp) ) {
				stat = -1;
				break;
			}
			UpdateOverview( p_group, 0 );
			PageArticles( artnum, NULL );
			break;
		case 'z':
			if ( !IsNZB( p_article ) ) stat = -1;
			else {
				P_GROUP p_hold = p_group;
				int art_hold = artnum;
				stat = NZBRead( p_article );
				p_group = p_hold;
				num_arts = p_group->num_arts;
				p_articles = p_group->p_articles;
				artnum = art_hold;
			}
			if ( !stat )
				PageArticles( artnum, NULL );
			break;
		case 'Z':
			min = 0; max = num_arts;
		case 'K':
			pthread_mutex_lock( &article_sem );
			for ( i = min; i < max; i++ ) {
				p_article = p_articles[i];
				for ( p_next = p_article; p_next; p_next = p_next->p_next )
					p_next->flags |= AF_Kill;
			}
			num_arts = RemoveDeletes( p_group, min );
			pthread_mutex_unlock( &article_sem );
			PageArticles( artnum, NULL );
			break;
		case 10:
			i = ReadArticle( p_next );
			if ( i < 1 ) {
				if ( i == 0 ) PageArticles( artnum, NULL );
				break;
			}
		case 'k':
			pthread_mutex_lock( &article_sem );
			for ( ; p_next; p_next = p_next->p_next )
				p_next->flags |= AF_Kill;
			num_arts = RemoveDeletes( p_group, artnum );
			pthread_mutex_unlock( &article_sem );
/*
		deleteln( );
			move( bottomline, 0 );
			insertln( );
*/
			PageArticles( artnum,	NULL );
			break;
		case 32:
		case KEY_NPAGE:
			if ( max >= num_arts ) stat = -1;
			else {
				artnum += num_scroll;
				PageArticles( artnum, NULL );
			}
			break;
		case KEY_END:
			n = num_scroll;
			if ( n > num_arts ) n = num_arts;
			if ( artnum >= num_arts-n )
				PrintArticle( num_arts-1, n, 1 );
			else PageArticles( num_arts, NULL );
			break;
		case KEY_RIGHT:
			if ( !(p_next->p_next) )stat = -1;
			else {
				SubArtHandle( );
				PageArticles( artnum, NULL );
			}
			break;
		case 's':
			MakeSubj( p_next, 4 );
			break;
		case ':':
		case '>':
			DoArtCommand( key );
			break;
		case '?':
		case KEY_F(2):
			StatArticle( p_next, SA_PC );
			break;
		case '%':
		case KEY_F(3):
			StatMatched( );
			break;
		case 'D':
		case 'd':
		case 'i':
			stat = -1;
			i = AS_Stat|AS_Err|AS_Foreground;
			if ( key == 'D' ) i |= AS_Kill;
			else if ( key == 'i' ) i |= AS_GetPartial;
			for ( n = 0; p_next; p_next = p_next->p_next ) {
				if ( (p_next->flags&AF_Loads) ) continue;
				if ( !(i&AS_GetPartial) && !(p_next->flags&AF_Complete) ) continue;
				stat = SaveArticle( p_next, 0, i, 0, p_dir );
				if ( stat != 0 ) break;
				n++;
			}
			PrintDeletes( artnum, n );
			break;
		case 'B':
		case 'I':
			for ( i = 0; p_next; p_next = p_next->p_next ) {
				if ( (p_next->flags&AF_Loads) ) continue;
				if ( 'I' != key && !(p_next->flags&AF_Complete)	) continue;
				if ( QueueArt( p_next, p_dir, AS_Kill|AS_GetPartial ) < 0 )
					break;
				i++;
			}
			PrintCount( i, "articles queued" );
			break;
		case 'u':
	stat = Undelete( last_cmd );
			break;
		case '<':
			for ( i = 0; p_next; p_next = p_next->p_next ) {
				if ( !(p_next->flags&AF_Loads) ) continue;
				pthread_mutex_lock( &article_sem );
				p_next->flags &= ~(AF_Loads);
				pthread_mutex_unlock( &article_sem );
			i++;
			}
			PrintCount( i, "articles marked unloaded" );
			break;
		case 'a':
			MakeSubj( p_next, 5 );
			break;
		case 'm':
			i = p_group->mode;
			if ( ++i > 2 ) i = 0;
			p_group->mode = i;
			SortArticles( p_group, i );
			PageArticles( artnum, mtypes[i] );
			break;
		case 'S':
			i = p_group->subjtype;
			if ( ++i > 2 ) i = 0;
			p_group->subjtype = i;
			PageArticles( artnum, stypes[i] );
			break;
		case 'M':
			if ( (p_group->flags&GF_Temp) ) {
				stat = -1;
				break;
			}
			stat = FindMissing( p_group, 0 );
			PrintCount( stat, "missing articles found" );
			break;
		case '1':
			DelSingleArts( );
			break;
		case '!':
			UndelSingle( );
			break;
		case 'A':
			DeleteAuthor( p_articles[artnum]->author );
			break;
		case 'b':
			if ( PrintDownloads( lnum ) == 1 )
				PageArticles( artnum, NULL );
			break;
		case 'F':
			stat = PostArt( p_group->name, p_next );
			PageArticles( i, NULL );
			break;
		case 'f':
			stat = ApplyFilters( p_group, &i );
			if ( stat > 0 )
				PrintDeletes( i, stat );
			else PrintStatus( "no articles deleted", lnum );
			break;
		case 'C':
			i = CancelDownloads( );
			if ( i == -1 )
				PrintStatus( "cancel aborted", lnum );
			if ( i == 0 )
				PrintStatus( "no background downloads", lnum );
			else {
				PrintStatus( "downloads canceled", lnum );
				pthread_mutex_lock( &article_sem );
				num_arts = RemoveDeletes( p_group, 0 );
				pthread_mutex_unlock( &article_sem );
				PageArticles( artnum, NULL );
			}
			break;
		case '+':
			pthread_mutex_lock( &article_sem );
			if ( p_next->p_next ) p_next->flags |= AF_BaseMatch;
			for ( i = 0; p_next; p_next = p_next->p_next ) {
				if ( (p_next->flags&AF_Matched) ) continue;
				p_next->flags |= AF_Matched;
				i++;
			}
			pthread_mutex_unlock( &article_sem );
			PrintCount( i, "marked matched" );
			break;
		case '-':
			pthread_mutex_lock( &article_sem );
			p_next->flags &= ~AF_BaseMatch;
			for ( i = 0; p_next; p_next = p_next->p_next ) {
				if ( !(p_next->flags&AF_Matched) ) continue;
				p_next->flags &= ~AF_Matched;
				i++;
			}
			pthread_mutex_unlock( &article_sem );
			PrintCount( i, "unmarked matched" );
			break;
		case KEY_F(4):
			PrintInfo( p_next, lnum );
			break;
		case '@':
			if ( (p_group->flags&GF_Temp) ) {
				stat = -1;
				break;
			}
			QueueFromFile( p_group );
			break;
		default: stat = -1;
		}
	}
	return 0;
}
