@echo off
bison -l -d -o yysphinxexpr.c sphinxexpr.y
bison -l -d -o yysphinxselect.c sphinxselect.y
bison -l -d -o yysphinxquery.c sphinxquery.y
