%{
#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif
%}

%lex-param		{ JsonParser_c * pParser }
%parse-param	{ JsonParser_c * pParser }
%pure-parser
%error-verbose

%token	TOK_IDENT
%token	TOK_INT
%token	TOK_FLOAT
%token	TOK_STRING
%token	TOK_TRUE
%token	TOK_FALSE
%token	TOK_NULL

%%

json:
	// empty
	| '{' key_value_list '}'	{ if ( !pParser->WriteNode ( $2 ) ) YYERROR; }
	| '[' value_list ']' 		{ if ( !pParser->WriteNode ( $2 ) ) YYERROR; }
	;

key:
	TOK_STRING
	| TOK_IDENT
	;

value:
	TOK_INT
	| TOK_FLOAT
	| TOK_STRING
	| TOK_TRUE
	| TOK_FALSE
	| TOK_NULL
	| '{' key_value_list '}'	{ $$=$2; }
	| '[' value_list ']' 		{ $$=$2; }
	;

key_value_list:
	// empty
		{
			$$.m_eType = JSON_OBJECT;
			$$.m_iHandle = -1;
		}
	| key ':' value
		{ 
			$$.m_eType = JSON_OBJECT;
			$$.m_iHandle = pParser->m_dNodes.GetLength();
			$3.m_iKeyStart = $1.m_iStart;
			$3.m_iKeyEnd = $1.m_iEnd;
			pParser->m_dNodes.Add().Add($3);
		}
	| key_value_list ',' key ':' value
		{ 
			if ( $1.m_iHandle<0 )
			{
				yyerror ( pParser, "unexpected ','" );
				YYERROR;
			}
			$$ = $1;
			$5.m_iKeyStart = $3.m_iStart;
			$5.m_iKeyEnd = $3.m_iEnd;
			pParser->m_dNodes[$$.m_iHandle].Add($5);
		}
	;

value_list:
	// empty
		{
			$$.m_eType = JSON_MIXED_VECTOR;
			$$.m_iHandle = -1;
		}
	| value
		{
			$$.m_eType = JSON_MIXED_VECTOR;
			$$.m_iHandle = pParser->m_dNodes.GetLength();
			pParser->m_dNodes.Add().Add($1);
		}
	| value_list ',' value
		{
			if ( $1.m_iHandle<0 )
			{
				yyerror ( pParser, "unexpected ','" );
				YYERROR;
			}
			$$ = $1;
			pParser->m_dNodes[$$.m_iHandle].Add($3);
		}
	;

%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
