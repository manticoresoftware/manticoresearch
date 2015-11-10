bison -l -d -o yysphinxexpr.c sphinxexpr.y
bison -l -d -o yysphinxselect.c sphinxselect.y
bison -l -d -o yysphinxquery.c sphinxquery.y
bison -l -d -o yysphinxql.c sphinxql.y
bison -l -d -o yysphinxjson.c sphinxjson.y
flex -i -ollsphinxql.c sphinxql.l
flex -i -ollsphinxjson.c -Pyy2 sphinxjson.l

perl -npe "s/\(size_t\) num_to_read/num_to_read/;s/size_t n; \\\\/int n; \\\\/" -i.bak llsphinxql.c
perl -npe "s/\(size_t\) num_to_read/num_to_read/;s/size_t n; \\\\/int n; \\\\/" -i.bak llsphinxjson.c

rm *.bak 2>/dev/null
rm yysphinxql.c.orig 2>/dev/null
