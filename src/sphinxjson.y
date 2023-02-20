%{
#if _WIN32
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
%}

%lex-param	{ JsonParser_c * pParser }
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

key_value_pair:
	key ':' value
	{
		$$ = $3;
		$$.m_sName = $1.m_sValue;
	}

key_value_list:
	// empty
		{
			$$ = JsonNode_t(JSON_OBJECT);
		}
	| key_value_pair
		{
			$$ = JsonNode_t(JSON_OBJECT);
			$$.m_dChildren.m_iStart = $$.m_iNext = pParser->m_dNodes.GetLength();
			$$.m_dChildren.m_iLen = 1;
			pParser->AddNode($1);

		}
	| key_value_list ',' key_value_pair
		{
			if ( !$1.m_dChildren.m_iLen )
			{
				yyerror ( pParser, "unexpected ','" );
				YYERROR;
			}
			$$ = $1;
			++$$.m_dChildren.m_iLen;
			$$.m_iNext = pParser->m_dNodes[$$.m_iNext].m_iNext = pParser->m_dNodes.GetLength();
			pParser->AddNode($3);
		}
	;

value_list:
	// empty
		{
			$$ = JsonNode_t(JSON_MIXED_VECTOR);
		}
	| value
		{
			$$ = JsonNode_t(JSON_MIXED_VECTOR);
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
			$$ = $1;
			++$$.m_dChildren.m_iLen;
			$$.m_iNext = pParser->m_dNodes[$$.m_iNext].m_iNext = pParser->m_dNodes.GetLength();
			pParser->AddNode($3);
		}
	;

%%

#if _WIN32
#pragma warning(pop)
#endif
