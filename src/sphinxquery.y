%{
#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif
%}

%lex-param		{ XQParser_t * pParser }
%parse-param	{ XQParser_t * pParser }
%pure-parser
%error-verbose

%union {
	XQNode_t *		pNode;			// tree node
	struct
	{
		int			iValue;
		int			iStrIndex;
		float		fValue;
	} tInt;
	struct							// field spec
	{
		FieldMask_t	dMask;			// acceptable fields mask
		int			iMaxPos;		// max allowed position within field
	} tFieldLimit;
	int				iZoneVec;
};

%token <pNode>			TOK_KEYWORD
%token <tInt>			TOK_NEAR
%token <tInt>			TOK_INT
%token <tInt>			TOK_FLOAT
%token <tFieldLimit>	TOK_FIELDLIMIT
%token <iZoneVec>		TOK_ZONE
%token <iZoneVec>		TOK_ZONESPAN
%token					TOK_BEFORE
%token					TOK_SENTENCE
%token					TOK_PARAGRAPH
%token					TOK_MAYBE
%type <pNode>			keyword
%type <pNode>			phrasetoken
%type <pNode>			phrase
%type <pNode>			sp_item
%type <pNode>			sentence
%type <pNode>			paragraph
%type <pNode>			atom
%type <pNode>			orlist
%type <pNode>			orlistf
%type <pNode>			beforelist
%type <pNode>			expr

%left TOK_BEFORE TOK_NEAR

%%

query:
	expr								{ pParser->AddQuery ( $1 ); }
	;

expr:
	beforelist						{ $$ = $1; }
	| expr beforelist					{ $$ = pParser->AddOp ( SPH_QUERY_AND, $1, $2 ); }
	;

tok_limiter:
	// empty
	| TOK_FIELDLIMIT			{ pParser->SetFieldSpec ( $1.dMask, $1.iMaxPos ); }
	| TOK_ZONE				{ pParser->SetZoneVec ( $1 ); }
	| TOK_ZONESPAN				{ pParser->SetZoneVec ( $1, true ); }
	;

beforelist:
	orlistf					{ $$ = $1; }
	| beforelist TOK_BEFORE orlistf		{ $$ = pParser->AddOp ( SPH_QUERY_BEFORE, $1, $3 ); }
	| beforelist TOK_NEAR orlistf		{ $$ = pParser->AddOp ( SPH_QUERY_NEAR, $1, $3, $2.iValue ); }
	;

orlistf:
	orlist								{ $$ = $1; }
	| tok_limiter '-' orlist					{ $$ = pParser->AddOp ( SPH_QUERY_NOT, $3, NULL ); }
	| tok_limiter '!' orlist					{ $$ = pParser->AddOp ( SPH_QUERY_NOT, $3, NULL ); }
	;

orlist:
	tok_limiter atom								{ $$ = $2; }
	| orlist '|' tok_limiter atom					{ $$ = pParser->AddOp ( SPH_QUERY_OR, $1, $4 ); }
	| orlist TOK_MAYBE tok_limiter atom				{ $$ = pParser->AddOp ( SPH_QUERY_MAYBE, $1, $4 ); }
	;

atom:
	keyword								{ $$ = $1; }
	| sentence							{ $$ = $1; }
	| paragraph							{ $$ = $1; }
	| '"' '"'							{ $$ = NULL; }
	| '"' '"' '~' TOK_INT				{ $$ = NULL; }
	| '"' '"' '/' TOK_INT				{ $$ = NULL; }
	| '"' '"' '/' TOK_FLOAT				{ $$ = NULL; }
	| '"' phrase '"'					{ $$ = $2; pParser->SetPhrase ( $$, false ); }
	| '"' phrase '"' '~' TOK_INT		{ $$ = $2; if ( $$ ) { assert ( $$->m_dWords.GetLength() ); $$->SetOp ( SPH_QUERY_PROXIMITY ); $$->m_iOpArg = $5.iValue; pParser->m_iAtomPos = $$->FixupAtomPos(); } }
	| '"' phrase '"' '/' TOK_INT		{ $$ = $2; if ( $$ ) { assert ( $$->m_dWords.GetLength() ); $$->SetOp ( SPH_QUERY_QUORUM ); $$->m_iOpArg = $5.iValue; } }
	| '"' phrase '"' '/' TOK_FLOAT		{ $$ = $2; if ( $$ ) { assert ( $$->m_dWords.GetLength() ); $$->SetOp ( SPH_QUERY_QUORUM ); $$->m_iOpArg = $5.fValue * 100; $$->m_bPercentOp = true; } }
	| '(' expr ')'						{ $$ = $2; }
	| '=' '"' phrase '"'				{ $$ = $3; pParser->SetPhrase ( $$, true ); }
	;

keyword:
	TOK_KEYWORD							{ $$ = $1; }
	| TOK_INT							{ $$ = pParser->AddKeyword ( ( $1.iStrIndex>=0 ) ? pParser->m_dIntTokens[$1.iStrIndex].cstr() : NULL ); }
	| TOK_FLOAT							{ $$ = pParser->AddKeyword ( ( $1.iStrIndex>=0 ) ? pParser->m_dIntTokens[$1.iStrIndex].cstr() : NULL ); }
	| '=' keyword						{ $$ = $2; assert ( $$->m_dWords.GetLength()==1 ); if ( !($$->m_dWords[0].m_sWord.IsEmpty()) ) $$->m_dWords[0].m_sWord.SetSprintf ( "=%s", $$->m_dWords[0].m_sWord.cstr() ); }
	;

sentence:
	sp_item TOK_SENTENCE sp_item		{ $$ = pParser->AddOp ( SPH_QUERY_SENTENCE, $1, $3 ); }
	| sentence TOK_SENTENCE sp_item		{ $$ = pParser->AddOp ( SPH_QUERY_SENTENCE, $1, $3 ); }
	;

paragraph:
	sp_item TOK_PARAGRAPH sp_item		{ $$ = pParser->AddOp ( SPH_QUERY_PARAGRAPH, $1, $3 ); }
	| paragraph TOK_PARAGRAPH sp_item	{ $$ = pParser->AddOp ( SPH_QUERY_PARAGRAPH, $1, $3 ); }
	;

sp_item:
	keyword								{ $$ = $1; }
	| '"' phrase '"'					{ $$ = $2; if ( $$ ) { assert ( $$->m_dWords.GetLength() ); $$->SetOp ( SPH_QUERY_PHRASE); } }
	;

phrase:
	phrasetoken							{ $$ = $1; }
	| phrase phrasetoken				{ $$ = pParser->AddKeyword ( $1, $2 ); }
	;

phrasetoken:
	keyword								{ $$ = $1; }
	| '('								{ $$ = NULL; }
	| ')'								{ $$ = NULL; }
	| '-'								{ $$ = NULL; }
	| '|'								{ $$ = NULL; }
	| '~'								{ $$ = NULL; }
	| '/'								{ $$ = NULL; }
	;


%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
