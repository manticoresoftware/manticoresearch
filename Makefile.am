if USE_INTERNAL_LIBSTEMMER
LIBSTEMMER_DIRS = libstemmer_c
else
LIBSTEMMER_DIRS =
endif

if USE_RE2
LIBRE2_DIRS = $(LIBRE2_PATH)
else
LIBRE2_DIRS =
endif

SUBDIRS = $(LIBSTEMMER_DIRS) $(LIBRE2_DIRS) src test doc

EXTRA_DIST = api storage sphinx.conf.in sphinx-min.conf.in example.sql
sysconf_DATA = sphinx.conf.dist sphinx-min.conf.dist example.sql

install-data-hook:
	mkdir -p $(DESTDIR)$(localstatedir)/data && mkdir -p $(DESTDIR)$(localstatedir)/log
