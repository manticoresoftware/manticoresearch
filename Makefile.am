if USE_LIBSTEMMER
SUBDIRS = libstemmer_c src
AM_CPPFLAGS = -I$(top_srcdir)/libstemmer_c/include
else
SUBDIRS = src
endif

EXTRA_DIST = api storage sphinx.conf.in sphinx.sln todo.txt example.sql
sysconf_DATA = sphinx.conf.dist example.sql

install-data-hook:
	mkdir -p $(localstatedir)/data && mkdir -p $(localstatedir)/log


