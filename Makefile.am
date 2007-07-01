if USE_LIBSTEMMER
SUBDIRS = libstemmer_c src
else
SUBDIRS = src
endif

EXTRA_DIST = api storage sphinx.conf.in sphinx.sln todo.txt example.sql
sysconf_DATA = sphinx.conf.dist example.sql

install-data-hook:
	mkdir -p $(localstatedir)/data && mkdir -p $(localstatedir)/log


