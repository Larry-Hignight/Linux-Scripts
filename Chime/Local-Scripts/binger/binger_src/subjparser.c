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
#include "hash.h"
#include "decoders.h"
#include "artdata.p"

enum {
	MISC = 0, DELM, LBRACK, RBRACK, SLASH, SPACE, DASH, DOT, APOST, PLUS,
	OTHER, NUM, ALPHA, OF1, OF2, FINAL1, FINAL2, FINAL3
};
static u_short chartab[256] = {
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 0-7 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 8-15 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /*16-23 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 24-31 */
SPACE, DELM, DELM, DELM, DELM, MISC, SPACE, APOST, /*	!"#$%&' */
LBRACK, RBRACK, DELM, PLUS, OTHER, DASH, DOT, SLASH, /* ()*+,-./ */
NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, /* 01234567 */
NUM, NUM, DELM, DELM, DELM, DELM, DELM, DELM, /* 89:;<=>? */
MISC, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, OF2, ALPHA, /* @ABCDEFG */
ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, OF1, /* HIJKLMNO */
ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, /* PQRSTUVW */
ALPHA, ALPHA, ALPHA, LBRACK, MISC, RBRACK, MISC, DASH, /* XYZ[\]^_ */
MISC, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, OF2, ALPHA, /* `abcdefg */
ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, OF1, /* hijklmno */
ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, ALPHA, /* pqrstuvw */
ALPHA, ALPHA, ALPHA, LBRACK, MISC, RBRACK, MISC, MISC, /* xyz{|}~ */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 128-135 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 136-143 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 144-151 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 152-159 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 160-167 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 168-175 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 176-183 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 184-191 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 192-199 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 200-207 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 208-215 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 216-223 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 224-231 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 232-239 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC, /* 240-247 */
MISC, MISC, MISC, MISC, MISC, MISC, MISC, MISC /* 248-255 */
};

char *def_ext[] = { "unknown", "single", "multi" };
typedef struct S_EXT EXT, *P_EXT;
struct S_EXT {
	char ext[8];
	short flags, num;
	P_EXT p_next;
};

static int num_ext = 0, par2id = 0, nzbid = 0;
static P_EXT ext_starts[32];
static EXT f_ext[64];
char *ext_buf = "nfo|,sfv|,txt|,nzb|,log|,cue|,m3u|,flac,pdf,m4a,mp3,rar,r#,par2*,zip,jpg";

int IsNZB( P_ARTICLE p_in ) {
	short id = p_in->file_ext;
	if ( id == nzbid ) return 1;
	return 0;
}

int IsPar2( P_ARTICLE p_in ) {
	short id = p_in->file_ext;
	if ( id == par2id+1 ) return 2;
	if ( id == par2id ) return 1;
	return 0;
}

char *GetExtension( P_ARTICLE p_in ) {
	short id = p_in->file_ext;
	P_EXT this;
	if ( id < EX_MIN ) return def_ext[id];
	id -= EX_MIN;
	this = &f_ext[id/2];
	return this->ext;
}

int
SetExtensions( char *in ) {
	char *cp = in, *cp1, *cp2;
	int n, id;
	P_EXT prev, this;
	if ( num_ext > 0 ) return -1;
	bzero( ext_starts, 32*sizeof(P_EXT) );
	if ( cp == NULL ) cp = ext_buf;
	do {
		for ( cp1 = cp; *cp1 && *cp1 != ','; cp1++); /* empty */
		if ( cp1-cp > 6 ) return -1;
		this = &f_ext[num_ext++];
		n = (*cp)&31;
		prev = ext_starts[n];
		if ( prev == NULL ) ext_starts[n] = this;
		else {
			while ( prev->p_next != NULL ) prev = prev->p_next;
			prev->p_next = this;
		}
		this->p_next = NULL;
		cp2 = this->ext;
		id = num_ext*2+EX_MIN;
		this->num = id;
		while ( cp < cp1 ) {
			if ( *cp == '*' || *cp == '|' ) break;
			*cp2++ = *cp++;
		}
		*cp2 = '\0';
		if ( cp < cp1 ) {
			this->flags = *cp;
		 if ( *cp == '*' ) par2id = id;
		}
		if ( !strcmp( this->ext, "nzb" ) ) nzbid = id;
		cp = cp1+1;
	} while ( *cp1 == ',' );
	return num_ext;
}

puchar
CheckPar( P_ART_INFO p_info, puchar p_sub, puchar p_in ) {
	puchar cp = p_in;
	if ( *cp-- != '.' ) return p_in;
	while ( *cp == '+' || isdigit( *cp ) ) {
		if ( cp < p_sub+4 ) return p_in;
		cp--;
	}
	cp -= 3;
	if ( !IsWord( cp, ".vol" ) ) return p_in;
	p_info->file_ext++;
	return cp;
}


int
MakeSubjInfo( P_ART_INFO p_info ) {
	uchar ch;
	puchar p_extend = NULL, p_name = NULL, p_sub = p_info->subject;
	int file_ext = 0, n = 0;
	puchar cp1 = p_sub+p_info->subjlen, cp2, cp3;
	short state= MISC, laststate = MISC;
	P_EXT p_ext;
	for ( cp2 = p_sub; cp2 < cp1; cp2++ )
		if ( *cp2 < ' ' ) *cp2 = ' ';
	while ( --cp1 > p_sub ) {
		ch = *cp1;
		if ( ch != '.' ) {
			if ( isalnum(ch) ) n++;
			else n = 0;
			continue;
		}
		if ( cp1[-1] == ' ' || n < 2 || n > 4 ) continue;
		p_ext = ext_starts[(cp1[1]&31)];
		while ( p_ext != NULL ) {
			cp2 = (puchar)p_ext->ext;
			for ( cp3 = cp1+1; *cp2; cp2++ ) {
				if ( *cp2 == '#' ) {
					while ( isdigit( *cp3 ) ) cp3++;
					continue;
				}
				ch = *cp3++;
				if ( ch >= CAPA && ch <= CAPZ ) ch |= 32;
				if ( ch != *cp2 ) break;
			}
			if ( *cp2 == '\0' ) break;
			p_ext = p_ext->p_next;
		}
		if ( p_ext != NULL ) {
			file_ext = p_ext->num;
			break;
		}
	}
	if ( file_ext == 0 ) {
		p_info->file_ext = ( p_info->parts > 1 ) ? EX_multipart : EX_onepart;
		if ( IsWord( p_sub, "re: " ) ) p_info->flags |= AF_HasRe;
		return 0;
	}
	p_info->file_ext = file_ext;
	p_extend = cp3;
	cp1--; /* before . */
	if ( *cp3 == '"' ) { /* name in quotes? */
		for ( cp2 = cp1; cp2 > p_sub; cp2-- )
			if ( *cp2 == '"' ) break;
		if ( *cp2 == '"' ) {
			p_name = cp2+1;
			if ( p_ext->flags == '*' )
				CheckPar( p_info, p_sub, cp1+1 );
			goto set_data;
		}
	}
	if ( chartab[(u_char)*cp1] == RBRACK ) {
		for ( cp2 = cp1-1; cp2 >= p_sub; cp2-- ) {
			if ( (state = chartab[(u_char)*cp2]) == LBRACK ) break;
		}
		if ( state == LBRACK ) cp1 = cp2;
		else state = MISC;
	}
	if ( p_ext->flags == '|' ) {
		p_name = cp1+1;
		goto set_data;
	}
	if ( p_ext->flags == '*' ) {
		p_name = CheckPar( p_info, p_sub, cp1+1 );
		goto set_data;
	}
	while ( --cp1 >= p_sub ) {
		state = chartab[(u_char)*cp1];
		if ( state < DOT || state > OF2) break;
	}
	p_name = ++cp1; /* at least this much is name */
	while ( state > DELM && state < FINAL1 && --cp1 >= p_sub ) {
		if ( state > SPACE ) laststate = state;
		state = chartab[(u_char)*cp1];
		if ( state == NUM ) {
			cp2 = cp1;
			while ( --cp1 >= p_sub && isdigit( *cp1 ) );
			cp1++;
			if ( laststate == DASH )
				state = FINAL3;
			else if ( laststate == RBRACK )
				state = FINAL2;
		}
		else if ( state == OF1 && laststate != NUM )
			state = ALPHA;
		else if ( state == OF2 )
			state = ( laststate == OF1 ) ?	FINAL1 : ALPHA;
		else if ( state == SLASH && laststate == NUM )
			state = FINAL1;
	}
	if ( state == DELM ) {
		cp1++;
		state = FINAL2;
	} else if ( state == FINAL1 || state == FINAL2 ) {
		for ( cp1 = cp2; *cp1 == ' ' || *cp1 == ']'; cp1++ ); /* empty */
		state = FINAL2;
	}
	if ( state >= FINAL2 ) {
		while ( --cp1 >= p_sub )
		if ( *cp1 != ' ' && *cp1 != '-' ) break;
	if ( cp1 >= p_sub) cp1++;
		p_name = cp1;
	}
set_data:
	p_info->name_pos = p_name-p_sub;
	p_info->ext_end = p_extend-p_sub;
	return file_ext;
}

int
ParseSubject( P_ART_INFO p_info ) {
	int part = 0, parts = 0, len = p_info->subjlen;
	puchar p_sub = p_info->subject;
	puchar cp1, cp2, cp3 = NULL;
	uchar ch = 0;
	len = DecodeLine( p_sub, (int)p_info->subjlen );
	cp2 = p_sub+len;
	if ( p_info->subjlen != len ) {
		for ( cp1 = p_sub; cp1 < cp2; cp1++ )
			if ( *cp1 < ' ' || *cp1 > 127 ) *cp1 = ' ';
		p_info->subjlen = len;
	}
	while ( --cp2 > p_sub ) {
		if ( *cp2 == ')' ) ch = '(';
		else if ( *cp2 == ']' ) ch = '[';
		else continue;
		cp1 = cp2;
		cp3 = NULL;
		while ( --cp1 > p_sub && *cp1 >= '/' && *cp1 <= '9' ) {
			if ( *cp1 == '/' ) cp3 = cp1+1;
		}
		if ( cp3 != NULL && *cp1 == ch ) break;
		cp3 = NULL;
	}
	if ( cp3 != NULL ) {
		part = atoi( (char *)(cp1+1) );
		parts = atoi( (char *)cp3 );
	}
	if ( p_info->bytes < 5000 && IsWord( p_sub, "Re: " ) ) parts = 0;
	if ( parts > 1 && part < parts && p_info->bytes < 5000 ) parts = 0;
	if ( part == 0 || parts == 0 || part > parts ) {
		p_info->parts = 1;
		p_info->part = 0;
		return 0;
	} else { /* multipart articles */
		p_info->parts = parts;
		p_info->part = part;
		/* remove the (m/n) */
		while ( cp1[-1] == ' ' ) cp1--;
		for ( ++cp2; *cp2; cp2++ ) *cp1++ = *cp2;
	}
	*cp1 = '\0';
	p_info->subjlen = cp1-p_sub;
	return part;
}
