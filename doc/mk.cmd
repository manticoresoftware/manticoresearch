@echo off
pwd | sed -e s/\\/\//g > 1.tmp
for /F "usebackq" %%i in (1.tmp) do set PWD=%%i
rm 1.tmp
xsltproc ^
	--stringparam section.autolabel 1 ^
	--stringparam output.html.stylesheets 1 ^
	--stringparam html.stylesheet %PWD%/sphinx.css ^
	--stringparam toc.section.depth 4 ^
	%DOCBOOKXSL%/html/docbook.xsl sphinx.xml ^
	| fromdos | towin ^
	| sed -e "s/ /\&nbsp;/g" ^
	| sed -e "s/©/\&copy;/g" ^
	> sphinx.html
