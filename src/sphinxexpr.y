%lex-param		{ ExprParser_t * pParser }
%parse-param	{ ExprParser_t * pParser }
%pure-parser
%error-verbose

%union {
	int64_t			iConst;			// constant value
	float			fConst;			// constant value
	int				iAttrLocator;	// attribute locator (rowitem for int/float; offset+size for bits)
	int				iFunc;			// function id
	int				iNode;			// node index
};

%token <iConst>			TOK_CONST_INT
%token <fConst>			TOK_CONST_FLOAT
%token <iAttrLocator>	TOK_ATTR_INT
%token <iAttrLocator>	TOK_ATTR_BITS
%token <iAttrLocator>	TOK_ATTR_FLOAT
%token <iAttrLocator>	TOK_ATTR_MVA
%token <iFunc>			TOK_FUNC
%token <iFunc>			TOK_FUNC_IN

%token	TOK_ID
%token	TOK_WEIGHT

%type <iNode>			attr
%type <iNode>			expr
%type <iNode>			arglist
%type <iNode>			function

%left TOK_AND TOK_OR
%nonassoc TOK_NOT
%left TOK_EQ TOK_NE
%left '<' '>' TOK_LTE TOK_GTE
%left '+' '-'
%left '*' '/'
%nonassoc TOK_NEG

%%

exprline:
	expr							{ pParser->m_iParsed = $1; }
	;

attr:
	TOK_ATTR_INT					{ $$ = pParser->AddNodeAttr ( TOK_ATTR_INT, $1 ); }
	| TOK_ATTR_BITS					{ $$ = pParser->AddNodeAttr ( TOK_ATTR_BITS, $1 ); }
	| TOK_ATTR_FLOAT				{ $$ = pParser->AddNodeAttr ( TOK_ATTR_FLOAT, $1 ); }
	;

expr:
	attr
	| function
	| TOK_CONST_INT					{ $$ = pParser->AddNodeInt ( $1 ); }
	| TOK_CONST_FLOAT				{ $$ = pParser->AddNodeFloat ( $1 ); }
	| TOK_ID						{ $$ = pParser->AddNodeID(); }
	| TOK_WEIGHT					{ $$ = pParser->AddNodeWeight(); }
	| '-' expr %prec TOK_NEG		{ $$ = pParser->AddNodeOp ( TOK_NEG, $2, -1 ); }
	| TOK_NOT expr					{ $$ = pParser->AddNodeOp ( TOK_NOT, $2, -1 ); if ( $$<0 ) YYERROR; }
	| expr '+' expr					{ $$ = pParser->AddNodeOp ( '+', $1, $3 ); }
	| expr '-' expr					{ $$ = pParser->AddNodeOp ( '-', $1, $3 ); }
	| expr '*' expr					{ $$ = pParser->AddNodeOp ( '*', $1, $3 ); }
	| expr '/' expr					{ $$ = pParser->AddNodeOp ( '/', $1, $3 ); }
	| expr '<' expr					{ $$ = pParser->AddNodeOp ( '<', $1, $3 ); }
	| expr '>' expr					{ $$ = pParser->AddNodeOp ( '>', $1, $3 ); }
	| expr TOK_LTE expr				{ $$ = pParser->AddNodeOp ( TOK_LTE, $1, $3 ); }
	| expr TOK_GTE expr				{ $$ = pParser->AddNodeOp ( TOK_GTE, $1, $3 ); }
	| expr TOK_EQ expr				{ $$ = pParser->AddNodeOp ( TOK_EQ, $1, $3 ); }
	| expr TOK_NE expr				{ $$ = pParser->AddNodeOp ( TOK_NE, $1, $3 ); }
	| expr TOK_AND expr				{ $$ = pParser->AddNodeOp ( TOK_AND, $1, $3 ); if ( $$<0 ) YYERROR; }
	| expr TOK_OR expr				{ $$ = pParser->AddNodeOp ( TOK_OR, $1, $3 ); if ( $$<0 ) YYERROR; }
	| '(' expr ')'					{ $$ = $2; }
	;

arglist:
	expr							{ $$ = $1; }
	| arglist ',' expr				{ $$ = pParser->AddNodeOp ( ',', $1, $3 ); }
	;

function:
	TOK_FUNC '(' arglist ')'		{ $$ = pParser->AddNodeFunc ( $1, $3 ); if ( $$<0 ) YYERROR; }
	| TOK_FUNC '(' ')'				{ $$ = pParser->AddNodeFunc ( $1, -1 ); if ( $$<0 ) YYERROR; }
	| TOK_FUNC_IN '(' attr ',' arglist ')'
		{
			$$ = pParser->AddNodeFunc ( $1, $3, $5 );
		}
	| TOK_FUNC_IN '(' TOK_ATTR_MVA ',' arglist ')'
		{
			$$ = pParser->AddNodeAttr ( TOK_ATTR_MVA, $3 );
			$$ = pParser->AddNodeFunc ( $1, $$, $5 );
		}
	;

%%
