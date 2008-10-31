SRC_SPHINX = sphinx.cpp sphinxexcerpt.cpp sphinxquery.cpp \
	sphinxsoundex.cpp sphinxmetaphone.cpp sphinxstemen.cpp sphinxstemru.cpp sphinxstemcz.cpp \
	sphinxutils.cpp md5.cpp sphinxstd.cpp sphinxsort.cpp sphinxexpr.cpp sphinxfilter.cpp

noinst_LIBRARIES = libsphinx.a
libsphinx_a_SOURCES = $(SRC_SPHINX)

bin_PROGRAMS = indexer searchd search spelldump
noinst_PROGRAMS = tests

indexer_SOURCES = indexer.cpp
searchd_SOURCES = searchd.cpp
search_SOURCES = search.cpp
spelldump_SOURCES = spelldump.cpp
tests_SOURCES = tests.cpp

BUILT_SOURCES = extract-version
.PHONY: extract-version
extract-version:
	if test -d ../.svn; then svn info .. --xml | perl svnxrev.pl; fi;

if USE_LIBSTEMMER
LIBSTEMMER_LIBS = $(top_srcdir)/libstemmer_c/libstemmer.a
AM_CPPFLAGS = -I$(top_srcdir)/libstemmer_c/include -DSYSCONFDIR="\"$(sysconfdir)\""
else
LIBSTEMMER_LIBS =
AM_CPPFLAGS = -DSYSCONFDIR="\"$(sysconfdir)\""
endif

COMMON_LIBS = libsphinx.a $(LIBSTEMMER_LIBS) $(MYSQL_LIBS) $(PGSQL_LIBS)
LDADD = $(COMMON_LIBS)

EXTRA_DIST = indexer.vcproj libsphinx.vcproj searchd.vcproj search.vcproj
