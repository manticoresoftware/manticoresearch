man1_MANS = searchd.1 indexer.1 indextool.1 spelldump.1

# apt-get install docbook-xsl
# apt-get install xsltproc
#

DOCBOOKXSL = /usr/share/xml/docbook/stylesheet/docbook-xsl
XSLTARGSMAN = $(DOCBOOKXSL)/manpages/docbook.xsl

all:
	@echo "All docs are already pre-built by developer."
	@echo "If you want to rebuild them, install docbook-xsl"
	@echo "and xsltproc and then run 'make docs' instead of simple 'make'."

docs: sphinx.html sphinx.txt my_manpages

my_manpages: manpages.xml
	xsltproc $(XSLTARGSMAN) manpages.xml

sphinx.html: sphinx.xml
	sed -r 's/<b>/<emphasis role="bold">/g;s/<\/b>/<\/emphasis>/g;s/(fixed|bug) #([0-9]+)/\1 <ulink url="http:\/\/sphinxsearch.com\/bugs\/view.php\?id=\2">\#\2<\/ulink>/g' sphinx.xml \
		| xsltproc sphinx.xsl - \
		| sed -r 's/\xA0/\&nbsp;/g;s/\xA9/\&copy;/g;s/((<\/(li|dt|dt|head|div)>)+)/\1\n/g;s/<a name="id[0-9]+"><\/a>//g;s/<\/head>/\n<style type="text\/css">pre.programlisting { background-color: #f0f0f0; padding: 0.5em; margin-left: 2em; margin-right: 2em; }<\/style>\n<\/head>/' \
		> sphinx.html

sphinx.txt: sphinx.html
	perl html2txt.pl < sphinx.html > sphinx.txt
