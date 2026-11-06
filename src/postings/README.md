# Posting codec primitives (M1)

This is an experimental component, not a new table format yet. No production
reader/writer selects these codecs. M2 will provide chunk metadata, separate
row/field/TF streams and dictionaries. File offsets, hit references, norms and
impacts are outside this module's present scope. There are no inline hits.

The implementation reuses FastPFOR's packing kernels, not its codec framing or
page-wide exception buffers. `fastpfor/PROVENANCE.json` records the upstream
commit, file paths and SHA-256 hashes. Files there are upstream sources with LF
normalization only; original copyright/license notices remain intact.

## Stream descriptor and integer frames, version 1 proposal

Select the integer codec once per logical stream (for example a term's TF or
position stream). The M1 descriptor is one little-endian uint16 codec ID:
FOR=1, PFOR=2, VARINT=3, CONSTANT=4; zero and unknown IDs are errors.
Read this descriptor once and pass the resulting StreamDesc_t to every block
operation. It is not prepended to every frame. The enclosing M2 metadata will
identify the stream kind, boundaries and descriptor location.

Changing the selected registered codec changes the stream descriptor, not the
index/component format version. Each ID specifies a stable codec byte grammar;
a future incompatible codec grammar gets a new ID and decoder, while existing
IDs remain readable. An older reader that does not implement a required ID
rejects that stream explicitly. This does not promise that old binaries can
decode future codecs.

All multibyte integers use little-endian encoding. Frames are independent and
contain 1..256 unsigned 32-bit integers. The caller supplies the exact frame
length; trailing bytes are rejected. IDs/layouts below are experimental until
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
DecodeIntegers validates before writing any caller output. It uses bounded
stack scratch, explicit byte-order conversion and aligned kernel buffers,
requiring no input alignment or padding.
It accepts input/output overlap, but the source must not be changed concurrently.

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
does decode deltas. `DecodeRows` materializes rows for tests/adapters. Future
native bitmap consumers must use the borrowed view rather than reconstructing
rows. These are storage primitives, not `PostingWindow_i` integration (M3).

There is no checksum at this layer: structurally valid payload changes remain
valid. Whole-file/block integrity policy belongs to the enclosing format.

## Backends and build

The portable reference packs/unpacks the same four-lane layout. On x86-64,
AUTO selects the vendored SSE2 kernels; SSE2 is a baseline feature there. ARM
and other targets use the portable implementation, without an x86 dependency
or a SIMDe requirement. An explicit unavailable SIMD request fails. Future ISA
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
including headers/padding/exceptions. Decode timings include validation and
copies, not just the packing kernels. Encoding includes block parameter selection/allocation. The benchmark-only
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
