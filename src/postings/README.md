# Block postings (component 2)

Experimental opt-in format: set `postings_format = block` when building a plain
table, or `postings_format='block'` in `CREATE TABLE`. The default is `legacy`.
Block tables require `dict=keywords` (the default) or `dict=keywords_32k`.
`dict=crc` remains supported for legacy postings only. Creation, reconfiguration
and loading reject block+CRC tables. Existing CRC tables must be rebuilt from
source with a word dictionary to adopt block postings; their hashes cannot
recover the original keywords for offline conversion.

A table has one immutable postings format. All of its disk chunks use that
format; RT RAM segments retain their existing representation. Cross-format
ATTACH (including into an empty table), merge, chunk loading and reconfiguration
are rejected. One daemon can serve separate legacy and block tables.

Block tables use disk header version 75, RT metadata version 26 and postings
component version 2. Legacy output retains its existing header/metadata versions.
Discarded component-3 position experiments require rebuilding their indexes.
Unknown component versions or required stream codecs are errors. These wire
layouts are experimental until publication.

The shared writer serves plain indexing, RT flush, merging and offline conversion.
The block qword adapter serves the existing document/hit consumers. Eligible
posting-window consumers select the native block cursor: bitmap words and
consecutive ranges feed OR/intersection directly, without reconstructing rows or
staging QwordDoc records. Sparse containers decode row IDs; restricted fields read
only low-32-bit field masks, with a row fallback for mixed membership. All-field
queries skip field decoding when the schema fits the requested mask. Membership
reads no TF, hit-reference or position data. Other consumers currently decode TF
and references according to their payload requirements and seek external positions when needed.
Hitlists retain the existing zero-terminated packed-hit varints and per-document
byte offsets. Compressed-position experiments are deferred after hot-query
benchmarks; their codec and checkpoint machinery is not part of this format.
Query construction selects the mapped or buffered hit decoder. Mapped varint
hitlists decode directly from mapped bytes; buffered-file access keeps its
existing reader and error handling. Native length-normalized
BM25 and WAND/BMW remain later milestones.

Encoded column access borrows the immutable file mapping when available. Buffered
readers reuse byte buffers across leaves. Row-container views retain their own
buffer until the next leaf, so field reads cannot overwrite a bitmap in use.
Other column views are consumed before their backing storage changes; lazy
per-field TF similarly retains a mapping view or its own file buffer. The reader
decodes into its destination arrays through the non-overlapping
`DecodeIntegersInto`/`DecodeRowsInto` APIs; delta rows are prefix-summed in place.
Full 128-value packing frames use the destination directly when it is 16-byte
aligned. Short tails and unaligned destinations retain bounded output scratch;
encoded packing words still use aligned native-endian input scratch. Field-mask
transposition and hit-reference low/high assembly remain separate steps.

Block qwords fill document batches directly from decoded leaf columns. Positioning
and hints are shared with single-document reads; a batch continues across leaves
until its capacity is reached or the term ends. Current-document state is updated
once per drained span. Membership batches retain lazy TF/hit-reference decoding.
Expanded-term payload wrappers keep their existing posting-list chaining path.

Mapped block readers share the legacy OS-page prefetch policy: term setup
prefetches the doclist head (or the whole short list), expanded payloads prefetch
their slices, and hit consumers prefetch only the matched external hitlists they
are about to read. Requests use the shared residency probe and power-of-two gate;
mlocked/resident maps avoid readahead and buffered file readers retain their
existing read buffering. This does not eagerly decode TF/positions or add CPU
cache-line prefetch instructions.

`index_field_lengths=1` is enabled for block tables. Existing row-addressable
token-count attributes are the exact norm store; immutable per-field nonempty
document counts and summed lengths are stored in the chunk header. Per-term
field document counts and summed TF are stored separately. Hitless terms
advertise no positions/per-field TF; they cannot supply those scoring inputs.
Full-text field schema changes require rebuilding a block table; ordinary
attribute ALTER remains supported.

Offline conversion creates a separate plain table/disk chunk:

```
indextool --config source.conf --convert-postings SOURCE OUTPUT_BASE
```

The source must use a word dictionary, already have field lengths and be
offline/unlocked. Existing
destination files are rejected. Attributes, row IDs, dead-row maps and other
table data are preserved; all postings and external positions are rewritten.
Sources without lengths need rebuilding into a separate block table. Converting
an RT table as a whole is not implemented: convert offline plain tables/chunks
and attach only to a separately created block RT table. The header is written
last. No conversion or mandatory rewrite occurs at daemon startup.

## Term and leaf framing (component 2)

The `.spd`, `.spp` and `.spe` files start with byte 1, a four-byte magic
(`BPD1`, `BPH1`, `BPS1`) and little-endian uint32 component version.

A dictionary posting offset points to a 40-byte term header:

| Offset | Bytes | Meaning |
|---|---|---|
| 0 | 4 | `PST2` |
| 4 | 2 | Component version |
| 6 | 2 | Capabilities: positions=1, per-field TF=2 |
| 8 | 2 | Field count, 1..256 |
| 10 | 2 | Reserved, zero |
| 12 | 8 | Four uint16 codec IDs: rows, fields, TF, hit references |
| 20 | 4 | Reserved, zero |
| 24 | 8 | Absolute term end |
| 32 | 8 | Absolute statistics footer offset |

Codec IDs are selected once per term stream; per-field TF uses the TF codec.
Changing a registered codec selection does not require a component-version bump.
New output selects PFOR (2) for rows, masks and TF, and BASE_DELTA (6) for hit references. FOR/PFOR use constant and short-array modes plus unpadded varint tails. The earlier experimental padded encoding is no longer supported; existing block tables must be reindexed. The enclosing component version remains 2. Legacy-format tables are unaffected.

Each 1..256-posting leaf has a 44-byte header: `BLK1`, byte length (uint32),
posting count and field-mask word count (uint16 each), first/last row and maximum
TF (uint32 each) and five uint32 section lengths. Sections are rows, fields, TF, hit references and per-field TF.
Rows use the row-container grammar below. Other integer frames have a uint32
byte-length prefix. Field masks store one frame per 32-bit mask word. Hit references decode to absolute 64-bit byte offsets into `.spp`. With BASE_DELTA (6), the section stores a little-endian uint64 base (minimum reference in the leaf), then a one-byte flag (bit 0: high parts present; other bits zero), then a length-prefixed PFOR low-32-bit frame and, only when needed, a PFOR high-32-bit frame. Frames contain offsets relative to the base. All relative offsets fit in uint32 in the common case, even when the base exceeds 4 GiB. The reader adds the base to reconstruct the same stable absolute bookmarks. Unordered references and spans above 4 GiB are supported. Codecs 1-4 in this section use the two-frame absolute low/high layout; ID 6 is valid only for hit-reference sections, not ordinary integer/row streams. Per-field TF contains sorted nonempty field entries `(field:uint16, frame length:uint32, integer frame)`.

The footer contains `(document count:uint64, summed TF:uint64)` per field.
Hitless terms store membership counts and zero field TF.
Positions are always external: existing packed-hit deltas, big-endian varints,
terminated by zero per document. Singleton/single-hit postings are not inline.

A hit reference points directly to the first varint of that document's hitlist:

```text
.spi term entry -> .spd term header -> posting leaf
                                      hit-reference[i] = byte offset
                                                          |
                                                          v
.spp at that offset:                         delta, delta, ..., 0
```

Deltas reconstruct the existing packed field/position/end-marker values, with
an initial value of zero for each document. The zero terminator ends the list;
TF prefix sums are not used to locate it or the next document. A copied byte
offset remains usable after the posting cursor advances, while its owning
hitlist file remains alive. Posting ordinals select entries from the reference
column; they are not position-stream ordinals encoded into a bookmark.

There are no PFOR position frames, position checkpoints, enlarged typed hit
bookmarks or separate field/end arrays in component 2. Its `.spp` grammar has
no per-term position codec selector. The compressed-position prototypes were
removed; revisiting them requires an explicit format/compatibility design and
new measurements. Retained varints are the selected layout, not an unfinished
transition to compressed positions.

The `.spe` directory contains `SKP1`, leaf count (uint32), then entries
`(first:uint32, last:uint32, absolute leaf offset:uint64, max TF:uint32)`.
The checker compares entries with decoded leaves. M2 seeks by leaf headers;
using the directory for hierarchical skipping and score pruning is later work.
There is no position directory in `.spe`; hit access uses the per-document
offsets in `.spd`. Maximum TF alone is not a safe complete score bound for every ranker.

Normal readers trust index data; structural/semantic checks run only in debug
assertions. `BlockIndexQword_T` and `Reader_c` have no validation-mode flag.
`indextool --check` constructs a separate `BlockIndexQwordValidation_c`, backed
by `ReaderValidation_c`; it checks framing and codec payloads before decoding.
The checker walks every leaf and checks all columns, directory
entries, footer statistics and per-field TF against external hits. No CRC fields
are written or checked. The previous experimental component-1 files require
rebuilding or conversion from legacy; their layout is rejected at table loading.
Unknown component/codec selection and I/O errors still fail explicitly.
Lazy readers do not inspect unused columns.
The entire block qword and its posting-window adapter are specialized for mapped
or reader-backed hitlists. Query construction selects the specialization from
the hitlist factory independently of doclist access. Hit seeks, position reporting,
reader binding and decoding use that compile-time choice. `BlockIndexQword_c` is
an alias for the reader-backed specialization used by utilities that bind readers
later; it accepts both file and mmap readers.
Writers buffer at most one leaf of postings plus one directory entry per leaf.

The implementation reuses FastPFOR's packing kernels, not its codec framing or
page-wide exception buffers. `fastpfor/PROVENANCE.json` records the upstream
commit, file paths and SHA-256 hashes. Files there are upstream sources with LF
normalization only; original copyright/license notices remain intact.

## Integer stream descriptors and frames (M1; used by component 2)

Select the integer codec once per encoded posting stream (rows, field masks,
TF or hit references). These frames do not describe `.spp` hitlists. The M1
descriptor is one little-endian uint16 codec ID:
FOR=1, PFOR=2, VARINT=3, CONSTANT=4; zero and unknown IDs are errors. BASE_DELTA=6 is reserved for the hit-reference section grammar described above.
Read this descriptor once and pass the resulting StreamDesc_t to every block
operation. It is not prepended to every frame. The term metadata above identifies the stream kind, boundaries and descriptor location.

Changing the selected registered codec changes the stream descriptor, not the
index/component format version. After publication, each ID specifies a stable codec byte grammar;
a future incompatible codec grammar gets a new ID and decoder, while existing
IDs remain readable. An older reader that does not implement a required ID
rejects that stream explicitly. This does not promise that old binaries can
decode future codecs.

All fixed-width multibyte integers in these frames use little-endian encoding.
Frames are independent and contain 1..256 unsigned 32-bit integers. The caller supplies the exact frame
length; the checker rejects trailing bytes. IDs/layouts below are experimental until
publication. No codec ID or codec override is present in an integer frame:

| Offset | Bytes | Meaning |
|---|---|---|
| 0 | 1 | FOR/PFOR: width 0..32, constant mode 33, short-array mode 34; zero for VARINT/CONSTANT |
| 1 | 1 | Exception count, 0..7; zero except PFOR |
| 2 | 2 | Value count |
| 4 | 2 | Payload byte length |
| 6 | variable | Payload |

FOR/PFOR packed payloads contain `floor(count/128)` packing frames, followed by exceptions (PFOR only) and a varint tail. Each frame has four
interleaved lanes of 32 values: lane L contains input indices `L+4*j` for j=0..31.
Pack each lane's low `width` bits in increasing j order, least significant bits
first. Output words interleave the lanes: word `4*k+L` is word k of lane L.
Serialize each word little-endian. Each 128-value frame occupies `16*width`
bytes. No unused slots are stored. Width zero has no packed
payload; width 32 preserves all bits. Neither layout nor bytes depend on CPU ISA.

PFOR appends exception pairs `(ordinal:u8, high_bits:u8)` in strictly increasing
ordinal order across the packed prefix. Patch `value |= high_bits << width`.
High bits must be nonzero and fit the remaining 32-bit value; width 32 cannot
have exceptions. Width selection minimizes payload size subject to these
constraints. More than seven exceptions, or high parts exceeding eight bits,
cause a wider width to be chosen. Zero-exception PFOR is permitted. FOR has none.

The integer-frame VARINT codec is canonical unsigned LEB128, at most five
bytes/value, with no redundant zero final group. It is distinct from the
big-endian packed-hit varints used in `.spp`. CONSTANT stores one uint32 per block; it requires every
value within that block to be equal, but the constant can differ between blocks.
Blocks cannot independently switch codecs. The constant/short-array modes and
varint tails below are part of the FOR/PFOR grammar.

InspectIntegers validates counts, lengths, widths, exceptions, canonical
varints and tail lengths using the supplied stream descriptor.
DecodeIntegersInto trusts the supplied frame, with validation in debug assertions
only, and requires disjoint input/output ranges. The checker calls InspectIntegers
before decoding. Full aligned outputs avoid a decoded scratch copy; unaligned packed outputs
use bounded scratch and varint tails decode directly so the API writes exactly the declared
count. Encoded input needs no alignment or extra padding. DecodeIntegers retains
overlap support through a scratch-buffer wrapper; the source must not be changed
concurrently. DecodeRowsInto and DecodeRows provide the same respective contracts
for row containers.

### FOR/PFOR constant, short-array and tail modes

The six-byte integer header is retained. Its first byte is either a packed width 0..32, CONSTANT=33, or VARINT=34. The second byte is the exception count; bytes 2..3 are the total value count, and bytes 4..5 are the complete payload length.

- CONSTANT: one canonical uint32 LEB128 value, repeated `count` times. Exception count is zero. Applies at any count, including full blocks.
- VARINT: `count` canonical uint32 LEB128 values for a non-packed array shorter than 128. Exception count is zero.
- Width 0..32: pack `floor(count/128)*128` values using the packing layout above (FOR without exceptions, PFOR with optional exceptions). Exceptions refer only to that packed prefix. Append `count%128` canonical uint32 LEB128 values after the exceptions. No unused packed slots are written. Width zero is valid for an all-zero packed prefix with optional exceptions.

Counts remain 1..256. Selection belongs to the FOR/PFOR grammar; no per-block codec ID is added. The earlier padded FOR/PFOR grammar and experimental codec ID 5 have been removed. Membership, TF, field masks and hit references remain independently readable. Singleton dictionary framing and inline position hits are not part of this change.

## Row container

The 20-byte header consists of magic `PST1`, layout version:u8 (=1), kind:u8,
count:u16, first:u32, last:u32, and payload length:u32. Kind is DELTA=1,
BITMAP=2, RANGE=3. Bounds are inclusive. Counts are 1..256; rows must strictly
increase and must not equal `UINT32_MAX` (`INVALID_ROWID`).

- DELTA: one integer frame using the row stream descriptor's integer codec; first delta is zero, later deltas are positive.
  Accumulate from `first` without overflow and require the final row to be `last`.
- BITMAP: bit zero represents `first`; byte bits run from least to most
  significant. Span is `last-first+1`, capped at `count*32` (at most 8192).
  Exact payload length is `ceil(span/8)`. First/last bits must be set, trailing
  padding bits zero, and cardinality equal to count.
- RANGE: no payload, and `last-first+1 == count`.

The row container grammar still supports per-block DELTA/BITMAP/RANGE representations independently of the fixed integer codec. Row AUTO chooses RANGE when possible, otherwise compares bitmap bytes against the
entire encoded delta frame. `InspectRows` validates before exposing a bounded
borrowed bitmap/integer payload. The view has the lifetime of its source buffer.
It does not allocate a row-ID array for bitmap/range input; delta validation
does decode deltas. `DecodeRows` materializes rows for document consumers.
`Reader_c::ReadRowBlock` exposes trusted persisted containers to the native
`BlockPostingWindow_c`; structural checks remain assertions/checker work.

The native cursor implements the same forward-only `PostingWindow_i` contract as
the legacy buffered cursor. OR/adaptive AND and their document adapter select it
through the qword factory. `AdvanceTo`, `Probe`, `FillOr`, `FillAnd` and bounded
`ReadRows` share retained lookahead. Intersections update only each leaf's covered
interval, clearing gaps and the unmatched suffix. Ranges preserve candidates
inside their span; bitmaps and decoded rows intersect one destination word at a
time, without a window-sized scratch bitmap. A final shared bitmap count returns
the survivor count. Field rejection, clipped windows and interrupted output
retain the existing semantics, including clearing an unprocessed suffix on stop.
Continuation checks run at entry and before each leaf refill/skip.
Dead-row masks and attribute filters remain in the consuming search/ranker path.
Expanded-term wrappers and unsupported query shapes retain their existing path.

There are no checksums in the block postings format. Structurally and semantically
valid payload changes are not detectable by the checker.

## Backends and build

The portable reference packs/unpacks the same four-lane layout. On x86-64,
AUTO selects the vendored SSE2 kernels; SSE2 is a baseline feature there. ARM
and other targets use the portable implementation, without an x86 dependency
or a SIMDe requirement. Encoding rejects an unavailable SIMD request; trusted
decoding requires the caller to select an available backend. Future ISA
variants require runtime feature dispatch; compiler flags must stay local to
their object targets. No global `-march=native` is required.

Standalone Linux build (outside the checkout):

```sh
cmake -S /path/to/source/src/postings -B /home/bench/postings-build \
  -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build /home/bench/postings-build -j2
ctest --test-dir /home/bench/postings-build --output-on-failure
/home/bench/postings-build/postings_codec_bench /path/to/corpus.txt > results.csv
```

`WITH_POSTINGS_SIMD=OFF` builds only the portable reference.
`WITH_POSTINGS_TOOLS=ON` enables these tools within the daemon's main build;
standalone builds enable them by default. Tests include multiple blocks sharing one descriptor and rejection of unknown
stream codec IDs. They use runtime checks even in Release, random/property cases, golden bytes, malformed frames, cross-backend
equality and Linux guard pages. The test reports a deterministic fingerprint of
encoded cases for comparison across architectures (not a cryptographic hash).

The RT differential suite requires a disposable daemon and creates its own tables.
It does not drop existing data and fails on reused table names:

```sh
python3 src/postings/tests/integration_rt.py --url http://127.0.0.1:PORT \\
  --prefix postings_m2 --output /home/bench/postings-m2-rt.json
```

Pass `--access file` to exercise buffered file readers instead of mmap.
It checks plain/columnar attributes, wide field masks, hitless terms and keyword-v2
dictionaries through RAM inserts, flushes, deletions, attribute ALTER and actual
chunk merges, including rejection of changing the table's format.

The positional suite also compares complete result rows, scores and warnings
against a baseline binary. Create a daemon-readable stopword file containing
`the`, then run against a fresh disposable RT daemon for each binary:

```sh
python3 src/postings/tests/integration_positional.py --url http://127.0.0.1:PORT \
  --prefix positional_before --stopwords /daemon/path/stopwords.txt \
  --output /home/bench/positional-before.json
python3 src/postings/tests/integration_positional.py --url http://127.0.0.1:OTHER_PORT \
  --prefix positional_after --stopwords /daemon/path/stopwords.txt \
  --oracle /home/bench/positional-before.json --output /home/bench/positional-after.json
```

Repeat with `--access file` and fresh prefixes. Coverage includes flat and nested
phrases/proximity/NEAR, repeated terms, stopword gaps, field masks, hitless
fallbacks, RAM/disk transitions, deletions and merges. One compound hitless
expression-ranker case is excluded because it crashes the baseline.

## Benchmarks

The optional text corpus has one record per line:
`dataset_name count value0 value1 ...`. Count is 1..512. Names beginning with
`rows-` identify strictly increasing row IDs; other names identify uint32
streams. Keep term/document boundaries when constructing row blocks and position
deltas. Export real postings using the existing reader (e.g. `indextool
--dumphitlist`) against a private or read-only table, record source chunk and
binary identity, and discard any final document truncated by a sample limit.
Include rare/dense terms, TFs and position deltas; do not infer TF from a partial
document hit list.

The benchmark checks every round trip, then reports median encode/decode
nanoseconds per value over five timed samples and complete frame bytes/value,
including headers/padding/exceptions. Release decode timings include copies, but no validation; they measure more
than just the packing kernels. Encoding includes block parameter selection/allocation. The benchmark-only
stream-auto policy selects one codec by comparing total encoded bytes over the
whole sampled stream before timing; it never selects a different codec per block.
Reported bytes include one stream descriptor. The CSV reports requested policy
and actual selected codec separately. It measures stream-auto, FOR, PFOR and
VARINT against scalar and AUTO backends, using
128/256-value frames. The 512-value grouping uses two independent 256-value
frames, so its bytes are intentionally identical to the 256 case; no claim is
made about a future shared 512-value header or a larger PFOR primitive. Synthetic
patterns are included even when a real corpus is supplied.

Codec microbenchmarks are not evidence of end-to-end query gains. Linux results
and source/corpus hashes belong in the external experiment directory and plan.

The main daemon build also provides `postings_qword_test` when
`WITH_POSTINGS_TOOLS=ON`. Run it with a disposable fixture-file path outside the
source checkout. It compares direct and generic batches over mmap/file readers,
membership/full metadata, leaf boundaries, hints, mixed reads, reset and EOF.
The standalone codec build does not include this daemon-linked test.


### Leaf-bounded candidates and lazy payloads

`PostingBlock_i` provides an exclusive view of a set-up qword for consumers that
need scores or hits after finding matching rows. `Prepare` finds a candidate and
pins its current leaf until a later `Prepare` advances to another leaf.
`Intersect` and `FilterRows` restrict candidates without releasing that leaf;
`ReadPayloads` gathers low field masks and optionally TF/hit references for
survivors. These operations must not be mixed with qword document reads. Hit
reads through the qword remain independent.

`PostingBlockAnd_c` intersects intervals bounded by all participating leaves and
at most 4096 row IDs. Once few candidates remain (interval length / 32), remaining
clauses filter a sorted row array. Output batches retain all participating leaves
until the next batch request, allowing consumers to gather payloads first.
Bitmap posting ordinals use a lazily built prefix-popcount table; ranges use row
offsets, and decoded leaves use a forward merge with the survivor batch. TF and
hit-reference columns are decoded once per accessed leaf only when requested.

Eligible block qwords in `ExtMultiAnd_T` use this producer and retain the existing
document-chunk, scoring and hit-merging interfaces. Legacy/RAM/unsupported qwords
and wide-field schemas keep the existing qword advancement path. Flat phrase,
proximity and NEAR nodes also use MultiAND candidates while retaining their
positional verification and hit ordering.

Eligible AND-NOT nodes attach excluded membership cursors to the same producer.
Dense candidates use `FillAndNot` directly against stored ranges/bitmaps; sparse
candidates use exact field-aware probes. Negative TFs and hit references are not
read. Positive leaves remain available for gathering survivor payloads. Negative
EOF preserves candidates; interruption discards the unfinished candidate batch.

The native AND-NOT path accepts ordinary positive terms (one term or a flat AND)
and ordinary negative terms (one term or an OR). Legacy/RAM qwords, wide fields,
hitless tables, positional exclusions, expansions and compound positive operands
retain the existing tree. It also retains the tree beneath optional operands
such as OR/MAYBE: existing hit forwarding there can include positive-term hits
from rejected branches, so collecting only survivors would change scores.

`integration_andnot.py` uses the same arguments as the positional suite above.
Run it against baseline/candidate disposable daemons with both mmap and file
access. It compares rows, weights and warnings through RAM, mixed, flushed and
merged stages, including field restrictions, empty/missing exclusions, nested
fallbacks, multiple rankers, row bounds and deletions. Low-level qword tests also
check subtraction boundaries, padding, cancellation and lazy negative payloads.


### Quorum membership windows

Distinct plain-term quorum queries with 2..31 required matches across at most
32 clauses use the shared window producer for count and `ranker=none` consumers.
Each clause fills an exact field-aware membership bitmap. Two-of-N uses seen-once
and seen-at-least-twice masks; other thresholds add bit-sliced counters and compare
them to the required count. Windows before the threshold-th clause's next posting
cannot satisfy quorum and are skipped. Interrupted windows are never published.

Block qwords fill these bitmaps from stored containers; legacy disk qwords use
the buffered membership adapter. Neither path decodes positions for quorum
membership. `SHOW META` reports `WindowQuorumCount` for eligible counts.

Scored queries, repeated terms (including dictionary-normalized duplicates),
expanded/positional/compound operands, hitless/wide-field tables, RAM segments,
more than 32 clauses, and thresholds normalized to ordinary AND/OR retain their
existing paths. Repeated-term quorum counts occurrences, so it must not be treated
as distinct-clause bitmap membership. This step does not accelerate scored Q42.

`integration_quorum.py` accepts the same baseline/oracle arguments as the other
RT differential suites. Add `--expect-windows` for the candidate binary to assert
executor selection. Coverage includes absolute/percentage thresholds, 32/33
clauses, sparse/missing terms, duplicates, restrictions and scored fallbacks;
run both mmap and file access. `postings_qword_test` compares the bitmap counter
against scalar per-row counts through all 32 clause/threshold values and reset.

### Single-term consumers

`ExtTerm_T` declares the payloads it needs before reading. Block qwords decode
TFs for BM25 and TFs plus hit references for consumers that collect hits;
unranked consumers decode neither column. Positional wrappers may request hits
before attaching their qword, so initialization preserves that request. The
existing batch reader has separate membership, TF-only and full-payload loops.
Legacy qwords retain their existing decoding behavior.

Eligible single-term count and `ranker=none` roots on block-format disk chunks
use the shared window producer. Stored bitmap/range membership fills windows
without first reconstructing posting records. Counts consume those windows
directly; document consumers use the existing window-to-document adapter, filters
and sorters. Dense single-term windows use the bitmap path. Terms below 1/32 of
the chunk's documents retain the ordinary batch reader with selective payload
decoding: both bitmap windows and the cursor's row-batch path cost more CPU on
the measured sparse fixture. Field restrictions, bounds, deleted rows
and interruption checks retain the existing window contracts.

Dictionary `FastCount` remains the first choice when eligible. Window counts
report `WindowTermCount` in `SHOW META`. Legacy postings, RAM segments, wide-field
and hitless tables, expanded terms and positional restrictions retain their
existing paths. BM25-only roots use selective payload decoding, not membership
windows. This does not implement score-bound pruning or change the index format.

`tests/integration_singleterm.py` compares rows, scores and warnings against an
optional baseline oracle in mmap/file modes and across RAM, mixed, deleted disk
and merged RT stages. It also checks positional/scored fallbacks and window
selection for field-restricted counts.
