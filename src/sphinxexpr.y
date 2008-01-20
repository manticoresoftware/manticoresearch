%lex-param		{ ExprParser_t * pParser }
%parse-param	{ ExprParser_t * pParser }
%pure-parser
%error-verbose

%union {
	float			fNumber;	// constant value
	int				iRowitem;	// attribute rowitem index
	int				iFunc;		// function id
	Docinfo_e		eDocinfo;	// docinfo entry id
	int				iNode;		// node index
};

%token <fNumber>	TOK_NUMBER
%token <iRowitem>	TOK_ATTR
%token <iFunc>		TOK_FUNC
%token <eDocinfo>	TOK_DOCINFO
%type <iNode>		function
%type <iNode>		arglist
%type <iNode>		expr

%left '<' '>' TOK_LTE TOK_GTE
%left '+' '-'
%left '*' '/'
%nonassoc TOK_NEG


%%

exprline:
	expr							{ pParser->m_iParsed = $1; }
	;

expr:
	TOK_NUMBER						{ $$ = pParser->AddNodeNumber ( $1 ); }
	| TOK_ATTR						{ $$ = pParser->AddNodeAttr ( $1 ); }
	| TOK_DOCINFO					{ $$ = pParser->AddNodeDocinfo ( $1 ); }
	| '-' expr %prec TOK_NEG		{ $$ = pParser->AddNodeOp ( TOK_NEG, $2, -1 ); }
	| expr '+' expr					{ $$ = pParser->AddNodeOp ( '+', $1, $3 ); }
	| expr '-' expr					{ $$ = pParser->AddNodeOp ( '-', $1, $3 ); }
	| expr '*' expr					{ $$ = pParser->AddNodeOp ( '*', $1, $3 ); }
	| expr '/' expr					{ $$ = pParser->AddNodeOp ( '/', $1, $3 ); }
	| expr '<' expr					{ $$ = pParser->AddNodeOp ( '<', $1, $3 ); }
	| expr '>' expr					{ $$ = pParser->AddNodeOp ( '>', $1, $3 ); }
	| expr TOK_LTE expr				{ $$ = pParser->AddNodeOp ( TOK_LTE, $1, $3 ); }
	| expr TOK_GTE expr				{ $$ = pParser->AddNodeOp ( TOK_GTE, $1, $3 ); }
	| '(' expr ')'					{ $$ = $2; }
	| function						{ $$ = $1; }
	;

arglist:
	expr							{ $$ = $1; }
	| arglist ',' expr				{ $$ = pParser->AddNodeOp ( ',', $1, $3 ); }
	;

function:
	TOK_FUNC '(' arglist ')'		{ $$ = pParser->AddNodeFunc ( $1, $3 ); if ( $$<0 ) YYERROR; }
	;

%%
