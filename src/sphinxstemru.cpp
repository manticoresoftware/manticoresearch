//
// $Id$
//

#include "sphinx.h"

// in cp-1251 encoding!
enum
{
	RUS_A	= 0xE0,
	RUS_B	= 0xE1,
	RUS_V	= 0xE2,
	RUS_G	= 0xE3,
	RUS_D	= 0xE4,
	RUS_E	= 0xE5,
	RUS_YO	= 0xB8,
	RUS_ZH	= 0xE6,
	RUS_Z	= 0xE7,
	RUS_I	= 0xE8,
	RUS_IY	= 0xE9,
	RUS_K	= 0xEA,
	RUS_L	= 0xEB,
	RUS_M	= 0xEC,
	RUS_N	= 0xED,
	RUS_O	= 0xEE,
	RUS_P	= 0xEF,
	RUS_R	= 0xF0,
	RUS_S	= 0xF1,
	RUS_T	= 0xF2,
	RUS_U	= 0xF3,
	RUS_F	= 0xF4,
	RUS_H	= 0xF5,
	RUS_TS	= 0xF6,
	RUS_CH	= 0xF7,
	RUS_SH	= 0xF8,
	RUS_SCH	= 0xF9,
	RUS_TVY	= 0xFA, // TVYordiy znak
	RUS_Y	= 0xFB,
	RUS_MYA	= 0xFC, // MYAgkiy znak
	RUS_EE	= 0xFD,
	RUS_YU	= 0xFE,
	RUS_YA	= 0xFF
};

struct stem_table_entry_t
{
	BYTE	suffix[8];
	int		remove, len;
};

struct stem_table_index_t
{
	BYTE	first;
	int		count;
};

// TableStringN, where N is numbe of chars
#define TS1(c1) { RUS_##c1## }
#define TS2(c1,c2) { RUS_##c1##, RUS_##c2## }
#define TS3(c1,c2,c3) { RUS_##c1##, RUS_##c2##, RUS_##c3## }
#define TS4(c1,c2,c3,c4) { RUS_##c1##, RUS_##c2##, RUS_##c3##, RUS_##c4## }
#define TS5(c1,c2,c3,c4,c5) { RUS_##c1##, RUS_##c2##, RUS_##c3##, RUS_##c4##, RUS_##c5## }

static stem_table_index_t ru_adj_i[] =
{
	{ RUS_E,  4 },
	{ RUS_I,  2 },
	{ RUS_IY, 4 },
	{ RUS_M,  7 },
	{ RUS_O,  2 },
	{ RUS_U,  2 },
	{ RUS_H,  2 },
	{ RUS_YU, 4 },
	{ RUS_YA, 4 },
};

static stem_table_entry_t ru_adj[] = {
	{ TS2(E,E),  2 },
	{ TS2(I,E),  2 },
	{ TS2(Y,E),  2 },
	{ TS2(O,E),  2 },

	{ TS3(I,M,I), 3 },
	{ TS3(Y,M,I), 3 },

	{ TS2(E,IY),  2 },
	{ TS2(I,IY),  2 },
	{ TS2(Y,IY),  2 },
	{ TS2(O,IY),  2 },
	
	{ TS3(A,E,M),  0 },
	{ TS3(U,E,M),  0 },
	{ TS3(YA,E,M), 0 },
	{ TS2(E,M),    2 },
	{ TS2(I,M),    2 },
	{ TS2(Y,M),    2 },
	{ TS2(O,M),    2 },

	{ TS3(E,G,O), 3 },
	{ TS3(O,G,O), 3 },

	{ TS3(E,M,U), 3 },
	{ TS3(O,M,U), 3 },

	{ TS2(I,H),   2 },
	{ TS2(Y,H),   2 },

	{ TS2(E,YU),  2 },
	{ TS2(O,YU),  2 },
	{ TS2(U,YU),  2 },
	{ TS2(YU,YU), 2 },

	{ TS2(A,YA),   2 },
	{ TS2(YA,YA),  2 }
};

static stem_table_index_t ru_part_i[] = {
	{ RUS_A, 3 },
	{ RUS_M, 1 },
	{ RUS_N, 3 },
	{ RUS_O, 3 },
	{ RUS_Y, 3 },
	{ RUS_SH, 4 },
	{ RUS_SCH, 5 }
};

static stem_table_entry_t ru_part[] = {
	{ TS4(A,N,N,A),  2 },
	{ TS4(E,N,N,A),  2 },
	{ TS4(YA,N,N,A), 2 },

	{ TS3(YA,E,M), 2 },

	{ TS3(A,N,N),  1 },
	{ TS3(E,N,N),  1 },
	{ TS3(YA,N,N), 1 },

	{ TS4(A,N,N,O),  2 },
	{ TS4(E,N,N,O),  2 },
	{ TS4(YA,N,N,O), 2 },

	{ TS4(A,N,N,Y),  2 },
	{ TS4(E,N,N,Y),  2 },
	{ TS4(YA,N,N,Y), 2 },

	{ TS3(A,V,SH),  2 },
	{ TS3(I,V,SH),  3 },
	{ TS3(Y,V,SH),  3 },
	{ TS3(YA,V,SH), 2 },

	{ TS3(A,YU,SCH),  2 },
	{ TS2(A,SCH),     1 },
	{ TS3(YA,YU,SCH), 2 },
	{ TS2(YA,SCH),    1 },
	{ TS3(U,YU,SCH),  3 }
};

static stem_table_index_t ru_verb_i[] = {
	{ RUS_A,   7 },
	{ RUS_E,   9 },
	{ RUS_I,   4 },
	{ RUS_IY,  4 },
	{ RUS_L,   4 },
	{ RUS_M,   5 },
	{ RUS_O,   7 },
	{ RUS_T,   9 },
	{ RUS_Y,   3 },
	{ RUS_MYA, 10 },
	{ RUS_YU,  4 },
	{ RUS_YA,  1 }
};

static stem_table_entry_t ru_verb[] = {
	{ TS3(A,L,A),  3 },
	{ TS3(A,N,A),  3 },
	{ TS3(YA,L,A), 3 },
	{ TS3(YA,N,A), 3 },
	{ TS3(I,L,A),  3 },
	{ TS3(Y,L,A),  3 },
	{ TS3(E,N,A),  3 },

	{ TS4(A,E,T,E),   4 },
	{ TS4(A,IY,T,E),  4 },
	{ TS3(MYA,T,E),   3 },
	{ TS4(U,E,T,E),   4 },
	{ TS4(YA,E,T,E),  4 },
	{ TS4(YA,IY,T,E), 4 },
	{ TS4(E,IY,T,E),  4 },
	{ TS4(U,IY,T,E),  4 },
	{ TS3(I,T,E),     3 },

	{ TS3(A,L,I),  3 },
	{ TS3(YA,L,I), 3 },
	{ TS3(I,L,I),  3 },
	{ TS3(Y,L,I),  3 },

	{ TS2(A,IY),   2 },
	{ TS2(YA,IY),  2 },
	{ TS2(E,IY),   2 },
	{ TS2(U,IY),   2 },

	{ TS2(A,L),   2 },
	{ TS2(YA,L),  2 },
	{ TS2(I,L),   2 },
	{ TS2(Y,L),   2 },

	{ TS3(A,E,M),  3 },
	{ TS3(YA,E,M), 3 },
	{ TS3(U,E,M),  3 },
	{ TS2(I,M),    2 },
	{ TS2(Y,M),    2 },

	{ TS3(A,L,O),  3 },
	{ TS3(A,N,O),  3 },
	{ TS3(YA,L,O), 3 },
	{ TS3(YA,N,O), 3 },
	{ TS3(I,L,O),  3 },
	{ TS3(Y,L,O),  3 },
	{ TS3(E,N,O),  3 },

	{ TS3(A,E,T),   3 },
	{ TS3(A,YU,T),  3 },
	{ TS3(YA,E,T),  3 },
	{ TS3(YA,YU,T), 3 },
	{ TS2(YA,T),    2 },
	{ TS3(U,E,T),   3 },
	{ TS3(U,YU,T),  3 },
	{ TS2(I,T),     2 },
	{ TS2(Y,T),     2 },

	{ TS3(A,N,Y),  3 },
	{ TS3(YA,N,Y), 3 },
	{ TS3(E,N,Y),  3 },

	{ TS4(A,E,SH,MYA),  4 },
	{ TS4(U,E,SH,MYA),  4 },
	{ TS4(YA,E,SH,MYA), 4 },
	{ TS3(A,T,MYA),     3 },
	{ TS3(E,T,MYA),     3 },
	{ TS3(I,T,MYA),     3 },
	{ TS3(U,T,MYA),     3 },
	{ TS3(Y,T,MYA),     3 },
	{ TS3(I,SH,MYA),    3 },
	{ TS3(YA,T,MYA),    3 },

	{ TS2(A,YU),  2 },
	{ TS2(U,YU),  2 },
	{ TS2(YA,YU), 2 },
	{ TS1(YU),    1 },

	{ TS2(U,YA),  2 }
};

static stem_table_index_t ru_dear_i[] = {
	{ RUS_K,  3 },
	{ RUS_A,  2 },
	{ RUS_V,  2 },
	{ RUS_E,  2 },
	{ RUS_I,  4 },
	{ RUS_IY, 2 },
	{ RUS_M,  4 },
	{ RUS_O,  2 },
	{ RUS_U,  2 },
	{ RUS_H,  2 },
	{ RUS_YU, 2 }
};

static stem_table_entry_t ru_dear[] = {
	{ TS3(CH,E,K), 3 },
	{ TS3(CH,O,K), 3 },
	{ TS3(N,O,K),  3 },

	{ TS3(CH,K,A),     3 },
	{ TS3(N, K,A),     3 },
	{ TS4(CH,K,O,V),   4 },
	{ TS4(N, K,O,V),   4 },
	{ TS3(CH,K,E),     3 },
	{ TS3(N, K,E),     3 },
	{ TS3(CH,K,I),     3 },
	{ TS3(N, K,I),     3 },
	{ TS5(CH,K,A,M,I), 5 },
	{ TS5(N, K,A,M,I), 5 },
	{ TS4(CH,K,O,IY),  4 },
	{ TS4(N, K,O,IY),  4 },
	{ TS4(CH,K,A,M),   4 },
	{ TS4(N, K,A,M),   4 },
	{ TS4(CH,K,O,M),   4 },
	{ TS4(N, K,O,M),   4 },
	{ TS3(CH,K,O),     3 },
	{ TS3(N, K,O),     3 },
	{ TS3(CH,K,U),     3 },
	{ TS3(N, K,U),     3 },
	{ TS4(CH,K,A,H),   4 },
	{ TS4(N, K,A,H),   4 },
	{ TS4(CH,K,O,YU),  4 },
	{ TS4(N, K,O,YU),  4 }
};

static stem_table_index_t ru_noun_i[] = {
	{ RUS_A,   1 },
	{ RUS_V,   2 },
	{ RUS_E,   3 },
	{ RUS_I,   6 },
	{ RUS_IY,  4 },
	{ RUS_M,   5 },
	{ RUS_O,   1 },
	{ RUS_U,   1 },
	{ RUS_H,   3 },
	{ RUS_Y,   1 },
	{ RUS_MYA, 1 },
	{ RUS_YU,  3 },
	{ RUS_YA,  3 }
};

static stem_table_entry_t ru_noun[] = {
	{ TS1(A),  1 },

	{ TS2(E,V),   2 },
	{ TS2(O,V),   2 },

	{ TS2(I,E),   2 },
	{ TS2(MYA,E),   2 },
	{ TS1(E),    1 },

	{ TS4(I,YA,M,I), 4 },
	{ TS3(YA,M,I),   3 },
	{ TS3(A,M,I),    3 },
	{ TS2(E,I),      2 },
	{ TS2(I,I),      2 },
	{ TS1(I),        1 },

	{ TS3(I,E,IY),  3 },
	{ TS2(E,IY),    2 },
	{ TS2(O,IY),    2 },
	{ TS2(I,IY),    2 },

	{ TS3(I,YA,M),  3 },
	{ TS2(YA,M),    2 },
	{ TS3(I,E,M),   3 },
	{ TS2(A,M),     2 },
	{ TS2(O,M),     2 },

	{ TS1(O),    1 },

	{ TS1(U),    1 },

	{ TS2(A,H),    2 },
	{ TS3(I,YA,H), 3 },
	{ TS2(YA,H),   2 },

	{ TS1(Y),    1 },

	{ TS1(MYA),  1 },

	{ TS2(I,YU),   2 },
	{ TS2(MYA,YU), 2 },
	{ TS1(YU),     1 },

	{ TS2(I,YA),   2 },
	{ TS2(MYA,YA), 2 },
	{ TS1(YA),     1 }
};

int stem_ru_table_i ( BYTE * word, int len,
	stem_table_entry_t *table, stem_table_index_t *itable, int icount)
{
	int i, j, k, m;
	BYTE l = word[--len];

	for (i = 0, j = 0; i < icount; i++) {
		if (l == itable[i].first) {
			m = itable[i].count;
			i = j-1;
			while (m--) {
				i++;
				j = table[i].len;
				k = len;
				if (j > k) continue;
				for (; j >= 0; k--, j--)
					if (word[k] != table[i].suffix[j]) break;
				if (j >= 0) continue;

				return table[i].remove;
			}
			return 0;
		}
		j += itable[i].count;
	}
	return 0;
}

#define STEM_RU_FUNC(func,table) \
	int func(BYTE *word, int len) \
	{ \
		return stem_ru_table(word, len, \
			table, sizeof(table) / sizeof(stem_table_entry_t)); \
	}

#define STEM_RU_FUNC_I(table) \
	int stem_##table##_i(BYTE *word, int len) \
	{ \
		return stem_ru_table_i(word, len, table, \
			table##_i, sizeof(table##_i) / sizeof(stem_table_index_t)); \
	}

STEM_RU_FUNC_I(ru_adj)
STEM_RU_FUNC_I(ru_part)
STEM_RU_FUNC_I(ru_dear)
STEM_RU_FUNC_I(ru_verb)
STEM_RU_FUNC_I(ru_noun)

static int stem_ru_adjectival(BYTE *word, int len)
{
	register int i;

	if ( (i = stem_ru_adj_i(word, len)) )
		i += stem_ru_part_i(word, len-i);
	return i;
}

static int stem_ru_verb_ov(BYTE *word, int len)
{
	register int i;

	if ( (i = stem_ru_verb_i(word, len)) )
		if (word[len-i-2] == RUS_O && word[len-i-1] == RUS_V) return i+2;
	return i;
}

void stem_ru_init()
{
	int i;

	#define STEM_RU_INIT_TABLE(table) \
		for (i = 0; i < int(sizeof(table)/sizeof(stem_table_entry_t)); i++) \
		   	table[i].len = strlen((char*)table[i].suffix)-1;

	STEM_RU_INIT_TABLE(ru_adj)
	STEM_RU_INIT_TABLE(ru_part)
	STEM_RU_INIT_TABLE(ru_verb)
	STEM_RU_INIT_TABLE(ru_noun)
	STEM_RU_INIT_TABLE(ru_dear)
}

void stem_ru(BYTE *word)
{
	int r1, r2;
	int i, len;

	// IsVowel
	#define IV(c) (c == RUS_A || c == RUS_E || c == RUS_YO || c == RUS_I || c == RUS_O \
		|| c == RUS_U || c == RUS_Y || c == RUS_EE || c == RUS_YU || c == RUS_YA )

	while (*word) if (IV(*word)) break; else word++;
	if (*word) word++; else return;
	len = strlen((char*)word);

	r1 = r2 = len;
	for (i = -1; i < len-1; i++)
		if (IV(word[i]) && !IV(word[i+1])) { r1 = i+2; break; }
	for (i = r1; i < len-1; i++)
		if (IV(word[i]) && !IV(word[i+1])) { r2 = i+2; break; }

	#define C(p) word[len-p]
	#define W(p,c) (word[len-p] == c)
	#define SUFF2(c2,c1) (len >= 2 && W(1,c1) && W(2,c2))
	#define SUFF3(c3,c2,c1) (len >= 3 && W(1,c1) && W(2,c2) && W(3,c3))
	#define SUFF4(c4,c3,c2,c1) (len >= 4 && W(1,c1) && W(2,c2) && W(3,c3) && W(4,c4))
	#define SUFF5(c5,c4,c3,c2,c1) (len >= 5 && W(1,c1) && W(2,c2) && W(3,c3) && W(4,c4) && W(5,c5))
	#define SUFF6(c6,c5,c4,c3,c2,c1) (len >= 6 && W(1,c1) && W(2,c2) && W(3,c3) && W(4,c4) && W(5,c5) && W(6,c6))
	#define SUFF7(c7,c6,c5,c4,c3,c2,c1) (len >= 7 && W(1,c1) && W(2,c2) && W(3,c3) && W(4,c4) && W(5,c5) && W(6,c6) && W(7,c7))

	while (1) {
		if ( (i = stem_ru_dear_i(word, len)) ) { len -= i; break; }

		if ( C(1) == RUS_V && (len>=2) ) {
			if (C(2) == RUS_I || C(2) == RUS_Y || C(2) == RUS_YA) { len -= 2; break; }
			if (C(2) == RUS_A) {
				if (C(3) == RUS_V && C(4) == RUS_A) { len -= 4; break; }
				len -= 2; break;
			}
		}
		if (SUFF3(RUS_V, RUS_SH, RUS_I) && (C(4) == RUS_A || C(4) == RUS_I || C(4) == RUS_Y || C(4) == RUS_YA)) { len -= 4; break; }
		if (SUFF5(RUS_V, RUS_SH, RUS_I, RUS_S, RUS_MYA) && (C(6) == RUS_A || C(6) == RUS_I || C(6) == RUS_Y || C(6) == RUS_YA)) { len -= 6; break; }

		if ( (i = stem_ru_adjectival(word, len)) ) { len -= i; break; }

		if (SUFF2(RUS_S, RUS_MYA) || SUFF2(RUS_S, RUS_YA)) {
			len -= 2;
			if ( (i = stem_ru_adjectival(word, len)) ) { len -= i; break; }
			if ( (i = stem_ru_verb_ov(word, len)) ) { len -= i; break; }
		} else {
			if ( (i = stem_ru_verb_ov(word, len)) ) { len -= i; break; }
		}

		if ( (i = stem_ru_noun_i(word, len)) ) { len -= i; break; }
		break;
	}

	if (len && (word[len-1] == RUS_IY || word[len-1] == RUS_I)) len--;

	if (len-r2 >= 3 && word[len-1] == RUS_T && word[len-2] == RUS_S && word[len-3] == RUS_O) len -= 3;
		else if (len-r2 >= 4 && word[len-1] == RUS_MYA && word[len-2] == RUS_T && word[len-3] == RUS_S && word[len-4] == RUS_O) len -= 4;

	if (len >= 3 && word[len-1] == RUS_SH && word[len-2] == RUS_IY && word[len-3] == RUS_E) len -= 3;
		else if (len >= 4 && word[len-1] == RUS_E && word[len-2] == RUS_SH && word[len-3] == RUS_IY && word[len-4] == RUS_E) len -= 4;

	if (len > 1 && word[len-1] == RUS_N && word[len-2] == RUS_N) len--;
	if (len && word[len-1] == RUS_MYA) len--;

	word[len] = '\0';
}

//
// $Id$
//
