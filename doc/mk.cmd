@echo off
for /F "usebackq" %%i in (`pwd`) do set PWD=%%i
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
