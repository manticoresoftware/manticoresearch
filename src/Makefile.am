SRC_SPHINX = sphinx.cpp sphinxexcerpt.cpp sphinxquery.cpp \
	sphinxsoundex.cpp sphinxmetaphone.cpp sphinxstemen.cpp sphinxstemru.cpp sphinxstemcz.cpp sphinxstemar.cpp \
	sphinxutils.cpp sphinxstd.cpp sphinxsort.cpp sphinxexpr.cpp sphinxfilter.cpp \
	sphinxsearch.cpp sphinxrt.cpp sphinxjson.cpp sphinxudf.c sphinxaot.cpp

noinst_LIBRARIES = libsphinx.a
libsphinx_a_SOURCES = $(SRC_SPHINX)

bin_PROGRAMS = indexer searchd search spelldump indextool wordbreaker
noinst_PROGRAMS = tests

indexer_SOURCES = indexer.cpp
searchd_SOURCES = searchd.cpp
search_SOURCES = search.cpp
spelldump_SOURCES = spelldump.cpp
indextool_SOURCES = indextool.cpp
tests_SOURCES = tests.cpp
wordbreaker_SOURCES = wordbreaker.cpp

BUILT_SOURCES = extract-version
.PHONY: extract-version
extract-version:
	/bin/sh svnxrev.sh ..

if USE_RE2
LIBRE2_LIBS = $(top_srcdir)/libre2/obj/libre2.a
LIBRE2_INC = -I$(top_srcdir)/libre2
else
LIBRE2_LIBS =
LIBRE2_INC =
endif

if USE_RLP
RLP_LIBS = -L$(top_srcdir)/rlp/lib/amd64-glibc25-gcc42 -lbtrlpc -lbtrlpcore -lbtutils
RLP_INC = -I$(top_srcdir)/rlp/rlp/include -I$(top_srcdir)/rlp/utilities/include -D_REENTRANT
else
LIBRLP_LIBS =
LIBRLP_INC =
endif

AM_CPPFLAGS = $(LIBRE2_INC) $(RLP_INC) -DSYSCONFDIR="\"$(sysconfdir)\"" -DDATADIR="\"$(localstatedir)/data\""
COMMON_LIBS = libsphinx.a $(LIBSTEMMER_LIBS) $(MYSQL_LIBS) $(PGSQL_LIBS) $(LIBRE2_LIBS) $(RLP_LIBS)
LDADD = $(COMMON_LIBS)
