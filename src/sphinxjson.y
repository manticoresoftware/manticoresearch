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

%%

json:
	'{' opt_key_value_list '}'
	;

opt_key_value_list:
	| key_value_list
	;

key_value_list:
	key_value
	| key_value_list ',' key_value
	;

key_value:
	key ':' value
		{
			if ( !pParser->Add ( $1.m_sValue.cstr(), $3 ) )
				YYERROR;
		}
	;

key:
	TOK_STRING
	| TOK_IDENT
	;

value:
	TOK_INT
	| TOK_FLOAT
	| TOK_STRING
	| '[' string_list ']'				{ $$ = $2; }
	;

string_list:
	TOK_STRING
		{
			$$.m_eType = JSON_STRING_VECTOR;
			$$.m_dValue.Reset();
			$$.m_dValue.Add ( $1.m_sValue );
		}
	| string_list ',' TOK_STRING
		{
			$$ = $1;
			$$.m_dValue.Add ( $3.m_sValue );
			assert ( $$.m_eType==JSON_STRING_VECTOR );
		}
	;

%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
