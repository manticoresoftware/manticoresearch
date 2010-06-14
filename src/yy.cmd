@echo off

bison -l -d -o yysphinxexpr.c sphinxexpr.y
bison -l -d -o yysphinxselect.c sphinxselect.y
bison -l -d -o yysphinxquery.c sphinxquery.y
bison -l -d -o yysphinxql.c sphinxql.y
flex -i -ollsphinxql.c sphinxql.l

perl -npe "s/  __attr/\/\/  __attr/" -i.bak yysphinxexpr.c
perl -npe "s/^yyerrlab1:/\/\/yyerrlab1:/m;s/  __attr/\/\/  __attr/" -i.bak yysphinxselect.c
perl -npe "s/^yyerrlab1:/\/\/yyerrlab1:/m;s/  __attr/\/\/  __attr/" -i.bak yysphinxquery.c
perl -npe "s/(#include <unistd.h>)/#if !USE_WINDOWS\n\1\n#endif/;s/\(size_t\) num_to_read/num_to_read/" -i.bak llsphinxql.c
patch -s -p0 -i yysphinxql.patch

rm -f *.bak
rm -f yysphinxql.c.orig
