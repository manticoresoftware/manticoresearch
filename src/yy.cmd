@echo off
bison -l -d -o yysphinxexpr.c sphinxexpr.y
bison -l -d -o yysphinxselect.c sphinxselect.y
bison -l -d -o yysphinxquery.c sphinxquery.y
bison -l -d -o yysphinxql.c sphinxql.y
flex -i -ollsphinxql.c sphinxql.l

type yysphinxexpr.c | perl -npe "s/  __attr/\/\/  __attr/" > yysphinxexpr.new
type yysphinxselect.c | perl -npe "s/^yyerrlab1:/\/\/yyerrlab1:/m" | perl -npe "s/  __attr/\/\/  __attr/" > yysphinxselect.new
type yysphinxquery.c | perl -npe "s/^yyerrlab1:/\/\/yyerrlab1:/m" | perl -npe "s/  __attr/\/\/  __attr/" > yysphinxquery.new
type yysphinxql.c | perl -npe "s/  __attr/\/\/  __attr/" > yysphinxql.new
type llsphinxql.c | perl -npe "s/(#include <unistd.h>)/#if !USE_WINDOWS\n\1\n#endif/" > llsphinxql.new

del /q yy*.c ll*.c
rename yy*.new yy*.c
rename ll*.new ll*.c
