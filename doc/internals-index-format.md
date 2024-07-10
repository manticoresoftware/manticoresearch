# Manticore plain index format

    WARNING. This document is just an internal note. It might or might not be
    in sync with the actual code. Use it as an overview; refer to the source
    for precise details.

## General

Manticore plain index consists of the following files:

* `.sph`, header file
* `.spi`, dictionary (aka wordlist)
* `.spd`, document lists (aka doclists)
* `.spp`, keyword positions lists (aka hitlists)
* `.spa`, attribute values
* `.spm`, MVA values
* `.spk`, kill list (aka klist)
* `.spt`, lookup table from public document ID to internal row ID.

Also, indexer and searchd utilize a dummy `.spl` file to establish locks
on the whole index.

## Compression

Values that Sphinx internally stores can frequently be compressed well.
For instance, an ascending sequence of row IDs can clearly be stored
much more efficiently than at 4 or 8 bytes per ID.

Two techniques we currently use are delta encoding, and variable length
byte string (or VLB) compression.

Delta encoding is used when the sequence of the values to store is
monotonically non-decreasing. Each value is replaced with its difference
(delta) from the previous value. Example:

	source-sequence = 3, 5, 7, 11, 13, 17, ...
	delta-encoded = 3, 2, 2, 4, 2, 4, ...

The resulting deltas are smaller, and compress more efficiently.

Lists of deltas are 0-terminated. So zero is a magic value, that marks
the end of the encoded sequence.

VLB compression encodes a fixed-length (32-bit or 64-bit) integer value
to a variable-length byte string, depending on the value. 7 lower bits
of every byte contain next 7 low bits of the compressed value; and 8-th bit
signals whether there are more bytes following. Note that high bits come first!
Hence, values that take 7 bits (0 to 127, inclusive) are stored using 1 byte,
values that fit in 14 bits (128 to 16383) are stored using 2 bytes, etc.
Examples:

	source-value = 0x37
	encoded-value = 0x37

	source-value = 0x12345
	encoded-value = 0x84 0xC6 0x45
		// 0x84 == ( ( 0x12345>>14 ) & 0x7F ) | 0x80
		// 0xC6 == ( ( 0x12345>>7 ) & 0x7F ) | 0x80
		// 0x45 == ( ( 0x12345>>0 ) & 0x7F )

For VLB implementation, refer to `ZipInt()` and `UnzipInt()` functions.

## Header `.sph`

Header file (`.sph`) always contains index format version, index schema,
index statistics, and misc other settings.

Starting from 0.9.9, header now also contains the *complete* dictionary
and tokenizer settings, except the external files contents. This was not
the case in 0.9.8 and below, where these settings were always taken from
config file, and thus could easily go out of sync.

There are certain settings (stopwords, wordforms) that refer to external
files that are possibly (even likely) shared between different indexes.
For these, header in 0.9.9 stores the file name, modification time,
and CRC32, but not the file contents.

For specific data layout, refer to `LoadHeader()`, `WriteHeader()`, and
`DumpHeader()` methods.

## Dictionary `.spi`

Dictionary file (`.spi`) lets us quickly map keywords to document lists.
All keywords are internally replaced with their IDs, either CRC32 or FNV64
(depending on `--enable-id64` configure time setting). Dictionary essentially
is a huge list of per-keyword entries, sorted by keyword ID. (See the entry
format below.)
	
wordid-type and offset-type might vary (ie. be either 32-bit or 64-bit)
depending on compile-time settings.

To avoid zero offsets into dictionary (zero is a magic value), a dummy byte
is written at the very start of the file.

To save space, these entries are stored in a compressed form. All fields
are VLB compressed. Additionally, keyword-id and doclist-offset fields
(that are guaranteed to grow) are delta-encoded before compression.

To speedup lookups by an arbitrary keyword ID, delta encoding is restarted
after every `WORDLIST_CHECKPOINT` entries. Zero delta marker, i.e. a single
value of 0 for delta (without any additional data) is injected into
the stream at the point of every such checkpoint.

Locations (offsets) and starting keyword IDs of these checkpoints are
accumulated in RAM during indexing, and then written to disk at the end
of the dictionary file.

Almost all the dictionary writing happens in `cidxHit()` method.

### dict=crc format, v.31

	byte dummy = 0x01
	keyword[] keyword_blocks
	keyword is:
		zint wordid_delta
		zint doclist_offset_delta
		if wordid_delta == 0:
			return block_end
		zint num_docs
		zint num_hits
		if ver >= 31 and num_docs > SKIPLIST_BLOCK:
			zint skiplist_pos
			zint skiplist_len
	
	checkpoint[] checkpoints
	checkpoint is:
		qword wordid
		qword dict_offset

### dict=keywords format, v.31

	byte dummy = 0x01
	keyword[] keyword_blocks
	keyword is:
		byte keyword_editcode
		byte[] keyword_delta
		if keyword_editcode == 0:
			assert keyword_delta = { 0 }
			return block_end
		zint doclist_offset
		zint num_docs
		zint num_hits
		if num_docs >= DOCLIST_HINT_THRESH:
			byte doclist_sizehint
		if ver >= 31 and num_docs > SKIPLIST_BLOCK:
			zint skiplist_pos
			zint skiplist_len
	
	if min_infix_len > 0:
		tag "infix-entries"
		infix_entry[] infix_hash_entries
	
	checkpoint[] checkpoints
	checkpoint is:
		dword keyword_len
		byte[] keyword [ keyword_len ]
		qword dict_offset
	
	if min_infix_len > 0:
		tag "infix-blocks"
		infix_block[] infix_hash_blocks
	
	tag "dict-header"
	zint num_checkpoints
	zint checkpoints_offset
	zint infix_codepoint_bytes
	zint infix_blocks_offset

## Document lists `.spd`

For every indexed keyword, a list of all matching row IDs is stored
in document lists file (`.spd`).

By construction, document lists are laid out in ascending keyword ID order.
However, this is just a side effect, and not really a requirement.

The entry format is as follows:

	doclist-entry =
		row-id : uint32, delta-encoded
		[ inline-attrs ]
		hitlist-offset : offset-type, delta-encoded
		fields-mask : int32
		hits-count : int32

inline-attrs component is optional, its presence depends on docinfo setting.
For indexes built with `docinfo=extern` (the default value), there's no such
component. When `docinfo=inline`, it carries all the attribute values:

	inline-attrs =
		attr-rowitems : rowitem-type[], delta-encoded

hitlist-offset points to a location in hit list file (see below) where
the list of current keyword's occurences in current document is stored.

fields-mask is a bit mask. Bit number N is set when there were keyword
occurences in field number N; cleared otherwise. We precompute this mask
based on hitlist data and store it in doclist to accelerate certain
early rejection tests when searching.

hits-count is just a total number of keyword occurrences within the current
document, or term frequency (TF). It's precomputed from hitlist data too,
also for performance reasons.

To avoid zero offsets into document lists (zero is a magic value),
a dummy byte is written at the very start of the file.

Document lists are terminated by zero delta marker. I.e. when reading next
row-id delta returns 0, it means there's no more data in this doclist.

To save space, these entries are stored in a compressed form. All fields
are VLB compressed. Additionally, row-id and hitlist-offset fields
(that are guaranteed to grow) are delta-encoded before compression.

All of doclist writing happens in `cidxHit()` method.

## Hit lists `.spp`

In Manticore terms, hits are specific occurrences of a keyword at a given
position within the document. When a keyword "hello" occurs 5 times
in the document you index, that will result in 5 hits in that document.

Hit lists file (`.spp`) stores all such in-document keyword positions,
for every given document and keyword.

These positions are used by certain search operators, such as phrase or
proximity operator, to determine whether the document matches. They may
also be used by the relevance ranking code to compute phrase proximity,
if chosen ranker takes that factor into account.

By construction, hit lists are laid out in ascending keyword ID order.
However, this is just a side effect, and not really a requirement.

The entry format is as follows:

	hitlist-entry =
		word-position : int32, delta-encoded

word-position integer has the following bit fields:

	struct word-position
	{
		int field_id : 8; // bits 25-31
		int is_last : 1; // bit 24
		int word_position_in_field : 23; // bits 0-23
	};

is_last indicates that this hit was the very last (!) hit in this field.
This flag is required for "keyword$" searches (ie. with field end marker).

Positions are counted in words, *not* bytes. Positions start from 1.
Full-text field IDs start from 0. So, for example, when you index
the following 2-field document:

	title = woodchuck chuck
	content = just how many wood would a woodchuck chuck,
		if a woodchuck could chuck wood?

Then the resulting hitlist for "chuck" is going to be:

	raw-word-positions = 2, 16777224, 16777229

Because it occurs 3 times: in field number 0 position number 2,
and in field number 1 positions 8 and 13. And (1<<24) is 16777216.

For the sake of completeness, after delta encoding and adding
a trailing zero (end marker) this hitlist would transform to
the following sequence of integers:

	uncompressed-hitlist = 2, 16777222, 5, 0

And after VLB compression, the final byte stream would be:

	compressed-hitlist-bytes = 0x02, 0x88, 0x80, 0x80, 0x06, 0x05, 0x00

To avoid zero offsets into hitlists (zero is a magic value),
a dummy byte is written at the very start of the file.

Hitlists are terminated by zero delta marker. I.e. when reading next
word-id delta returns 0, it means there's no more data in this hitlist.

Note that we don't keep anything but the positions themselves here.
That's because keyword and row IDs are already known by the time
we read from hitlist, from the reffering dictionary and doclist
entries.

All of hitlist writing happens in `cidxHit()` method.


## Dockid to rowid lookup table `.spt`

Document IDs came from indexing and generally unpredictable. So, it is hard to determine whether they anyway ordered.
So, storing them 'as is' into document list may cause all kind of VLB and delta encoding to be useless.
To deal with it, Manticore just count all the documents with single uint32 numbers, starting from 0 and monotonically
increasing up to `0xFFFFFFFE` (that is 2^32-1 values). Value `0xFFFFFFFF` is reserved as special `INVALID_ROWID`.
These internal row IDs are actually used in doclists and matching. When we need to return final documents to user,
we read real document info by this row ID, and extract real wild document ID from there.

To perform the opposite task - get internal Row ID by real Document ID, we have special lookup table stored in `.spt` file.

It has the following structure:

    dword num_of_docs
    dword docs_per_checkpoint
    docid-type max_doc_id
    checkpoints[]
    checkpoint is:
		docid-type base_doc_id
        offset block_offset
    blocks[]
    block is:
        uint32 base_row_id
        lookup_pair[]
        lookup_pair is:
            zint : docid-type, delta-encoded
            uint64 rowid

All set of docids is divided to blocks by `docs_per_checkpoint` in each, last block may be non-complete.
So, total number of checkpoints may be calculated from header as `(num_of_docs + docs_per_checkpoint - 1) / docs_per_checkpoint`.

Checkpoints are placed in accending base_doc_id order, that is important.

Each block provides `docs_per_checkpoint` pairs of document ID and corresponding row ID. Very first element is a single row ID (as corresponding document ID is jus base_doc_id of the block). Next elements are pairs; first
follows VLB delta from previos document ID, then uncompressed row ID.

When we need to make a lookup, we first binary search over array of checkpoints, locating the one which may
include interesting document ID. Then we step into the block using block_offset, and iterate linearly over the documents there. After matching document ID we read row ID, and that is the one we are looking for.

--eof--
