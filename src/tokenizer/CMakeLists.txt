cmake_minimum_required ( VERSION 3.20 )

# compile globalaliases.h
include ( genheader )
set ( CHARSET_FILTER "${MANTICORE_SOURCE_DIR}/src/charsets/*.txt" )
set ( CHARSET_TEMPLATE "${MANTICORE_SOURCE_DIR}/src/charsets/globalaliases.h.in" )
hsnippetfolder ( "${CHARSET_FILTER}" "${CHARSET_TEMPLATE}" )

# populate charset files for IDE
file ( GLOB CHARSET_FILES "${CHARSET_FILTER}" )
source_group ( "Charset sources" FILES ${CHARSET_FILES} ${CHARSET_TEMPLATE} )

add_library ( lowercaser lowercaser.cpp charset_definition_parser.cpp remap_range.h lowercaser_impl.h lowercaser.h
		charset_definition_parser.h )
target_link_libraries ( lowercaser PRIVATE lextra )

add_library ( tokenizer tokenizer.cpp tokenizer_bigram.cpp tokenizer_multiform.cpp tokenizer_plugin.cpp
		tokenizer_utf8.cpp tokenizerbase.cpp tokenizerbase2.cpp exceptions_trie.cpp exceptions_trie.h
		tokenizerbase_impl.h tokenizerbase2_impl.h multiform_container.h token_filter.h tokenizer.h
		tok_internals.h )
target_link_libraries ( tokenizer PRIVATE lextra lowercaser )

