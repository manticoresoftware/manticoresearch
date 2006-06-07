CCFLAGS         = -Wall -O3 -fomit-frame-pointer  -D_FILE_OFFSET_BITS=64

SRC_SPHINX      = sphinx.cpp sphinxstemen.cpp sphinxstemru.cpp sphinxutils.cpp sphinxexcerpt.cpp sphinxsoundex.cpp
HEADERS_SPHINX  = sphinxexcerpt.h  sphinx.h  sphinxstd.h  sphinxstem.h  sphinxtimers.h  sphinxutils.h sphinxstemru.inl

noinst_LIBRARIES = libsphinx.a
libsphinx_a_SOURCES = $(SRC_SPHINX)
libsphinx_a_CPPFLAGS = $(MYSQL_CFLAGS) $(CCFLAGS)

noinst_HEADERS = $(HEADERS_SPHINX)


bin_PROGRAMS = indexer searchd search

indexer_SOURCES= indexer.cpp
searchd_SOURCES= searchd.cpp
search_SOURCES= search.cpp

searchd_CPPFLAGS= $(MYSQL_CFLAGS) $(CCFLAFS)
indexer_CPPFLAGS= $(MYSQL_CFLAGS) $(CCFLAGS)
search_CPPFLAGS= $(MYSQL_CFLAGS) $(CCFLAGS)

indexer_LDADD= libsphinx.a $(MYSQL_LIBS)
searchd_LDADD= libsphinx.a $(MYSQL_LIBS)
search_LDADD= libsphinx.a $(MYSQL_LIBS)


EXTRA_DIST= indexer.vcproj  libsphinx.vcproj  searchd.vcproj  search.vcproj

