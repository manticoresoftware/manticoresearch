//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include <string.h>

struct stem_table
{
	int len, nremove, nadd;
	BYTE suffix[10], add[3];
};

static stem_table en_except[] =
{
	{ 5,	3, 1, "skies",		"y-" },
	{ 5,	4, 2, "dying",		"ie" },
	{ 5,	4, 2, "lying",		"ie" },
	{ 5,	4, 2, "tying",		"ie" },
	{ 7,	1, 0, "innings",	"--" },
	{ 7,	1, 0, "outings",	"--" },
	{ 8,	1, 0, "cannings",	"--" },
	{ 4,	1, 0, "idly",		"--" },
	{ 6,	1, 0, "gently",		"--" },
	{ 4,	1, 1, "ugly",		"i-" },
	{ 5,	1, 1, "early",		"i-" },
	{ 4,	1, 1, "only",		"i-" },
	{ 6,	1, 0, "singly",		"--" },
	{ 3,	-1, 0, "sky",		"--" },
	{ 4,	-1, 0, "news",		"--" },
	{ 4,	-1, 0, "howe",		"--" },
	{ 6,	-1, 0, "inning",	"--" },
	{ 6,	-1, 0, "outing",	"--" },
	{ 7,	-1, 0, "canning",	"--" },
	{ 7,	-1, 0, "proceed",	"--" },
	{ 6,	-1, 0, "exceed",	"--" },
	{ 7,	-1, 0, "succeed",	"--" }
};

static stem_table en_step2[] =
{
	{ 4, 1, 1, "enci",		"e-" },
	{ 4, 1, 1, "anci",		"e-" },
	{ 4, 1, 1, "abli",		"e-" },
	{ 3, 1, 1, "bli",		"e-" },
	{ 5, 2, 0, "entli",		"--" },
	{ 5, 3, 0, "aliti",		"--" },
	{ 5, 2, 0, "ousli",		"--" },
	{ 5, 3, 1, "iviti",		"e-" },
	{ 6, 5, 2, "biliti",	"le" },
	{ 5, 2, 0, "fulli",		"--" },
	{ 6, 2, 0, "lessli",	"--" },

	{ 7, 5, 1, "ational",	"e-" },
	{ 6, 2, 0, "tional",	"--" },

	{ 5, 3, 0, "alism",		"--" },

	{ 7, 5, 1, "ization",	"e-" },
	{ 5, 3, 1, "ation",		"e-" },

	{ 4, 1, 0, "izer",		"--" },
	{ 4, 2, 1, "ator",		"e-" },

	{ 7, 4, 0, "fulness",	"--" },
	{ 7, 4, 0, "ousness",	"--" },
	{ 7, 4, 0, "iveness",	"--" }
};

static stem_table en_step3[] =
{
	{ 5, 3, 0, "alize",	"--" },
	{ 5, 3, 0, "icate",	"--" },
	{ 5, 5, 0, "ative",	"--" },

	{ 5, 3, 0, "iciti",	"--" },

	{ 4, 2, 0, "ical",	"--" },
	{ 3, 3, 0, "ful",	"--" },

	{ 4, 4, 0, "ness",	"--" },
};

static stem_table en_step4[] =
{
	{ 2, 2, 0, "ic",	"--" },
	{ 4, 4, 0, "ance",	"--" },
	{ 4, 4, 0, "ence",	"--" },
	{ 4, 4, 0, "able",	"--" },
	{ 4, 4, 0, "ible",	"--" },
	{ 3, 3, 0, "ate",	"--" },
	{ 3, 3, 0, "ive",	"--" },
	{ 3, 3, 0, "ize ",	"--" },
	{ 3, 3, 0, "iti",	"--" },
	{ 2, 2, 0, "al",	"--" },
	{ 3, 3, 0, "ism",	"--" },
	{ 2, 2, 0, "er",	"--" },
	{ 3, 3, 0, "ous",	"--" },
	{ 5, 5, 0, "ement",	"--" },
	{ 4, 4, 0, "ment",	"--" },
	{ 3, 3, 0, "ant",	"--" },
	{ 3, 3, 0, "ent",	"--" },
};

static BYTE stem_en_vowels[] = "aeiouy";
static BYTE stem_en_vwxy[] = "aeiouywxY";
static BYTE stem_en_doubles[] = "bdfgmnprt";
static BYTE stem_en_li[] = "bcdeghkmnrt";

inline int stem_en_iv(BYTE l)
{
	register BYTE *v = stem_en_vowels;
	while (*v && *v != l) v++;
	return (*v == l) ? 1 : 0;
}

inline int stem_en_id(BYTE l)
{
	register BYTE *v = stem_en_doubles;
	while (*v && *v != l) v++;
	return (*v == l) ? 1 : 0;
}

inline int stem_en_il(BYTE l)
{
	register BYTE *v = stem_en_li;
	while (*v && *v != l) v++;
	return (*v == l) ? 1 : 0;
}

inline int stem_en_ivwxy(BYTE l)
{
	register BYTE *v = stem_en_vwxy;
	while (*v && *v != l) v++;
	return (*v == l) ? 1 : 0;
}

inline int stem_en_suffix(BYTE *word, int *len, int r,
	stem_table *table, int ntable)
{
	int i, j, k;

	for (i = 0; i < ntable; i++) {
		j = table[i].len-1;
		k = *len-1;
		if (j > k) continue;
		for (; j >= 0; k--, j--)
			if (word[k] != table[i].suffix[j]) break;
		if (j >= 0) continue;

		if (*len - table[i].len < r) return 1;

		*len -= table[i].nremove;
		word += *len;
		switch (table[i].nadd) {
			case 1:
				*word = table[i].add[0];
				break;
			case 2:
				*word++ = table[i].add[0];
				*word = table[i].add[1];
				break;
		}
		*len += table[i].nadd;
		return 1;
	}
	return 0;
}


void stem_en_init ()
{
}


void stem_en ( BYTE * word )
{
	int i, j, len, r1, r2;

	len = (int)strlen((char*)word);
	if (len <= 2) return;

	for (i = 0; i < (int)(sizeof(en_except)/sizeof(stem_table)); i++) {
		if (len != en_except[i].len) continue;
		if (strcmp((char*)word, (char*)en_except[i].suffix)) continue; // FIXME!! slow..
		word += len - en_except[i].nremove;
		switch (en_except[i].nadd) {
			case 1:
				*word++ = en_except[i].add[0];
				break;
			case 2:
				*word++ = en_except[i].add[0];
				*word++ = en_except[i].add[1];
				break;
		}
		*word = 0;
		return;
	}

	if (word[0] == 'y') word[0] = 'Y';
	for (i = 1; i < len; i++)
		if (word[i] == 'y' && stem_en_iv(word[i-1])) word[i] = 'Y';

	r1 = r2 = len;
	if (strncmp((char*)word, "gener", 5) == 0) {
		r1 = 5;
	} else {
		for (i = 0; i < len-1; i++)
			if (stem_en_iv(word[i]) && !stem_en_iv(word[i+1])) { r1 = i+2; break; }
	}
	for (i = r1; i < len-1; i++)
		if (stem_en_iv(word[i]) && !stem_en_iv(word[i+1])) { r2 = i+2; break; }

	#define W(p,c) (word[len-p] == c)
	#define SUFF2(c2,c1) (len >= 2 && W(1,c1) && W(2,c2))
	#define SUFF3(c3,c2,c1) (len >= 3 && W(1,c1) && W(2,c2) && W(3,c3))
	#define SUFF4(c4,c3,c2,c1) (len >= 4 && W(1,c1) && W(2,c2) && W(3,c3) && W(4,c4))
	#define SUFF5(c5,c4,c3,c2,c1) (len >= 5 && W(1,c1) && W(2,c2) && W(3,c3) && W(4,c4) && W(5,c5))
	#define SUFF6(c6,c5,c4,c3,c2,c1) (len >= 6 && W(1,c1) && W(2,c2) && W(3,c3) && W(4,c4) && W(5,c5) && W(6,c6))
	#define SUFF7(c7,c6,c5,c4,c3,c2,c1) (len >= 7 && W(1,c1) && W(2,c2) && W(3,c3) && W(4,c4) && W(5,c5) && W(6,c6) && W(7,c7))

	// *** STEP 1A ***

	if (SUFF4('s','s','e','s')) len -= 2;
	if (W(3,'i') && W(2,'e') && (W(1,'d') || W(1,'s')))
		len -= (len == 4) ? 1 : 2;
	if (W(1,'s') && !(W(2,'u') || W(2,'s'))) len--;

	// *** STEP 1B ***

	i = 0;
	for ( ;; )
	{
		if (SUFF3('e','e','d')) {
			if (len-3 >= r1) len--;
			break;
		}
		if (SUFF5('e','e','d','l','y')) {
			if (len-5 >= r1) len -= 3;
			break;
		}
		if (SUFF2('e','d')) { i = 2; break; }
		if (SUFF3('i','n','g')) { i = 3; break; }
		if (SUFF4('e','d','l','y')) { i = 4; break; }
		if (SUFF5('i','n','g','l','y')) { i = 5; break; }
		break;
	}
	if (i) for (j = 0; j < len-i; j++) if (stem_en_iv(word[j])) {
		len -= i;
		if (SUFF2('a','t') || SUFF2('b','l') || SUFF2('i','z')) {
			word[len++] = 'e'; break;
		}
		if (len >= 2 && word[len-1] == word[len-2] && stem_en_id(word[len-1])) {
			len--; break;
		}
		if (len == 2 && (stem_en_iv(word[0]) && !stem_en_iv(word[1]))
			|| (len == r1 && !stem_en_iv(word[len-3]) && stem_en_iv(word[len-2]) && !stem_en_ivwxy(word[len-1])))
		{
			word[len++] = 'e';
		}
		break;
	}

	// *** STEP 1C ***

	if (len > 2 && (word[len-1] == 'y' || word[len-1] == 'Y')
		&& !stem_en_iv(word[len-2]))
	{
		word[len-1] = 'i';
	}

	// *** STEP 2 ***

	for ( ;; )
	{
		if (stem_en_suffix(word, &len, r1, en_step2, sizeof(en_step2)/sizeof(stem_table)))
			break;
		if (len-3 >= r1 && SUFF3('o','g','i')) { len -= 1; break; }

		if (len-2 >= r1 && SUFF2('l','i')) len -= 2; else break;
		if (len-2 >= r1 && SUFF2('a','l')) {
			len -= 2;
			if (len-5 >= r1 && SUFF5('a','t','i','o','n')) {
				len -= 3;
				word[len++] = 'e';
				break;
			}
			if (SUFF4('t','i','o','n')) break;
			len += 2;
		} else {
			if (!stem_en_il(word[len-1])) len += 2;
		}

		break;
	}

	// *** STEP 3 ***

	stem_en_suffix(word, &len, r1, en_step3, sizeof(en_step3)/sizeof(stem_table));

	// *** STEP 4 ***

	i = stem_en_suffix(word, &len, r2, en_step4, sizeof(en_step4)/sizeof(stem_table));
	if (!i && len-3 >= r2 && SUFF3('i','o','n') && (word[len-4] == 't' || word[len-4] == 's')) {
		len -= 3;
	}

	// *** STEP 5A ***

	while (word[len-1] == 'e') {
		if (len > r2) { len--; break; }
		if (len <= r1) break;
		if (len > 3 && !stem_en_iv(word[len-4]) && stem_en_iv(word[len-3])
			&& !stem_en_ivwxy(word[len-2])) break;
		if (len == 3 && stem_en_iv(word[0]) && !stem_en_iv(word[1])) break;
		len--;
		break;
	}

	// *** STEP 5B ***

	if (len > r2 && word[len-1] == 'l' && word[len-2] == 'l') len--;

	// *** FINALIZE ***

	word[len] = 0;
	for (i = 0; i < len; i++) if (word[i] == 'Y') word[i] = 'y';
}

//
// $Id$
//
