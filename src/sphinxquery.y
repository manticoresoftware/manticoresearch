%{
#if USE_WINDOWS
#pragma warning(push,1)
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
		bool		bKeyword;
	} tInt;
	struct							// field spec
	{
		DWORD		uMask;			// acceptable fields mask
		int			iMaxPos;		// max allowed position within field
	} tFieldLimit;
};

%token <pNode>			TOK_KEYWORD
%token <tInt>			TOK_INT
%token <tFieldLimit>	TOK_FIELDLIMIT
%type <pNode>			keyword
%type <pNode>			phrasetoken
%type <pNode>			phrase
%type <pNode>			atom
%type <pNode>			atom2
%type <pNode>			orlist
%type <pNode>			expr

%%

query:
	expr								{ pParser->AddQuery ( $1 ); }
	;

keyword:
	TOK_KEYWORD							{ $$ = $1; }
	| TOK_INT							{ $$ = pParser->AddKeywordFromInt ( $1.iValue, $1.bKeyword ); }
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

phrase:
	phrasetoken							{ $$ = $1; }
	| phrase phrasetoken				{ $$ = pParser->AddKeyword ( $1, $2 ); }
	;


atom:
	keyword								{ $$ = $1; }
	| '"' phrase '"'					{ $$ = $2; assert ( $$->IsPlain() ); $$->m_tAtom.m_iMaxDistance = 0; }
	| '"' phrase '"' '~' TOK_INT		{ $$ = $2; assert ( $$->IsPlain() ); $$->m_tAtom.m_iMaxDistance = $5.iValue; $$->m_tAtom.m_bQuorum = false; }
	| '"' phrase '"' '/' TOK_INT		{ $$ = $2; assert ( $$->IsPlain() ); $$->m_tAtom.m_iMaxDistance = $5.iValue; $$->m_tAtom.m_bQuorum = true; }
	| '(' expr ')'						{ $$ = $2; $2->m_bFieldSpec = false; }
	;

atom2:
	atom								{ $$ = $1; }
	| TOK_FIELDLIMIT atom				{ $$ = $2; $2->SetFieldSpec ( $1.uMask, $1.iMaxPos ); }
	;

orlist:
	atom2								{ $$ = $1; }
	| orlist '|' atom2					{ $$ = pParser->AddOp ( SPH_QUERY_OR, $1, $3 ); }
	;

expr:
	orlist								{ $$ = $1; }
	| '-' orlist						{ $$ = pParser->AddOp ( SPH_QUERY_NOT, $2, NULL ); }
	| expr orlist               		{ $$ = pParser->AddOp ( SPH_QUERY_AND, $1, $2 ); }
	| expr '-' orlist               	{ $$ = pParser->AddOp ( SPH_QUERY_AND, $1, pParser->AddOp ( SPH_QUERY_NOT, $3, NULL ) ); }
	;

%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
