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
#include "decoders.h"
#include <ctype.h>
#include <stdarg.h>

#define ENC_QP -1
#define ENC_B64 -2
typedef unsigned int	crc32_t;
#define	CRC_START 0xFFFFFFFF
#define	CRC_FINISH(_Y_crc)		(_Y_crc ^= 0xFFFFFFFF)
#define	CRC_UPDATE(_Y_crc,_Y_oct)	((_Y_crc) = crc32_tab[((_Y_crc)^(_Y_oct)) & 0xff] ^ (((_Y_crc) >> 8) & 0x00FFFFFF))

crc32_t crc32_tab[] = { /* CRC polynomial 0xedb88320 */
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

struct S_YDATA {
	crc32_t crc32, crc_hold, pcrc32;
	int partSize, currentBytes, filePos;
	int lineSize, part, parts;
	int totalSize, state, outfile;
	int partial, errors;
	char msg[80];
};

int
GetYEncError( P_YDATA p_ydata, char *buf ) {
	char *cp = p_ydata->msg, *cp1= buf;
	if ( *cp == '\0' && p_ydata->state != 0 )
		cp = "no yend found";
	while ( *cp ) *cp1++ = *cp++;
	*cp1 = '\0';
	*p_ydata->msg = '\0';
	p_ydata->state = 0;
	return cp1-buf;
}

static void
YEncError( P_YDATA p_ydata, char *msg, ... ) {
	va_list arg;
	va_start( arg, msg );
	vsprintf( p_ydata->msg, msg, arg );
	va_end( arg );
	p_ydata->errors++;
}

P_YDATA
InitYenc( int outfile, int partial	) {
	P_YDATA p_ydata = calloc( 1, sizeof( YDATA ) );
	p_ydata->pcrc32 = CRC_START;
	p_ydata->crc32 = CRC_START;
	p_ydata->crc_hold = CRC_START;
	p_ydata->outfile = outfile;
	p_ydata->partial = partial;
	return p_ydata;
}

int
FreeYenc(P_YDATA p_ydata ) {
	int errs = p_ydata->errors;
	free( p_ydata );
	return errs;
}

static int
DoBegin(P_YDATA p_ydata, char *in ) {
	char *cp1, key[16];
	while ( ( in = NextToken( in, key, &cp1 ) ) ) {
		if ( !cp1 ) continue;
		if ( !strcmp( key, "part" ) )
			p_ydata->part = atoi( cp1 );
		else if ( !strcmp( key, "total" ) )
			p_ydata->parts = atoi( cp1 );
		else if ( !strcmp( key, "size" ) )
			p_ydata->totalSize = atoi( cp1 );
		else if ( !strcmp( key, "name" ) ) break;
	}
	p_ydata->state = 1;
	if ( p_ydata->parts == 0 ) p_ydata->state = 2;
	return 0;
}

static int
NewPart(P_YDATA p_ydata, char *in ) {
	char *cp1, key[16];
	int val;
	p_ydata->pcrc32 = CRC_START;
	p_ydata->crc_hold = p_ydata->crc32;
	while ( ( in = NextToken( in, key, &cp1 ) ) ) {
		if ( !strcmp( key, "begin" ) ) {
			val = atoi( cp1	 )-1;
			if ( val == p_ydata->filePos ) continue;
			if ( p_ydata->partial ) {
				if ( lseek( p_ydata->outfile, val, SEEK_SET ) == -1 )
					YEncError( p_ydata, "seek error %d to pos %d", errno, val );
				else p_ydata->filePos = val;
			} else {
				YEncError( p_ydata, "part start says	%d file pos %d", val, p_ydata->filePos );
			}
		}
	}
	p_ydata->state = 2;
	return 0;
}

static int
FinishPart( P_YDATA p_ydata, char *in ) {
	char key[16], *cp1;
	crc32_t temp_crc;
	while ( ( in = NextToken( in, key, &cp1 ) ) ) {
		if ( !strcmp( key, "crc32" ) ) {
			CRC_FINISH( p_ydata->crc32 );
			temp_crc = (crc32_t)strtoll( cp1, NULL, 16 );
			if ( !p_ydata->partial && p_ydata->crc32 != temp_crc )
				YEncError( p_ydata, "file crc, expect %x, got %x",
					temp_crc, p_ydata->crc32 );
		} else if ( !strcmp( key, "pcrc32" ) ) {
			CRC_FINISH( p_ydata->pcrc32 );
			temp_crc = (crc32_t)strtoll( cp1, NULL, 16 );
			if ( p_ydata->pcrc32 != temp_crc )
				YEncError( p_ydata, "part crc, expect %x, got %x",
					temp_crc, p_ydata->pcrc32 );
		} else if ( !strcmp( key, "part" ) ) {
			p_ydata->part = atoi( cp1 );
		} else if ( !strcmp( key, "size" ) ) {
			p_ydata->partSize = atoi( cp1 );
			if ( p_ydata->partSize != p_ydata->currentBytes )
				YEncError( p_ydata, "part bytes read, %d, expect %d", p_ydata->partSize,
					p_ydata->currentBytes );
		}
	}
	p_ydata->filePos += p_ydata->currentBytes;
	p_ydata->currentBytes = 0;
	p_ydata->state = 0;
	return 0;
}

int
RestartPart(P_YDATA p_ydata ) {
	int val = p_ydata->filePos;
	p_ydata->pcrc32 = CRC_START;
	p_ydata->crc32 = p_ydata->crc_hold;
	p_ydata->currentBytes = 0;
	p_ydata->state = 2;
	if ( lseek( p_ydata->outfile, val, SEEK_SET ) == -1 )
		YEncError( p_ydata, "seek error %d to pos %d", errno, val );
	return 0;
}

int
YDecodeLine( P_YDATA p_ydata, uchar *in, uchar *out, int len ) {
	uchar ch, *cp = out;
	if ( *in == '=' && in[1] == 'y' ) {
		in += 2;
		if ( IsWord( in, "end " ) ) return FinishPart( p_ydata, (char *)in );
		if ( IsWord( in, "begin " ) ) return DoBegin( p_ydata, (char *)in );
		if ( IsWord( in, "part" ) ) return NewPart( p_ydata, (char *)in );
		if ( len > 60 ) len = 60;
		in[len] = '\0';
		YEncError( p_ydata, "unknown =y%s", in );
		return 0;
	} else if ( p_ydata->state != 2 )
		return 0;
	for ( ; ( ch = *in ); in++ ) {
		if ( ch == '=' ) {
			ch = *( ++in );
			ch -= 64;
		}
		ch -= 42;
		*cp++ = ch;
		CRC_UPDATE( p_ydata->crc32, ch );
		CRC_UPDATE( p_ydata->pcrc32, ch );
	}
	len = cp-out;
	p_ydata->currentBytes += len;
	return len;
}

int
UUDecodeLine( uchar *in, uchar *out, int len ) {
	uchar i0, i1, i2, i3, ch;
	uchar *p_end = in+len, *cp = out;
	int rem;
	if ( len < 2 ) return 0;
	if ( len == 3 && IsWord( in, "end" ) ) return 0;
	if ( in[5] == ' ' && IsWord( in, "begin" ) ) return 0;
	rem = *in++;
	if ( rem < 32 ) return -1;
	rem -= 32;
	while ( in < p_end ) {
		i0 = ((*in++)-32)&63;
		i1 = ((*in++)-32)&63;
		ch = i0<<2 | i1>>4;
		*cp++ = ch;
		if ( --rem <= 0 ) break;
		i2 = ((*in++)-32)&63;
		ch = i1<<4 | i2>>2;
		*cp++ = ch;
		if ( --rem <= 0 ) break;
		i3 = ((*in++)-32)&63;
		ch = i2<<6 | i3;
		*cp++ = ch;
		if ( --rem <= 0 ) break;
	}
	return cp-out;
}

#define XX 255
static uchar index_b64[256] = {
	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,
	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,
	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, 62,	XX, XX, XX, 63,
	52, 53, 54, 55,	56, 57, 58, 59,	60, 61, XX, XX,	XX, XX, XX, XX,
	XX,	0,	1,	2,	 3,	4,	5,	6,	 7,	8,	9, 10,	11, 12, 13, 14,
	15, 16, 17, 18,	19, 20, 21, 22,	23, 24, 25, XX,	XX, XX, XX, XX,
	XX, 26, 27, 28,	29, 30, 31, 32,	33, 34, 35, 36,	37, 38, 39, 40,
	41, 42, 43, 44,	45, 46, 47, 48,	49, 50, 51, XX,	XX, XX, XX, XX,
	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,
	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,
	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,
	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,
	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,
	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,
	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,
	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,	XX, XX, XX, XX,
};

int
B64DecodeLine( uchar *in, uchar *out, int len ) {
	uchar ch1, ch2, ch3, ch4, *cp = out;
	if ( len < 2 ) return 0;
	while ( *in && *in != '=' ) {
		ch1 = index_b64[*in++];
		if ( ch1 == XX ) continue;
		do {
			if ( !*in || *in == '=' ) goto dbl_break;
			ch2 = index_b64[*in++];
		} while ( ch2 == XX );
		*cp++ = ( ch1 << 2 ) | ( ch2 >> 4 );
		do {
			if ( !*in || *in == '=' ) goto dbl_break;
			ch3 = index_b64[*in++];
		} while ( ch3 == XX );
		*cp++ = ( ( ch2 & 0x0f ) << 4 ) | ( ch3 >> 2 );
		do {
			if ( !*in || *in == '=' ) goto dbl_break;
			ch4 = index_b64[*in++];
		} while ( ch4 == XX );
		*cp++ = ( ( ch3 & 0x03 ) << 6 ) | ch4;
	}
	dbl_break:
	*cp = '\0';
	return cp-out;
}

#undef XX

int
QPDecodeLine( uchar *in, uchar *out, int len ) {
	uchar ch, ch1, ch2, *cp = out, *p_end = in+len;
	while ( in < p_end && ( ch = *in++ ) ) {
		if ( ch != '=' ) {
			*cp++ = ch;
			continue;
		}
		ch1 = *in++;
		if ( ch1 == '\0' ) {
			*cp++ = '\n';
			continue;
		}
		ch2 = *in++;
		if ( !(isxdigit(ch1)) || !(isxdigit(ch2)) ) break;
		if ( ch1 <= '9' ) ch = (ch1&15)<<4;
		else ch = ((ch1+9)&15)<<4;
		if ( ch2 <= '9' ) ch1 = ch2&15;
		else ch1 = ((ch2+9)&15);
		ch |= ch1;
		*cp++ = ch;
	}
	*cp = '\0';
	return cp-out;
}

int
DecodeLine( uchar *line, int len ) {
	puchar cp1 = line+3, cp2=line+len;
	uchar ch;
	int l;
	if ( len < 16 || !IsWord( line, "=?" ) ) return len;
	while ( *cp1++ != 63 ) {
		if ( cp1 > line+16 ) return len;
	}
	ch = *cp1++;
	if ( *cp1++ != '?' ) return len;
	while ( --cp2 > cp1 ) {
		if ( !strchr( "?=_-", *cp2 ) ) break;
	}
	l = (++cp2)-cp1;
	if ( ch == 'B' )
		return B64DecodeLine( cp1, line, l );
	if ( ch == 'Q' )
		return QPDecodeLine( cp1, line, l );
	return len;
}

int
GetEncoding( puchar in, char *name, int *encoding, int *len ) {
	puchar cp, np = (puchar)name;
	if ( *len < 12 ) {
		if ( *encoding == ENC_QP ) *encoding = ENC_Mime_QP;
		else if ( *encoding == ENC_B64 ) *encoding = ENC_Mime_B64;
		return *encoding;
	}
	*name = '\0';
	if ( IsWord( in, "begin " ) && in[9] == ' ' && atoi( (char *)(in+6) ) >= 400 ) {
		for ( cp = in+10; *cp && *cp == ' '; cp++ );
		if ( *cp ) {
			*encoding = ENC_UUenc;
			goto make_name1;
		}
	}
	if ( IsWord( in, "=ybegin " ) && ( cp = FindLast( in, '=' ) ) &&
			IsWord( cp-4, "name" ) ) {
		*encoding = ENC_Yenc;
		goto make_name;
	}
	if ( IsWord( in, "name=" ) ) {
		cp = in+5;
		goto make_name;
	}
	if ( !IsWord( in, "content-" ) ) return *encoding;
	cp = in+8;
	if ( IsWord( cp, "transfer-encoding" ) ) {
		if ( ( cp = FindLast( in, ' ' ) ) ) {
			if ( IsWord( ++cp, "quoted-printable" ) ) *encoding = ENC_QP;
			else if ( IsWord( cp, "base64" ) ) *encoding = ENC_B64;
		}
		return *encoding;
	}
	if ( !( IsWord( cp, "type" ) || IsWord( cp, "disposition" ) ) )
		return *encoding;
	if ( ( cp = FindLast( in, '=' ) ) && IsWord( cp-4, "name" ) ) {
make_name:
	cp++;
		if ( *cp == '"' ) cp++;
/* do a little name fixing */
make_name1:
		if ( *cp == '.' ) *np++ = '_'; /* for .par2 with no name */
		for ( ;*cp && *cp != '"'; cp++ ) {
			if ( *cp == '/' || *cp == '~' ) continue;
			*np++ = *cp;
		}
		*np = '\0';
	}
	return *encoding;
}
