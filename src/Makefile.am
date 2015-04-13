SRC_SPHINX = sphinx.cpp sphinxexcerpt.cpp sphinxquery.cpp \
	sphinxsoundex.cpp sphinxmetaphone.cpp sphinxstemen.cpp sphinxstemru.cpp sphinxstemcz.cpp sphinxstemar.cpp \
	sphinxutils.cpp sphinxstd.cpp sphinxsort.cpp sphinxexpr.cpp sphinxfilter.cpp \
	sphinxsearch.cpp sphinxrt.cpp sphinxjson.cpp sphinxudf.c sphinxaot.cpp sphinxplugin.cpp sphinxqcache.cpp

noinst_LIBRARIES = libsphinx.a
libsphinx_a_SOURCES = $(SRC_SPHINX)

bin_PROGRAMS = indexer searchd spelldump indextool wordbreaker
noinst_PROGRAMS = tests

indexer_SOURCES = indexer.cpp
searchd_SOURCES = searchd.cpp searchdha.cpp http/http_parser.c searchdhttp.cpp
spelldump_SOURCES = spelldump.cpp
indextool_SOURCES = indextool.cpp
tests_SOURCES = tests.cpp
wordbreaker_SOURCES = wordbreaker.cpp

BUILT_SOURCES = extract-version
.PHONY: extract-version
extract-version:
	/bin/sh svnxrev.sh ..

if USE_RLP
RLP_LIBS = -L$(top_srcdir)/rlp/lib/amd64-glibc25-gcc42 -lbtrlpc -lbtrlpcore -lbtutils
RLP_INC = -I$(top_srcdir)/rlp/rlp/include -I$(top_srcdir)/rlp/utilities/include -D_REENTRANT
else
RLP_LIBS =
RLP_INC =
endif

AM_CPPFLAGS = $(LIBRE2_CFLAGS) $(RLP_INC) -DSYSCONFDIR="\"$(sysconfdir)\"" -DDATADIR="\"$(localstatedir)/data\""
COMMON_LIBS = libsphinx.a $(LIBSTEMMER_LIBS) $(MYSQL_LIBS) $(PGSQL_LIBS) $(LIBRE2_LIBS) $(RLP_LIBS)
LDADD = $(COMMON_LIBS)
