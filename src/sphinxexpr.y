%lex-param		{ ExprParser_t * pParser }
%parse-param	{ ExprParser_t * pParser }
%pure-parser
%error-verbose

%union {
	int64_t			iConst;			// constant value
	float			fConst;			// constant value
	uint64_t		iAttrLocator;	// attribute locator (rowitem for int/float; offset+size for bits)
	int				iFunc;			// function id
	int				iNode;			// node, or uservar, or udf index
	const char *	sIdent;			// generic identifier (token does NOT own ident storage; ie values are managed by parser)
};

%token <iConst>			TOK_CONST_INT
%token <fConst>			TOK_CONST_FLOAT
%token <iConst>			TOK_CONST_STRING
%token <iConst>			TOK_SUBKEY
%token <iConst>			TOK_DOT_NUMBER
%token <iAttrLocator>	TOK_ATTR_INT
%token <iAttrLocator>	TOK_ATTR_BITS
%token <iAttrLocator>	TOK_ATTR_FLOAT
%token <iAttrLocator>	TOK_ATTR_MVA32
%token <iAttrLocator>	TOK_ATTR_MVA64
%token <iAttrLocator>	TOK_ATTR_STRING
%token <iAttrLocator>	TOK_ATTR_FACTORS
%token <iFunc>			TOK_FUNC
%token <iFunc>			TOK_FUNC_IN
%token <iNode>			TOK_USERVAR
%token <iNode>			TOK_UDF
%token <iNode>			TOK_HOOK_IDENT
%token <iNode>			TOK_HOOK_FUNC
%token <sIdent>			TOK_IDENT
%token <iAttrLocator>	TOK_ATTR_JSON


%token	TOK_ATID
%token	TOK_ATWEIGHT
%token	TOK_ID
%token	TOK_GROUPBY
%token	TOK_WEIGHT
%token	TOK_COUNT
%token	TOK_DISTINCT
%token	TOK_CONST_LIST
%token	TOK_ATTR_SINT
%token	TOK_CONST_HASH
%token	TOK_FOR
%token	TOK_ITERATOR


%type <iNode>			attr
%type <iNode>			expr
%type <iNode>			arg
%type <iNode>			arglist
%type <iNode>			constlist
%type <iNode>			constlist_or_uservar
%type <iNode>			consthash
%type <sIdent>			hash_key
%type <iNode>			function
%type <sIdent>			ident
%type <iNode>			stringlist
%type <iNode>			json_field
%type <iNode>			subkey
%type <iNode>			subscript
%type <iNode>			for_loop
%type <iNode>			iterator
%type <iNode>			streq
%type <iNode>			strval


%left TOK_OR
%left TOK_AND
%left '|'
%left '&'
%left TOK_EQ TOK_NE
%left '<' '>' TOK_LTE TOK_GTE
%left '+' '-'
%left '*' '/' '%' TOK_DIV TOK_MOD
%nonassoc TOK_NOT
%nonassoc TOK_NEG


%%

exprline:
	expr							{ pParser->m_iParsed = $1; }
	;

attr:
	TOK_ATTR_INT					{ $$ = pParser->AddNodeAttr ( TOK_ATTR_INT, $1 ); }
	| TOK_ATTR_BITS					{ $$ = pParser->AddNodeAttr ( TOK_ATTR_BITS, $1 ); }
	| TOK_ATTR_FLOAT				{ $$ = pParser->AddNodeAttr ( TOK_ATTR_FLOAT, $1 ); }
	| TOK_ATTR_JSON					{ $$ = pParser->AddNodeAttr ( TOK_ATTR_JSON, $1 ); }
	;

expr:
	attr
	| function
	| TOK_CONST_INT					{ $$ = pParser->AddNodeInt ( $1 ); }
	| TOK_CONST_FLOAT				{ $$ = pParser->AddNodeFloat ( $1 ); }
	| TOK_DOT_NUMBER				{ $$ = pParser->AddNodeDotNumber ( $1 ); }
	| TOK_ATID						{ $$ = pParser->AddNodeID(); }
	| TOK_ATWEIGHT					{ $$ = pParser->AddNodeWeight(); }
	| TOK_ID						{ $$ = pParser->AddNodeID(); }
	| TOK_WEIGHT '(' ')'			{ $$ = pParser->AddNodeWeight(); }
	| TOK_GROUPBY '(' ')'			{ $$ = pParser->AddNodeGroupby(); }
	| TOK_HOOK_IDENT				{ $$ = pParser->AddNodeHookIdent ( $1 ); }
	| '-' expr %prec TOK_NEG		{ $$ = pParser->AddNodeOp ( TOK_NEG, $2, -1 ); }
	| TOK_NOT expr					{ $$ = pParser->AddNodeOp ( TOK_NOT, $2, -1 ); if ( $$<0 ) YYERROR; }
	| expr '+' expr					{ $$ = pParser->AddNodeOp ( '+', $1, $3 ); }
	| expr '-' expr					{ $$ = pParser->AddNodeOp ( '-', $1, $3 ); }
	| expr '*' expr					{ $$ = pParser->AddNodeOp ( '*', $1, $3 ); }
	| expr '/' expr					{ $$ = pParser->AddNodeOp ( '/', $1, $3 ); }
	| expr '<' expr					{ $$ = pParser->AddNodeOp ( '<', $1, $3 ); }
	| expr '>' expr					{ $$ = pParser->AddNodeOp ( '>', $1, $3 ); }
	| expr '&' expr					{ $$ = pParser->AddNodeOp ( '&', $1, $3 ); }
	| expr '|' expr					{ $$ = pParser->AddNodeOp ( '|', $1, $3 ); }
	| expr '%' expr					{ $$ = pParser->AddNodeOp ( '%', $1, $3 ); }
	| expr TOK_DIV expr				{ $$ = pParser->AddNodeFunc ( FUNC_IDIV, pParser->AddNodeOp ( ',', $1, $3 ) ); }
	| expr TOK_MOD expr				{ $$ = pParser->AddNodeOp ( '%', $1, $3 ); }
	| expr TOK_LTE expr				{ $$ = pParser->AddNodeOp ( TOK_LTE, $1, $3 ); }
	| expr TOK_GTE expr				{ $$ = pParser->AddNodeOp ( TOK_GTE, $1, $3 ); }
	| expr TOK_EQ expr				{ $$ = pParser->AddNodeOp ( TOK_EQ, $1, $3 ); }
	| expr TOK_NE expr				{ $$ = pParser->AddNodeOp ( TOK_NE, $1, $3 ); }
	| expr TOK_AND expr				{ $$ = pParser->AddNodeOp ( TOK_AND, $1, $3 ); if ( $$<0 ) YYERROR; }
	| expr TOK_OR expr				{ $$ = pParser->AddNodeOp ( TOK_OR, $1, $3 ); if ( $$<0 ) YYERROR; }
	| '(' expr ')'					{ $$ = $2; }
	| json_field
	| iterator
	| streq
	;

consthash:
	hash_key TOK_EQ TOK_CONST_INT					{ $$ = pParser->AddNodeConsthash ( $1, NULL, $3 ); }
	| hash_key TOK_EQ TOK_IDENT						{ $$ = pParser->AddNodeConsthash ( $1, $3, 0 ); }
	| consthash ',' hash_key TOK_EQ TOK_CONST_INT	{ pParser->AppendToConsthash ( $$, $3, NULL, $5 ); }
	| consthash ',' hash_key TOK_EQ TOK_IDENT		{ pParser->AppendToConsthash ( $$, $3, $5, 0 ); }
	;

hash_key:
	ident							{ $$ = $1; }
	| TOK_ATTR_STRING				{ $$ = pParser->Attr2Ident($1); }
	| TOK_FUNC_IN					{ $$ = strdup("in"); }
	;

arg:
	expr
	| '{' consthash '}'				{ $$ = $2; }
	| TOK_ATTR_STRING				{ $$ = pParser->AddNodeAttr ( TOK_ATTR_STRING, $1 ); }
	| TOK_ATTR_MVA32				{ $$ = pParser->AddNodeAttr ( TOK_ATTR_MVA32, $1 ); }
	| TOK_ATTR_MVA64				{ $$ = pParser->AddNodeAttr ( TOK_ATTR_MVA64, $1 ); }
	| TOK_ATTR_FACTORS				{ $$ = pParser->AddNodeAttr ( TOK_ATTR_FACTORS, $1 ); }
	| TOK_CONST_STRING				{ $$ = pParser->AddNodeString ( $1 ); }
	;

arglist:
	arg								{ $$ = $1; }
	| arglist ',' arg				{ $$ = pParser->AddNodeOp ( ',', $1, $3 ); }
	;

constlist:
	TOK_CONST_INT						{ $$ = pParser->AddNodeConstlist ( $1 ); }
	| '-' TOK_CONST_INT					{ $$ = pParser->AddNodeConstlist ( -$2 );}
	| TOK_CONST_FLOAT					{ $$ = pParser->AddNodeConstlist ( $1 ); }
	| '-' TOK_CONST_FLOAT				{ $$ = pParser->AddNodeConstlist ( -$2 );}
	| constlist ',' TOK_CONST_INT		{ pParser->AppendToConstlist ( $$, $3 ); }
	| constlist ',' '-' TOK_CONST_INT	{ pParser->AppendToConstlist ( $$, -$4 );}
	| constlist ',' TOK_CONST_FLOAT		{ pParser->AppendToConstlist ( $$, $3 ); }
	| constlist ',' '-' TOK_CONST_FLOAT	{ pParser->AppendToConstlist ( $$, -$4 );}
	;

stringlist:
	TOK_CONST_STRING					{ $$ = pParser->AddNodeConstlist ( $1 ); }
	| stringlist ',' TOK_CONST_STRING	{ pParser->AppendToConstlist ( $$, $3 ); }
	;

constlist_or_uservar:
	constlist
	| stringlist
	| TOK_USERVAR					{ $$ = pParser->AddNodeUservar ( $1 ); }
	;

ident:
	TOK_ATTR_INT 					{ $$ = pParser->Attr2Ident ( $1 ); }
	| TOK_IDENT
	;

function:
	TOK_FUNC '(' arglist ')'		{ $$ = pParser->AddNodeFunc ( $1, $3 ); if ( $$<0 ) YYERROR; }
	| TOK_FUNC '(' ')'				{ $$ = pParser->AddNodeFunc ( $1, -1 ); if ( $$<0 ) YYERROR; }
	| TOK_UDF '(' arglist ')'		{ $$ = pParser->AddNodeUdf ( $1, $3 ); if ( $$<0 ) YYERROR; }
	| TOK_UDF '(' ')'				{ $$ = pParser->AddNodeUdf ( $1, -1 ); if ( $$<0 ) YYERROR; }
	| TOK_FUNC_IN '(' arg ',' constlist_or_uservar ')'{ $$ = pParser->AddNodeFunc ( $1, $3, $5 ); }
	| TOK_HOOK_FUNC '(' arglist ')' { $$ = pParser->AddNodeHookFunc ( $1, $3 ); if ( $$<0 ) YYERROR; }
	| TOK_FUNC '(' expr for_loop ')' { $$ = pParser->AddNodeFunc ( $1, $3, $4 ); }
	;

json_field:
	TOK_ATTR_JSON subscript			{ $$ = pParser->AddNodeJsonField ( $1, $2 ); }

subscript:
	subkey
	| subscript subkey				{ $$ = pParser->AddNodeOp ( ',', $1, $2 ); }
	;

subkey:
	'[' expr ']'					{ $$ = $2; }
	| TOK_SUBKEY					{ $$ = pParser->AddNodeJsonSubkey ( $1 ); }
	| TOK_DOT_NUMBER				{ $$ = pParser->AddNodeJsonSubkey ( $1 ); }
	| '[' TOK_CONST_STRING ']'		{ $$ = pParser->AddNodeString ( $2 ); }
	| '[' TOK_ATTR_STRING ']'		{ $$ = pParser->AddNodeAttr ( TOK_ATTR_STRING, $2 ); }
	;

for_loop:
	TOK_FOR TOK_IDENT TOK_FUNC_IN json_field { $$ = pParser->AddNodeIdent ( $2, $4 ); }
	;

iterator:
	TOK_IDENT						{ $$ = pParser->AddNodeIdent ( $1, -1 ); }
	| TOK_IDENT subscript			{ $$ = pParser->AddNodeIdent ( $1, $2 ); }
	;

streq:
	expr TOK_EQ strval				{ $$ = pParser->AddNodeOp ( TOK_EQ, $1, $3 ); }
	| strval TOK_EQ expr			{ $$ = pParser->AddNodeOp ( TOK_EQ, $3, $1 ); }
	;

strval:
	TOK_CONST_STRING				{ $$ = pParser->AddNodeString ( $1 ); }
	| TOK_ATTR_STRING				{ $$ = pParser->AddNodeAttr ( TOK_ATTR_STRING, $1 ); }
	;


%%
