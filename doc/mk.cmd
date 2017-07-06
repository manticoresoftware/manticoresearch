@echo off

if "%1" EQU "chunked" (
	set XSLTARGS=^
		--stringparam toc.section.depth 1 ^
		--stringparam generate.section.toc.level 2 ^
		--stringparam chunk.first.sections 1 ^
		--stringparam chunk.section.depth 2 ^
		--stringparam base.dir chunked/ ^
		--stringparam use.id.as.filename 1 ^
		%DOCBOOKXSL%/html/chunk.xsl
) else (
	set XSLTARGS=^
		--stringparam toc.section.depth 4 ^
		%DOCBOOKXSL%/html/docbook.xsl
)

type sphinx.xml ^
	| perl -pe "s/<b>/<emphasis role=\"bold\">/g" ^
	| perl -pe "s/<\/b>/<\/emphasis>/g" ^
	| perl -pe "s/(fixed|bug) #(\d+)/\1 <ulink url=\"http:\/\/sphinxsearch.com\/bugs\/view.php\?id=\2\">#\2<\/ulink>/" ^
	| xsltproc ^
		--nonet ^
		--stringparam section.autolabel 1 ^
		--stringparam section.label.includes.component.label 1 ^
		%XSLTARGS% ^
		- ^
	| perl -pe "s/\xA0/\&nbsp;/g" ^
	| perl -pe "s/\xA9/\&copy;/g" ^
	| perl -pe "s/\xEF/\&iuml;/g" ^
	| perl -pe "s/((<\/(li|dt|dt|head|div)>)+)/\1\n/g" ^
	| perl -pe "s/<a name=\"id\d+\"><\/a>//g" ^
	| perl -pe "s/<\/head>/\n<style type=\"text\/css\">pre.programlisting { background-color: #f0f0f0; padding: 0.5em; margin-left: 2em; margin-right: 2em; }<\/style>\n<\/head>/" ^
	| perl -e "undef $/; $_ = <>; s/<li><p>(([^<]|(<\/?(a|code|span|strong|b|i|pre)\b[^>]*?>))+)<\/p><\/li>/<li>\1<\/li>/gms; print;" ^
	> sphinx.html

perl html2txt.pl < sphinx.html > sphinx.txt

fromdos sphinx.html
fromdos sphinx.txt
