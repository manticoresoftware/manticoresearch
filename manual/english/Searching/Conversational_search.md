# Conversational search

Conversational search adds retrieval-augmented chat over an existing vectorized table. It searches the table, builds context from matching documents, and asks a large language model (LLM) to answer using that context and the current conversation history.

Conversational search is provided by the `ConversationalSearch` Buddy plugin and is managed with SQL commands:

- `CREATE CHAT MODEL`
- `SHOW CHAT MODELS`
- `DESCRIBE CHAT MODEL`
- `DROP CHAT MODEL`
- `CALL CHAT`

> NOTE: Conversational search requires [Manticore Buddy](../Installation/Manticore_Buddy.md) with the `ConversationalSearch` plugin and the `llm` PHP extension installed. Provider support depends on the installed `llm` extension.

## How it works

At query time, `CALL CHAT`:

1. Loads the chat model configuration.
2. Loads conversation history for the provided conversation UUID, or creates a new UUID when none is provided.
3. Inspects the target table and selects a `FLOAT_VECTOR` field.
4. Routes the user message with the LLM to decide whether to:
   - run a new search
   - answer from previous search context
   - answer without search when retrieval is not needed
5. Runs KNN search with the selected vector field.
6. Builds prompt context from the selected vector field's `from='...'` source fields.
7. Generates the final answer with the configured LLM.
8. Stores the user and assistant messages in conversation history.

The `fields` argument in `CALL CHAT` is a historical name. It means which `FLOAT_VECTOR` field to use for KNN, not which fields to return. Search results are selected with `SELECT *`, but vector columns are removed from the returned `sources` payload.

## Table requirements

The searched table must contain at least one `FLOAT_VECTOR` field with auto-embedding source fields:

<!-- example conversational_search_create_vector_table -->

<!-- request SQL -->

```sql
CREATE TABLE docs (
	id BIGINT,
	title TEXT,
	content TEXT,
	embedding FLOAT_VECTOR
		knn_type='hnsw'
		hnsw_similarity='cosine'
		model_name='onnx-models/all-MiniLM-L12-v2-onnx'
		from='title,content'
) TYPE='rt';
```

<!-- end -->

`from='title,content'` matters for two reasons:

- Manticore uses it to build embeddings for the vector field.
- Conversational search uses the same fields to build the text context sent to the LLM.

With multiple vector fields, each vector field may use different source fields:

<!-- example conversational_search_create_table_multiple_vectors -->

<!-- request SQL -->

```sql
CREATE TABLE docs (
	id BIGINT,
	title TEXT,
	content TEXT,
	title_embedding FLOAT_VECTOR
		knn_type='hnsw'
		hnsw_similarity='cosine'
		model_name='onnx-models/all-MiniLM-L12-v2-onnx'
		from='title',
	content_embedding FLOAT_VECTOR
		knn_type='hnsw'
		hnsw_similarity='cosine'
		model_name='onnx-models/all-MiniLM-L12-v2-onnx'
		from='content'
) TYPE='rt';
```

<!-- end -->

If `CALL CHAT` does not specify a vector field, Buddy uses the first detected `FLOAT_VECTOR` field from `SHOW CREATE TABLE`.

For more information about vector fields and auto embeddings, see [K-nearest neighbor vector search](../Searching/KNN.md).

## Creating a chat model

Use `CREATE CHAT MODEL` to define the LLM and retrieval settings.

Minimal model:

<!-- example conversational_search_create_chat_model_minimal -->

<!-- request SQL -->

```sql
CREATE CHAT MODEL assistant (
	model='openai:gpt-4o-mini'
);
```

<!-- end -->

Model with provider options and retrieval settings:

<!-- example conversational_search_create_chat_model_options -->

<!-- request SQL -->

```sql
CREATE CHAT MODEL support_assistant (
	model='openai:gpt-4o-mini',
	api_key='sk-...',
	base_url='http://host.docker.internal:8787/v1',
	timeout=60,
	retrieval_limit=5,
	max_document_length=3000
);
```

<!-- end -->

Common options:

| Option | Required | Description |
|---|---:|---|
| `model` | Yes | LLM model id in `provider:model` format |
| `description` | No | Stored description |
| `api_key` | No | Provider API key passed to the `llm` extension |
| `base_url` | No | Provider or proxy base URL |
| `timeout` | No | LLM request timeout |
| `retrieval_limit` | No | Number of documents requested from KNN, from `1` to `50` |
| `max_document_length` | No | Per-document context limit; `0` disables truncation |

`model` is validated as `provider:model`, for example:

```sql
model='openai:gpt-4o-mini'
```


Provider support comes from the installed `llm` [PHP extension](https://github.com/manticoresoftware/llm-php-ext). Buddy forwards provider options such as `api_key`, `base_url`, and `timeout` to that extension.

`api_key` is optional when the provider key is available in Buddy's environment. For example, a Docker Compose service can expose provider keys like this:


```yaml
environment:
  - OPENAI_API_KEY=${OPENAI_API_KEY}
  - OPENROUTER_API_KEY=${OPENROUTER_API_KEY}
```

If `api_key` is not set in `CREATE CHAT MODEL`, the `llm` extension can use the matching provider environment variable. Pass `api_key` only when you want the model configuration to override the environment.

## Asking questions

Basic call:

<!-- example conversational_search_call_basic -->

<!-- request SQL -->

```sql
CALL CHAT(
	'What is vector search?',
	'docs',
	'assistant'
);
```

<!-- end -->

Continue a conversation:

<!-- example conversational_search_call_continue -->

<!-- request SQL -->

```sql
CALL CHAT(
	'Can you explain it with an example?',
	'docs',
	'assistant',
	'docs-chat-001'
);
```

<!-- end -->

Use a specific vector field for KNN:

<!-- example conversational_search_call_vector_field -->

<!-- request SQL -->

```sql
CALL CHAT(
	'Find documents where the title is about vector search',
	'docs',
	'assistant',
	'',
	'title_embedding'
);
```

<!-- end -->

When the fifth argument is provided, Buddy validates that the field exists and is a `FLOAT_VECTOR`. If it is omitted, Buddy detects the first `FLOAT_VECTOR` field from `SHOW CREATE TABLE`.

### CALL CHAT syntax

```sql
CALL CHAT(
	'query',
	'table',
	'model_name_or_uuid',
	'conversation_uuid',
	'vector_field'
);
```

Arguments are positional only:

| Position | Argument | Required | Description |
|---:|---|---:|---|
| 1 | `query` | Yes | User question |
| 2 | `table` | Yes | Table to search |
| 3 | `model_name_or_uuid` | Yes | Chat model name or UUID |
| 4 | `conversation_uuid` | No | Existing conversation id, or an empty string |
| 5 | `fields` / vector field | No | `FLOAT_VECTOR` field used in `knn(...)` |

The fifth argument is stored internally as `fields` for compatibility with the current parser, but it must be a single vector field name.

## Search and context details

KNN search uses this shape:

```sql
SELECT *, knn_dist() AS knn_dist
FROM <table>
WHERE knn(<vector_field>, <k>, '<search query>')
  AND knn_dist < <threshold>
LIMIT <retrieval_limit>
```

Behavior:

- `retrieval_limit` controls the final `LIMIT`.
- The default KNN threshold is currently `0.8`.
- If the conversation route includes an exclusion query, Buddy first finds matching IDs with a stricter KNN threshold and excludes those IDs from the main search.
- The final `sources` include normal result fields and `knn_dist`.
- `FLOAT_VECTOR` columns are removed from `sources` to avoid returning large embedding payloads.
- LLM context is built only from the source results.

`max_document_length` applies per source document:

- `0` disables truncation.
- `100..65536` truncates each document context to that many characters.
- Smaller values reduce prompt size.
- Larger values preserve more source text but increase token usage.

## Response

`CALL CHAT` returns one row with these columns:

| Column | Description |
|---|---|
| `conversation_uuid` | Existing or generated conversation id |
| `user_query` | Original user query |
| `search_query` | Standalone search query used for retrieval |
| `response` | LLM answer |
| `sources` | JSON string containing retrieved source rows |

Example response shape:

```json
{
  "conversation_uuid": "docs-chat-001",
  "user_query": "What is vector search?",
  "search_query": "What is vector search and how does it use embeddings to find similar documents?",
  "response": "Vector search finds similar items by comparing embeddings...",
  "sources": "[{\"id\":1,\"title\":\"Vector Search\",\"content\":\"...\",\"knn_dist\":0.12}]"
}
```

Vector fields are intentionally absent from `sources`.

## Model management

List models:

<!-- example conversational_search_show_chat_models -->

<!-- request SQL -->

```sql
SHOW CHAT MODELS;
```

<!-- end -->

Describe a model:

<!-- example conversational_search_describe_chat_model -->

<!-- request SQL -->

```sql
DESCRIBE CHAT MODEL assistant;
```

<!-- end -->

Describe by UUID:

<!-- example conversational_search_describe_chat_model_uuid -->

<!-- request SQL -->

```sql
DESCRIBE CHAT MODEL '550e8400-e29b-41d4-a716-446655440000';
```

<!-- end -->

Drop a model:

<!-- example conversational_search_drop_chat_model -->

<!-- request SQL -->

```sql
DROP CHAT MODEL assistant;
```

<!-- end -->

Drop safely:

<!-- example conversational_search_drop_chat_model_if_exists -->

<!-- request SQL -->

```sql
DROP CHAT MODEL IF EXISTS assistant;
```

<!-- end -->

## Complete example

<!-- example conversational_search_complete -->

<!-- request SQL -->

```sql
CREATE TABLE docs (
	id BIGINT,
	title TEXT,
	content TEXT,
	embedding FLOAT_VECTOR
		knn_type='hnsw'
		hnsw_similarity='cosine'
		model_name='onnx-models/all-MiniLM-L12-v2-onnx'
		from='title,content'
) TYPE='rt';

INSERT INTO docs(id, title, content) VALUES
	(1, 'Vector search', 'Vector search compares embeddings to find semantically similar documents.'),
	(2, 'Full-text search', 'Full-text search matches terms and phrases in indexed text.');

CREATE CHAT MODEL assistant (
	model='openai:gpt-4o-mini',
	retrieval_limit=5,
	max_document_length=3000
);

CALL CHAT(
	'How is vector search different from full-text search?',
	'docs',
	'assistant',
	'search-demo-1'
);

CALL CHAT(
	'Which one is better for semantic similarity?',
	'docs',
	'assistant',
	'search-demo-1'
);
```

<!-- end -->

## Troubleshooting

`Table '<table>' has no FLOAT_VECTOR field`

The table does not contain a vector field. Add a `FLOAT_VECTOR` field before using it with `CALL CHAT`.

`FLOAT_VECTOR field '<field>' not found in table '<table>'`

The fifth `CALL CHAT` argument names a field that is not a vector field in the target table.

`FLOAT_VECTOR field '<field>' has no auto-embedding source fields`

The vector field does not have `from='...'`. Add source fields so Buddy knows which text fields to use for LLM context.

`Search query must contain at least one term`

The routed standalone search query is empty. This usually means the input query was empty or the routing LLM returned invalid search text.
