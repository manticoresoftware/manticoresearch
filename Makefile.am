if USE_LIBSTEMMER
SUBDIRS = libstemmer_c src test
else
SUBDIRS = src test
endif

EXTRA_DIST = api storage sphinx.conf.in sphinx-min.conf.in example.sql
sysconf_DATA = sphinx.conf.dist sphinx-min.conf.dist example.sql

install-data-hook:
	mkdir -p $(DESTDIR)$(localstatedir)/data && mkdir -p $(DESTDIR)$(localstatedir)/log
