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
typedef struct S_YDATA YDATA, *P_YDATA;
enum { ENC_Unknown = 0, ENC_Text, ENC_UUenc, ENC_Yenc, ENC_Mime_QP,
ENC_Mime_B64, ENC_Binary };

int GetYEncError( P_YDATA p_ydata, char *buf );
int GetEncoding( puchar in, char *name, int *encoding, int *len );
int DecodeLine( uchar *line, int len );
int UUDecodeLine( uchar *in, uchar *out, int len );
int B64DecodeLine( uchar *in, uchar *out, int len );
int QPDecodeLine( uchar *in, uchar *out, int len );
int YDecodeLine( P_YDATA p_ydata, uchar *in, uchar *out, int len );
int RestartPart( P_YDATA p_ydata );
P_YDATA InitYenc( int outfile, int partial );
int FreeYenc( P_YDATA p_ydata );
