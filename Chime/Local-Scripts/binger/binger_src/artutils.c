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
#include "hash.h"
#include "server.p"
#include <curses.h>
#include <ctype.h>

extern int topline, bottomline, num_scroll, lastline, COLS;
#define HASHGROWINC 16
#define ustrcmp( x, y ) strcmp( ((char*)x), ((char *)y) )
P_HASHTABLE art_hash = NULL, auth_hash = NULL;

void
InitArticles( ) {
	if ( art_hash != NULL ) return;
	art_hash = NewHashTable( "article", 7171, 0 );
	auth_hash = NewHashTable( "author", 717, 0 );
	SetExtensions( NULL );
}

void
FreeArticles( ) {
	if ( art_hash == NULL ) return;
	FreeHashTable( art_hash );
	FreeHashTable( auth_hash );
	art_hash = NULL;
	auth_hash = NULL;
}

void
CheckHash( ) {
	int new_size;
	pthread_mutex_lock( &article_sem );
	new_size = art_hash->size*HASHGROWINC;
	if ( new_size < art_hash->entries ) {
		art_hash = GrowHashTable( art_hash, new_size );
	} else new_size = 0;
	pthread_mutex_unlock( &article_sem );
	if ( new_size > 0 ) {
		LOG_Debug ( "grow article hash %d", new_size );
	}
	pthread_mutex_lock( &article_sem );
	new_size = auth_hash->size*HASHGROWINC;
	if ( new_size < auth_hash->entries ) {
		auth_hash = GrowHashTable( auth_hash, new_size );
	} else new_size = 0;
	pthread_mutex_unlock( &article_sem );
	if ( new_size > 0 ) {
		LOG_Debug ( "grow auth hash %d", new_size );
	}
}

int
ArtCmp( P_ARTICLE p1, P_ARTICLE p2, int mode ) {
	int l1, l2, len = 0;
	puchar s1 = p1->subject, s2 = p2->subject;
	puchar e1, e2;
	if ( p1->author != p2->author || p1->file_ext < EX_MIN || p2->file_ext < EX_MIN ) {
		if ( p1->file_ext == EX_onepart && p2->file_ext == EX_onepart ) {
			/* unknown file ext and only one part */
			l1 = l2 = 0;
			if ( (p1->flags&AF_HasRe) ) {
				for ( l1++, s1 += 4; IsWord( s1, "re: " ); s1 += 4 ) l1++;
			}
			if ( (p2->flags&AF_HasRe) ) {
				for ( l2++, s2 += 4; IsWord( s2, "re: " ); s2 += 4 ) l2++;
			}
			if (	ustrcmp( s1, s2 ) == 0 ) {
/* for mode > 0, compare on number of re: and then date */
				if ( mode == 0 ) return 0;
				if ( l1 != l2 ) return l1-l2;
				return p1->artdate - p2->artdate;
			}
		}
		/* different authors or at least one of the file exts is unknown */
		if ( ( l1 = ustrcmp( s1, s2 ) ) != 0 ) return l1;
		return p1->artdate - p2->artdate;
	}
/* same author, both file exts are known, attempt to compare for threading */
	if ( p1->name_pos != p2->name_pos )
		return ustrcmp( s1, s2 );
	e1 = p1->subject+p1->name_pos;
	e2 = p2->subject+p2->name_pos;
	while ( s1 < e1 && s2 < e2 ) {
		if ( *s1 == *s2 ) {
			s1++; s2++;
			len++;
		} else if ( isdigit( *s1 ) && isdigit( *s2 ) ) {
			for ( l1 = 1,s1++; isdigit( *s1 ); s1++ ) l1++;
			for ( l2 = 1,s2++; isdigit( *s2 ); s2++ ) l2++;
/* if more than 3 digits not a part/parts value so don't skip */
			len +=	( l1 < l2 ) ? l1 : l2;
		if ( l1 > 3 || l2 > 3 )
			return ustrcmp( s1-l1, s2-l2 );
		} else return *s1-*s2;
	}
	if ( p1->name_pos < 5 ) {
		s1 = p1->subject+p1->ext_end;
		s2 = p2->subject+p2->ext_end;
		while ( *s1 && *s1 != '\t' ) {
			len++;
			if ( *s1 == *s2 ) {
				s1++; s2++;
			} else if ( isdigit(*s1) && isdigit(*s2) ) {
	/* skip long numbers as could be byte count */
				do s1++; while ( isdigit(*s1) );
				do s2++; while ( isdigit(*s2) );
			} else return *s1-*s2;
		}
	}
	if ( mode == 0 ) {
		if ( len > 9 ) return 0; /* don't false thread yenc only */
	return ustrcmp( p1->subject, p2->subject );
	}
	if ( p1->file_ext != p2->file_ext )
		return p1->file_ext-p2->file_ext;
	s1 = p1->subject+p1->name_pos;
	s2 = p2->subject+p2->name_pos;
	e1 = p1->subject+p1->ext_end;
	e2 = p2->subject+p2->ext_end;
	while ( s1 < e1 && s2 < e2 ) {
		if ( *s1 != *s2 ) return *s1-*s2;
		s1++; s2++;
	}
	if ( s1 < e1 ) return 1;
	if ( s2 < e2 ) return -1;
	return 0;
}

int
ArtCompareThread( p1, p2 )
P_ARTICLE *p1, *p2;
{
	return ArtCmp( *p1, *p2, 0 );
}

int
ArtCompareSubj( p1, p2 )
P_ARTICLE *p1, *p2;
{
	return ArtCmp( *p1, *p2, 1 );
}

int
ArtCompareAuth( p1, p2 )
P_ARTICLE *p1, *p2;
{
	puchar a1 = (*p1)->author, a2 = (*p2)->author;
	if ( a1 != a2 )
		return ustrcmp( a1, a2 );
	return ArtCmp( *p1, *p2, 1 );
}

int
ArtCompareDate( p1, p2 )
P_ARTICLE *p1, *p2;
{
	return (*p1)->artdate - (*p2)->artdate;
}

int
SortArticles( P_GROUP p_group, int mode ) {
	int count = p_group->num_arts;
	P_ARTICLE *p_articles = p_group->p_articles;
	if ( count < 2 ) return count;
	switch( p_group->mode ) {
		case 2:
			qsort( (void *)p_articles, count, sizeof( P_ARTICLE ), ArtCompareDate );
			break;
		case 1:
			qsort( (void *)p_articles, count, sizeof( P_ARTICLE ), ArtCompareAuth );
			break;
		case 4:
			qsort( (void *)p_articles, count, sizeof( P_ARTICLE ), ArtCompareThread );
			break;
		default:
		 qsort( (void *)p_articles, count, sizeof( P_ARTICLE ), ArtCompareSubj );
	}
	return count;
}

int
ThreadArticles( P_GROUP p_group ) {
	int count, m = 0, n;
	P_ARTICLE p_this, p_prev, *p_articles;
	if ( p_group->delete_ctr > 0 ) {
		pthread_mutex_lock( &article_sem );
		RemoveDeletes( p_group, 0 );
		pthread_mutex_unlock( &article_sem );
	}
	count = p_group->num_arts;
	p_articles = p_group->p_articles;
	if ( count < 2 ) return count;
	SortArticles( p_group, 4 );
	p_prev = p_articles[0];
	for ( n = 1; n < count; p_prev = p_this, n++ ) {
		p_this = p_articles[n];
		if ( ArtCmp( p_this, p_prev, 0 ) != 0 )
			continue;
		while ( p_prev->p_next != NULL ) p_prev = p_prev->p_next;
		p_prev->p_next = p_this;
		p_articles[n] = NULL;
		while ( p_this->p_next != NULL ) p_this = p_this->p_next;
	}
	for ( n = 0; n < count && p_articles[n] != NULL; n++ ) ;
	for ( m = n; n < count; n++ ) {
		if ( p_articles[n] != NULL )
			p_articles[m++] = p_articles[n];
	}
	p_articles[m] = NULL;
	p_group->num_arts = m;
	SortArticles( p_group, p_group->mode );
	return m;
}

P_HASHTABLE
NewHashTable( char *name, int size, int flags ) {
	P_HASHTABLE p_hash = calloc( 1, sizeof( HASHTABLE ) );
	p_hash->size = size;
	strncpy( p_hash->name, name, 7 );
	p_hash->pentries = calloc( size, sizeof( P_HASHENT ) );
	return p_hash;
}

void FreeHashTable( P_HASHTABLE p_hashtable ) {
	P_HASHENT p_hash, p_next;
	int i;
	for ( i = 0; i < p_hashtable->size; i++ ) {
	p_hash = p_hashtable->pentries[i];
		for ( ; p_hash; p_hash = p_next ) {
			p_next = p_hash->next;
			free( p_hash );
			p_hashtable->entries--;
		}
	}
	free( p_hashtable->pentries );
	free( p_hashtable );
}

int
HashWord( puchar word, int len, ushort extra ) {
	puchar cp = word;
	int hashval = *cp++;
	if ( extra > 1 ) hashval *= extra;
	while ( --len > 0 ) {
		hashval *= 11;
		hashval += *cp++;
		hashval &= 0x7fffffff;
	}
	return hashval;
}

P_HASHTABLE
GrowHashTable( P_HASHTABLE p_hashtable, int newsize ) {
	P_HASHENT p_prev = NULL, p_hash, p_next, p_old;
	int i, len, hashval, cnt;
	P_HASHTABLE p_newhash = NewHashTable( p_hashtable->name, newsize, 0 );
	LOG_Debug (	"growing %s to %d\n",
		p_hashtable->name, p_hashtable->size );
	for ( cnt = 0; cnt < p_hashtable->size; cnt++ ) {
		p_old = p_hashtable->pentries[cnt];
		p_hashtable->pentries[cnt] = NULL;
		for ( ; p_old; p_old = p_next ) {
			p_next = p_old->next;
			len = p_old->len;
			hashval = HashWord( p_old->key, len, p_old->extra )%newsize;
			p_hash = p_newhash->pentries[hashval];
			for ( ; p_hash; p_hash = p_hash->next ) {
				i = len - p_hash->len;
				if ( i == 0 )
					i = memcmp( p_old->key, p_hash->key, len );
				if ( i >= 0 ) break;
				p_prev = p_hash;
			}
			if ( p_hash && i == 0 ) break;
			p_newhash->entries++;
			p_hashtable->entries--;
			if ( p_hash ) p_old->next = p_hash;
						else p_old->next = NULL;
			if ( p_prev ) p_prev->next = p_old;
			else p_newhash->pentries[hashval] = p_old;
		}
	}
	if ( p_hashtable->entries > 0 ) {
		fprintf( stderr, "growhash for %s failed, entries = %d\n",
			p_hashtable->name, p_hashtable->entries );
		exit( 128 );
	}
	free( p_hashtable );
	return p_newhash;
}

P_HASHENT
FindHash( P_HASHTABLE p_hashtable, puchar word, int len, ushort extra ) {
	P_HASHENT p_prev = NULL, p_hash, p_new;
	int i, hashval = HashWord( word, len, extra )%p_hashtable->size;
	p_hash = p_hashtable->pentries[hashval];
	for ( ; p_hash; p_hash = p_hash->next ) {
		i = len - p_hash->len;
		if ( i == 0 )
			i = memcmp( word, p_hash->key, len );
		if ( i >= 0 ) break;
		p_prev = p_hash;
	}
	if ( p_hash && i == 0 )
		return p_hash;
	p_hashtable->entries++;
	i = sizeof( HASHENT)+len-3; /* key is char[4] */
	p_new = malloc( i );
	p_new->entry = (ushort)(p_hashtable->entries);
	p_new->extra = extra;
	p_new->n_refs = 1;
	memcpy( p_new->key, word, len+1 );
	p_new->len = len;
	p_new->data.ptr = NULL;
	 p_new->next = p_hash;
	if ( p_prev ) p_prev->next = p_new;
	else p_hashtable->pentries[hashval] = p_new;
	return p_new;
}

int DelHash( P_HASHTABLE p_hashtable, puchar word, int len, ushort extra ) {
	P_HASHENT p_prev = NULL, p_hash, p_first;
	int i;
	int hashval = HashWord( word, len, extra )%p_hashtable->size;
	p_hash = p_hashtable->pentries[hashval];
	p_first = p_hash;
	for ( ; p_hash; p_hash = p_hash->next ) {
		i = len - p_hash->len;
		if ( i == 0 )
			i = memcmp( word, p_hash->key, len );
		if ( i >= 0 ) break;
		p_prev = p_hash;
	}
	if ( i != 0 || p_hash == NULL )
		return -1;
	if ( p_hash == p_first )
		p_hashtable->pentries[hashval] = p_hash->next;
	else p_prev->next = p_hash->next;
	free( p_hash );
	return --(p_hashtable->entries);
}

int
PrintInfo( P_ARTICLE p_in, int lnum ) {
 P_GROUP p_group = NULL;
	P_ARTICLE p_article = p_in->p_xpost;
	P_HASHENT p_art_hash, p_auth_hash;
	ushort a_hash;
	int i = bottomline;
	if ( p_in->parts > 1 )
		a_hash = p_in->parts;
	else a_hash = (ushort)(p_in->artdate&0xffff);
	p_auth_hash = FindHash( auth_hash, p_in->author, p_in->authlen, 0 );
	p_art_hash = FindHash( art_hash, p_in->subject, p_in->subjlen, a_hash );
	if ( p_art_hash->data.ptr == NULL ) return -1;
	mvprintw( lastline-2, 0, "art hash %hu refs %hu, auth hash %hu refs %hu, artflags %u",
		p_art_hash->entry, p_art_hash->n_refs, p_auth_hash->entry,
		p_auth_hash->n_refs, p_in->flags );
	while ( p_article != p_in ) {
		p_group = p_article->p_group;
		mvprintw( ++i, 0, "also in %s, flags %u", p_group->name, p_article->flags );
		if ( i > lastline ) break;
		p_article = p_article->p_xpost;
	}
	move( lnum, 0 );
	return 0;
}

P_ARTICLE
AllocArticle( P_ART_INFO p_info, P_GROUP p_group, P_ARTICLE p_in,
							uchar *subject, uchar *author ) {
	P_ARTICLE p_article;
	int i;
	short flags = (p_info->flags&(AF_Loads|AF_Kill));
	p_article = calloc( 1, sizeof(ARTICLE) );
	p_article->p_group = p_group;
	if ( p_in != NULL ) {
		flags |= (p_in->flags&(AF_Loads|AF_Kill));
		p_article->subject= p_in->subject;
		p_article->author= p_in->author;
		p_article->artdate= p_in->artdate;
		p_article->parts= p_in->parts;
		p_article->file_ext= p_in->file_ext;
		p_article->name_pos= p_in->name_pos;
		p_article->ext_end= p_in->ext_end;
		p_article->p_xpost = p_in->p_xpost;
		p_in->p_xpost = p_article;
		p_article->subjlen= p_in->subjlen;
		p_article->authlen= p_in->authlen;
	} else {
		p_article->subject= subject;
		p_article->author= author;
		p_article->p_xpost = p_article;
		if ( p_info->file_ext == 0 ) MakeSubjInfo( p_info );
		p_article->artdate= p_info->artdate;
		p_article->parts= p_info->parts;
		p_article->file_ext= p_info->file_ext;
		p_article->name_pos= p_info->name_pos;
		p_article->ext_end= p_info->ext_end;
		p_article->subjlen= p_info->subjlen;
		p_article->authlen= p_info->authlen;
	}
	p_article->flags = flags;
	if ( (flags&AF_Kill) ) {
		if ( !p_group->p_dels )
			p_group->p_dels = p_article;
		else p_group->p_lastdel->p_next = p_article;
		p_group->p_lastdel = p_article;
		return p_article;
	}
	if ( p_group->num_arts >= p_group->max_arts ) {
		if ( p_group->max_arts < 4096 )
			p_group->max_arts += 512;
		else if ( p_group->max_arts < 65536 )
			p_group->max_arts += 4096;
		else p_group->max_arts += 65536;
		i = sizeof( P_ARTICLE )*(p_group->max_arts+1);
		p_group->p_articles = realloc( p_group->p_articles, i );
	}
	p_group->p_articles[p_group->num_arts++] = p_article;
	return p_article;
}

void
FreeArticle( P_ARTICLE p_article, int ctl ) {
	int i;
	while ( p_article != NULL ) {
		P_ARTICLE p_next = p_article->p_next;
		if ( p_article->msg_id != NULL ) {
			for ( i = 0; i < p_article->parts; i++ ) {
				if ( p_article->msg_id[i] != NULL )
					free( p_article->msg_id[i] );
			}
			free( p_article->msg_id );
		}
		if ( p_article->p_parts != NULL ) free( p_article->p_parts );
		if ( p_article->p_extras != NULL ) free( p_article->p_extras );
		if ( (ctl&1) && p_article->subject != NULL ) free( p_article->subject );
		free( p_article );
		p_article = p_next;
	}
	}

P_ARTICLE
AddArticle( P_ART_INFO p_info, P_GROUP p_group ) {
	P_ARTICLE p_article;
	P_HASHENT p_art_hash, p_auth_hash;
	ushort a_hash = 0;
	if ( p_info->parts > 1 )
		a_hash = p_info->parts;
	pthread_mutex_lock( &article_sem );
	p_auth_hash = FindHash( auth_hash, p_info->author, p_info->authlen, 0 );
	p_art_hash = FindHash( art_hash, p_info->subject, p_info->subjlen, a_hash );
	if ( p_art_hash->data.ptr == NULL ) {
		p_article = AllocArticle( p_info, p_group, NULL, p_art_hash->key,
			p_auth_hash->key );
		p_art_hash->data.ptr = (void *)p_article;
		pthread_mutex_unlock( &article_sem );
		return p_article;
	}
	p_article = (P_ARTICLE)p_art_hash->data.ptr;
	if ( p_article->author != (uchar *)p_auth_hash->key ) {
		p_article = AllocArticle( p_info, p_group, NULL, p_art_hash->key,
			p_auth_hash->key );
		p_art_hash->n_refs++;
		p_auth_hash->n_refs++;
		pthread_mutex_unlock( &article_sem );
		return p_article;
	}
	if ( p_group != p_article->p_group ) {
		P_ARTICLE p_next = p_article->p_xpost;
		while ( p_next != p_article ) {
			if ( p_next->p_group == p_group ) break;
			p_next = p_next->p_xpost;
		}
		if ( p_next->p_group != p_group ) {
			p_article = AllocArticle( p_info, p_group, p_article, NULL, NULL );
			p_art_hash->n_refs++;
			p_auth_hash->n_refs++;
		} else p_article = p_next;
	}
	pthread_mutex_unlock( &article_sem );
	return p_article;
}

int ReadParts( P_ARTICLE p_article ) {
	P_GROUP p_group = p_article->p_group;
	int parts = p_article->parts, size = sizeof(PARTDATA);
	p_article->p_parts = calloc( parts, size );
	if ( fseek( p_group->p_stream, p_article->parts_pos, SEEK_SET ) < 0 )
		Oops( "parts seek error", p_article->parts_pos );
	size = fread( p_article->p_parts, size, parts, p_group->p_stream );
	if ( size < 0 ) return size;
	return 0;
}

int AddPart( P_ART_INFO p_info, P_ARTICLE p_article ) {
	P_PARTDATA p_part;
P_EXTRAPART p_extra, p_prev = NULL, p_next;
	P_GROUP p_group = p_article->p_group;
	int i, sn, server = p_info->server;
	P_SRVRDATA p_srvrdata = &p_group->srvrdata[server];
	short part = p_info->part;
	if ( part > 0 ) part--;
	if ( part < 0 || part >= p_info->parts ) return -1;
	pthread_mutex_lock( &article_sem );
	if ( p_info->artdate < p_article->artdate )
		p_article->artdate = p_info->artdate;
	if ( p_srvrdata->lowest > p_info->artnum )
		p_srvrdata->lowest = p_info->artnum;
	if ( p_srvrdata->highest < p_info->artnum )
		p_srvrdata->highest = p_info->artnum;
	if ( NULL == p_article->p_parts ) {
		if ( p_article->parts_pos > 0 ) ReadParts( p_article );
		else p_article->p_parts = calloc( p_info->parts, sizeof(PARTDATA) );
	} else if ( part >= p_article->parts ) {
		int i = p_info->parts*sizeof(PARTDATA);
		p_article->p_parts = realloc( p_article->p_parts, i );
		i = (p_info->parts-p_article->parts)*sizeof(PARTDATA);
		p_part = p_article->p_parts + p_article->parts;
		bzero( p_part, i );
	}
	p_part = p_article->p_parts + part;
	p_article->flags |= AF_Dirty;
	if ( p_part->artnum == 0 ) {
		p_part->artnum = p_info->artnum;
		p_part->bytes = p_info->bytes;
		p_part->server = server;
		p_article->bytes[server] += p_info->bytes;
		p_part->next = 0;
		if ( (p_group->flags&GF_MsgIds) && p_info->msglen > 0 ) {
			if ( p_article->msg_id == NULL )
			p_article->msg_id = calloc( sizeof(uchar *), p_info->parts );
			p_article->msg_id[part] = (puchar)strdup( (char *)p_info->msg_id );
		}
		if ( ++(p_article->got_parts) == p_article->parts )
			p_article->flags |= AF_Complete;
		goto done;
	}
	p_extra = p_article->p_extras;
	p_next = (P_EXTRAPART)p_part;
	do {
		sn = p_next->server;
		if ( server <= sn ) break;
		p_prev = p_next;
		p_next =	( p_next->next > 0 ) ? p_extra+(p_next->next)-1 : NULL;
	} while ( p_next );
	if ( !p_next ) sn = MAXSERVERS;
	if ( server == sn ) goto done; /* duplicate */
	if ( p_article->extras >= p_article->maxextras ) {
		p_article->maxextras += 32;
		i = p_article->maxextras*sizeof( EXTRAPART );
		p_article->p_extras = realloc( p_article->p_extras, i );
	}
	p_extra = p_article->p_extras+p_article->extras;
	p_article->extras++;
	if ( !p_prev ) { /* move part to extra and insert before */
		p_extra->artnum = p_part->artnum;
		p_extra->server = p_part->server;
		p_article->bytes[p_part->server] -= p_info->bytes;
		p_part->artnum = p_info->artnum;
		p_part->server = server;
		p_article->bytes[server] += p_info->bytes;
		p_part->next = p_article->extras;
	} else {
		p_extra->artnum = p_info->artnum;
		p_extra->server = server;
		p_extra->next = p_prev->next;
		p_prev->next = p_article->extras;
	}
done:
	pthread_mutex_unlock( &article_sem );
	return 0;
}
