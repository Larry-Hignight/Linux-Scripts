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
enum {
	NZ_Init = 0, NZ_Nzb, NZ_File, NZ_Groups, NZ_Group, NZ_Segs, NZ_Seg
};

enum { NZ_Unknown = -3, NZ_BadTag, NZ_Comment, NZ_End = 0, NZ_Start, NZ_WithAttr = 4, NZ_Text = 8 };

int	FindTag( char **in, char **attribVals, int *tagState );
char *GetText( char *in, char *buf, int size, int *p_chars );
int ProcessNZB( char *in );
int ProcessNZBFile( char *name );
