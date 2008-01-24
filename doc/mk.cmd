@echo off

pwd | sed -e s/\\/\//g > 1.tmp
for /F "usebackq" %%i in (1.tmp) do set PWD=%%i
cat sphinx.xml ^
	| sed -e "s/<b>/<emphasis role=\"bold\">/g" ^
	| sed -e "s/<\/b>/<\/emphasis>/g" ^
	> 1.tmp
xsltproc ^
	--stringparam section.autolabel 1 ^
	--stringparam output.html.stylesheets 1 ^
	--stringparam html.stylesheet %PWD%/sphinx.css ^
	--stringparam toc.section.depth 4 ^
	%DOCBOOKXSL%/html/docbook.xsl 1.tmp ^
	| fromdos | towin ^
	| sed -e "s/ /\&nbsp;/g" ^
	| sed -e "s/©/\&copy;/g" ^
	> sphinx.html
rm 1.tmp

perl html2txt.pl < sphinx.html > sphinx.txt
