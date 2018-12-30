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
#ifndef ARTDATA_H
#include "hash.h"

#define ARTDATA_H

enum { // extension identifiers
	EX_undef, EX_onepart, EX_multipart, EX_nfo, EX_sfv, EX_txt,
	EX_nzb, EX_log, EX_cue, EX_m3u, EX_flac, EX_pdf,
	EX_m4a, EX_mp3, EX_rar, EX_rar1, EX_par2, EX_zip,
	EX_jpg
};
#define EX_MIN EX_nfo

enum { /* field identifiers */
	F_Article = 0, F_Subj, F_From, F_Date,
	F_Msg, F_Ref, F_Bytes, F_Lines, F_Xref
};

enum { /* error returns */
	E_GroupSel = 1, E_ArtUnavail = 2, E_LongHdr = 3,
	E_TimeOut = 4, E_FileCreate = 5, E_WriteError = 6,
	E_FileOpen = 7, E_Cancel = 8, E_Full = 9,
	E_ConnErr = 10
};

enum { /* overview ctl bit defs */
	OV_Goback = 1, OV_Bg = 2, OV_Batch = 4
};

enum { /* article flag bit defs */
	AF_Kill = 1, AF_Loaded = 2, AF_HasRe = 4, AF_Author = 8,
	AF_Complete = 16, AF_IOMask = 7, AF_Cancel = 32,
	AF_BaseMatch = 64, AF_Matched = 128,
	AF_Get = 256, AF_B_Get = 512, AF_Incomplete = 1024,
	AF_Mask = 2047, AF_Dirty = 2048, AF_Filter = 4096,
	AF_Undel = 8192, AF_AuthSearch = 16384, AF_Add = 32768, AF_Unload = 65536
};
#define AF_Loads (AF_Loaded|AF_Get|AF_B_Get)
#define AF_Gets (AF_Get|AF_B_Get)

#define SUBJINFOLEN (3*sizeof(short))
enum { /* bit defs for SaveArticle how parm */
	AS_Article = 1, AS_OnePart = 2, AS_Stat = 4,
	AS_NoLoad = 8, AS_GetPartial = 16, AS_Kill = 32,
	AS_Binary = 64, AS_Foreground = 128, AS_Err = 256
};
enum { /* values for read and save group */
	GR_Current = 1, GR_Save = 2, GR_HaveSem = 4
};
#define EO_TimeOut 1
#define EO_Cancel 2

typedef struct S_EXTRAPART EXTRAPART, *P_EXTRAPART;
typedef struct S_PARTDATA PARTDATA, *P_PARTDATA;
typedef struct S_ARTICLE ARTICLE, *P_ARTICLE;
typedef struct S_SRVRDATA SRVRDATA, *P_SRVRDATA;
typedef struct S_GROUP GROUP, *P_GROUP;
typedef struct S_FILTER FILTER, *P_FILTER;
typedef struct S_OVDATA OVDATA, *P_OVDATA;
typedef struct S_ART_INFO ART_INFO, *P_ART_INFO;
typedef struct S_ART_DIR ART_DIR, *P_ART_DIR;
struct S_ART_INFO {
	uchar *subject, *author, *msg_id;
	short parts, file_ext, name_pos, ext_end;
	short part, flags, subjlen;
	short authlen, msglen, server;
	ARTNUM artnum;
	int bytes;
	time_t artdate, last_art;
};
struct S_ART_DIR {
	int nref, len;
	char *dir;
};

int InitGroups( int no_sync );
void FreeGroups();
int SetExtensions( char *in );
char *GetExtensions( P_ARTICLE in );
void InitArticles( );
void FreeArticles( );
void CheckHash( );
void Oops( char *msg, int lc );
P_GROUP AllocGroup( char *name );
void FreeGroup( P_GROUP p_group );
P_GROUP AddGroup( char *name, ARTNUM first, ARTNUM last, int servernum );
int DelGroup( P_GROUP p_group, int pos );
int ReadGroup( P_GROUP p_group, int reading );
int SaveGroup( P_GROUP p_group, int reading );
int ReadFilters( P_GROUP p_group, char *name );
int SaveFilters( P_GROUP p_group, char *name );
int ApplyFilters( P_GROUP p_group, int *p_first );
void AddFilter( P_GROUP p_group, char *pattern, int author );
void FreeFilters( P_GROUP p_group );
#define A2Art atoll
void FreeArticle( P_ARTICLE p_article, int ctl );
P_ARTICLE AllocArticle( P_ART_INFO p_info, P_GROUP p_group, P_ARTICLE p_in,
							uchar *subject, uchar *author );
P_ARTICLE AddArticle( P_ART_INFO p_info, P_GROUP p_group );
int AddPart( P_ART_INFO p_info, P_ARTICLE p_in );
int ReadParts( P_ARTICLE p_article );
int ParseSubject( P_ART_INFO p_info );
int MakeSubjInfo( P_ART_INFO p_info );
int IsPar2( P_ARTICLE p_in );
int IsNZB( P_ARTICLE p_in );
int ArticleProgress( P_GROUP p_group );
int SaveGroups( void );
int SortArticles( P_GROUP p_group, int mode );
int ThreadArticles( P_GROUP p_group );
int ArticleHandler( P_GROUP p_group, int ctl, int start );
int GetRetain( P_GROUP p_group, int lnum );
int RemoveDeletes( P_GROUP p_group, int start );
void HandleSigInt( int sig );
int GetGroupInfo( P_GROUP p_group, int server, int ctl );
int PostArt( char *groupname, P_ARTICLE p_article );
FILE *LoadGroups( int server, char *err_msg );
int BuildGroups( int server, char *err_msg );
int UpdateGroupInfo( int server );
int GetFields( P_CONNECTION p_conn );
void SetDefaultRetain( int val );
int SaveArticle( P_ARTICLE p_article, int part, int how, int thread,
								 P_ART_DIR p_dir );
int ReadArticle( P_ARTICLE p_article );
int NZBRead( P_ARTICLE p_article );
int ShowHelp( char *what );
int UpdateOverview( P_GROUP p_group, int ctl );
int FindMissing( P_GROUP p_group, int server );
int QueueArt( P_ARTICLE p_article, P_ART_DIR p_dir, int cmd );
int CancelDownloads( void );
void PrintCount( int count, char * what );
int PrintInfo( P_ARTICLE p_article, int lnum );

#ifndef ARTICLE_SEM
extern pthread_mutex_t article_sem;
#endif
#ifndef GROUP_SEM
extern pthread_mutex_t group_sem;
#endif
#endif
