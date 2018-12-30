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
#include "common.h"
#include "server.h"
#include <unistd.h>
#include <curses.h>
#include <time.h>

extern int nntp_debug;
static char pattern[80], cmd_buf[80];

int
GetKey( int timeout ) {
	fd_set readset;
	struct timeval tm = { 0, 0 };
	if ( timeout > 0 ) {
		tm.tv_sec = timeout;
		FD_ZERO( &readset );
		FD_SET( 0, &readset );
		if ( select( 1, &readset, 0, 0, &tm ) == 0 ) return 0;
		}
	return getch( );
}

char *
GetBuff( char *buf, int line, int key ) {
	if ( !buf ) buf = cmd_buf;
	*buf = 0;
	move( line,0 );
	clrtoeol( );
	if ( key > 0 ) ungetch( key );
	echo();
	refresh( );
	getnstr( buf, 75 );
	noecho();
	move( line,0 );
	clrtoeol( );
	refresh( );
	Trim( buf );
	return buf;
}

enum { P_RANGE = 1, P_RPT0, P_RPT1, P_END, P_STAR, P_CHAR_RANGE, P_QUESTION };

char *
LastPattern( void ) {
	return pattern;
}

char *
GetPattern( char *in, char *patt ) {
	char *inrange = NULL, *ps = in;
	char ch;
	if ( !ps ) ps = GetBuff( NULL, LINES-1, '/' );
	if ( !*ps ) return 0;
	ps++;
	ch = *ps;
	if ( '?' == ch || '*' == ch || '\0' == ch )
		return 0;
	if ( !patt ) patt = pattern;
	while ( (ch = *ps ) ) {
		ps++;
		if ( ch == '/' ) break;
		switch ( ch ) {
			case '*':
				*patt++ = (inrange) ? ch : P_STAR;
				break;
			case '?':
				*patt++ = (inrange) ? ch : P_QUESTION;
				break;
			case '[':
				if ( inrange ) *patt++ = ch;
				else {
					inrange = patt;
					*patt++ = P_RANGE;
				}
				break;
			case ']':
				if ( !inrange ) *patt++ = ch;
				else {
					if ( '+' == *ps ) {
						*inrange = P_RPT1;
						ps++;
					} else if ( '*' == *ps ) {
						*inrange = P_RPT0;
						ps++;
					}
					*patt++ = P_END;
					inrange = NULL;
				}
				break;
			case '-':
				if ( inrange && *ps && patt[-1] != P_CHAR_RANGE ) {
				*patt = patt[-1];
					patt[-1] = P_CHAR_RANGE;
					patt++;
				*patt++ = *ps++;
				} else *patt++ = ch;
				break;
			case '\\':
				if ( inrange ) *patt++ = ch;
				else if ( *ps ) *patt++ = *ps++;
				break;
			default:
				*patt++ = ch;
		}
	}
	if ( inrange )	*patt++ = P_END;
	ch = patt[-1];
	if ( '?' == ch || '*' == ch )
		patt--;
	*patt = '\0';
	return ps;
}

static uchar *MatchString( uchar *in, uchar *p_in, int scan ) {
	uchar *pp, *ps;
	uchar ch, ch1;
	while ( *in >= ' ' ) {
		pp = p_in;
		ps = in;
		while ( ( ch = *pp ) > P_QUESTION ) {
			if ( ( ch1 = *ps ) < ' ' )
				return 0;
			if ( ch != ch1 ) {
				if ( ch1 >= CAPA && ch1 <= CAPZ ) {
					if ( ch != (ch1|32) )
						break;
				} else break;
			}
			pp++; ps++;
		}
		if ( ch <= P_QUESTION )
			return in;
		if ( !scan )
			return 0;
		in++;
	} 
	return 0;
}

static uchar *MatchRange( uchar *in, uchar *p_in ) {
	uchar ch, ch1 = *in;
	while ( ( ch = *p_in ) != P_END ) {
		if ( ch == P_CHAR_RANGE ) {
			if ( ch1 >= p_in[1] && ch1 <= p_in[2] ) break;
			p_in += 3;
		} else if ( ch1 == ch ) break;
		else p_in++;
	}
	if ( *p_in == P_END ) return 0;
	while ( *p_in != P_END ) p_in++;
	p_in++;
	return p_in;
}

char *MatchPattern( char *in, uchar *p_in ) {
	uchar *pp, *pp1, *ps = (uchar *)in;
	uchar ch, ch1;
	int scan = 1;
	if ( !p_in ) p_in = (puchar)pattern;
	pp = p_in;
	while ( ( ch = *pp ) ) {
		if ( !( ch1 = *ps ) ) break;
		if ( ch > P_QUESTION ) {
			ps = MatchString( ps, pp, scan );
			if ( !ps ) return 0;
			while ( *pp > P_QUESTION ) {
				pp++; ps++;
			}
			scan = 0;
		} else if ( P_STAR == ch ) {
			pp++;
			scan = 1;
		} else if ( P_RANGE == ch ) {
			while ( !( pp1 = MatchRange( ps, pp ) ) ) {
				if ( !scan ) return 0;
				ps++;
				if ( *ps < ' ' ) return 0;
			}
			scan = 0;
			pp = pp1;
		} else if ( P_QUESTION == ch ) {
			pp++; ps++;
			scan = 0;
		}
	}
	return (char *)ps;
}

int
PatternInfo( int doclear ) {
	if ( !doclear ) return strlen( pattern );
	*pattern = '\0';
	return 0;
}

puchar
FindLast( puchar in, int what ) {
	puchar cp = in;
	while ( *cp ) cp++;
	while ( --cp >= in) {
		if ( (int)(*cp) == what ) return cp;
	}
	return NULL;
}

int
IsWord( puchar in, char *test ) {
	uchar ch, *cp = (puchar)test;
	while ( *cp ) {
		ch = *in++;
		if ( ch >= 'A' && ch <= 'Z' ) ch |= 32;
		if ( ch != *cp ) return 0;
		cp++;
	}
return 1;
}

char *
Trim( char *str ) {
	char *cp = str, *cp1 = str;
	while ( *cp && *cp <= ' ' ) cp++;
	if ( cp > str ) {
		for ( ; *cp; cp++ ) *cp1++ = *cp;
		*cp1 = '\0';
	} else while ( *cp1 ) cp1++;
	while ( --cp1 > str && *cp1 <= ' ' );
	*(++cp1) = '\0';
	return str;
}

#include "timetab.h"

#define SECS_DAY (24L * 60L * 60L)

const int monthdays[] = {
	0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

int IsLeap(int year) {
	if ((year&3) != 0) return 0;
	if ((year%400) == 0) return 1;
	return ((year%100) != 0);
}

time_t mkgmtime(struct tm *tmbuf, int offset)
{
 long day = 0l, year, seconds = 0l;
 int tm_year, n, yday = (tmbuf->tm_mday-1);
	if ((year = tmbuf->tm_year-70) < 0 || yday < 0 || yday > 30) return (time_t)-1;
	if (year >= 68 || tmbuf->tm_mon < 0 ||tmbuf->tm_mon >11) return (time_t)-1;
	tm_year = tmbuf->tm_year + 1900;
	for (n = 1972; n < tm_year; n += 4) {
		day++;
		if (n%100 == 0 && n%400 != 0) day--;
	}
	day += year*365;
	yday += monthdays[tmbuf->tm_mon];
	if (IsLeap(tm_year) && tmbuf->tm_mon >= 2) yday++;
	day += yday;
	seconds = ((tmbuf->tm_hour*60L) + tmbuf->tm_min)*60L + tmbuf->tm_sec;
	seconds += day*SECS_DAY;
	if (offset != 0) {
		n = offset%100;
		offset = (offset/100)*60;
		if (offset > 0) offset += n;
		else offset -= n;
		offset *= 60;
		seconds -= (long)offset;
	}
 return (time_t) seconds;
}

int starts[32];

time_t
ParseDate( char *indate ) {
	char *tokens[8], *cp = indate;
	char ch, cht;
	int i = 1, j, n, offset = 0;
	time_t now;
	struct tm in_time;
	bzero( &in_time, sizeof( in_time ) );
	tokens[0] = indate;
	while ( (ch = *cp) ) {
		if ( ch >= 'A' && ch <= 'Z' ) *cp = ch|32;
		if ( ch != ' ' ) cp++;
		else {
			*cp++ = '\0';
			tokens[i++] = cp;
			if ( i >= 7 ) break;
		}
	}
	tokens[i] = 0;
	for ( i = 0; (cp = tokens[i] ); i++ ) {
		ch = *cp;
		if ( ch >= 'a' ) {
			j = starts[ch&31];
			if ( j < 0 ) continue;
			for ( ; date_info[j].type < 'z'; j++ ) {
				if ( date_info[j].type < 's' )
					n = strncmp( date_info[j].name, cp, 3 );
				else n = strcmp( date_info[j].name, cp );
				if ( n >= 0 ) break;
			}
			if ( n != 0 ) continue;
			n = date_info[j].value;
			cht = date_info[j].type;
			if ( cht == 'm' )
				in_time.tm_mon = n;
			else if ( cht == 'd' )
				in_time.tm_wday = n-1;
			else if ( cht == 's' || cht == 't' )
				offset = n;
		} else if (	ch >= '0' && ch <= '9' ) {
			n = atoi( cp );
			if ( n < 24 && strchr( cp, ':' ) ) sscanf( cp, "%d:%d:%d", 
				&in_time.tm_hour, &in_time.tm_min, &in_time.tm_sec );
			else if ( n < 32 ) in_time.tm_mday = n;
			else if ( n >= 2000 ) in_time.tm_year = n-1900;
		} else if ((ch == '+' || ch == '-') && strlen(cp) == 5) { /* gmt offset */
			sscanf( cp, "%d", &offset );
		}
	}
	if ( --in_time.tm_mon >= 0 && in_time.tm_mday > 0 && in_time.tm_year > 0 ) {
		now = mkgmtime( &in_time, offset );
		if ( nntp_debug&32 ) fprintf( stderr, "date %d %d %d %d:%d:%d +%d =%s",
			in_time.tm_year, in_time.tm_mon, in_time.tm_mday, in_time.tm_hour,
			in_time.tm_min, in_time.tm_sec, offset, ctime(&now) );
		return now;
	}
	if ( nntp_debug&32 ) {
		for ( i = 1; (cp = tokens[i] ); i++ ) cp[-1] = ' ';
		fprintf( stderr, "bad date %s\n", indate );
	}
	return (time_t)0;
}

time_t
GetCurrDate( int offset ) {
	time_t now = time( NULL );
	if ( offset > 0 ) now -= (offset*86400); /* seconds/day */
	return now;
}

char *
NextToken( char *in, char *key, char **value ) {
	char *cp1 = key, *cp = in;
	*key = '\0';
	if ( value ) *value = NULL;
	if ( !cp || *cp == '\0' ) return NULL;
	while ( *cp > '\0' && *cp <= ' ' ) cp++;
	if ( *cp == '\0' ) return NULL;
	while ( *cp > ' ' && *cp != '=' ) {
		*cp1++ = tolower( (uchar)(*cp) );
		cp++;
		if ( cp1 > key+14 ) break;
	}
	*cp1 = '\0';
	while ( *cp > '\0' && *cp <= ' ' ) cp++;
	if ( *cp != '=' ) return cp;
	cp++;
	while ( *cp > '\0' && *cp <= ' ' ) cp++;
	if ( value ) *value = cp;
	while ( *cp > ' ' ) cp++;
	if ( *cp ) *cp++ = '\0';
	return cp;
}

char *
GetHomeDir( ) {
	static char *home = NULL;
	if ( !home ) home = getenv( "HOME" );
	if ( !home ) {
		fprintf( stderr, "$HOME not defined\n" );
		exit ( 1 );
	}
	return home;
}

char *
MakeDir( char *name, char *dir ) {
	char *cp, *cp1;
	struct stat statbuf;
	if ( *name == '~' ) {
		strcpy( dir, GetHomeDir( ) );
		strcat( dir, name+1 );
	} else if ( *name == '$' ) {
		cp1 = strchr( name, '/' );
		if ( cp1 ) *cp1++ = '\0';
		cp = getenv( name+1 );
		if ( !cp ) {
			fprintf( stderr, "%s variable not defined\n", name );
			return NULL;
		}
		if ( cp1 )sprintf( dir, "%s/%s", cp, cp1 );
		else strcpy( dir, name );
	} else strcpy( dir, name );
	if ( stat( dir, &statbuf ) != 0 && mkdir( dir, 0777 ) != 0 ) {
		perror( dir );
		return NULL;
	}
	return dir;
}

void
InitDate( ) {
	uchar ch;
	int i;
	for ( i = 0; i < 32; i++ ) starts[i] = -1;
	i = (sizeof(date_info)/sizeof(struct DATE_DATA))-1;
	while ( --i >= 0 ) {
		ch = (date_info[i].name[0])&31;
		starts[ch] = i;
	}
}

char *
MakeFileName( char *buf, int buflen, char *dir, char *name, char *ext ) {
	char *ps, *pd = buf, *pend = buf+buflen-2;
	if ( dir ) {
		for ( ps = dir; *ps; ps++ ) {
			if ( pd >pend ) break;
			*pd++ = *ps;
		}
		*pd++ = '/';
	}
	if ( name && pd < pend ) {
		for ( ps = name; *ps; ps++ ) {
			if ( pd >pend ) break;
			*pd++ = *ps;
		}
	}
	if ( ext && pd < pend ) {
		*pd++ = '.';
		for ( ps = ext; *ps; ps++ ) {
			if ( pd >pend ) break;
			*pd++ = *ps;
		}
	}
	*pd = '\0';
	return buf;
}

unsigned int
GetInt( uchar **in ) {
	unsigned int value = 0;
	uchar *cp = *in;
	int ch;
	ch = *cp++;
	if ( ch < '0' ) return 0;
	while ( ch > 148 ) {
		value += (ch-148);
		value *= 100;
	ch = *cp++;
		if ( ch < '0' ) break;
	}
	if ( ch >= '0' ) value += (ch-'0');
	*in = cp;
	return value;
}

uchar *
GetInts( unsigned int *ptr, int count, uchar *in ) {
	int n;
	for ( n = 0; n < count; n++ ) {
		ptr[n] = GetInt( &in );
	}
	return in;
}

uchar *
GetShorts( ushort *ptr, int count, uchar *in ) {
	int n;
	int value;
	for ( n = 0; n < count; n++ ) {
		value = GetInt( &in );
		if ( value < 65536 ) ptr[n] = (ushort)value;
		else return NULL;
	}
	return in;
}

uchar *
PutInts( unsigned int *ptr, int count, uchar *out ) {
	u_char ch;
	static unsigned int const mults[] = { 100, 10000, 1000000, 100000000 };
	unsigned int value, v;
	int n, i;
	for ( n = 0; n < count; n++ ) {
		value = ptr[n];
		if ( value >= 100 ) {
			for ( i = 0; i < 4; i++ )
				if ( mults[i] > value ) break;
			i--;
			do {
				v = value/mults[i];
				ch = (v&127)+148;
				*out++ = ch;
				value %= mults[i];
			} while ( --i >= 0 );
		}
		ch = (value&127)+48;
		*out++ = ch;
	}
	return out;
}

uchar *
PutShorts( unsigned short *ptr, int count, uchar *out ) {
	u_char ch;
	static unsigned short const mults[] = { 100, 10000 };
	unsigned short value, v;
	int n, i;
	for ( n = 0; n < count; n++ ) {
		value = ptr[n];
		if ( value >= 100 ) {
			for ( i = 0; i < 2; i++ )
				if ( mults[i] > value ) break;
			i--;
			do {
				v = value/mults[i];
				ch = (v&127)+148;
				*out++ = ch;
				value %= mults[i];
			} while ( --i >= 0 );
		}
		ch = (value&127)+48;
		*out++ = ch;
	}
	return out;
}
