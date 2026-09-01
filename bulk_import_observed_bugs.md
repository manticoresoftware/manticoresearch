# Observed bugs related to `bulk_import`

## `/_bulk?bulk_import=<table>` does not hit the unsupported-mode guard

### Status

Fixed in the separate commit `fix: reject bulk_import option on Elasticsearch bulk`.

### Location

`src/searchdhttp.cpp`, `HttpHandlerEsBulk_c::Process()`.

The guard currently checks the query parameter only when its value is exactly `1`:

```cpp
m_hOpts.Exists ( "bulk_import" ) && m_hOpts["bulk_import"]=="1"
```

The public Manticore `/bulk` API, however, activates direct-to-disk loading with a table name:

```text
?bulk_import=<table>
```

### Consequence

A request such as:

```text
POST /_bulk?bulk_import=products
```

uses `products`, not `1`, so the explicit "unsupported on Elasticsearch `/_bulk`" branch is skipped. The request can continue through the regular Elasticsearch-compatible bulk path instead of failing because direct-to-disk mode is unsupported there.

This conflicts with the documented contract that Elasticsearch-compatible `/_bulk` does not support direct-to-disk bulk insertion.

### Reproduction

1. Create an existing local RT table named `products`.
2. Send valid Elasticsearch NDJSON to:

```bash
printf '%s\n' \
  '{"index":{"_index":"products","_id":"1"}}' \
  '{"title":"unexpected regular-path insert"}' |
curl -sS -X POST \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary @- \
  'http://localhost:9308/_bulk?bulk_import=products'
```

3. Check whether the request is accepted and document `1` appears in `products`.

### Observed result

Confirmed locally against the minimally renamed build:

```text
POST /_bulk?bulk_import=products
→ HTTP bulk item reports `result: created`, `status: 201`, `errors: false`
→ `SELECT COUNT(*) FROM products WHERE id=4` returns `1`
```

Thus the request reached the regular Elasticsearch-compatible insertion path and created the document instead of returning the unsupported-mode error.

### Expected behavior

Any `bulk_import` query parameter on Elasticsearch-compatible `/_bulk` should produce the explicit unsupported-mode error, regardless of its value.

### Resolution

The handler now rejects the presence of the `bulk_import` option regardless of its value. A focused CLT regression covers `/_bulk?bulk_import=products` and verifies that no document is inserted.

## Native `/bulk` accepts document ID `0` in bulk import

### Status

Fixed in the separate commit `fix: reject zero document ids in bulk import`.

### Location

The native Manticore `/bulk` path accepts an operation with `"id":0` while `bulk_import=<table>` is active. The resulting disk chunk contains a row whose document ID is `0`, despite the documented requirement that each row provide an explicit, non-zero numeric ID.

### Reproduction

```bash
printf '%s\n' \
  '{"insert":{"table":"products","id":0,"doc":{"title":"zero id"}}}' |
curl -sS -X POST \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products'
```

Then query the table:

```sql
SELECT id, title FROM products;
```

### Observed result

The request succeeds and the table contains a row with `id = 0`.

### Expected behavior

Bulk import should reject ID `0` consistently with its explicit non-zero-ID contract.

### Resolution

Bulk import validates every staged row before starting the disk build and rejects ID `0`. A focused CLT regression verifies the error and that the table remains empty.
