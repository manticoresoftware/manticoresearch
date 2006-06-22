CCFLAGS = -Wall -O3 -fomit-frame-pointer -D_FILE_OFFSET_BITS=64

SRC_SPHINX = sphinx.cpp sphinxexcerpt.cpp sphinxquery.cpp sphinxsoundex.cpp sphinxstemen.cpp sphinxstemru.cpp sphinxutils.cpp md5.cpp
HEADERS_SPHINX = sphinx.h sphinxexcerpt.h sphinxquery.h sphinxstd.h sphinxstem.h sphinxtimers.h sphinxutils.h sphinxstemru.inl md5.h

noinst_LIBRARIES = libsphinx.a
libsphinx_a_SOURCES = $(SRC_SPHINX)
libsphinx_a_CPPFLAGS = $(MYSQL_CFLAGS) $(PGSQL_CFLAGS) $(CCFLAGS)

noinst_HEADERS = $(HEADERS_SPHINX)

bin_PROGRAMS = indexer searchd search

indexer_SOURCES = indexer.cpp
searchd_SOURCES = searchd.cpp
search_SOURCES = search.cpp

searchd_CPPFLAGS = $(MYSQL_CFLAGS) $(PGSQL_CFLAGS) $(CCFLAFS)
indexer_CPPFLAGS = $(MYSQL_CFLAGS) $(PGSQL_CFLAGS) $(CCFLAGS)
search_CPPFLAGS = $(MYSQL_CFLAGS) $(PGSQL_CFLAGS) $(CCFLAGS)

indexer_LDADD = libsphinx.a $(MYSQL_LIBS) $(PGSQL_LIBS)
searchd_LDADD = libsphinx.a $(MYSQL_LIBS) $(PGSQL_LIBS)
search_LDADD = libsphinx.a $(MYSQL_LIBS) $(PGSQL_LIBS)

EXTRA_DIST = indexer.vcproj libsphinx.vcproj searchd.vcproj search.vcproj
