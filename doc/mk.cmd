@echo off

type sphinx.xml ^
	| perl -pe "s/<b>/<emphasis role=\"bold\">/g" ^
	| perl -pe "s/<\/b>/<\/emphasis>/g" ^
	| perl -pe "s/bug #(\d+)/<ulink url=\"http:\/\/sphinxsearch.com\/bugs\/view.php\?id=\1\">bug #\1<\/ulink>/" ^
	| xsltproc ^
		--stringparam section.autolabel 1 ^
		--stringparam output.html.stylesheets 1 ^
		--stringparam html.stylesheet %CD%/sphinx.css ^
		--stringparam toc.section.depth 4 ^
		%DOCBOOKXSL%/html/docbook.xsl ^
		- ^
	| perl -pe "s/\xA0/\&nbsp;/g" ^
	| perl -pe "s/\xA9/\&copy;/g" ^
	| perl -pe "s/((<\/(li|dt|dt|head|div)>)+)/\1\n/g" ^
	| perl -pe "s/<a name=\"id\d+\"><\/a>//g" ^
	> sphinx.html

perl html2txt.pl < sphinx.html > sphinx.txt
