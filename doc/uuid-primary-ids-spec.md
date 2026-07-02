# UUID primary IDs implementation specification

**Status:** branch implementation and QA spec for GitHub issue #1429, synced with branch `fix/uuid-primary-ids-1429` at `de7dd5a04`.

## Goal

Support UUID/GUID values as user-facing document IDs for RT/configless tables while keeping Manticore's internal `DocID_t` machinery numeric. This applies to both ordinary row-wise RT tables and RT tables with `engine='columnar'`. The design avoids widening `DocID_t`, adding a broad UUID attribute type, or adding native UUID/128-bit support to Columnar/MCL.

## Design summary

The minimal implementation keeps two identities for UUID-ID tables:

1. **Internal ID**: the existing numeric `id` / `DocID_t` (`SPH_ATTR_BIGINT`) used by RT chunks, docstore, kill lists, updates, deletes, replication, and other existing internals.
2. **Public ID**: a hidden exact string attribute named `@uuid_id`, exposed to users as `id` on UUID-ID tables.

Public `id` operations are translated as follows:

- incoming public `id` values are validated as strict UUID strings, normalized to lowercase, and stored in `@uuid_id`;
- omitted `id` values generate a UUIDv8-style string derived from the unique internal numeric `DocID_t` surrogate;
- explicit duplicate checks and `REPLACE`/`UPDATE`/`DELETE` resolution find the internal numeric docid through exact string lookup on `@uuid_id`; generated UUID inserts skip that lookup;
- result formatting maps `@uuid_id` back to public `id` and hides the internal numeric surrogate.

This preserves exact UUID semantics without hashing UUIDs into 64-bit IDs.

UUID-ID correctness does **not** depend on query-time secondary indexes being enabled. The hidden `@uuid_id` string attribute is marked indexed so exact string filters can use the normal acceleration path when secondary indexes are enabled, but UUID lookup, duplicate resolution, `REPLACE`, `UPDATE`, and `DELETE` must still work with `searchd.secondary_indexes = 0` by falling back to the ordinary exact string filter path.

## Public behavior

### DDL

```sql
CREATE TABLE products_uuid ( id uuid, title text, price int );
```

Rules:

- `id uuid` is valid only for RT/configless tables, including `engine='columnar'` RT tables.
- Only the primary `id` column can use the `uuid` type.
- Regular attributes cannot use `uuid`.
- PQ/plain/indexer-created tables are not supported for UUID IDs.
- Existing tables cannot be converted to or from `id uuid` with `ALTER TABLE`.
- `SHOW CREATE TABLE` renders the public schema as `id uuid`; the hidden storage attribute is not exposed as a normal column.

### ID format

Accepted explicit IDs are quoted UUID strings in canonical `8-4-4-4-12` hexadecimal layout:

```text
550e8400-e29b-41d4-a716-446655440000
```

Validation is strict enough to reject malformed or reserved UUID-like values:

- exactly 36 bytes;
- hyphens at positions 8/13/18/23;
- hexadecimal digits elsewhere;
- version nibble at position 14 must be `1` through `8`;
- RFC variant nibble at position 19 must be `8`, `9`, `a`, `A`, `b`, or `B`.

Uppercase hexadecimal input is accepted and normalized to lowercase. The nil UUID and version-0 / invalid-variant values are rejected by the same checks. Generated IDs use a UUIDv8-style layout that encodes the internal numeric `DocID_t`, making generated public UUID IDs unique under the same guarantees as internal numeric auto-IDs. Generated IDs are not random UUIDv4 values.

### SQL DML

Supported:

- `INSERT INTO t (id, ...) VALUES ('uuid', ...)`
- `INSERT INTO t (...) VALUES (...)` with generated UUID `id`
- `REPLACE INTO t (id, ...) VALUES ('uuid', ...)`
- `UPDATE t SET ... WHERE id='uuid'`
- `DELETE FROM t WHERE id='uuid'`
- equality and `IN` filters on public `id`
- `ORDER BY id`, `GROUP BY id`, `COUNT(DISTINCT id)`, and facets over public `id` as string semantics
- `LAST_INSERT_ID()` and `@@session.last_insert_id` return UUID strings for UUID-ID tables

Rejected/unsupported:

- numeric values for UUID `id`
- invalid UUID strings, including nil UUIDs, version-0 strings, and invalid RFC variants
- direct use of `@uuid_id` in public select/filter/group/sort/update/insert paths
- range filters (`<`, `<=`, `>`, `>=`) and arithmetic expressions on UUID `id`
- treating `0` as an auto-ID marker; omit `id` instead

### JSON / Elasticsearch-compatible APIs

Supported UUID-aware paths include:

- `/json/insert`, `/json/replace`, `/json/update`, `/json/delete`, `/json/search`
- `/json/bulk` valid insert/replace/update/delete actions
- ES-style `_bulk` `index`/`create` actions with string `_id`
- ES-style `_search` response IDs/docvalue fields for public `id`

For numeric-ID tables, previous numeric/compatibility behavior is preserved.

Client-side UUID validation errors should be reported as request validation errors, not daemon/internal errors. The intended status contract is HTTP 400 for invalid UUID strings, numeric IDs sent to UUID-ID tables, hidden internal ID access (`@id`/`@uuid_id`), and unsupported UUID range/arithmetic operations. Current branch QA shows the data-level behavior is reject/no-write, but a few HTTP status/envelope paths still need follow-up; see [Known current follow-up gaps](#known-current-follow-up-gaps).

## Code structure

### Core helpers

`src/indexsettings.cpp` / `src/indexsettings.h` define the UUID-ID helpers:

- `sphGetUuidDocidName()` returns `@uuid_id`;
- `sphHasUuidDocid(schema)` detects UUID-ID tables by hidden string attr presence;
- `sphIsValidUuidDocid()` validates strict UUID layout, version, and RFC variant;
- `sphCheckUuidDocid()` returns user-facing validation errors;
- `sphNormalizeUuidDocid()` lowercases UUID strings.

`src/searchd.cpp` defines `sphGenerateUuidDocid(DocID_t)` and insert-time helpers for:

- encoding the unique internal numeric `DocID_t` into a UUIDv8-style public string;
- locating the hidden string attr ordinal in insert string storage;
- resolving explicit UUID strings to existing numeric `DocID_t` values;
- detecting duplicate UUIDs inside a single multi-row statement with `sph::StringSet`.

### DDL and schema

`src/ddl.y` and `src/searchdddl.cpp` add a special `id uuid` grammar branch. The DDL parser:

- validates that the type token is `uuid`;
- validates that the column name is exactly `id`;
- creates the normal internal numeric `id` attr;
- adds hidden string attr `@uuid_id` and marks it indexed. This is an optional acceleration hook for exact string filters, not a correctness dependency on `secondary_indexes=1`.

`src/attribute.cpp`, `src/indexcheck.cpp`, and create-table validation treat `@uuid_id` as internal but allow it only when generated by the UUID-ID DDL path.

### Insert/replace

`src/searchd.cpp`, `src/searchdhttp.cpp`, and `src/searchdhttpcompat.cpp` route public `id` values to `@uuid_id` for UUID-ID tables. They do not make `SPH_ATTR_BIGINT` accept UUID strings.

For insert/replace:

1. map public `id` input to `@uuid_id`;
2. validate and lowercase the UUID string;
3. generate a UUID when `id` is omitted;
4. resolve existing UUID on `REPLACE` and reuse existing internal docid when present;
5. reject duplicates for `INSERT` and duplicate UUIDs staged inside the same statement/bulk request.

### Lookup, filters, updates, deletes

UUID public `id` filters are handled as string filters against the hidden `@uuid_id` attribute where public query semantics need string behavior, or resolved to internal numeric docids before existing write/delete internals need `DocID_t`.

The implementation intentionally uses existing exact string/filter/query machinery rather than adding a new persistent UUID→docid map or changing RT/disk chunk storage layout. When secondary indexes are disabled globally (`searchd.secondary_indexes = 0`), the same public UUID operations still resolve through the ordinary string filter path; they may be slower, but they are semantically supported.

### Result rendering

Changed result-formatting paths map `@uuid_id` back to public `id`:

- SQL column names and values;
- JSON search hits and `_source`/docvalue handling;
- ES-compatible responses;
- joins, facets, sorting/grouping, and aggregate keys involving public `id`.

The internal numeric `id` remains hidden on UUID-ID tables.

### Distributed, sharding, and replication

Distributed agent result paths send the hidden UUID string when the master needs public `id` output.

Shard write buffering preserves the UUID string and routes UUID-ID rows deterministically from the UUID value. Sharded UUID `REPLACE` deletes are scheduled only on the routed target, not blindly on every route target.

Replication tests cover create/insert/update/replace/delete behavior through a cluster table.

### Columnar/MCL

Columnar RT tables are supported for UUID primary IDs. UUID public IDs use the existing hidden string attribute path (`@uuid_id`) and the normal internal numeric `DocID_t` surrogate, so the minimal implementation does not require Columnar/MCL storage-format changes. This avoids adding a native UUID or 128-bit type to MCL.

The supported columnar contract is the same public UUID-ID contract as row-wise RT tables: explicit UUID inserts, generated UUID inserts where applicable, equality/`IN` lookup by public `id`, result rendering as UUID strings, and delete/update/replace paths that first resolve the public UUID to the internal numeric docid. Operations that are generally limited by columnar storage rules, such as updating a columnar stored string attribute, remain governed by those existing columnar limitations; the UUID primary-id layer does not add a new MCL UUID type.

## Performance and memory considerations

- Internal `DocID_t` stays 64-bit, so existing row, kill-list, and docstore memory layouts are unchanged.
- Each UUID-ID row stores one additional exact string (`@uuid_id`).
- Per-statement duplicate detection uses `sph::StringSet`, avoiding O(N²) linear duplicate checks for large multi-row inserts/bulk batches.
- The branch does not add a table-wide UUID→docid RAM map. Explicit UUID duplicate/replace checks reuse existing query/filter lookup paths with low limits. Server-generated UUIDs are derived from the internal numeric auto-ID, so generated inserts skip the UUID lookup and go straight through the normal RT insert path.
- ID-based operations that start from a public UUID string (`SELECT ... WHERE id='uuid'`, `UPDATE`, `REPLACE`, and `DELETE`) are expected to be slower than the same operations on numeric IDs because they must resolve `@uuid_id` back to the internal numeric `DocID_t`. Current `manticore-load` measurements on 50k-row RT tables show UUID id operations at roughly 8-21% of numeric-id throughput. A future direct UUID→DocID lookup index/map would be needed to close that gap.
- Sharded UUID `REPLACE` now targets the deterministic UUID route instead of broadcasting delete predicates to every route target.

## Limitations / non-goals

- No native `SPH_ATTR_UUID` type.
- No widening of `DocID_t` or internal row IDs.
- No UUID IDs for plain/indexer-created tables or PQ tables.
- No `ALTER TABLE` conversion to/from UUID IDs.
- No numeric/range/arithmetic semantics for UUID `id`.
- No direct public access to `@uuid_id`.
- Generated UUIDs are for document identifiers, not cryptographic secrets.

## Known current follow-up gaps

Exploratory QA after `de7dd5a04` confirmed the following remaining UUID-related gaps. These are status/envelope classification issues unless noted otherwise: the branch rejects the bad request and does not intentionally write data, but the HTTP status or error wrapper does not yet match the desired client-validation contract.

1. `POST /sql?mode=raw` with a UUID `id` range predicate, for example `WHERE id > 'aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaa0001'`, currently returns HTTP 500 with `uuid id range filters are not supported`; it should be HTTP 400.
2. `POST /json/search` with a UUID `id` range predicate currently returns HTTP 500 with the same validation error; it should be HTTP 400.
3. `POST /json/update` with an invalid UUID string in top-level `id` currently returns HTTP 409/action-request-validation; it should be HTTP 400 parse/request validation.
4. `POST /json/insert` with a numeric top-level `id` into a UUID-ID table currently returns HTTP 409/action-request-validation; it should be HTTP 400 because UUID IDs must be strings.
5. `POST /json/replace` with an invalid UUID string in top-level `id` currently returns HTTP 409/action-request-validation; it should be HTTP 400.
6. `POST /json/bulk` with an invalid UUID action `id` currently returns top-level HTTP 500 and per-item 409; it should not be a server error. Prefer either top-level HTTP 400 or the endpoint's established bulk-envelope behavior with per-item 400.
7. ES-compatible `POST /_bulk` with an invalid UUID `_id` currently gives per-item 400 but bubbles the response to top-level HTTP 409. Prefer ES-style top-level HTTP 200 with item status 400, or top-level HTTP 400 if the project standardizes on request rejection.

The likely minimal fix area for these gaps is HTTP error classification/envelope handling around `searchdhttp.cpp` and the JSON/ES bulk adapters, not the UUID storage model.

## Test coverage in this branch

New tests:

- `test/test_517/test.xml` — RT UUID document IDs across SQL, JSON, ES-compatible bulk/search, generated IDs, validation errors, duplicate handling, `REPLACE`, `UPDATE`, `DELETE`, flush-to-disk, columnar RT, joins, distributed agent path, facets/aggs, and numeric-ID regression checks.
- `test/test_518/test.xml` — replicated RT UUID document IDs through cluster create/join/insert/update/replace/delete convergence.
- `test/test_519/test.xml` — focused UUID-ID regression with `searchd.secondary_indexes = 0`, covering row-wise UUID mutation/lookup paths, columnar RT lookup before/after `FLUSH RAMCHUNK`, columnar delete by UUID, and JSON lookup by public UUID `id`.

Focused verification should include:

```bash
# from /Users/sn/manticore_uuid/build
cmake --build . --target searchd indexer indextool

# from /Users/sn/manticore_uuid/test
php ubertest.php gen -s /Users/sn/manticore_uuid/build/src/searchd -i /Users/sn/manticore_uuid/build/src/indexer test_517
php ubertest.php t   -s /Users/sn/manticore_uuid/build/src/searchd -i /Users/sn/manticore_uuid/build/src/indexer test_517
php ubertest.php gen -s /Users/sn/manticore_uuid/build/src/searchd -i /Users/sn/manticore_uuid/build/src/indexer test_518
php ubertest.php t   -s /Users/sn/manticore_uuid/build/src/searchd -i /Users/sn/manticore_uuid/build/src/indexer test_518
php ubertest.php gen -s /Users/sn/manticore_uuid/build/src/searchd -i /Users/sn/manticore_uuid/build/src/indexer test_519
php ubertest.php t   -s /Users/sn/manticore_uuid/build/src/searchd -i /Users/sn/manticore_uuid/build/src/indexer test_519
```

If the local build/test harness uses a different build directory, run the equivalent existing configured target and the same test directories.
