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
#include "artdata.p"
#include "server.p"
#include "decoders.h"
#include "nzbparse.h"
#include <curses.h>
enum {
	R_More = 0, R_Done, R_Save, R_Kill, R_Prev, R_Next
};

extern char infodir[], help_dir[];
extern int topline, bottomline, num_scroll, lastline, COLS;

extern int numservers;
extern int bg_downloads, bg_errors;
P_ARTICLE p_download[MAXTHREAD+1];
static int encodings[MAXTHREAD+1], parts[MAXTHREAD+1], k_bytes[MAXTHREAD+1];
static char filenames[MAXTHREAD+1][256];

#define BUFFSIZE 16384

static char *art_msgs[] = { "save done",
	"can't select group", "article unavailable",
	"article header too long", "article get timed out",
	"cannot create file", "error saving article",
	"error opening file", "save canceled", "connection error"
};

#define MAXPAGES 126
static char pagebuf[1024];
static long p_pages[MAXPAGES+2];
static int first_page = 0, maxpage = 0;

int
GetPage( FILE *infile, int pagenum ) {
	long pos, lastpos, page_end;
	int len, maxln = COLS-1, lnum = 0;
	char *cp, *cp1, *cp2, *cp3, ch;
	lastpos = p_pages[pagenum++];
	page_end = p_pages[pagenum];
	if ( lastpos < 0 || page_end < 0 ) return -1;
	if ( fseek(infile, lastpos, SEEK_SET) == -1 ) return -1;
	ch = getc( infile );
	if ( ch != (char)12 ) ungetc( ch, infile );
	clear();
	refresh();
	while ( ( cp = fgets( pagebuf, 800, infile ) ) ) {
		pos = ftell( infile );
		len = pos-lastpos;
		cp2 = cp+len;
		for ( cp1 = cp; cp1 < cp2; cp1++ ) {
			if ( *cp1 >= ' ' && *cp1 < 127 ) continue;
			if ( *cp1 == '\t' || *cp1 == '\n' ) continue;
			*cp1 = ' ';
		}
		while ( len > maxln-2 ) {
			cp3 = cp+maxln;
			cp1 = cp3-9;
			for ( cp2 = cp3; cp2 > cp1; cp2-- )
				if ( *cp2 < '0' || *cp2 > 'z' ) break;
			if ( cp2 <= cp1 ) cp2 = cp3;
			ch = *cp2;
			*cp2 = '\0';
			len = cp2-cp;
			lastpos += len;
			mvprintw( lnum++, 0, "%s", cp );
			*cp2 = ch;
			cp = cp2;
			if ( lastpos >= page_end || lnum >= lastline ) goto alldone;
			len = pos-lastpos;
		}
		mvprintw( lnum++, 0, "%s", cp );
		lastpos = pos;
		if ( lastpos >= page_end || lnum >= lastline ) break;
	}
alldone:
	move( 0, 0 );
	return 0;
}

int
PageArticle( FILE *infile, int pagectl ) {
	long pos, lastpos;
	int len, ln = 0, maxln = COLS-1, page = 0;
	char *cp, *cp1, *cp2, *cp3;
	first_page = 0;
	lastpos = ftell( infile );
	p_pages[page++] = lastpos;
	while ( ( cp = fgets( pagebuf, 800, infile ) ) ) {
		pos = ftell( infile );
		len = pos-lastpos;
		if ( pagectl >= 1 && *cp == (char)12 ) {
		if ( first_page == 0 ) first_page = page;
		p_pages[page++] = lastpos;
		ln = 0;
		pagectl--;
	}
		while ( len > maxln-2 ) {
				cp3 = cp+maxln;
				cp1 = cp3-9;
				for ( cp2 = cp3; cp2 > cp1; cp2-- )
					if ( *cp2 < '0' || *cp2 > 'z' ) break;
				if ( cp2 > cp1 ) len = cp2-cp;
				else len = cp3-cp;
			cp += len;
			lastpos += len;
			if ( ++ln >= lastline ) {
				ln = 0;
				p_pages[page++] = lastpos;
		}
		len = pos-lastpos;
		}
		lastpos = pos;
		if ( ++ln >= lastline ) {
			ln = 0;
			p_pages[page++] = lastpos;
		}
		if ( page > MAXPAGES ) break;
	}
	p_pages[page++] = lastpos;
	p_pages[page] = -1;
	return page-2;
}

int
ReadFile( FILE *infile, int part, int maxparts ) {
	int pagenum, stat, key, done = R_More;
	stat = GetPage( infile, first_page );
	pagenum = first_page;
	while ( done == R_More ) {
		if ( stat == -1 ) beep( );
		else refresh( );
		stat = 0;
		key = GetKey( 0 );
		switch (key) {
		case KEY_LEFT:
		case 'q':
			done = R_Done;
			break;
		case KEY_PPAGE:
		case 'b':
			if ( pagenum <= 0 ) stat = -1;
			else GetPage( infile, --pagenum );
			break;
		case KEY_HOME:
		case 't':
			if ( pagenum > 0 )
				GetPage( infile, pagenum = 0 );
			else stat = -1;
			break;
		case 10:
		case 32:
		case KEY_NPAGE:
			if ( pagenum >= maxpage) stat = -1;
			else GetPage( infile, ++pagenum );
			break;
		case KEY_END:
		case 'e':
			if ( pagenum >= maxpage) stat = -1;
			else GetPage( infile, pagenum = maxpage );
			break;
		case 's':
			if ( maxparts < 1 ) stat = -1;
			else done = R_Save;
			break;
		case 'p':
			if ( part == 0 ) stat = -1;
			else done = R_Prev;
			break;
		case 'n':
			if ( part+1 >= maxparts ) stat = -1;
			else done = R_Next;
			break;
		case 'k':
			done = R_Kill;
			break;
		default: stat = -1;
		}
	}
	fclose( infile );
	clear( );
	return done;
}

int
NZBRead( P_ARTICLE p_article ) {
	int i, stat;
	char *name;
	i = AS_Stat|AS_NoLoad|AS_Err|AS_Foreground;
	stat = SaveArticle( p_article, 0, i, 0, NULL );
	if ( stat != 0 ) return stat;
	name = strdup( filenames[0] );
	stat = ProcessNZBFile( name );
	unlink( name );
	free( name );
	return stat;
}

int
ReadArticle( P_ARTICLE p_article ) {
	int i, part = 0, stat;
	FILE *infile;
	char *cp;
	do {
		i = AS_OnePart|AS_NoLoad|AS_Err|AS_Foreground| AS_Article;
		stat = SaveArticle( p_article, part, i, 0, NULL );
		if ( stat != 0 ) return stat;
		infile = fopen( filenames[0], "r" );
		if ( infile == NULL )	{
			PrintStatus( "cannot open article file", lastline );
			return E_FileOpen;
		}
	maxpage = PageArticle( infile, 1 );
		stat = ReadFile( infile, part, p_article->parts );
		if ( stat == R_Prev )
			part--;
		else if ( stat == R_Next )
			part++;
	} while ( stat >= R_Prev );
	while ( stat == R_Save ) {
		mvprintw( lastline-1, 0, "enter filename" );
		cp = GetBuff( NULL, lastline, 0 );
		if ( !*cp )	{
			stat = R_Done;
			break;
		}
		i = rename( filenames[0], cp );
		if ( i == 0 ) break;
		else PrintStatus( "rename failed, enter name or cr to cancel", lastline );
	}
	if ( stat != R_Save ) unlink( filenames[0] );
	return ( stat == R_Kill ) ? 1 : 0;
}

int
ShowHelp( char *what ) {
	FILE *infile;
	char f_name[512];
	MakeFileName( f_name, 512, help_dir, what, "hlp" );
	infile = fopen( f_name, "r" );
	if ( infile == NULL )	{
		PrintStatus( "cannot open help file", lastline );
		return E_FileOpen;
	}
	maxpage = PageArticle( infile, 9 );
	ReadFile( infile, 0, 0 );
	return 0;
}

int
FlushArticle( P_CONNECTION p_conn ) {
	char *cp;
	int len, artend = 0;
/* read for two timeouts or art end and one timeout */
	while ( 1 ) {
		if ( ( cp = GetLine( p_conn, 1, &len ) ) == NULL ) {
			if ( ++artend > 1 ) break;
		} else if ( len == 1 && *cp == '.' ) artend++;
	}
	return -1;
}

int
CancelActiveDownloads( ) {
	int i, n, count = 0;
	P_ARTICLE p_article;
	pthread_mutex_lock( &article_sem );
	for ( i = 1; i < MAXTHREAD; i++ ) {
		p_article = p_download[i];
		if ( p_article == NULL ) continue;
		p_article->flags |= AF_Cancel;
		count++;
	}
	pthread_mutex_unlock( &article_sem );
	if ( count == 0 ) return 0;
	for ( n = 0; n < 10; n++ ) {
		sleep( 1 );
		pthread_mutex_lock( &article_sem );
		for ( i = 1; i < MAXTHREAD; i++ ) {
			p_article = p_download[i];
			if ( p_article != NULL && (p_article->flags&AF_Cancel) ) break;
		}
		pthread_mutex_unlock( &article_sem );
		if ( i >= MAXTHREAD ) break;
	}
	return count;
}

int
CancelDownloads( void ) {
	int i, n = bg_downloads, key, count = 0;
	if ( n == 0 ) return 0;
	pthread_mutex_lock( &article_sem );
	for ( i = 0; i < MAXTHREAD; i++ )
		if ( p_download[i] != NULL ) count++;
	pthread_mutex_unlock( &article_sem );
	move( lastline-1, 0 );
	clrtobot( );
	mvprintw( lastline-1, 0, "background download status: %d pending, %d active",
		n-count, count );
	mvprintw( lastline, 0, "press: a for active, p for pending, e for everything" );
	refresh( );
	key = GetKey( 30 );
	move( lastline-1, 0 );
	clrtobot( );
	count = 0;
	switch( key ) {
		case 'p':
			count = CancelQueuedDownloads( );
			break;
		case 'e':
			count = CancelQueuedDownloads( );
/* fall through */
		case 'a':
			count += CancelActiveDownloads( );
			break;
		default:
			count = -1;
	}
return count;
}

char *
GetEncodingName( int encoding ) {
	static char *encodes[] = {
		"saving", "saving text", "uu decoding", "y decoding", "mime qp	decoding",
		"mime 64 decoding", "saving binary"
	};
	if ( encoding <= ENC_Unknown ) encoding = 0;
	return encodes[encoding];
}

static puchar
CopyBuf( puchar dest, puchar src, int len ) {
	while ( --len >= 0 ) *dest++ = *src++;
	*dest++ = '\n';
	*dest = '\0';
	return dest;
}

int
SaveArticle( P_ARTICLE p_article, int part, int how, int thread,
						P_ART_DIR p_dir ) {
	static char *artcmds[] = { "BODY", "ARTICLE" };
	P_GROUP p_group = p_article->p_group;
	P_CONNECTION p_conn = NULL;
	int outFile = -1, lines, p_high = p_article->parts, p_low = part;
	int i, len, encoding = ENC_Unknown;
	int timeout, written, k_conn;
	char buf[256], filename[256];
		P_PARTDATA p_part = p_article->p_parts+part, p_nextpart;
	P_EXTRAPART p_extra;
	P_YDATA p_ydata = NULL;
	uchar *outbuf = NULL, *out, *outmax, *cp, *msg_id;
				char *p_what = artcmds[how&AS_Article];
	int server , lastServer = -1, status = 0;
	int prefetch = 0, presend = 0, pri = (how&AS_Foreground) ? PRI_FG : PRI_BG;
	struct stat statbuf;
	ARTNUM artnum, first_art = p_part->artnum;
	pthread_mutex_lock( &article_sem );
	encodings[thread] = ENC_Unknown;
	filenames[thread][0] = '\0';
	*filename = '\0';
	if ( (p_article->flags&AF_Cancel) ) {
		pthread_mutex_unlock( &article_sem );
		goto oops;
	}
	p_download[thread] = p_article;
	if ( (how&AS_Foreground) ) {
		p_article->flags |= AF_Get;
		if ( p_dir != NULL ) p_dir->nref++;
	}
	pthread_mutex_unlock( &article_sem );
	outbuf = malloc( BUFFSIZE );
	outmax = outbuf+(BUFFSIZE-164);
	out = outbuf; 
	if ( (how&AS_OnePart) ) p_high = part+1;
	while ( part < p_high && !(p_article->flags&AF_Cancel) ) {
		written = 0;
		p_part = p_article->p_parts+part;
		p_extra = (P_EXTRAPART)p_part;
		artnum = p_part->artnum;
		if ( artnum == 0 && p_article->msg_id != NULL )
			msg_id = p_article->msg_id[part];
		else msg_id = NULL;
		server = p_part->server;
		part++;
		if ( artnum == 0 && msg_id == NULL ) {
			if ( (how&AS_GetPartial) ) continue;
			status = E_ArtUnavail;
			goto oops;
		}
get_extra:
		if ( server != lastServer ) {
			CloseConnect( p_conn, 0 );
			if ( ( p_conn = OpenConnect( server, pri, STAT_Article ) ) == NULL ) {
				status = E_ConnErr;
				goto oops;
			}
			if ( ( how&AS_Stat ) ) {
				sprintf( buf, "getting from %s", GetDispName( server ) );
				move( lastline-2, 0 );
				clrtoeol( );
				mvprintw( lastline-2, 0, buf );
				refresh( );
			}
			lastServer = server;
			if ( SelectGroup( p_conn, p_group, UP_Change ) != 0 ) {
				status = E_GroupSel;
				goto oops;
			}
			timeout = p_conn->timeout;
			prefetch = p_servers[server]->prefetch;
		}
		if ( !presend ) {
			if ( (p_conn = SurrenderConnect( p_conn ) ) == NULL ) {
				status = E_ConnErr;
				goto oops;
			}
		}
		p_nextpart = NULL;
		if ( part < p_high && !(p_article->flags&AF_Cancel) ) {
			p_nextpart = p_part+1;
			if ( p_nextpart->server != server ) p_nextpart = NULL;
		}
		if ( !presend ) {
			if ( artnum > 0 ) sprintf( buf, "%s %lu", p_what, artnum );
			else sprintf( buf, "%s <%s>", p_what, msg_id );
			LOG_Debug ( "sending %s for %u part %d",
				buf, first_art, part );
			i = SendCommand( p_conn, buf );
		} else {
			i = GetResponse( p_conn, timeout );
			presend = 0;
		}
		LOG_Debug ( "getting %s for %u part %d stat %d",
			p_what, first_art, part, i );
		if ( i == NNTP_CLASS_ERR ) goto try_next;
		if ( i != NNTP_CLASS_OK ) {
			status = E_ConnErr;
			sprintf( buf, "error, status %d", NntpStatus( p_conn, NULL ) );
			LOG_Error ( buf );
			if ( how&AS_Err ) {
				PrintStatus( buf, lastline );
			}
			FlushArticle( p_conn );
			goto oops;
		}
		if ( how&AS_Article ) {
			i = 50;
			while ( 1 ) {
				cp = (uchar *)GetLine( p_conn, timeout, &len );
				if ( cp == NULL ) {
					status = E_TimeOut;
					FlushArticle( p_conn );
					goto oops;
				}
				if ( len == 0 ) break;
				if ( --i <= 0 ) {
					status = E_LongHdr;
					FlushArticle( p_conn );
					goto oops;
				}
				if ( IsWord( cp, "subject: =?" ) )
					len = DecodeLine( cp+9, len-9 )+9;
				if ( p_low+1 == part && IsWord( cp, "subject" ) )
					LOG_ArtGet( "getting %s", cp );
				out = CopyBuf( out, cp, len );
				if ( encoding == ENC_Unknown )
					GetEncoding( cp, filename, &encoding, &len );
			}
			*out++ = (char)12;
			*out = '\0';
			if ( encoding < ENC_Unknown )
				GetEncoding( cp, filename, &encoding, &len );
		}
		lines = p_part->bytes/128;
		if ( outFile == -1 ) {
			if ( how&AS_Binary ) {
				encoding = ENC_Binary;
			} else if ( encoding == ENC_Unknown ) {
				for ( i = 15; i > 0; i-- ) {
					cp = (uchar *)GetLine( p_conn, timeout, &len );
					if ( !cp ) {
						status = E_TimeOut;
						FlushArticle( p_conn );
						goto oops;
					}
					lines--;
					if ( len == 1 && *cp == '.' ) break;
					if ( GetEncoding( cp, filename, &encoding, &len ) > ENC_Unknown )
						break;
					if ( len == 0 ) continue;
					out = CopyBuf( out, cp, len );
				}
			}
			if ( *filename == '\0' ) {
				if ( p_dir == NULL ) sprintf( filename, "art%lu", artnum );
				else sprintf( filename, "%s/art%lu", p_dir->dir, artnum );
			} else {
				char *cp1;
				if ( p_dir != NULL ) {
					strcpy( buf, filename );
					sprintf( filename, "%s/%s", p_dir->dir, buf );
				}
				cp1 = filename+strlen(filename);
				if ( !(how&AS_GetPartial) ) {
					for ( i = 1; stat( filename, &statbuf ) == 0; i++ ) {
						sprintf( cp1, "_%d", i );
					}
				}
			}
			outFile = open( filename, O_WRONLY|O_CREAT, 0666 );
			if ( outFile == -1 ) {
				FlushArticle( p_conn );
				status = E_FileCreate;
				break;
			}
			pthread_mutex_lock( &article_sem );
			k_bytes[thread] = p_conn->k_bytes_sec;
			encodings[thread] = encoding;
			strcpy( filenames[thread], filename );
			pthread_mutex_unlock( &article_sem );
			sprintf( buf, "%s to %s", GetEncodingName( encoding ), filename );
			LOG_Status ( buf );
			if ( how&AS_Stat ) {
				move( lastline-1, 0 );
				clrtoeol( );
				mvprintw( lastline-1, 0, buf );
				refresh( );
			}
			if ( !(how&AS_Article) && encoding != ENC_Unknown )
				out = outbuf;	/* junk buf for body and known encoding */
			if ( len == 1 && *cp == '.' ) goto art_done;
			if ( encoding == ENC_Yenc ) {
				i = ( (how&AS_GetPartial) ) ? 1 : 0;
				p_ydata = InitYenc( outFile, i );
				len = YDecodeLine( p_ydata, cp, out, len );
			}
		}
		while ( 1 ) {
			if ( ( cp = (uchar *)GetLine( p_conn, timeout, &len ) ) == NULL ) {
				status = E_TimeOut;
				FlushArticle( p_conn );
				goto oops;
			}
			if ( *cp == '.' ) {
				if ( len == 1 ) break;
				if ( cp[1] == '.' ) {
					cp++;
					len--;
				}
			}
			if ( --lines == prefetch && p_nextpart != NULL &&
					TestConnect( p_conn ) == 0 && !(p_article->flags&AF_Cancel) ) {
				artnum = p_nextpart->artnum;
				msg_id = NULL;
				*buf = '\0';
				if ( artnum > 0 )
					sprintf( buf, "%s %lu", p_what, artnum );
				else {
					if ( p_article->msg_id != NULL ) msg_id = p_article->msg_id[part];
					if ( msg_id != NULL )	sprintf( buf, "%s <%s>", p_what, msg_id );
				}
				if ( *buf != '\0' ) {
					LOG_Debug ( "presending %s for %u part %d",
						buf, first_art, part );
					if ( ( i = SendData( p_conn, buf ) ) == 0 ) presend = 1;
				}
			}
			if ( len < 1 ) continue;
			switch( encoding ) {
				case ENC_Yenc:
					len = YDecodeLine( p_ydata, cp, out, len );
					if ( len > 0 ) out += len;
					break;
				case ENC_UUenc:
					len = UUDecodeLine( cp, out, len );
					if ( len > 0 ) out += len;
					break;
				case ENC_Mime_B64:
					len = B64DecodeLine( cp, out, len );
					if ( len > 0 ) out += len;
					break;
				case ENC_Mime_QP:
					len = QPDecodeLine( cp, out, len );
					if ( len > 0 ) out += len;
					break;
				default:
					out = CopyBuf( out, cp, len );
					break;
			}
			if ( out < outmax ) continue;
			len = out-outbuf;
			if ( len != write( outFile, outbuf, len ) ) {
				status = E_WriteError;
				goto oops;
			}
			written = 1;
			out = outbuf; 
		}
art_done:
		len = out-outbuf;
		if ( len > 0 ) {
			if ( len != write( outFile, outbuf, len ) ) {
				status = E_WriteError;
				goto oops;
			}
			written = 1;
		}
		out = outbuf; 
		if ( lines > 2 && written == 0 ) {
try_next:
			if ( p_extra->next == 0 ) {
				if ( (how&AS_GetPartial) ) continue;
				status = E_ArtUnavail;
				goto oops;
			}
			p_extra = &p_article->p_extras[p_extra->next-1];
			sprintf( buf, "part %d unavailable, trying %s", part,
				GetDispName( p_extra->server ) );
			LOG_Status ( buf );
			if ( how&AS_Stat )
				PrintStatus( buf, lastline );
			goto get_extra;
		}
		pthread_mutex_lock( &article_sem );
		parts[thread] = part;
		k_bytes[thread] = p_conn->k_bytes_sec;
		pthread_mutex_unlock( &article_sem );
		sprintf( buf, "part %3d", part );
		if ( p_ydata && ( len = GetYEncError( p_ydata, buf+8 ) ) > 0 ) {
			LOG_Error ( "YENC Error on %s %s", filename, buf );
		}
		if ( how&AS_Stat )
			PrintStatus( buf, lastline );
	}
oops:
	if ( p_conn != NULL ) {
		if ( presend == 1 && (p_article->flags&AF_Cancel) ) 
			FlushArticle( p_conn );
		CloseConnect( p_conn, 0 );
		k_conn = p_conn->k_bytes_sec;
	}
	if ( outFile != -1 ) close( outFile );
	if ( p_ydata ) {
		i = FreeYenc( p_ydata );
		if ( !(how&AS_Foreground) && i > 0 ) bg_errors++;
	}
	if ( outbuf != NULL ) free( outbuf );
	pthread_mutex_lock( &article_sem );
	p_download[thread] = NULL;
	k_bytes[thread] = k_conn;
	if ( (p_article->flags&AF_Cancel) ) status = E_Cancel;
	p_article->flags &= ~(AF_Cancel|AF_Gets);
	if ( status == 0 ) {
		if ( !(how&AS_NoLoad) ) p_article->flags |= AF_Loaded;
		if ( (how&AS_Kill) ) {
			p_article->flags |= AF_Kill;
			p_group->delete_ctr++;
		}
	}
	if ( p_dir != NULL && --p_dir->nref <= 0 ) {
		free( p_dir->dir );
		free( p_dir );
	}
	if ( !(how&AS_Foreground) ) {
		if ( status != 0 && status != E_Cancel ) {
			bg_errors++;
			p_group->bg_errors++;
		}
		bg_downloads--;
		if ( --p_group->bg_downloads == 0 ) {
			if ( p_group->flags&GF_SaveRequest )
				QueueWork	( 0, TASK_GroupSave, p_group, NULL, PRI_High, 0, NULL );
			if ( (p_group->flags&GF_NZB) )
				FreeGroup( p_group );
		}
	}
	pthread_mutex_unlock( &article_sem );
	LOG_Status ( "article %u part %hd status %s",
		first_art, part, art_msgs[status] );
	if ( status != 0 ) {
		if ( (how&AS_Err) && status != E_ConnErr )
			PrintStatus( art_msgs[status], lastline );
		unlink( filename );
	} else if ( (how&AS_Stat) )
		PrintStatus( art_msgs[status], lastline );
	return status;
}

int
PrintDownloads( int line ) {
	int i, n = bg_downloads, count, changed;
	int part, encoding, ln, total_k;
P_ARTICLE p_lastart[MAXTHREAD+1];
	char buf[256];
	if ( n == 0 ) {
		strcpy( buf, "no background downloads" );
		if ( bg_errors == 1 ) strcat( buf, " 1 error" );
		else if ( bg_errors > 1 )
			sprintf( buf+strlen( buf ), " %d errors", bg_errors);
		PrintStatus( buf, line );
		return 0;
	}
	bzero( p_lastart, sizeof( p_lastart ) );
	clear( );
	refresh( );
	do {
		ln = 2;
		changed = count = 0;
		total_k = 0;
		for ( i = 1; i < MAXTHREAD; i++ ) {
			pthread_mutex_lock( &article_sem );
			if ( p_download[i] != p_lastart[i] ) {
				changed++;
				p_lastart[i] = p_download[i];
			}
			if ( p_download[i] == NULL ) {
				pthread_mutex_unlock( &article_sem );
				if ( changed == 1 ) {
					move( ln, 0 );
					clrtobot( );
				}
				continue;
			}
			count++;
			encoding = encodings[i];
			part = parts[i];
			strcpy( buf, filenames[i] );
			total_k += k_bytes[i];
			pthread_mutex_unlock( &article_sem );
			if ( changed == 1 ) {
				move( ln, 0 );
				clrtobot( );
			}
			mvprintw( ln, 0, "%s to %s", GetEncodingName( encoding ), buf );
			if ( part > 0 ) mvprintw( ln+1, 0, "part %3d", part );
			ln += 2;
		}
		move( lastline, 0 );
		clrtoeol( );
		mvprintw( lastline, 0, "background download status: %d pending, %d active",
			n-count, count );
		if ( bg_errors > 0 ) printw( " %d error%c",
			bg_errors, (bg_errors == 1 ) ? ' ' : 's' );
		if ( total_k > 0 ) printw( " %3d k/sec", total_k );
		move( lastline, 0 );
		refresh( );
		if ( GetKey( 5 ) == 10 ) break;
	} while ( ( n = bg_downloads ) > 0 );
	return 1;
}
