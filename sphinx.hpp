//
// $Id$
//

#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>

#define SPH_MAX_QUERY_WORDS 10

#define SPH_RLOG_MAX_BLOCKS 128
#define SPH_RLOG_BIN_SIZE   262144
#define SPH_RLOG_BLOCK_SIZE 1048576

#define SPH_CLOG_BITS_DIR   10
#define SPH_CLOG_BITS_PAGE  22
#define SPH_CLOG_DIR_PAGES  (1 << SPH_CLOG_BITS_DIR)

#define SPH_CACHE_WRITE 1048576

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

typedef unsigned int uint;
typedef unsigned char byte;

typedef unsigned int		DWORD;
typedef unsigned char		BYTE;

// ***

void sphDie(char *message, ...);
void *sphMalloc(size_t size);
void *sphRealloc(void *ptr, size_t size);
void sphFree(void *ptr);
char *sphDup(char *s);

// ***

struct CSphHit
{
	uint docID, wordID, pos;
};

struct CSphList_Hit
{
	int count;
	CSphHit *data;

	CSphList_Hit();
	virtual ~CSphList_Hit();

	void add(int docID, uint wordID, int pos);
	void clear();
	void grow(int entries);

private:
	int max;
	CSphHit *pData;
};

struct CSphDict
{
	virtual uint wordID(byte *word) = 0;
};

struct CSphDict_CRC32 : CSphDict
{
	virtual uint wordID(byte *word);
};

struct CSphSource
{
	CSphList_Hit hits;
	CSphDict *dict;

	int fetchedDocs, fetchedBytes;

	CSphSource()
	{
		dict = NULL;
		fetchedDocs = fetchedBytes = 0;
	}

	virtual ~CSphSource() {}
	void setDict(CSphDict *dict) { this->dict = dict; }
	virtual int next() = 0;
};

struct CSphSource_Document : CSphSource
{
	int lastID, numFields, callWordCallback;

	CSphSource_Document() { callWordCallback = 0; }
	virtual int next();
	virtual void wordCallback(char *word) {}
	virtual byte **nextDocument() = 0;
};

struct CSphSource_Text : CSphSource_Document
{
	CSphSource_Text() { this->numFields = 1; }
	byte **nextDocument();
	virtual byte *nextText() = 0;
};

struct CSphSource_MySQL : CSphSource_Document
{
	char *sqlQuery;

	CSphSource_MySQL(char *sqlQuery);
	virtual ~CSphSource_MySQL();

	virtual int connect(char *host, char *user, char *pass, char *db,
		int port, char *usock);
	virtual byte **nextDocument();

private:
	MYSQL_RES *sqlResult;
	MYSQL_ROW sqlRow;
	MYSQL sqlDriver;
};

// ***

struct CSphMatch
{
	int docID;
	int weight;
};

struct CSphList_Match
{
	int count;
	CSphMatch *data;

	CSphList_Match();
	virtual ~CSphList_Match();

	void add(int docID, int weight);

private:
	int max;
	CSphMatch *pData;

	void grow();
};

// ***

struct CSphList_Int
{
	uint *data;
	int count;

	CSphList_Int();
	virtual ~CSphList_Int();

	void add(uint value);
	void clear();

private:
	uint *pData;
	int max;

	void grow();
};

struct CSphWriter_VLN
{
	char *name;
	int pos;

	CSphWriter_VLN(char *name);
	virtual ~CSphWriter_VLN();

	int open();
	void putbytes(void *data, int size);
	void zipInts(CSphList_Int *data);
	void close();
	void seek(int pos);

private:
	int fd, poolUsed, poolOdd;
	byte pool[SPH_CACHE_WRITE], *pPool;

	void putNibble(int data);
	void flush();
};

struct CSphReader_VLN
{
	char *name;

	CSphReader_VLN(char *name);
	virtual ~CSphReader_VLN();

	int  open();
	void getbytes(void *data, int size);
	int  unzipInt();
	void unzipInts(CSphList_Int *data);
	int  decodeHits(CSphList_Int *hl);
	void close();
	void seek(int pos);

private:
	int fd, pos, filePos, bufPos, bufOdd, bufUsed, bufSize;
	byte *buf;

	int getNibble();
	void cache();
};

struct CSphQueryWord
{
	int queryPos;
	uint wordID;
	char *word;
	CSphList_Int *hits, *docs;

	CSphQueryWord()
	{
		this->word = NULL;
		this->hits = NULL;
		this->docs = NULL;
		isLink = 0;
	}

	void shareListsFrom(CSphQueryWord *qw)
	{
		this->hits = qw->hits;
		this->docs = qw->docs;
		isLink = 1;
	}

	void newLists()
	{
		this->hits = new CSphList_Int();
		this->docs = new CSphList_Int();
		isLink = 0;
	}

	~CSphQueryWord()
	{
		if (this->word) sphFree(this->word);
		if (!isLink) {
			if (this->hits) delete this->hits;
			if (this->docs) delete this->docs;
		}
	}

private:
	int isLink;
};

struct CSphQueryResult
{
	struct {
		char *word;
		int docs, hits;
	} wordStats[SPH_MAX_QUERY_WORDS];
	int numWords;
	int queryTime;

	CSphList_Match *matches;
};

struct CSphIndex
{
	virtual int build(CSphDict *dict, CSphSource *source) = 0;
	virtual CSphQueryResult *query(CSphDict *dict, char *query) = 0;
};

#define BIN_ERR_READ -2 // bin read error
#define BIN_ERR_END  -1 // bin end
#define BIN_POS  0      // bin is in "expect pos delta" state
#define BIN_DOC  1      // bin is in "expect doc delta" state
#define BIN_WORD 2      // bin is in "expect word delta" state

struct CSphBin
{
	byte *data, *pData;
	int left, done, filePos, fileLeft, filePtr, state;
	uint lastDocID, lastWordID, lastPos;

	CSphBin()
	{
		data = (byte*)sphMalloc(SPH_RLOG_BIN_SIZE);
		pData = data; 
		left = done = filePos = fileLeft = 0;
		lastDocID = lastWordID = lastPos = 0;
		state = BIN_POS;
	}

	~CSphBin()
	{
		sphFree(data);
	}
};

struct CSphIndex_VLN : CSphIndex
{
	CSphIndex_VLN(char *filename);
	virtual ~CSphIndex_VLN();

	virtual int build(CSphDict *dict, CSphSource *source);
	virtual CSphQueryResult *query(CSphDict *dict, char *query);

private:
	char *filename;
	int fdRaw, filePos;
	CSphWriter_VLN *fdIndex, *fdData;
	CSphBin *bins[SPH_RLOG_MAX_BLOCKS];

	CSphList_Int *vChunk, *vChunkHeader, *vIndexPage;
	int  cidxPagesDir[SPH_CLOG_DIR_PAGES];
	int  cidxDirUsed;
	int  cidxPageUsed;
	int  cidxIndexPos;
	int  cidxDataPos;
	int  lastDocDelta;
	uint lastDocID;
	int  lastDocHits;

	int  open(char *ext, int mode);

	int  binsInit(int blocks);
	void binsDone(int blocks);
	uint binsReadVLB(int b);
	int  binsReadByte(int b);
	int  binsRead(int b, CSphHit *e);

	int  cidxCreate();
	int  cidxWriteRawVLB(int fd, CSphHit *hit, int count);
	void cidxFlushHitList();
	void cidxFlushChunk();
	void cidxFlushIndexPage();
	void cidxHit(CSphHit *hit);
	void cidxDone();
};

// ***

struct CSphHash
{
	int count;
	char **keys, **values;

	CSphHash();
	~CSphHash();

	void add(char *key, char *value);
	char *get(char *key);

private:
	int max;
};

struct CSphConfig
{
	CSphConfig();
	~CSphConfig();

	int open(char *file);
	CSphHash *loadSection(char *section);

private:
	FILE *fp;
};

/////////////////////////////////////////////////////////////////////////////

class CSphSource_XMLPipe : public CSphSource
{
public:
	/// ctor
					CSphSource_XMLPipe ();

	/// dtor
					~CSphSource_XMLPipe ();

	/// initializer
	bool			Init ( const char * sCommand );

	/// hit chunk getter
	virtual int		next ();

private:
	enum Tag_e
	{
		TAG_DOCUMENT = 0,
		TAG_ID,
		TAG_GROUP,
		TAG_TITLE,
		TAG_BODY
	};

private:
	/// we either expect some tag or are in tag 
	bool			m_bExpectTag;

	/// what's our current tag
	Tag_e			m_eTag;

	/// tag name
	const char *	m_pTag;

	/// tag name length
	int				m_iTagLength;

	/// incoming stream
	FILE *			m_pPipe;

	/// buffer
	BYTE			m_sBuffer [ 4096 ];

	/// current buffer pos
	BYTE *			m_pBuffer;

	/// buffered end pos
	BYTE *			m_pBufferEnd;

private:
	/// set tag
	void			SetTag ( const char * sTag, Tag_e eTag, bool bExp );
};

//
// $Id$
//
