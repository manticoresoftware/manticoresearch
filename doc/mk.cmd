@echo off

cd | perl -pe s/\\/\//g > 1.tmp
for /F "usebackq" %%i in (1.tmp) do set PWD=%%i
type sphinx.xml ^
	| perl -pe "s/<b>/<emphasis role=\"bold\">/g" ^
	| perl -pe "s/<\/b>/<\/emphasis>/g" ^
	> 1.tmp
xsltproc ^
	--stringparam section.autolabel 1 ^
	--stringparam output.html.stylesheets 1 ^
	--stringparam html.stylesheet %PWD%/sphinx.css ^
	--stringparam toc.section.depth 4 ^
	%DOCBOOKXSL%/html/docbook.xsl 1.tmp ^
	| perl -pe "s/\xA0/\&nbsp;/g" ^
	| perl -pe "s/\xA9/\&copy;/g" ^
	> sphinx.html
del 1.tmp

perl html2txt.pl < sphinx.html > sphinx.txt
