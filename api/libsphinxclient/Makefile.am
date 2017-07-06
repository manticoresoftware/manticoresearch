AUTOMAKE_OPTIONS = foreign no-dependencies

lib_LTLIBRARIES = libsphinxclient.la
noinst_PROGRAMS = test

libsphinxclient_la_SOURCES = sphinxclient.c 
test_SOURCES = test.c

libsphinxclient_la_LIBADD = @LTLIBOBJS@
libsphinxclient_la_LDFLAGS = -release @VERSION@

include_HEADERS =   sphinxclient.h 
test_LDADD = .libs/libsphinxclient.a
