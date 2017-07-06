#!/bin/sh
DOCBOOKXSL="/usr/share/xml/docbook/stylesheet/docbook-xsl"
CHUNKED=$1
if [ "$CHUNKED" = "chunked" ] ; then
 XSLTARGS="--stringparam toc.section.depth 1 --stringparam generate.section.toc.level 2 --stringparam chunk.first.sections 1 --stringparam chunk.section.depth 2 --stringparam base.dir chunked/  --stringparam use.id.as.filename 1 ${DOCBOOKXSL}/html/chunk.xsl"
else
 XSLTARGS="--stringparam toc.section.depth 4 ${DOCBOOKXSL}/html/docbook.xsl"
fi
sed -r 's/<b>/<emphasis role="bold">/g;s/<\/b>/<\/emphasis>/g;s/(fixed|bug) #([0-9]+)/\1 <ulink url="http:\/\/sphinxsearch.com\/bugs\/view.php\?id=\2">\#\2<\/ulink>/g' sphinx.xml \
	 | xsltproc --nonet --stringparam section.autolabel 1 --stringparam section.label.includes.component.label 1 ${XSLTARGS} - \
	| sed -r 's/\xA0/\&nbsp;/g;s/\xA9/\&copy;/g;s/((<\/(li|dt|dt|head|div)>)+)/\1\n/g;s/<a name="id[0-9]+"><\/a>//g;s/<\/head>/\n<style type="text\/css">pre.programlisting { background-color: #f0f0f0; padding: 0.5em; margin-left: 2em; margin-right: 2em; }<\/style>\n<\/head>/' \
	> sphinx.html

perl html2txt.pl < sphinx.html > sphinx.txt

