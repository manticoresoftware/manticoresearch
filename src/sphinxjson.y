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
	| '{' key_value_list '}'	{ if ( !pParser->WriteRoot ( $2 ) ) YYERROR; }
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
			$$ = JsonNode_t();
			$$.m_eType = JSON_OBJECT;
			$$.m_dChildren.m_iStart = -1;
			$$.m_dChildren.m_iLen = 0;

		}
	| key ':' value
		{
			$$ = JsonNode_t();
			$$.m_eType = JSON_OBJECT;
			$$.m_dChildren.m_iStart = $$.m_iNext = pParser->m_dNodes.GetLength();
			$$.m_dChildren.m_iLen = 1;
			$3.m_sName = $1.m_sValue;
			pParser->AddNode($3);

		}
	| key_value_list ',' key ':' value
		{
			if ( !$1.m_dChildren.m_iLen )
			{
				yyerror ( pParser, "unexpected ','" );
				YYERROR;
			}
			$$ = JsonNode_t();
			$$.m_eType = JSON_OBJECT;
			$$.m_dChildren.m_iStart = $1.m_dChildren.m_iStart;
			$$.m_dChildren.m_iLen = $1.m_dChildren.m_iLen+1;
			$$.m_iNext = pParser->m_dNodes[$1.m_iNext].m_iNext = pParser->m_dNodes.GetLength();
			$5.m_sName = $3.m_sValue;
			pParser->AddNode($5);
		}
	;

value_list:
	// empty
		{
			$$ = JsonNode_t();
			$$.m_eType = JSON_MIXED_VECTOR;
			$$.m_dChildren.m_iStart = -1;
			$$.m_dChildren.m_iLen = 0;
		}
	| value
		{
			$$ = JsonNode_t();
			$$.m_eType = JSON_MIXED_VECTOR;
			$$.m_dChildren.m_iStart = $$.m_iNext = pParser->m_dNodes.GetLength();
			$$.m_dChildren.m_iLen = 1;
			pParser->AddNode($1);
		}
	| value_list ',' value
		{
			if ( !$1.m_dChildren.m_iLen )
			{
				yyerror ( pParser, "unexpected ','" );
				YYERROR;
			}
			$$ = JsonNode_t();
			$$.m_eType = JSON_MIXED_VECTOR;
            $$.m_dChildren.m_iStart = $1.m_dChildren.m_iStart;
            $$.m_dChildren.m_iLen = $1.m_dChildren.m_iLen+1;
			$$.m_iNext = pParser->m_dNodes[$1.m_iNext].m_iNext = pParser->m_dNodes.GetLength();
			pParser->AddNode($3);
		}
	;

%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
