#include "artdata.h"

struct S_EXTRAPART {
	ARTNUM artnum;
	ushort server, next;
};
struct S_PARTDATA {
	ARTNUM artnum;
	ushort server, next;
//	ushort lines, extras;
	int bytes;
};

struct S_ARTICLE {
	uchar *subject, *author;
	time_t artdate;
	long long parts_pos;
	short parts, file_ext, name_pos, ext_end;
	short got_parts, flags, extras, maxextras;
short subjlen, authlen;
	P_ARTICLE p_next, p_xpost;
	P_GROUP p_group;
	uchar **msg_id;
	int bytes[MAXSERVERS];
	P_EXTRAPART p_extras;
	P_PARTDATA p_parts;
};

enum {
	UP_Force = 1, UP_Change = 2, UP_Check = 3
};
enum { /* group flags bit defs */
	GF_Saving = 1, GF_Reading = 2, GF_IO = 3, GF_Read = 4, GF_MsgIds = 8,
	GF_HaveData = 16, GF_SaveRequest = 32, GF_FilterMod = 64, GF_BackDated = 128,
	GF_Subscribed = 256, GF_Cancel = 512, GF_Temp = 1024, GF_NZB = 2048
};

struct S_FILTER {
	P_FILTER p_next;
	uchar pattern[4];
};

struct S_SRVRDATA {
	ARTNUM lowest, highest, first, last, start_adjust;
	time_t last_ov;
};

struct S_GROUP {
	char *name;
	int num_arts, max_arts, delete_ctr;
	short retain, subjtype, bg_downloads, flags;
	short ov_running, ov_req, mode, bg_errors;
	FILE *p_stream;
	time_t base_time;
	P_ARTICLE *p_articles, p_dels, p_lastdel;
	P_FILTER p_filter;
	SRVRDATA srvrdata[1];
};

int SelectGroup( P_CONNECTION p_conn, P_GROUP p_group, int upd_ctl );
int ArtCmp( P_ARTICLE p1, P_ARTICLE p2, int mode );
