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
#ifndef HASH_H

#define HASH_H
typedef struct S_HASHENT HASHENT, *P_HASHENT;
typedef struct S_HASHTABLE HASHTABLE, *P_HASHTABLE;

P_HASHTABLE NewHashTable( char *name, int size, int flags );
P_HASHTABLE GrowHashTable( P_HASHTABLE p_hashtable, int size );
void FreeHashTable( P_HASHTABLE );
int HashWord( puchar word, int len, ushort extra );
P_HASHENT FindHash( P_HASHTABLE p_hashtable, puchar word, int len, ushort extra );
int DelHash( P_HASHTABLE p_hashtable, puchar word, int len, ushort extra );

struct S_HASHENT {
	union {
		void *ptr;
		int val;
		puchar string;
	} data;
	P_HASHENT next;
	ushort extra, len, entry, n_refs;
	uchar key[4]; /* longer to hold key */
};

struct S_HASHTABLE {
	int entries, size;
	char name[8];
	P_HASHENT *pentries;
};

/* flags */
#define H_EXTRA_HAS_ENTRIES 1

#endif
