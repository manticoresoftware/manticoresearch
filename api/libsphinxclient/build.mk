
SUPPRESS_WARNINGS = 2>&1 | (egrep -v '(AC_TRY_RUN called without default to allow cross compiling|AC_PROG_CXXCPP was called before AC_PROG_CXX|defined in acinclude.m4 but never used|AC_PROG_LEX invoked multiple times|AC_DECL_YYTEXT is expanded from...|the top level)'||true)

AUTOCONF ?= 'autoconf'
ACLOCAL ?= 'aclocal'
AUTOHEADER ?= 'autoheader'
AUTOMAKE ?= 'automake'
AUTOUPDATE ?= 'autoupdate'
LIBTOOLIZE ?= 'libtoolize'

config_h_in = sphinxclient_config.h.in
targets = $(config_h_in) configure makefiles

all: $(targets)

aclocal.m4:
	$(ACLOCAL)

$(config_h_in): configure
	@echo rebuilding $@
	@rm -f $@
	$(AUTOHEADER) $(SUPPRESS_WARNINGS)

configure: aclocal.m4 configure.in
	@echo rebuilding $@
	$(LIBTOOLIZE) --copy
	$(AUTOCONF) $(SUPPRESS_WARNINGS)

makefiles: configure Makefile.am
	@echo rebuilding Makefile.in files
	$(AUTOMAKE) --add-missing --copy

cvsclean:
	@rm -rf *.lo *.la *.o *.a .libs Makefile Makefile.in stamp-h1 test sphinxclient_config.h*
	rm -rf aclocal.m4 autom4te.cache install.sh libtool Makefile Makefile.in 'configure.in~' missing config.h* configure
	rm -f config.guess config.log config.status config.sub cscope.out install-sh ltmain.sh

