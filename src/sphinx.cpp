//
// $Id$
//

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include "sphinx.h"
#include "sphinxstem.h"

// *** LOWERCASING TABLE ***

static byte sphLT_cp1251[] = {
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 0-10
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 10-20
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 20-30
	0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37, 0x38,0x39,0,0, 0,0,0,0, // 30-40
	0x00,0x61,0x62,0x63, 0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b, 0x6c,0x6d,0x6e,0x6f, // 40-50
	0x70,0x71,0x72,0x73, 0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0, 0,0,0,0, // 50-60
	0x00,0x61,0x62,0x63, 0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b, 0x6c,0x6d,0x6e,0x6f, // 60-70
	0x70,0x71,0x72,0x73, 0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0, 0,0,0,0, // 70-80
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 80-90
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 90-a0
	0,0,0,0, 0,0,0,0, 0xb8,0,0,0, 0,0,0,0, // a0-b0
	0,0,0,0, 0,0,0,0, 0xb8,0,0,0, 0,0,0,0, // b0-c0
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef, // c0-d0
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff, // d0-e0
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef, // e0-f0
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff // f0-ff
};

// *** FUNCTIONS ***

void sphDie(char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vfprintf(stderr, message, ap);
	va_end(ap);
	exit(1);
}

void *sphMalloc(size_t size)
{
	void *result;

	if (!(result = malloc(size)))
		sphDie("FATAL: out of memory (unable to allocate %d bytes)", size);
	return result;
}

void *sphRealloc(void *ptr, size_t size)
{
	void *result;

	if (!(result = realloc(ptr, size)))
		sphDie("FATAL: out of memory (unable to reallocate %d bytes)", size);
	return result;
}

void sphFree(void *ptr)
{
	free(ptr);
}


/// time, in mcs
int sphTimer()
{
	static int s_sec = -1, s_usec = -1;
	struct timeval tv;

	if (s_sec == -1) {
		gettimeofday(&tv, NULL);
		s_sec = tv.tv_sec;
		s_usec = tv.tv_usec;
	}
	gettimeofday(&tv, NULL);
	return (tv.tv_sec - s_sec) * 1000000 + (tv.tv_usec - s_usec);
}


/// time, in seconds
float sphLongTimer ()
{
	static int s_sec = -1, s_usec = -1;
	struct timeval tv;

	if ( s_sec == -1 )
	{
		gettimeofday ( &tv, NULL );
		s_sec = tv.tv_sec;
		s_usec = tv.tv_usec;
	}
	gettimeofday ( &tv, NULL );
	return float(tv.tv_sec-s_sec) + float(tv.tv_usec-s_usec)/1000000.0f;
}


char *sphDup(char *s)
{
	char *r;
	if (s) {
		r = (char*)sphMalloc(1 + strlen(s));
		strcpy(r, s);
		return r;
	} else {
		return NULL;
	}
}

// *** MATCH VECTOR ***

CSphList_Match::CSphList_Match()
{
	max = 4096;
	data = (CSphMatch*)sphMalloc(sizeof(CSphMatch) * max);
	pData = data;
	count = 0;
}

CSphList_Match::~CSphList_Match()
{
	sphFree(data);
}

void CSphList_Match::add(int docID, int weight)
{
	if (count >= max) grow();
	pData->docID = docID;
	pData->weight = weight;
	pData++;
	count++;
}

void CSphList_Match::grow()
{
	max *= 2;
	data = (CSphMatch*)sphRealloc(data, sizeof(CSphMatch) * max);
	pData = data + count;
}

// *** INT VECTOR ***

CSphList_Int::CSphList_Int()
{
	max = 4096;
	data = (uint*)sphMalloc(sizeof(uint) * max);
	pData = data;
	count = 0;
}

CSphList_Int::~CSphList_Int()
{
	sphFree(data);
}

void CSphList_Int::add(uint value)
{
	if (count >= max) grow();
	*pData++ = value;
	count++;
}

void CSphList_Int::clear()
{
	count = 0;
	pData = data;
}

void CSphList_Int::grow()
{
	max *= 2;
	data = (uint*)sphRealloc(data, sizeof(uint) * max);
	pData = data + count;
}

// *** DOCUMENT HITS ***

CSphList_Hit::CSphList_Hit()
{
	max = 1024;
	count = 0;
	data = (CSphHit*)sphMalloc(sizeof(CSphHit) * max);
	pData = data;
}

CSphList_Hit::~CSphList_Hit()
{
	sphFree(data);
}

void CSphList_Hit::add(int docID, uint wordID, int pos)
{
	if (count >= max) grow(max);
	pData->docID = docID;
	pData->wordID = wordID;
	pData->pos = pos;
	pData++;
	count++;
}

void CSphList_Hit::clear()
{
	count = 0;
	pData = data;
}

void CSphList_Hit::grow(int entries)
{
	if (entries <= 0) return;
	max += entries;
	data = (CSphHit*)sphRealloc(data, sizeof(CSphHit) * max);
	pData = data + count;
}

// *** BIT OUTPUT TO FILE ***

#ifdef O_BINARY
#define SPH_BINARY O_BINARY
#else
#define SPH_BINARY 0
#endif

CSphWriter_VLN::CSphWriter_VLN(char *name)
{
	this->name = sphDup(name);
	pPool = &pool[0];
	fd = 0;
	pos = 0;
}

CSphWriter_VLN::~CSphWriter_VLN()
{
//	close();
	sphFree(name);
}

int CSphWriter_VLN::open()
{
	if (fd) return 1;
	fd = ::open(name, O_CREAT | O_RDWR | O_TRUNC | SPH_BINARY, 0644);
	poolUsed = 0;
	poolOdd = 0;
	pos = 0;
	if (fd) return 1; else return 0;
}

void CSphWriter_VLN::close()
{
	if (fd) {
		flush();
		::close(fd);
		fd = 0;
	}
}

void CSphWriter_VLN::putNibble(int data)
{
	data &= 0x0f;
	if (poolOdd) {
		poolOdd = 0;
		*pPool |= data;
		pPool++;
		if (poolUsed == SPH_CACHE_WRITE) flush();
	} else {
		poolOdd = 1;
		*pPool = (data << 4);
		poolUsed++;
	}
	pos++;
}

void CSphWriter_VLN::putbytes(void *data, int size)
{
	byte *b = (byte*)data;

	while (size-- > 0) {
		putNibble((*b) >> 4);
		putNibble((*b) & 0x0f);
		b++;
	}
}

void CSphWriter_VLN::zipInts(CSphList_Int *data)
{
	register uint *p = data->data;
	register int n = data->count, b;
	uint v;

	while (n-- > 0) {
		v = *p++;
		do {
			b = v & 0x07;
			v >>= 3;
			if (v) b |= 0x08;
			putNibble(b);
		} while (v);
	}
}

void CSphWriter_VLN::flush()
{
	write(fd, pool, poolUsed);
	if (poolOdd) pos++;
	poolUsed = 0;
	poolOdd = 0;
	pPool = &pool[0];
}

void CSphWriter_VLN::seek(int pos)
{
	byte b;

	flush();
	lseek(fd, pos >> 1, SEEK_SET);
	if (pos & 1) {
		read(fd, &b, 1);
		lseek(fd, pos >> 1, SEEK_SET);
		putNibble(b & 0x0f);
	}
	this->pos = pos;
}

// *** BIT INPUT FROM FILE ***

CSphReader_VLN::CSphReader_VLN(char *name)
{
	this->name = sphDup(name);
	this->bufSize = 4096; // FIXME?
	this->buf = (byte*)sphMalloc(this->bufSize);
	this->fd = this->pos = this->filePos = this->bufPos = this->bufOdd = this->bufUsed = 0;
}

CSphReader_VLN::~CSphReader_VLN()
{
	sphFree(this->name);
}

int CSphReader_VLN::open()
{
	if (this->fd) return 1;
	this->fd = ::open(this->name, O_RDONLY | SPH_BINARY);
	return this->fd ? 1 : 0;
}

void CSphReader_VLN::close()
{
	if (!this->fd) return;
	::close(this->fd);
	this->fd = 0;
}

void CSphReader_VLN::seek(int pos)
{
	this->pos = pos;
	this->bufUsed = 0;
}

void CSphReader_VLN::cache()
{
	if (this->filePos != (this->pos >> 1)) {
		::lseek(fd, (this->pos >> 1), SEEK_SET);
		this->filePos = (this->pos >> 1);
	}
	this->bufPos = 0;
	this->bufUsed = ::read(fd, this->buf, this->bufSize);
	this->bufOdd = (this->pos & 1);
	this->filePos += this->bufUsed;
}

int CSphReader_VLN::getNibble()
{
	if (this->bufPos >= this->bufUsed) this->cache();
	this->pos++;
	if (this->bufOdd) {
		this->bufOdd = 0;
		return (this->buf[this->bufPos++] & 0x0f);
	} else {
		this->bufOdd = 1;
		return (this->buf[this->bufPos] >> 4);
	}
}

void CSphReader_VLN::getbytes(void *data, int size)
{
	byte *b = (byte*)data;

	while (size-- > 0) *b++ = (this->getNibble() << 4) + this->getNibble();
}

int CSphReader_VLN::unzipInt()
{
	register int b, offset = 0;
	register uint v = 0;

	do {
		b = getNibble();
		v += ((b & 0x07) << offset);
		offset += 3;
	} while (b & 0x08);
	return v;
}

void CSphReader_VLN::unzipInts(CSphList_Int *data)
{
	register int i = 0;

	while ( (i = this->unzipInt()) ) data->add(i);
}

int CSphReader_VLN::decodeHits(CSphList_Int *hl)
{
	register int i, v = 0, n = 0;

	while ( (i = this->unzipInt()) ) {
		v += i;
		hl->add(v);
		n++;
	}
	return n;
}

// *** INDEX ***

CSphIndex_VLN::CSphIndex_VLN(char *filename)
{
	this->filename = sphDup(filename);
	fdIndex = 0;
	fdData = 0;
	fdRaw = 0;
}

CSphIndex_VLN::~CSphIndex_VLN()
{
	sphFree(filename);
}

#define SPH_CMPHIT_LESS(a,b) \
	(a.wordID <  b.wordID || \
	(a.wordID == b.wordID && a.docID <  b.docID) || \
	(a.wordID == b.wordID && a.docID == b.docID && a.pos < b.pos))

#define SPH_CMPHIT_MORE(a,b) SPM_CMPHIT_LESS(b,a)

int cmpHitPos(const void *a, const void *b)
{
	register int r;

	if (!(r = ((CSphHit*)a)->docID - ((CSphHit*)b)->docID))
		r = ((CSphHit*)a)->pos - ((CSphHit*)b)->pos;
	return r;
}

int cmpMatch(const void *a, const void *b)
{
	return ((CSphMatch*)b)->weight - ((CSphMatch*)a)->weight;
}

void sphSortHits_WordID(CSphHit *s, int n)
{
	int st0[32], st1[32];
	int a, b, k, i, j;
	CSphHit t;
	uint x_word, x_doc, x_pos;

	k = 1;
	st0[0] = 0;
	st1[0] = n - 1;
	while (k != 0) {
		k--;
		i = a = st0[k];
		j = b = st1[k];
		x_word = s[(a+b) / 2].wordID;
		x_doc = s[(a+b) / 2].docID;
		x_pos = s[(a+b) / 2].pos;
//		x = s[(a+b) / 2];
		while (a < b) {
			while (i <= j) {
//				while (cmpHit(&s[i], &x) < 0) i++;
				while (s[i].wordID < x_word ||
					(s[i].wordID == x_word && s[i].docID < x_doc) ||
					(s[i].wordID == x_word && s[i].docID == x_doc && s[i].pos < x_pos))
						i++;
//				while (cmpHit(&x, &s[j]) < 0) j--;
				while (x_word < s[j].wordID ||
					(x_word == s[j].wordID && x_doc < s[j].docID) ||
					(x_word == s[j].wordID && x_doc == s[j].docID && x_pos < s[j].pos))
						j--;
				if (i <= j) {
					t = s[i]; s[i] = s[j]; s[j] = t;
					i++; j--;
				}
			}
			if (j - a >= b - i) {
				if (a < j) {
					st0[k] = a;
					st1[k] = j;
					k++;
				}
				a = i;
			} else {
				if (i < b) {
					st0[k] = i;
					st1[k] = b;
					k++;
				}
				b = j;
			}
		}
	}
}

int CSphIndex_VLN::open(char *ext, int mode)
{
	char *tmp = (char*)malloc(strlen(filename) + strlen(ext) + 1);

	strcpy(tmp, filename);
	strcat(tmp, ext);
	return ::open(tmp, mode | SPH_BINARY, 0644);
}

int CSphIndex_VLN::binsInit(int blocks)
{
	::lseek(fdRaw, 0, SEEK_SET); // FIXME
	filePos = 0;
	return 1;
}

void CSphIndex_VLN::binsDone(int blocks)
{
	int i;

	for (i = 0; i < blocks; i++) delete bins[i];
}

int CSphIndex_VLN::binsReadByte(int b)
{
	int n;
	byte r;

	if (!bins[b]->left) {
		if (filePos != bins[b]->filePos) {
			::lseek(fdRaw, bins[b]->filePos, SEEK_SET);
			filePos = bins[b]->filePos;
		}
		n = min(bins[b]->fileLeft, SPH_RLOG_BIN_SIZE);
		if (n == 0) {
			bins[b]->done = 1;
			bins[b]->left = 1;
		} else {
			if (::read(fdRaw, bins[b]->data, n) != n) return -2;
			bins[b]->left = n;
			bins[b]->filePos += n;
			bins[b]->fileLeft -= n;
			bins[b]->pData = bins[b]->data;
			filePos += n;
		}
	}
	if (bins[b]->done) return -1;

	bins[b]->left--;
	r = *(bins[b]->pData);
	bins[b]->pData++;
	return r;
}

uint CSphIndex_VLN::binsReadVLB(int b)
{
	uint v = 0, o = 0;
	int t;

	do {
		if ((t = binsReadByte(b)) < 0) return 0xffffffff;
		v += ((t & 0x7f) << o);
		o += 7;
	} while (t & 0x80);
	return v;
}

int CSphIndex_VLN::binsRead(int b, CSphHit *e)
{
	uint r;

	if (bins[b]->done) { // expected EOB
		e->wordID = 0;
		return 1;
	}

	while (1) {
		if ((r = binsReadVLB(b)) == 0xffffffff) return 0; // unexpected EOB
		if (r) switch (bins[b]->state) {

			case BIN_POS: 
				bins[b]->lastPos += r;
				e->docID = bins[b]->lastDocID;
				e->wordID = bins[b]->lastWordID;
				e->pos = bins[b]->lastPos;
				return 1;

			case BIN_DOC:
				bins[b]->lastDocID += r;
				bins[b]->lastPos = 0;
				bins[b]->state = BIN_POS;
				break;

			case BIN_WORD:
				bins[b]->lastWordID += r;
				bins[b]->lastDocID = bins[b]->lastPos = 0;
				bins[b]->state = BIN_DOC;
				break;

		} else switch (bins[b]->state) {

			case BIN_POS: bins[b]->state = BIN_DOC; break;
			case BIN_DOC: bins[b]->state = BIN_WORD; break;
			case BIN_WORD:
				bins[b]->done = 1;
				e->wordID = 0;
				return 1;
		}
	}

	return 1;
}

int CSphIndex_VLN::cidxCreate()
{
	char *tmp = (char*)malloc(strlen(filename) + 5);
	int i;

	strcpy(tmp, filename);
	strcat(tmp, ".spi");
	fdIndex = new CSphWriter_VLN(tmp);
	if (!fdIndex->open()) return 0;

	strcpy(tmp, filename);
	strcat(tmp, ".spd");
	fdData = new CSphWriter_VLN(tmp);
	if (!fdData->open()) return 0;

	// put dummy byte (otherwise offset would start from 0, first delta would be 0
	// and VLB encoding of offsets would fuckup)
	BYTE bDummy = 1;
	fdData->putbytes ( &bDummy, 1 );

	free(tmp);

	vChunk = new CSphList_Int();
	vChunkHeader = new CSphList_Int();
	vIndexPage = new CSphList_Int();

	for (i = 0; i < SPH_CLOG_DIR_PAGES; i++)
		cidxPagesDir[i] = -1;
	cidxDirUsed = 0;
	cidxPageUsed = 0;

	lastDocDelta = 0;
	lastDocHits = 0;

	fdIndex->putbytes(cidxPagesDir, sizeof(cidxPagesDir));
	return 1;
}

void CSphIndex_VLN::cidxFlushHitList()
{
	if (lastDocHits) {
		lastDocDelta <<= 1;
		if (lastDocHits == 1) lastDocDelta |= 1;
			else vChunk->add(0);
		vChunkHeader->add(lastDocDelta);
	}
	lastDocHits = 0;
}

void CSphIndex_VLN::cidxFlushChunk()
{
	if (vChunk->count) {
		vChunkHeader->add(0);
		fdData->zipInts(vChunkHeader);
		fdData->zipInts(vChunk);
	}
	vChunkHeader->clear();
	vChunk->clear();
}

void CSphIndex_VLN::cidxFlushIndexPage()
{
	if (vIndexPage->count) {
		vIndexPage->add(0);
		fdIndex->zipInts(vIndexPage);
	}
	vIndexPage->clear();
}

void CSphIndex_VLN::cidxHit(CSphHit *hit)
{
	static uint lastWordID = 0, lastPageID = 0xffffffff;
	static int lastPos = 0, lastIndexPos = 0;

	if (lastWordID != hit->wordID) {
		cidxFlushHitList();
		cidxFlushChunk();

		if (lastPageID != (hit->wordID >> SPH_CLOG_BITS_PAGE)) {
			cidxFlushIndexPage();
			lastPageID = hit->wordID >> SPH_CLOG_BITS_PAGE;
			lastWordID = 0;
			lastIndexPos = 0;
			cidxPagesDir[lastPageID] = fdIndex->pos;
		}

		vIndexPage->add(hit->wordID - lastWordID);
		vIndexPage->add(fdData->pos - lastIndexPos);
		lastWordID = hit->wordID;
		lastIndexPos = fdData->pos;

		lastDocID = 0;
		lastDocDelta = 0;
	}

	if (lastDocID != hit->docID) {
		cidxFlushHitList();

		lastDocDelta = hit->docID - lastDocID;
		lastDocID = hit->docID;
		lastPos = 0;
	}

	vChunk->add(hit->pos - lastPos);
	lastPos = hit->pos;
	lastDocHits++;
}

void CSphIndex_VLN::cidxDone()
{
	fdIndex->seek(0);
	fdIndex->putbytes(cidxPagesDir, sizeof(cidxPagesDir));
	
	fdIndex->close();
	fdData->close();

	delete vChunk;
	delete vChunkHeader;
	delete vIndexPage;
	delete fdIndex;
	delete fdData;
}

inline int encodeVLB(byte *buf, uint v)
{
	register byte b;
	register int n = 0;

	do {
		b = (v & 0x7f);
		v >>= 7;
		if (v) b |= 0x80;
		*buf++ = b;
		n++;
	} while (v);
	return n;
}

int CSphIndex_VLN::cidxWriteRawVLB(int fd, CSphHit *hit, int count)
{
	byte buf[65536+1024], *pBuf, *maxP;
	int n = 0, w;
	uint d1, d2, d3, l1 = 0, l2 = 0, l3 = 0;

	pBuf = &buf[0];
	maxP = &buf[65536-1];
	while (count--) {
		d1 = hit->wordID - l1;
		d2 = hit->docID - l2;
		d3 = hit->pos - l3;
		if (d1) d2 = hit->docID;
		if (d2) d3 = hit->pos;
		if (d1) pBuf += encodeVLB(pBuf, 0);
		if (d2) pBuf += encodeVLB(pBuf, 0);
		if (d1) pBuf += encodeVLB(pBuf, d1);
		if (d2) pBuf += encodeVLB(pBuf, d2);
		pBuf += encodeVLB(pBuf, d3);
		l1 = hit->wordID;
		l2 = hit->docID;
		l3 = hit->pos;

		hit++;

		if (pBuf > maxP) {
			w = (int)(pBuf - buf);
			if (::write(fd, buf, w) != w) return -1;
			n += w;
			pBuf = buf;
		}
	}
	pBuf += encodeVLB(pBuf, 0);
	pBuf += encodeVLB(pBuf, 0);
	pBuf += encodeVLB(pBuf, 0);
	w = (int)(pBuf - buf);
	if (::write(fd, buf, w) != w) return -1;
	n += w;

	return n;
}

int CSphIndex_VLN::build(CSphDict *dict, CSphSource *source)
{
	int i, n, docID, mini, rawBlockUsed, rawHits, rawBlockSize, rawBlocks;
	CSphHit *hit, *rawBlock, *pRawBlock, cur[SPH_RLOG_MAX_BLOCKS];

	source->setDict(dict);

	// create raw log
	if (!(fdRaw = this->open(".spr", O_CREAT | O_RDWR | O_TRUNC))) return 0;

	// allocate raw block
	rawBlockSize = SPH_RLOG_BLOCK_SIZE;
	rawBlockUsed = 0;
	rawBlocks = 0;
	rawHits = 0;
	rawBlock = (CSphHit*)sphMalloc(sizeof(CSphHit) * rawBlockSize);
	pRawBlock = rawBlock;

	// build raw log
	while ( (docID = source->next()) )
	{
		hit = source->hits.data;
		for (n = 0; n < source->hits.count; n++, hit++)
		{
			assert ( docID );
			assert ( hit->wordID );
			assert ( hit->pos );

			pRawBlock->docID = docID;
			pRawBlock->wordID = hit->wordID;
			pRawBlock->pos = hit->pos;
			pRawBlock++;
			rawBlockUsed++;
			rawHits++;
			if (rawBlockUsed == rawBlockSize)
			{
//				qsort(rawBlock, rawBlockUsed, sizeof(CSphHit), cmpHit);
				sphSortHits_WordID(rawBlock, rawBlockUsed);
				bins[rawBlocks] = new CSphBin();
				if ((bins[rawBlocks]->fileLeft = cidxWriteRawVLB(fdRaw,
					rawBlock, rawBlockUsed)) < 0)
				{
					fprintf(stderr, "ERROR: write() failed\n");
					return 0;
				}

				rawBlocks++;
				assert ( rawBlocks<=SPH_RLOG_MAX_BLOCKS );

				rawBlockUsed = 0;
				pRawBlock = rawBlock;
			}
		}
	}

	if (rawBlockUsed)
	{
//		qsort(rawBlock, rawBlockUsed, sizeof(CSphHit), cmpHit);
		sphSortHits_WordID(rawBlock, rawBlockUsed);

		bins[rawBlocks] = new CSphBin();
		if ((bins[rawBlocks]->fileLeft = cidxWriteRawVLB(fdRaw,
			rawBlock, rawBlockUsed)) < 0)
		{
			return 0;
		}

		rawBlocks++;
		assert ( rawBlocks<=SPH_RLOG_MAX_BLOCKS );
	}

	// calc bin positions from their lengths
	for (i = 0; i < rawBlocks; i++)
		bins[i]->filePos = 0;
	for (i = 1; i < rawBlocks; i++)
		bins[i]->filePos = bins[i-1]->filePos + bins[i-1]->fileLeft;

	// deallocate raw block
	free(rawBlock);

	close(fdRaw);

//rawHits = info.st_size / sizeof(CSphHit);
//	rawBlocks = (int)((rawHits + SPH_RLOG_BLOCK_SIZE - 1) / SPH_RLOG_BLOCK_SIZE);
	if (!(fdRaw = this->open(".spr", O_RDONLY))) return 0;

	// sort and write compressed index
	if (!(cidxCreate())) return 0;
//	binsInit(rawBlocks);
	for (i = 0; i < rawBlocks; i++)
		binsRead(i, &cur[i]);
	while (rawHits--) {
		mini = -1;
		for (i = 0; i < rawBlocks; i++) {
			if (!cur[i].wordID) continue;
			if (mini < 0) { mini = i; continue; }
//			if (cmpHit(&cur[i], &cur[mini]) < 0) mini = i;
			if (SPH_CMPHIT_LESS(cur[i], cur[mini])) mini = i;
		}

		cidxHit(&cur[mini]);
		binsRead(mini, &cur[mini]);
	}
	binsDone(rawBlocks);
	cidxFlushHitList();
	cidxFlushChunk();
	cidxFlushIndexPage();
	cidxDone();

	return 1;
}

struct CSphQueryParser : CSphSource_Text
{
	char *query;
	char *words[SPH_MAX_QUERY_WORDS];
	int numWords;

	CSphQueryParser(CSphDict *dict, char *query)
	{
		int i;

		for (i = 0; i < SPH_MAX_QUERY_WORDS; i++) words[i] = NULL;
		this->numWords = 0;
		this->query = sphDup(query);
		this->dict = dict;
		this->callWordCallback = 1;
		this->next();
	}

	~CSphQueryParser()
	{
		int i;

		for (i = 0; i < SPH_MAX_QUERY_WORDS; i++)
			if (words[i]) sphFree(words[i]);
	}

	void wordCallback(char *word)
	{
		if (numWords < SPH_MAX_QUERY_WORDS)
			this->words[numWords++] = sphDup(word);
	}

	byte *nextText()
	{
		lastID = 1;
		return (byte*)this->query;
	}
};

int cmpQueryWord(const void *a, const void *b)
{
	return (((CSphQueryWord*)a)->docs->count - ((CSphQueryWord*)b)->docs->count);
}

CSphQueryResult *CSphIndex_VLN::query(CSphDict *dict, char *query)
{
	CSphQueryParser *qp;
	CSphReader_VLN *rdIndex, *rdData;
	CSphQueryWord qwords[SPH_MAX_QUERY_WORDS];
	int i, j, nwords, chunkPos, weight[32], curWeight[32], weights[32],
		imin, nweights;
	uint *phits[SPH_MAX_QUERY_WORDS], *pdocs[SPH_MAX_QUERY_WORDS],
		wordID, docID, pmin, k;
	CSphQueryResult *result;
	struct timeval tv1, tv2;

	gettimeofday(&tv1, NULL);
	#ifdef SPH_SEARCH_TIMER
	int t1, t2;

	t1 = sphTimer();
	#define SPH_TIMER(msg) \
		t2 = sphTimer(); \
		fprintf(stderr, "DEBUG: %s %.2f\n", msg, (float)(t2 - t1) / 1000000); \
		t1 = t2;
	#else
	#define SPH_TIMER(msg)
	#endif

	// open files
	char *tmp = (char*)sphMalloc(strlen(this->filename) + 5);

	strcpy(tmp, filename);
	strcat(tmp, ".spi");
	rdIndex = new CSphReader_VLN(tmp);
	if (!rdIndex->open()) return NULL;

	strcpy(tmp, filename);
	strcat(tmp, ".spd");
	rdData = new CSphReader_VLN(tmp);
	if (!rdData->open()) return NULL;

	free(tmp);

	SPH_TIMER("open");

	// load index pages directory
	rdIndex->getbytes(cidxPagesDir, sizeof(cidxPagesDir));

	SPH_TIMER("load directory");

	// split query into words
	qp = new CSphQueryParser(dict, query);
	nwords = qp->hits.count;
	if (nwords > SPH_MAX_QUERY_WORDS) nwords = SPH_MAX_QUERY_WORDS; // FIXME
	for (i = 0; i < nwords; i++) {
		qwords[i].word = sphDup(qp->words[i]);
		qwords[i].wordID = qp->hits.data[i].wordID;
		qwords[i].queryPos = 1 + i;
	}
	delete qp;

	// init lists
	result = new CSphQueryResult();
	result->matches = new CSphList_Match();
	vIndexPage = new CSphList_Int();
	vChunkHeader = new CSphList_Int();

	SPH_TIMER("parse query");

	// load match list for each query word
	for (i = 0; i < nwords; i++)
	{
		// if the word was already loaded, just link to it
		for (j = 0; j < i; j++)
			if (qwords[i].wordID == qwords[j].wordID)
		{
			qwords[i].shareListsFrom(&qwords[j]);
			break;
		}
		if (qwords[i].hits) continue;
		qwords[i].newLists();

		// check index
		vIndexPage->clear();
		rdIndex->seek(cidxPagesDir[qwords[i].wordID >> SPH_CLOG_BITS_PAGE]);
		rdIndex->unzipInts(vIndexPage);

		wordID = chunkPos = 0;
		for (j = 0; j < vIndexPage->count - 1;)
		{
			wordID += vIndexPage->data[j++];
			chunkPos += vIndexPage->data[j++];
			if (wordID != qwords[i].wordID) continue;

			// found chunk for this query word, load it
			rdData->seek(chunkPos);
			vChunkHeader->clear();
			rdData->unzipInts(vChunkHeader);

			docID = 0;
			for (k = 0; k < (uint)vChunkHeader->count; k++)
			{
				docID += (vChunkHeader->data[k] >> 1);
				qwords[i].docs->add(docID);
				qwords[i].docs->add(qwords[i].hits->count);
				switch (vChunkHeader->data[k] & 1)
				{
					case 0:
						rdData->decodeHits(qwords[i].hits);
						break;
					case 1:
						qwords[i].hits->add(rdData->unzipInt());
						break;
				}
				qwords[i].hits->add(0);
			}
			qwords[i].docs->add(0);
			qwords[i].docs->add(0);
			break;
		}
	}

	SPH_TIMER("load hit lists");

	// close files
	delete rdIndex;
	delete rdData;

	// build word stats
	result->numWords = nwords;
	for (i = 0; i < nwords; i++) {
		result->wordStats[i].word = sphDup(qwords[i].word);
		result->wordStats[i].docs = (qwords[i].docs->count - 2) / 2;
		result->wordStats[i].hits = qwords[i].hits->count - 1;
	}

	// reorder hit lists and return if one of then is empty
	qsort(qwords, nwords, sizeof(CSphQueryWord), cmpQueryWord);
	if (!qwords[0].hits->count) return result;

	// find and proximity-weight matching documents
// FIXME!!!
nweights = 4;
weights[0] = 100;
weights[1] = 1;
weights[2] = 10;
weights[3] = 1;

	for (i = 0; i < nwords; i++) {
		pdocs[i] = qwords[i].docs->data;
	}
	i = docID = 0;
	while (1) {
		while (*pdocs[i] && docID > *pdocs[i]) pdocs[i] += 2;
		if (!*pdocs[i]) break;
		if (docID < *pdocs[i]) {
			docID = *pdocs[i];
			i = 0;
			continue;
		}
		if (++i != nwords) continue;

		// Houston, we have a match
		for (i = 0; i < nwords; i++) {
			phits[i] = &qwords[i].hits->data[*(1 + pdocs[i])];
		}
		for (i = 0; i < nweights; i++)
			weight[i] = curWeight[i] = 0;
		k = 2000000000;
		while (1) {
			pmin = 2000000000; imin = -1;
			for (i = 0; i < nwords; i++) {
				if (!*phits[i]) continue;
				if (pmin > *phits[i]) { pmin = *phits[i]; imin = i; }
			}
			if (imin < 0) break;
			phits[imin]++;

			j = pmin >> 24;
			if (qwords[imin].queryPos - pmin == k) {
				curWeight[j]++;
				if (weight[j] < curWeight[j]) weight[j] = curWeight[j];
			} else {
				curWeight[j] = 0;
			}
			k = qwords[imin].queryPos - pmin;
		}
		for (i = 0, j = 1; i < nweights; i++)
			j += weights[i] * weight[i];
		result->matches->add(docID, j);

		// continue looking for next matches
		i = 0;
		docID++;
	}

	SPH_TIMER("find matches");

	qsort(result->matches->data, result->matches->count,
		sizeof(CSphMatch), cmpMatch);

	SPH_TIMER("weight sort");

	gettimeofday(&tv2, NULL);
	result->queryTime = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// CRC32 DICTIONARY
/////////////////////////////////////////////////////////////////////////////

CSphDict_CRC32::CSphDict_CRC32 ( DWORD iMorph )
{
	m_iMorph = iMorph;
	if ( m_iMorph & SPH_MORPH_STEM_RU )
		stem_ru_init ();
}


DWORD CSphDict_CRC32::wordID ( BYTE * pWord )
{
	static const DWORD crc32tab [ 256 ] =
	{
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
		0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
		0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
		0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
		0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
		0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
		0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
		0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
		0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
		0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
		0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
		0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
		0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
		0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
		0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
		0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
		0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
		0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
		0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
		0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
		0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
		0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
		0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
		0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
		0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
		0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
		0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
		0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
		0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
		0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
		0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
		0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
		0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
		0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
		0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
		0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
		0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
		0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
		0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
	};

	DWORD crc = ~((DWORD)0);
	BYTE * p;

	if ( m_iMorph & SPH_MORPH_STEM_EN )
		stem_en ( pWord );
	if ( m_iMorph & SPH_MORPH_STEM_RU )
		stem_ru ( pWord );
	
	for ( p=pWord; *p; p++ )
		crc = (crc >> 8) ^ crc32tab[(crc ^ (*p)) & 0xff];
	return ~crc;
}

/////////////////////////////////////////////////////////////////////////////
// GENERIC SOURCE
/////////////////////////////////////////////////////////////////////////////

CSphSource::CSphSource() :
	dict ( NULL )
{
}


void CSphSource::setDict ( CSphDict * pDict )
{
	this->dict = pDict;
}


const CSphSourceStats * CSphSource::GetStats ()
{
	return &m_iStats;
}

/////////////////////////////////////////////////////////////////////////////
// DOCUMENT SOURCE
/////////////////////////////////////////////////////////////////////////////

int CSphSource_Document::next()
{
	byte **fields = nextDocument(), *data, *pData, *pWord;
	int pos, i, j, len;

	if (!fields || (lastID <= 0)) return 0;

	m_iStats.m_iTotalDocuments++;
	hits.clear();
	for (j = 0; j < numFields; j++) {
		if (!(data = fields[j])) continue;

		i = len = strlen((char*)data);
		m_iStats.m_iTotalBytes += len;

		pData = data;
		while (i-- > 0) { *pData = sphLT_cp1251[*pData]; pData++; }

		i = 0;
		pos = 1;
		pData = data;
		while (i < len) {
			while (!(*pData) && i < len) { pData++; i++; }
			if (i >= len) break;
			pWord = pData;
			while ((*pData) && i < len) { pData++; i++; }
			hits.add(lastID, dict->wordID(pWord), (j << 24) | pos++);
			if (callWordCallback) wordCallback((char*)pWord);
		}
	}

	return lastID;
}

// *** PLAIN TEXT SOURCE ***

byte **CSphSource_Text::nextDocument()
{
	static byte *t;

	if (!(t = nextText())) return NULL;
	return &t; 
};

// *** MYSQL SOURCE ***

CSphSource_MySQL::CSphSource_MySQL(char *sqlQuery)
{
	this->sqlQuery = sphDup(sqlQuery);
}

int CSphSource_MySQL::connect(char *host, char *user, char *pass, char *db,
	int port, char *usock)
{
	mysql_init(&sqlDriver);
	if (!mysql_real_connect(&sqlDriver, host, user, pass, db,
		port, usock, 0))
	{
		fprintf(stderr, "ERROR: %s\n", mysql_error(&sqlDriver));
		return 0;
	}
	if (mysql_query(&sqlDriver, sqlQuery))
	{
		fprintf(stderr, "ERROR: %s\n", mysql_error(&sqlDriver));
		return 0;
	}
	if (!(sqlResult = mysql_use_result(&sqlDriver)))
	{
		fprintf(stderr, "ERROR: %s\n", mysql_error(&sqlDriver));
		return 0;
	}
	numFields = mysql_num_fields(sqlResult) - 1;
	return 1;
}

CSphSource_MySQL::~CSphSource_MySQL()
{
	free(sqlQuery);
}

byte **CSphSource_MySQL::nextDocument()
{
	sqlRow = mysql_fetch_row(sqlResult);
	if (!sqlRow) return NULL;
	lastID = atoi(sqlRow[0]);
	return (byte**)(&sqlRow[1]);
}

// *** HASH ***

CSphHash::CSphHash()
{
	max = 32;
	count = 0;
	keys = (char**)sphMalloc(max * sizeof(char*));
	values = (char**)sphMalloc(max * sizeof(char*));
}

CSphHash::~CSphHash()
{
	sphFree(keys);
	sphFree(values);
}

void CSphHash::add(char *key, char *value)
{
	if (max == count) {
		max *= 2;
		keys = (char**)sphRealloc(keys, max * sizeof(char*));
		values = (char**)sphRealloc(values, max * sizeof(char*));
	}
	keys[count] = sphDup(key);
	values[count] = sphDup(value);
	count++;
}

char *CSphHash::get(char *key)
{
	int i;

	// NOTE: linear search is ok here, because these hashes
	// are intended to be rather tiny (config sections)
	if (key) for (i = 0; i < count; i++)
		if (strcmp(key, keys[i]) == 0) return values[i];
	return NULL;
}

// *** CONFIG ***

CSphConfig::CSphConfig()
{
	fp = NULL;
}

CSphConfig::~CSphConfig()
{
	if (fp) fclose(fp);
}

int CSphConfig::open(char *file)
{
	if (!(fp = fopen(file, "r"))) return 0;	
	return 1;
}

CSphHash *CSphConfig::loadSection(char *section)
{
	char buf[2048], *p, *pp, *key, *value;
	int l, ls;
	CSphHash *result;

	result = new CSphHash();
	if (fseek(fp, 0, SEEK_SET) < 0) return result;

	#define CLEAN_CONFIG_LINE() \
		l = strlen(buf); \
		if ( (pp = strchr(buf, '#')) ) { *pp = '\0'; l = strlen(buf); } \
		while (l && isspace(buf[l-1])) buf[--l] = '\0'; \
		while (l && isspace(*p)) { p++; l--; }

	// skip until we find the section
	ls = strlen(section);
	while ( (p = fgets(buf, sizeof(buf), fp)) ) {
		l = l;
		CLEAN_CONFIG_LINE();
		if (p[0] == '[' && p[l-1] == ']' && strncmp(p+1, section, ls) == 0)
			break;
	}
	if (!p) return result;

	// load all the config lines until next section or EOF
	key = value = NULL;
	while ( (p = fgets(buf, sizeof(buf), fp)) ) {
		CLEAN_CONFIG_LINE();
		if (p[0] == '[') break;

		// handle split strings
		if (p[l-1] == '\\') {
			p[l-1] = '\0';
			if (key) {
				value = (char*)sphRealloc(value, strlen(value) + strlen(p) + 1);
				strcat(value, p);
				continue;
			} else {
				if ( (pp = strchr(p, '=')) ) {
					*pp++ = '\0';
					key = p;
					value = pp;
					l = strlen(key); while (l && isspace(key[l-1])) key[--l] = '\0'; // rtrim key
					while (*value && isspace(*value)) value++; // ltrim value
					key = sphDup(key);
					value = sphDup(value);
					continue;
				} else {
					// FIXME: bad string, could bitch
					continue;
				}
			}
		} else if (key) {
			// previous split-string just ended, so add it to result
			value = (char*)sphRealloc(value, strlen(value) + strlen(p) + 1);
			strcat(value, p);
			result->add(key, value);
			sphFree(key);
			sphFree(value);
			key = NULL;
		}

		// handle empty strings
		if (!l) continue;

		// handle one-line pairs
		if ( (pp = strchr(p, '=')) ) {
			*pp++ = '\0';
			key = p;
			value = pp;
			l = strlen(key); while (l && isspace(key[l-1])) key[--l] = '\0'; // rtrim key
			while (*value && isspace(*value)) value++; // ltrim value
			result->add(key, value);
			key = NULL;
			continue;
		}

		// FIXME: bad string, could bitch
	}

	// return
	return result;
}

/////////////////////////////////////////////////////////////////////////////

CSphSource_XMLPipe::CSphSource_XMLPipe ()
{
	m_bBody			= false;
	m_pTag			= NULL;
	m_iTagLength	= 0;
	m_pPipe			= NULL;
	m_pBuffer		= NULL;
	m_pBufferEnd	= NULL;
}


CSphSource_XMLPipe::~CSphSource_XMLPipe ()
{
	if ( m_pPipe )
	{
		pclose ( m_pPipe );
		m_pPipe = NULL;
	}
}


bool CSphSource_XMLPipe::Init ( const char * sCommand )
{
	assert ( sCommand );

	m_pPipe = popen ( sCommand, "r" );
	m_bBody = false;
	return ( m_pPipe!=NULL );
}


int CSphSource_XMLPipe::next ()
{
	char sTitle [ 1024 ]; // FIXME?

	assert ( m_pPipe );
	hits.clear ();

	/////////////////////////
	// parse document header
	/////////////////////////

	if ( !m_bBody )
	{
		// scan for opening '<document>' tag if necessary
		SetTag ( "document" );
		if ( !SkipTag ( true, false ) )
			return 0;

		if ( !ScanInt ( "id", &m_iDocID ) )
			return 0;
		m_iStats.m_iTotalDocuments++;

		if ( !ScanInt ( "group", &m_iGroupID ) )
			return 0;

		if ( !ScanStr ( "title", sTitle, sizeof ( sTitle ) ) )
			return 0;

		SetTag ( "body" );
		if ( !SkipTag ( true ) )
			return 0;

		m_bBody = true;
		m_iWordPos = 0;
	}

	/////////////////////////////
	// parse body chunk by chunk
	/////////////////////////////

	assert ( m_bBody );
	bool bBodyEnd = false;

	while ( hits.count<1024 ) // FIXME!
	{
		// skip whitespace
		while ( true )
		{
			// suck in some data if needed
			if ( m_pBuffer>=m_pBufferEnd )
				if ( !UpdateBuffer() )
			{
				fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): unexpected EOF while scanning doc '%d' body.\n",
					m_iDocID );
				return 0;
			}

			// skip whitespace
			while ( (m_pBuffer<m_pBufferEnd) && (*m_pBuffer)!='<' && !sphLT_cp1251 [ *m_pBuffer ] )
				m_pBuffer++;

			if ( m_pBuffer<m_pBufferEnd )
				break; // we have found it
		}
		if ( (*m_pBuffer)=='<' )
		{
			bBodyEnd = true;
			break;
		}

		// the word
		BYTE sWord [ SPH_MAX_WORD_LEN+1 ];
		BYTE * pWord = &sWord [ 0 ];
		BYTE * pWordEnd = &sWord [ SPH_MAX_WORD_LEN ];

		while ( true )
		{
			// suck in some data if needed
			if ( m_pBuffer>=m_pBufferEnd )
				if ( !UpdateBuffer() )
			{
				fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): unexpected EOF while scanning doc '%d' body.\n",\
					m_iDocID );
				return 0;
			}

			// collect word
			while ( pWord<pWordEnd && sphLT_cp1251 [ *m_pBuffer ] && m_pBuffer<m_pBufferEnd )
				*pWord++ = sphLT_cp1251 [ *m_pBuffer++ ];

			// enough?
			if ( pWord==pWordEnd || m_pBuffer!=m_pBufferEnd )
				break;
		}
		*pWord++ = '\0';

		// if the word is too long, skip all the remaining non-whitespace
		if ( pWord==pWordEnd )
			while ( true )
		{
			// suck in some data if needed
			if ( m_pBuffer>=m_pBufferEnd )
				if ( !UpdateBuffer() )
			{
				fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): unexpected EOF while scanning doc '%d' body starting at '%s'.\n",
					m_iDocID, sWord );
				return 0;
			}

			// skip non-whitespace
			while ( (m_pBuffer<m_pBufferEnd) && sphLT_cp1251 [ *m_pBuffer ] )
				m_pBuffer++;

			if ( m_pBuffer!=m_pBufferEnd )
				break; // we have found it
		}

		// we found it, yes we did!
		hits.add ( m_iDocID, dict->wordID ( sWord ), ++m_iWordPos ); // field_id | (iPos++)
	}

	// some tag was found
	if ( bBodyEnd )
	{
		// let's check if it's '</body>' which is the only allowed tag at this point
		SetTag ( "body" );
		if ( !SkipTag ( false ) )
			return 0;

		// well, it is
		m_bBody = false;

		// let's check if it's '</document>' which is the only allowed tag at this point
		SetTag ( "document" );
		if ( !SkipTag ( false ) )
			return 0;
	}

	// if it was all correct, we have to flush our hits
	return m_iDocID;
}


void CSphSource_XMLPipe::SetTag ( const char * sTag )
{
	m_pTag = sTag;
	m_iTagLength = strlen ( sTag );
}


bool CSphSource_XMLPipe::UpdateBuffer ()
{
	assert ( m_pBuffer!=&m_sBuffer[0] );

	int iLeft = max ( m_pBufferEnd-m_pBuffer, 0 );
	if ( iLeft>0 )
		memmove ( m_sBuffer, m_pBuffer, iLeft );

	int iLen = fread ( &m_sBuffer [ iLeft ], 1, sizeof(m_sBuffer)-iLeft, m_pPipe );
	m_iStats.m_iTotalBytes += iLen;

	m_pBuffer = m_sBuffer;
	m_pBufferEnd = m_pBuffer+iLeft+iLen;

	return ( iLen!=0 );
}


bool CSphSource_XMLPipe::SkipWhitespace ()
{
	while ( true )
	{
		// suck in some data if needed
		if ( m_pBuffer>=m_pBufferEnd )
			if ( !UpdateBuffer() )
				return false;

		// skip whitespace
		while ( (m_pBuffer<m_pBufferEnd) && isspace ( *m_pBuffer ) )
			m_pBuffer++;

		// did we anything non-whitspace?
		if ( m_pBuffer<m_pBufferEnd )
			break;
	}

	assert ( m_pBuffer<m_pBufferEnd );
	return true;
}


bool CSphSource_XMLPipe::CheckTag ( bool bOpen )
{
	int iAdd = bOpen ? 2 : 3;

	// if case the tag is at buffer boundary, try to suck in some more data
	if ( m_pBufferEnd-m_pBuffer < m_iTagLength+iAdd )
		UpdateBuffer ();

	if ( m_pBufferEnd-m_pBuffer < m_iTagLength+iAdd )
	{
		fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected '<%s%s>', got EOF.\n",
			bOpen ? "" : "/", m_pTag );
		return false;
	}

	// check tag
	bool bOk = bOpen
		? ( ( m_pBuffer[0] == '<' )
			&& ( m_pBuffer[m_iTagLength+1] == '>' )
			&& strncmp ( (char*)(m_pBuffer+1), m_pTag, m_iTagLength ) == 0 )
		: ( ( m_pBuffer[0] == '<' )
			&& ( m_pBuffer[1] == '/' )
			&& ( m_pBuffer[m_iTagLength+2] == '>' )
			&& strncmp ( (char*)(m_pBuffer+2), m_pTag, m_iTagLength ) == 0 );
	if ( !bOk )
	{
		char sGot [ 64 ];
		int iCopy = min ( m_pBufferEnd-m_pBuffer, (int)sizeof(sGot)-1 );

		strncpy ( sGot, (char*)m_pBuffer, iCopy );
		sGot [ iCopy ] = '\0';

		fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected '<%s%s>', got '%s'.\n",
			bOpen ? "" : "/", m_pTag, sGot );
		return false;
	}

	// got tag
	m_pBuffer += iAdd+m_iTagLength;
	assert ( m_pBuffer<=m_pBufferEnd );
	return true;
}


bool CSphSource_XMLPipe::SkipTag ( bool bOpen, bool bWarnOnEOF )
{
	if ( !SkipWhitespace() )
	{
		if ( bWarnOnEOF )
			fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected '<%s%s>', got EOF.\n",
				bOpen ? "" : "/", m_pTag );
		return false;
	}

	return CheckTag ( bOpen );
}


bool CSphSource_XMLPipe::ScanInt ( const char * sTag, int * pRes )
{
	assert ( sTag );
	assert ( pRes );

	// scan for <sTag>
	SetTag ( sTag );
	if ( !SkipTag ( true ) )
		return false;

	if ( !SkipWhitespace() )
	{
		fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected <%s> data, got EOF.\n", m_pTag );
		return false;
	}

	*pRes = 0;
	while ( m_pBuffer<m_pBufferEnd )
	{
		// FIXME! could check for overflow
		while ( isdigit(*m_pBuffer) && m_pBuffer<m_pBufferEnd )
			(*pRes) = 10*(*pRes) + int( (*m_pBuffer++)-'0' );

		if ( m_pBuffer<m_pBufferEnd )
			break;
		else
			UpdateBuffer ();
	}

	// scan for </sTag>
	if ( !SkipTag ( false ) )
		return false;

	return true;
}


bool CSphSource_XMLPipe::ScanStr ( const char * sTag, char * pRes, int iMaxLength )
{
	assert ( sTag );
	assert ( pRes );
	
	char * pEnd = pRes+iMaxLength-1;

	// scan for <sTag>
	SetTag ( sTag );
	if ( !SkipTag ( true ) )
		return false;

	if ( !SkipWhitespace() )
	{
		fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected <%s> data, got EOF.\n", m_pTag );
		return false;
	}

	while ( m_pBuffer<m_pBufferEnd )
	{
		while ( (*m_pBuffer)!='<' && pRes<pEnd && m_pBuffer<m_pBufferEnd )
			*pRes++ = *m_pBuffer++;

		if ( m_pBuffer<m_pBufferEnd )
			break;
		else
			UpdateBuffer ();
	}
	*pRes++ = '\0';

	// scan for </sTag>
	if ( !SkipTag ( false ) )
		return false;

	return true;
}

//
// $Id$
//
