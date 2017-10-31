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

docs: my_manpages

my_manpages: manpages.xml
	xsltproc $(XSLTARGSMAN) manpages.xml
