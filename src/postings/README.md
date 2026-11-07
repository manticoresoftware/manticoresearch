# Block postings (M1/M2/M3)

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
and references with each posting leaf and seek external positions when needed.
M4 will replace the transitional external position varints. Native length-normalized BM25 and
WAND/BMW remain later milestones.

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
The initial writer selects PFOR for rows/TF and FOR for masks/references.

Each 1..256-posting leaf has a 44-byte header: `BLK1`, byte length (uint32),
posting count and field-mask word count (uint16 each), first/last row and maximum
TF (uint32 each) and five uint32 section lengths. Sections are rows, fields, TF, hit references and per-field TF.
Rows use the row-container grammar below. Other integer frames have a uint32
byte-length prefix. Field masks store one frame per 32-bit mask word. Hit
references store low/high uint32 frames. Per-field TF contains sorted nonempty
field entries `(field:uint16, frame length:uint32, integer frame)`.

The footer contains `(document count:uint64, summed TF:uint64)` per field.
Hitless terms store membership counts and zero field TF.
Positions are always external: existing packed-hit deltas, big-endian varints,
terminated by zero per document. Singleton/single-hit postings are not inline.

The `.spe` directory contains `SKP1`, leaf count (uint32), then entries
`(first:uint32, last:uint32, absolute leaf offset:uint64, max TF:uint32)`.
The checker compares entries with decoded leaves. M2 seeks by leaf headers;
using the directory for hierarchical skipping and score pruning is later work.
Maximum TF alone is not a safe complete score bound for every ranker.

Normal readers trust index data; structural/semantic checks run only in debug
assertions. `BlockIndexQword_c` and `Reader_c` have no validation-mode flag.
`indextool --check` constructs a separate `BlockIndexQwordValidation_c`, backed
by `ReaderValidation_c`; it checks framing and codec payloads before decoding.
The checker walks every leaf and checks all columns, directory
entries, footer statistics and per-field TF against external hits. No CRC fields
are written or checked. The previous experimental component-1 files require
rebuilding or conversion from legacy; their layout is rejected at table loading.
Unknown component/codec selection and I/O errors still fail explicitly.
Lazy readers do not inspect unused columns.
Writers buffer at most one leaf of postings plus one directory entry per leaf.

The implementation reuses FastPFOR's packing kernels, not its codec framing or
page-wide exception buffers. `fastpfor/PROVENANCE.json` records the upstream
commit, file paths and SHA-256 hashes. Files there are upstream sources with LF
normalization only; original copyright/license notices remain intact.

## Stream descriptor and integer frames, version 1 proposal

Select the integer codec once per logical stream (for example a term's TF or
position stream). The M1 descriptor is one little-endian uint16 codec ID:
FOR=1, PFOR=2, VARINT=3, CONSTANT=4; zero and unknown IDs are errors.
Read this descriptor once and pass the resulting StreamDesc_t to every block
operation. It is not prepended to every frame. The term metadata above identifies the stream kind, boundaries and descriptor location.

Changing the selected registered codec changes the stream descriptor, not the
index/component format version. Each ID specifies a stable codec byte grammar;
a future incompatible codec grammar gets a new ID and decoder, while existing
IDs remain readable. An older reader that does not implement a required ID
rejects that stream explicitly. This does not promise that old binaries can
decode future codecs.

All multibyte integers use little-endian encoding. Frames are independent and
contain 1..256 unsigned 32-bit integers. The caller supplies the exact frame
length; the checker rejects trailing bytes. IDs/layouts below are experimental until
publication. No codec ID or codec override is present in an integer frame:

| Offset | Bytes | Meaning |
|---|---|---|
| 0 | 1 | Packed width, 0..32; zero for VARINT/CONSTANT |
| 1 | 1 | Exception count, 0..7; zero except PFOR |
| 2 | 2 | Value count |
| 4 | 2 | Payload byte length |
| 6 | variable | Payload |

FOR/PFOR payloads contain `ceil(count/128)` packing frames. Each frame has four
interleaved lanes of 32 values: lane L contains input indices `L+4*j` for j=0..31.
Pack each lane's low `width` bits in increasing j order, least significant bits
first. Output words interleave the lanes: word `4*k+L` is word k of lane L.
Serialize each word little-endian. Each 128-value frame occupies `16*width`
bytes. Unused values in a short frame must be zero. Width zero has no packed
payload; width 32 preserves all bits. Neither layout nor bytes depend on CPU ISA.

PFOR appends exception pairs `(ordinal:u8, high_bits:u8)` in strictly increasing
ordinal order across the complete block. Patch `value |= high_bits << width`.
High bits must be nonzero and fit the remaining 32-bit value; width 32 cannot
have exceptions. Width selection minimizes payload size subject to these
constraints. More than seven exceptions, or high parts exceeding eight bits,
cause a wider width to be chosen. Zero-exception PFOR is permitted. FOR has none.

VARINT is canonical unsigned LEB128, at most five bytes/value, with no redundant
zero final group. CONSTANT stores one uint32 per block; it requires every
value within that block to be equal, but the constant can differ between blocks.
Blocks cannot independently switch codecs. In M1, a short packed block uses
zero-padded packing frames under the same stream codec; any future special
constant/tail modes must be part of a registered codec's specified grammar,
not an unrecorded change of decoder.

InspectIntegers validates counts, lengths, widths, exceptions, canonical
varints and packed tail padding using the supplied stream descriptor.
DecodeIntegersInto trusts the supplied frame, with validation in debug assertions
only, and requires disjoint input/output ranges. The checker calls InspectIntegers
before decoding. Full aligned outputs avoid a decoded scratch copy; short tails
and unaligned outputs use bounded scratch so the API writes exactly the declared
count. Encoded input needs no alignment or extra padding. DecodeIntegers retains
overlap support through a scratch-buffer wrapper; the source must not be changed
concurrently. DecodeRowsInto and DecodeRows provide the same respective contracts
for row containers.

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
`ReadRows` share retained lookahead. Intersections use the shared bitmap API;
field rejection, clipped windows and interrupted output retain the existing
semantics. Continuation checks run at entry and before each leaf refill/skip.
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
