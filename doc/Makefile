#
# apt-get install docbook-xsl
# apt-get install xsltproc
#

DOCBOOKXSL = /usr/share/xml/docbook/stylesheet/docbook-xsl
XSLTARGS = --stringparam toc.section.depth 4 $(DOCBOOKXSL)/html/docbook.xsl

all : sphinx.html sphinx.txt

sphinx.html: sphinx.xml
	cat sphinx.xml \
		| perl -pe "s/<b>/<emphasis role=\"bold\">/g" \
		| perl -pe "s/<\/b>/<\/emphasis>/g" \
		| perl -pe "s/(fixed|bug) #(\d+)/\1 <ulink url=\"http:\/\/sphinxsearch.com\/bugs\/view.php\?id=\2\">#\2<\/ulink>/" \
		| xsltproc \
			--stringparam section.autolabel 1 \
			--stringparam section.label.includes.component.label 1 \
			$(XSLTARGS) \
			- \
		| perl -pe "s/\xA0/\&nbsp;/g" \
		| perl -pe "s/\xA9/\&copy;/g" \
		| perl -pe "s/((<\/(li|dt|dt|head|div)>)+)/\1\n/g" \
		| perl -pe "s/<a name=\"id\d+\"><\/a>//g" \
		| perl -pe "s/<\/head>/\n<style type=\"text\/css\">pre.programlisting { background-color: #f0f0f0; padding: 0.5em; margin-left: 2em; margin-right: 2em; }<\/style>\n<\/head>/" \
		> sphinx.html

sphinx.txt: sphinx.html
	perl html2txt.pl < sphinx.html > sphinx.txt
