#
# $Id$
#
# Makefile to automate sphinxapi.jar source builds
#

PASS1SOURCES = \
	SphinxMatch.java \
	SphinxException.java \
	SphinxResult.java \
	SphinxWordInfo.java
PASS2SOURCES = \
	SphinxClient.java \
	test.java
SOURCES = $(PASS1SOURCES) $(PASS2SOURCES)
CLASSES = $(SOURCES:.java=.class)

all : sphinxapi.jar

clean:
	rm -fr org
	rm -f sphinxapi.jar

sphinxapi.jar: $(CLASSES)
	jar cfm sphinxapi.jar MANIFEST.MF org/sphx/api

.SUFFIXES: .java .class
.java.class:
	javac -cp . -d . $<

#
# $Id$
#
