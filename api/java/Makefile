#
# $Id$
#
# Makefile to automate sphinxapi.jar source builds
#

# order matters; full rebuild is always performed; but it somehow works
SOURCES = \
	SphinxMatch.java \
	SphinxException.java \
	SphinxWordInfo.java \
	SphinxResult.java \
	SphinxClient.java \
	test.java
CLASSES = $(SOURCES:.java=.class)

all : sphinxapi.jar

clean:
	rm -fr org
	rm -f sphinxapi.jar

sphinxapi.jar: $(CLASSES)
	jar cfm sphinxapi.jar MANIFEST.MF org/sphx/api

.SUFFIXES: .java .class
vpath %.class org/sphx/api
.java.class:
	javac -classpath . -d . $<

#
# $Id$
#
