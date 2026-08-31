# Hybrid search

Hybrid search combines [full-text (BM25) search](../Searching/Full_text_matching/Basic_usage.md) with [KNN vector search](../Searching/KNN.md) in a single query, fusing results using **Reciprocal Rank Fusion (RRF)**. This leverages the strengths of both retrieval methods: keyword precision from BM25 and semantic understanding from vector similarity.

Full-text search excels at exact keyword matching and rare terms but misses conceptually similar content. Vector search captures semantic meaning but can be noisy on ambiguous queries. Hybrid search combines both, so documents that score well on **either or both** signals are surfaced.

## Reciprocal Rank Fusion (RRF)

RRF is a rank-based fusion algorithm. It operates on **rank positions** rather than raw scores, which avoids the need to normalize incompatible score scales (BM25 scores are unbounded; KNN distances have a different scale).

### Formula

```
RRF_score(d) = SUM over all result sets r:  weight_r / (rank_constant + rank_r(d))
```

Where:
- `d` is a document
- `rank_r(d)` is the document's 1-based position in result set `r` (sorted by that retriever's score)
- `rank_constant` is a smoothing constant (default: 60, configurable via the [`rank_constant`](../Searching/Options.md#rank_constant) option)
- `weight_r` is an optional per-retriever weight (default: 1.0)

If a document does not appear in a particular result set, its contribution from that set is 0.

### The rank_constant parameter

- `rank_constant=60` is the default.
- Lower values (e.g. 10) amplify differences between top-ranked items.
- Higher values (e.g. 100) distribute influence more evenly across ranks.

## Basic hybrid query

Combine `MATCH(...)` and `KNN(...)` in the WHERE clause, with `OPTION fusion_method='rrf'`:

<!-- example hybrid_basic -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, hybrid_score()
FROM t
WHERE match('machine learning')
  AND knn(vec, (0.1, 0.1, 0.1, 0.1))
OPTION fusion_method='rrf';
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "t",
  "knn": {
    "field": "vec",
    "query_vector": [0.1, 0.1, 0.1, 0.1]
  },
  "query": { "match": { "title": "machine learning" } },
  "options": { "fusion_method": "rrf" }
}
```

<!-- end -->

This runs the text search and KNN search as independent parallel sub-queries, then fuses the results using RRF. Without `fusion_method='rrf'`, the query runs as a regular KNN search filtered by the text match (pre-hybrid behavior).

In that pre-hybrid mode, KNN ranking still takes precedence. If `knn_dist()` is available and you do not explicitly sort by it, Manticore prepends `knn_dist() ASC` to the sort order. In practice, `ORDER BY weight() DESC` becomes a secondary tie-breaker rather than a global BM25 sort.

### Available functions

- [`hybrid_score()`](../Functions/Other_functions.md#HYBRID_SCORE%28%29) - the RRF fusion score (only available in hybrid queries)
- [`weight()`](../Functions/Searching_and_ranking_functions.md#WEIGHT%28%29) - the BM25 text match score
- [`knn_dist()`](../Functions/Other_functions.md#KNN_DIST%28%29) - the vector distance (minimum across all KNN sub-queries if multiple)

### Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| [`fusion_method`](../Searching/Options.md#fusion_method) | string | *(none)* | Set to `'rrf'` to enable hybrid search. Required. |
| [`rank_constant`](../Searching/Options.md#rank_constant) | int | 60 | Smoothing constant in the RRF formula |
| [`window_size`](../Searching/Options.md#window_size) | int | 0 (auto) | How many results each sub-query retrieves before fusion. When 0, auto-computed from KNN `k` (with oversampling) and query `LIMIT` |
| [`fusion_weights`](../Searching/Options.md#fusion_weights) | tuple | *(all 1.0)* | Per-sub-query weights for RRF scoring |

## Tuning rank_constant

<!-- example hybrid_rank_constant -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
-- Default rank_constant=60 (gentler ranking)
SELECT id, hybrid_score() FROM t
WHERE match('machine learning') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
OPTION fusion_method='rrf';

-- rank_constant=10 (sharper top-rank differences)
SELECT id, hybrid_score() FROM t
WHERE match('machine learning') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
OPTION fusion_method='rrf', rank_constant=10;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "t",
  "knn": { "field": "vec", "query_vector": [0.1, 0.1, 0.1, 0.1] },
  "query": { "match": { "title": "machine learning" } },
  "options": { "fusion_method": "rrf", "rank_constant": 10 }
}
```

<!-- end -->

## Attribute filters

Standard WHERE filters work alongside hybrid search. Filters are applied to both the text and KNN sub-queries:

<!-- example hybrid_filters -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, category, hybrid_score()
FROM t
WHERE match('machine learning')
  AND knn(vec, (0.1, 0.1, 0.1, 0.1))
  AND category = 1
OPTION fusion_method='rrf';
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "t",
  "knn": { "field": "vec", "query_vector": [0.1, 0.1, 0.1, 0.1] },
  "query": {
    "bool": {
      "must": [
        { "match": { "title": "machine learning" } },
        { "equals": { "category": 1 } }
      ]
    }
  },
  "options": { "fusion_method": "rrf" }
}
```

<!-- end -->

## Sorting

By default, results are sorted by `hybrid_score() DESC`. You can override this:

This section applies to true hybrid queries, i.e. queries using `OPTION fusion_method='rrf'`. Without `fusion_method='rrf'`, a query that includes `KNN(...)` is not fused and remains KNN-first, so `ORDER BY weight() DESC` does not produce a globally weight-sorted result set.

<!-- example hybrid_sorting -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
-- Sort by hybrid score ascending
SELECT id, hybrid_score() FROM t
WHERE match('machine learning') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
ORDER BY hybrid_score() ASC
OPTION fusion_method='rrf';

-- Sort by text weight
SELECT id, weight() FROM t
WHERE match('machine learning') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
ORDER BY weight() DESC, id ASC
OPTION fusion_method='rrf';

-- Sort by KNN distance
SELECT id, knn_dist() FROM t
WHERE match('machine learning') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
ORDER BY knn_dist() ASC
OPTION fusion_method='rrf';
```

<!-- end -->

## Non-matching text

If the text query matches no documents, only KNN results contribute to the RRF score:

```sql
SELECT id, hybrid_score() FROM t
WHERE match('xyznonexistent') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
OPTION fusion_method='rrf';
-- Returns results ranked purely by KNN rank
```

## Multiple KNN searches

A single hybrid query can combine text search with multiple KNN searches on different vector attributes. All are fused together via RRF:

<!-- example hybrid_multi_knn -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
-- Three-way fusion: text + vec1 KNN + vec2 KNN
SELECT id, hybrid_score()
FROM t
WHERE match('machine learning')
  AND knn(vec1, (0.1, 0.1, 0.1, 0.1))
  AND knn(vec2, (1.0, 0.0, 0.0, 0.0))
OPTION fusion_method='rrf';

-- KNN-only fusion (no text), two vector searches
SELECT id, hybrid_score()
FROM t
WHERE knn(vec1, (0.1, 0.1, 0.1, 0.1))
  AND knn(vec2, (1.0, 0.0, 0.0, 0.0))
OPTION fusion_method='rrf';
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "t",
  "knn": [
    { "field": "vec1", "query_vector": [0.1, 0.1, 0.1, 0.1] },
    { "field": "vec2", "query_vector": [1.0, 0.0, 0.0, 0.0] }
  ],
  "query": { "match": { "title": "machine learning" } },
  "options": { "fusion_method": "rrf" }
}
```

<!-- end -->

Multiple KNN searches without `fusion_method` produce an error.

## Weighted fusion (fusion_weights)

By default, all sub-queries contribute equally (weight 1.0). To give different importance to text vs KNN searches, use [`fusion_weights`](../Searching/Options.md#fusion_weights) with explicit aliases:

<!-- example hybrid_weights -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, hybrid_score()
FROM t
WHERE match('machine learning') AS text
  AND knn(vec1, (0.1, 0.1, 0.1, 0.1)) AS dense1
  AND knn(vec2, (1.0, 0.0, 0.0, 0.0)) AS dense2
OPTION fusion_method='rrf',
       fusion_weights=(text=0.7, dense1=0.2, dense2=0.1);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "t",
  "knn": [
    { "field": "vec1", "query_vector": [0.1, 0.1, 0.1, 0.1], "name": "dense1" },
    { "field": "vec2", "query_vector": [1.0, 0.0, 0.0, 0.0], "name": "dense2" }
  ],
  "query": { "match": { "title": "machine learning" } },
  "options": {
    "fusion_method": "rrf",
    "fusion_weights": { "query": 0.7, "dense1": 0.2, "dense2": 0.1 }
  }
}
```

<!-- end -->

### Alias rules

**SQL:**
- Use `AS alias` on `MATCH(...)` and `KNN(...)` to name them. There are no implicit/default aliases.
- Omitted aliases default to weight 1.0.
- Referencing a non-existent alias produces an error.

**JSON:**
- `"query"` is the fixed alias for the full-text sub-query.
- KNN aliases are set via the `"name"` property on each KNN entry.
- A KNN entry named `"query"` collides with the text alias and produces an error.
- Implicit aliases (field names without explicit `"name"`) are not supported in `fusion_weights`.

### Partial weights

You can specify weights for only some sub-queries; the rest default to 1.0:

```sql
-- Only boost text, KNN searches default to weight 1.0
SELECT id, hybrid_score()
FROM t
WHERE match('machine learning') AS text
  AND knn(vec1, (0.1, 0.1, 0.1, 0.1)) AS dense1
  AND knn(vec2, (1.0, 0.0, 0.0, 0.0)) AS dense2
OPTION fusion_method='rrf', fusion_weights=(text=2.0);
```

## Simplified syntax: hybrid_match()

For tables with [auto-embeddings](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29) configured on a float_vector attribute, `hybrid_match()` provides a shorthand that automatically runs both text and KNN searches from a single query string:

<!-- example hybrid_match -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
-- Explicit vector field
SELECT id, hybrid_score() FROM t WHERE hybrid_match('machine learning', vec);

-- Auto-detect vector field (requires exactly one auto-embedding attribute)
SELECT id, hybrid_score() FROM t WHERE hybrid_match('machine learning');

-- With custom k and rank_constant
SELECT id, hybrid_score() FROM t
WHERE hybrid_match('machine learning', vec, {k=3})
OPTION rank_constant=10;

-- With attribute filter
SELECT id, hybrid_score() FROM t
WHERE hybrid_match('machine learning', vec) AND category=1;
```

<!-- end -->

`hybrid_match()` automatically:
1. Runs the text query as a BM25 full-text search
2. Generates an embedding from the same text string
3. Runs a KNN search using that embedding
4. Fuses results via RRF

**Requirement**: The vector attribute must have `model_name` and `from` configured for [auto-embeddings](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29). Without them, `hybrid_match()` returns an error.

## Simplified JSON syntax: hybrid

<!-- example hybrid_json -->

For tables with auto-embeddings, a `"hybrid"` property provides a shorthand in JSON:

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "hj",
  "hybrid": { "query": "machine learning" }
}

POST /search
{
  "table": "hj",
  "hybrid": { "query": "machine learning", "field": "vec" }
}

POST /search
{
  "table": "hj",
  "hybrid": { "query": "machine learning" },
  "options": { "rank_constant": 10 }
}
```

<!-- end -->

The `"hybrid"` property cannot be used together with `"knn"`.

## Auto-embedded KNN in hybrid queries

When the vector attribute has [auto-embeddings](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29), you can use `"query"` (string) instead of `"query_vector"` (array) in the `knn` object:

```json
POST /search
{
  "table": "ht",
  "knn": { "field": "vec", "query": "machine learning", "k": 5 },
  "query": { "match": { "title": "machine learning" } },
  "options": { "fusion_method": "rrf" }
}
```

The string is automatically embedded at query time. Without auto-embeddings configured, this returns an error.

## Execution architecture

Internally, a hybrid query is split into N+1 parallel sub-queries:

1. Job 0: Full-text (BM25) sub-query (skipped if text query is empty, to avoid polluting RRF with fullscan results)
2. Jobs 1..N: One KNN sub-query per `knn(...)` entry

All sub-queries run concurrently. After all complete, the RRF fusion:
1. Collects ranked results from each sub-query
2. For each document, accumulates RRF score contributions from every sub-query it appears in
3. Sorts by fused RRF score descending
4. Sets [`knn_dist()`](../Functions/Other_functions.md#KNN_DIST%28%29) to the minimum distance across all KNN sub-queries for each document
5. Preserves [`weight()`](../Functions/Searching_and_ranking_functions.md#WEIGHT%28%29) from the text sub-query

<!-- proofread -->
