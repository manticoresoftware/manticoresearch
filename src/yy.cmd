@echo off
bison -l -d -o yysphinxexpr.c sphinxexpr.y
bison -l -d -o yysphinxselect.c sphinxselect.y
bison -l -d -o yysphinxquery.c sphinxquery.y

type yysphinxexpr.c | perl -npe "s/  __attr/\/\/  __attr/" > yysphinxexpr.new
type yysphinxselect.c | perl -npe "s/^yyerrlab1:/\/\/yyerrlab1:/m" | perl -npe "s/  __attr/\/\/  __attr/" > yysphinxselect.new
type yysphinxquery.c | perl -npe "s/^yyerrlab1:/\/\/yyerrlab1:/m" | perl -npe "s/  __attr/\/\/  __attr/" > yysphinxquery.new

del /q yy*.c
rename yy*.new yy*.c
