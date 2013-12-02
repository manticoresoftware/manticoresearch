@echo off

bison -l -d -o yysphinxexpr.c sphinxexpr.y
bison -l -d -o yysphinxjson.c sphinxjson.y
bison -l -d -o yysphinxselect.c sphinxselect.y
bison -l -d -o yysphinxquery.c sphinxquery.y
bison -l -d -o yysphinxql.c sphinxql.y
bison -l -d -o yysphinxjson.c sphinxjson.y
flex -i -ollsphinxql.c sphinxql.l
flex -i -ollsphinxjson.c -Pyy2 sphinxjson.l

perl -npe "s/  __attr/\/\/  __attr/" -i.bak yysphinxexpr.c
perl -npe "s/^yyerrlab1:/\/\/yyerrlab1:/m;s/  __attr/\/\/  __attr/" -i.bak yysphinxselect.c
perl -npe "s/^yyerrlab1:/\/\/yyerrlab1:/m;s/  __attr/\/\/  __attr/" -i.bak yysphinxquery.c
perl -npe "s/  __attr/\/\/  __attr/" -i.bak yysphinxjson.c
perl -npe "s/(#include <unistd.h>)/#if !USE_WINDOWS\n\1\n#endif/;s/\(size_t\) num_to_read/num_to_read/" -i.bak llsphinxql.c
perl -npe "s/\(size_t\) num_to_read/num_to_read/" -i.bak llsphinxjson.c
perl -npe "s/size_t n; \\\\/int n; \\\\/" -i.bak llsphinxql.c
perl -npe "s/size_t n; \\\\/int n; \\\\/" -i.bak llsphinxjson.c

if exist ..\.git todos yysphinxql.patch
patch -s -p0 -i yysphinxql.patch
if exist ..\.git fromdos yysphinxql.patch

rem fix buffer overflows in generated files
perl -npe "s/if \(yycheck/if \(yyx\+yyn<int\(sizeof\(yycheck\)\/sizeof\(yycheck\[0\]\)\) && yycheck/" -i.bak yysphinxexpr.c
perl -npe "s/if \(yycheck/if \(yyx\+yyn<int\(sizeof\(yycheck\)\/sizeof\(yycheck\[0\]\)\) && yycheck/" -i.bak yysphinxselect.c
perl -npe "s/if \(yycheck/if \(yyx\+yyn<int\(sizeof\(yycheck\)\/sizeof\(yycheck\[0\]\)\) && yycheck/" -i.bak yysphinxquery.c
perl -npe "s/if \(yycheck/if \(yyx\+yyn<int\(sizeof\(yycheck\)\/sizeof\(yycheck\[0\]\)\) && yycheck/" -i.bak yysphinxql.c
perl -npe "s/if \(yycheck/if \(yyx\+yyn<int\(sizeof\(yycheck\)\/sizeof\(yycheck\[0\]\)\) && yycheck/" -i.bak yysphinxjson.c

if exist ..\.git (
	fromdos llsphinxjson.c
	fromdos llsphinxql.c
	fromdos yysphinxexpr.c
	fromdos yysphinxjson.c
	fromdos yysphinxql.c
	fromdos yysphinxquery.c
	fromdos yysphinxselect.c
	fromdos yysphinxexpr.h
	fromdos yysphinxql.h
	fromdos yysphinxquery.h
	fromdos yysphinxselect.h
	fromdos yysphinxjson.h
)

del /q *.bak 2>nul
del /q yysphinxql.c.orig 2>nul
