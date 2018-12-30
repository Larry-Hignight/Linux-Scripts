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
struct DATE_DATA {
	char name[6], type;
	int value;
};

struct DATE_DATA date_info[] = {
	{ "adt", 't', -400 },
	{ "akdt", 't', -900 },
	{ "akst", 't', -900 },
	{ "apr", 'm', 4 },
	{ "ast", 't', -400 },
	{ "aug", 'm', 8 },
	{ "bst", 't', 0 },
	{ "cadt", 't', 930 },
	{ "cast", 't', 930 },
	{ "cct", 't', 800 },
	{ "cdt", 't', -600 },
	{ "ces", 't', 100 },
	{ "cest", 't', 100 },
	{ "cet", 't', 100 },
	{ "cst", 't', -600 },
	{ "cut", 't', 0 },
	{ "dec", 'm', 12 },
	{ "eadt", 't', 1000 },
	{ "east", 't', 1000 },
	{ "edt", 't', -400 },
	{ "eet", 't', 200 },
	{ "est", 't', -500 },
	{ "feb", 'm', 2 },
	{ "fri", 'd', 6 },
	{ "gmt", 't', 0 },
	{ "hadt", 't', -1000 },
	{ "hast", 't', -1000 },
	{ "hkt", 't', 800 },
	{ "hst", 't', -1000 },
	{ "jan", 'm', 1 },
	{ "jst", 't', 900 },
	{ "jul", 'm', 7 },
	{ "jun", 'm', 6 },
	{ "kdt", 't', 900 },
	{ "kst", 't', 900 },
	{ "mar", 'm', 3 },
	{ "may", 'm', 5 },
	{ "mdt", 't', -700 },
	{ "met", 't', 100 },
	{ "mez", 't', 100 },
	{ "mezt", 't', 100 },
	{ "mon", 'd', 2 },
	{ "msd", 't', 300 },
	{ "msk", 't', 300 },
	{ "mst", 't', -700 },
	{ "ndt", 't', -330 },
	{ "nov", 'm', 11 },
	{ "nst", 't', -330 },
	{ "nzdt", 't', 1200 },
	{ "nzst", 't', 1200 },
	{ "oct", 'm', 10 },
	{ "pdt", 't', -800 },
	{ "pst", 't', -800 },
	{ "sat", 'd', 7 },
	{ "sep", 'm', 9 },
	{ "sun", 'd', 1 },
	{ "thu", 'd', 5 },
	{ "tue", 'd', 3 },
	{ "ut", 't', 0 },
	{ "utc", 't', 0 },
	{ "wadt", 't', 800 },
	{ "wast", 't', 800 },
	{ "wed", 'd', 4 },
	{ "wet", 't', 0 },
	{ "ydt", 't', -900 },
	{ "yst", 't', -900 },
	{ "z", 't', 0 },
	{ "zzz", 'z', 0 }
};
