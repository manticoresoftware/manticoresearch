CC         = cc
PP         = cc
LD         = cc
BINARIES   = indexer searchd
CCFLAGS    = -Wall -O3 -I/usr/local/include
PPFLAGS    = $(CCFLAGS)
#LDFLAGS    = -L/usr/local/mysql/lib -lc -lm -lz -lmysqlclient -static
LDFLAGS    = -L/usr/local/mysql/lib -lc -lm -lz -lmysqlclient
SRC_SPHINX = sphinx.cpp
OBJ_SPHINX = $(SRC_SPHINX:.cpp=.o)

all : $(BINARIES)

indexer: indexer.o $(OBJ_SPHINX)
	$(LD) -o indexer indexer.o $(OBJ_SPHINX) $(LDFLAGS)

searchd: searchd.o $(OBJ_SPHINX)
	$(LD) -o searchd searchd.o $(OBJ_SPHINX) $(LDFLAGS)

.cpp.o:
	$(PP) $(PPFLAGS) -c -o $*.o $*.cpp

cl clean:
	@rm -fr *.o
	@for tmp in $(BINARIES); do rm -fr $$tmp; done

# source deps

sphinx.o: Makefile sphinx.cpp sphinx.hpp
indexer.o: Makefile indexer.cpp sphinx.hpp
searchd.o: Makefile searchd.cpp sphinx.hpp
