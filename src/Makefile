CC         = cc
PP         = cc
LD         = cc
BINARIES   = indexer searchd search
CCFLAGS    = -Wall -O3 -fomit-frame-pointer -I/usr/local/include
#CCFLAGS    = -Wall -g -I/usr/local/include
PPFLAGS    = $(CCFLAGS)
LDFLAGS    = -L/usr/local/mysql/lib -lmysqlclient -lstdc++ -lm -lz
SRC_SPHINX = sphinx.cpp sphinxstemen.cpp sphinxstemru.cpp
INC_SPHINX = sphinx.h sphinxstem.h
OBJ_SPHINX = $(SRC_SPHINX:.cpp=.o)

all : $(BINARIES)

indexer: indexer.o $(OBJ_SPHINX)
	$(LD) -o indexer indexer.o $(OBJ_SPHINX) $(LDFLAGS)

searchd: searchd.o $(OBJ_SPHINX)
	$(LD) -o searchd searchd.o $(OBJ_SPHINX) $(LDFLAGS)

search: search.o $(OBJ_SPHINX)
	$(LD) -o search search.o $(OBJ_SPHINX) $(LDFLAGS)

.cpp.o:
	$(PP) $(PPFLAGS) -c -o $*.o $*.cpp

cl clean:
	@rm -fr core
	@rm -fr *.o
	@for tmp in $(BINARIES); do rm -fr $$tmp; done

# source deps

indexer.o: Makefile indexer.cpp $(INC_SPHINX)
searchd.o: Makefile searchd.cpp $(INC_SPHINX)
search.o: Makefile search.cpp $(INC_SPHINX)
$(OBJ_SPHINX): Makefile $(INC_SPHINX) $($*:.o=.cpp)
