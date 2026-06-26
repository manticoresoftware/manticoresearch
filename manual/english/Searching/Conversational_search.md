# Conversational search

Conversational search lets Manticore Buddy answer questions over an existing vectorized table. Buddy retrieves the most relevant rows with KNN search, turns those rows into context, and sends the context plus the conversation history to an LLM.

It is managed from SQL with:

* `CREATE CHAT MODEL`
* `SHOW CHAT MODELS`
* `DESCRIBE CHAT MODEL`
* `DROP CHAT MODEL`
* `CALL CHAT`

Conversation calls are also available through the HTTP JSON `/search` endpoint. Chat model management remains SQL-only.

## Before you start

You need a vectorized table and an LLM provider. The table requirements are covered below. Provider credentials can be set in `CREATE CHAT MODEL` with `api_key`, or supplied through the matching environment variable, such as `OPENAI_API_KEY`.

## How it works

When `CALL CHAT` runs, Buddy builds a retrieval-augmented answer in this order:

1. Buddy loads the chat model.
2. Buddy loads the conversation history for the supplied conversation UUID. If no UUID is supplied, it creates one.
3. Buddy inspects the target table and chooses a `FLOAT_VECTOR` field.
4. The LLM decides how to handle the message: search again, answer from the previous search context, or answer without retrieval.
5. Buddy runs KNN search with the selected vector field when retrieval is needed.
6. Buddy builds the LLM context from the vector field's `from='...'` source fields.
7. The configured LLM generates the answer with inline references in `[ref:<id>]` format when it uses retrieved sources.
8. Buddy saves the user message and the assistant reply in the conversation history.

The fifth argument of `CALL CHAT` is called `fields` internally, but for conversational search it means the vector field used by `knn(...)`. It is not a list of fields to return. Buddy selects rows with `SELECT *`, then removes vector columns from the `sources` payload so the response does not include large embedding values.

## Table requirements

The table must have at least one `FLOAT_VECTOR` field configured for [auto embeddings](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29). The vector field must include `from='...'`, because Buddy uses those source fields as LLM context.

The examples below use `onnx-models/all-MiniLM-L12-v2-onnx`, which runs through the recommended ONNX path and does not require an embedding API key.

<!-- example conversational_search_create_table -->

<!-- intro -->
##### SQL:

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
```

<!-- end -->

If `CALL CHAT` does not specify a vector field, Buddy uses the first `FLOAT_VECTOR` field found in the table definition.

## Creating a chat model

Use `CREATE CHAT MODEL` to store the LLM provider, model id, and retrieval settings.

<!-- example conversational_search_create_model -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE CHAT MODEL assistant (
    model='openai:gpt-4o-mini'
);
```


<!-- intro -->
##### JSON:

<!-- request JSON -->

```bash
curl -s -X POST 'http://localhost:9308/sql?mode=raw' \
  -H 'Content-Type: text/plain' \
  -d "CREATE CHAT MODEL assistant (model='openai:gpt-4o-mini')"
```

<!-- end -->

You can also set provider options and retrieval limits:

<!-- example conversational_search_create_model_extended -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE CHAT MODEL support_assistant (
    model='openai:gpt-4o-mini',
    api_key='your-provider-api-key',
    base_url='http://host.docker.internal:8787/v1',
    timeout=60,
    retrieval_limit=5,
    max_document_length=3000
);
```


<!-- intro -->
##### JSON:

<!-- request JSON -->

```bash
curl -s -X POST 'http://localhost:9308/sql?mode=raw' \
  -H 'Content-Type: text/plain' \
  -d "CREATE CHAT MODEL support_assistant (model='openai:gpt-4o-mini', api_key='your-provider-api-key', base_url='http://host.docker.internal:8787/v1', timeout=60, retrieval_limit=5, max_document_length=3000)"
```

<!-- end -->

Common options:

| Option | Required | Description |
|---|---:|---|
| `model` | Yes | LLM model id in `provider:model` format. |
| `description` | No | Stored description. |
| `api_key` | No | Provider API key passed to the `llm` extension. |
| `base_url` | No | Provider or proxy base URL. |
| `timeout` | No | LLM request timeout, `1..65536`. |
| `retrieval_limit` | No | Number of documents requested from KNN, `1..50`; default is `5`. |
| `max_document_length` | No | Per-document context limit. `0` disables truncation; `100..65536` truncates; default is `2000`. |

Chat model names may contain only letters, numbers, and underscores.

The `model` option must use `provider:model` format:

```sql
model='openai:gpt-4o-mini'
```

Provider `api_key` is optional if the provider key is already available in Buddy's environment. For example, a Docker Compose service can pass provider keys like this:

```yaml
environment:
  - OPENAI_API_KEY=${OPENAI_API_KEY}
  - OPENROUTER_API_KEY=${OPENROUTER_API_KEY}
```

If `api_key` is not set in `CREATE CHAT MODEL`, the `llm` extension can use the matching provider environment variable. Set `api_key` in the chat model only when you need this model to use a different key.

## CALL CHAT syntax

```sql
CALL CHAT(
    'query',
    'table',
    'model_name',
    'conversation_uuid',
    'vector_field'
);
```

Arguments are positional only:

| Position | Argument | Required | Description |
|---:|---|---:|---|
| 1 | `query` | Yes | User question. |
| 2 | `table` | Yes | Table to search. |
| 3 | `model_name` | Yes | Chat model name. |
| 4 | `conversation_uuid` | No | Existing conversation id, or an empty string. |
| 5 | `fields` / vector field | No | `FLOAT_VECTOR` field used in `knn(...)`. |

The table argument must be a plain table identifier, optionally qualified as `database.table`. The vector field argument must be a plain field identifier.

## Asking questions

Use `CALL CHAT` with a query, a table, and a chat model.

<!-- example conversational_search_call_chat -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL CHAT(
    'What is vector search?',
    'docs',
    'assistant'
);
```


<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```bash
curl -s -X POST http://localhost:9308/search \
  -H 'Content-Type: application/json' \
  -d '{
    "chat": {
      "query": "What is vector search?",
      "table": "docs",
      "model_name": "assistant"
    }
  }'
```

<!-- end -->

To continue a conversation, pass the same conversation UUID:

<!-- example conversational_search_continue_chat -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL CHAT(
    'Can you explain it with an example?',
    'docs',
    'assistant',
    'docs-chat-001'
);
```


<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```bash
curl -s -X POST http://localhost:9308/search \
  -H 'Content-Type: application/json' \
  -d '{
    "chat": {
      "query": "Can you explain it with an example?",
      "table": "docs",
      "model_name": "assistant",
      "conversation_uuid": "docs-chat-001"
    }
  }'
```

<!-- end -->

To search a specific vector field, pass it as the fifth argument:

<!-- example conversational_search_vector_field -->

<!-- intro -->
##### SQL:

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


<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```bash
curl -s -X POST http://localhost:9308/search \
  -H 'Content-Type: application/json' \
  -d '{
    "chat": {
      "query": "Find documents where the title is about vector search",
      "table": "docs",
      "model_name": "assistant",
      "conversation_uuid": "",
      "vector_field": "title_embedding"
    }
  }'
```

<!-- end -->

When the fifth argument is present, Buddy checks that the field exists and is a `FLOAT_VECTOR`. If the argument is omitted, Buddy detects the first `FLOAT_VECTOR` field from `SHOW CREATE TABLE`.


## HTTP JSON syntax

Conversation calls are also available over HTTP JSON through the standard `/search` endpoint.

Ask a question:

<!-- example conversational_search_http_chat -->

<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```bash
curl -s -X POST http://localhost:9308/search \
  -H 'Content-Type: application/json' \
  -d '{
    "chat": {
      "query": "What is vector search?",
      "table": "docs",
      "model_name": "assistant",
      "conversation_uuid": "docs-chat-001",
      "vector_field": "embedding"
    }
  }'
```

<!-- end -->

Required fields in the `chat` object are `query`, `table`, and `model_name`. Optional fields are `conversation_uuid` and `vector_field`. `vector_field` is the HTTP JSON name for the fifth SQL `CALL CHAT` argument. The legacy JSON field name `fields` is accepted as an alias, but a request must not include both `vector_field` and `fields`.

HTTP JSON conversation responses contain the same logical columns as `CALL CHAT`: `conversation_uuid`, `user_query`, `search_query`, `response`, `response_with_refs`, and `sources`. `sources` is returned as a JSON string containing retrieved source rows.


## Search and context details

When Buddy needs retrieval, it runs KNN search on the selected vector field and returns up to `retrieval_limit` rows. The default distance threshold is `0.8`.

Buddy uses the retrieved rows as LLM context. The same rows are returned in `sources`, with `knn_dist` included and `FLOAT_VECTOR` columns removed.

`max_document_length` limits how much text from each source row can be sent to the LLM. Use `0` to disable truncation; otherwise use a value from `100` to `65536`.

## Response

`CALL CHAT` returns one row:

| Column | Description |
|---|---|
| `conversation_uuid` | Existing or generated conversation id. |
| `user_query` | Original user query. |
| `search_query` | Standalone search query used for retrieval. |
| `response` | LLM answer with inline references removed. |
| `response_with_refs` | LLM answer as generated, including inline `[ref:<id>]` references to rows in `sources`. |
| `sources` | JSON string containing retrieved source rows. |

Example response shape:

```json
{
  "conversation_uuid": "docs-chat-001",
  "user_query": "What is vector search?",
  "search_query": "vector search, embeddings, similarity search",
  "response": "Vector search finds similar items by comparing embeddings...",
  "response_with_refs": "Vector search finds similar items by comparing embeddings... [ref:1]",
  "sources": "[{\"id\":1,\"title\":\"Vector Search\",\"content\":\"...\",\"knn_dist\":0.12}]"
}
```

Vector fields are not included in `sources`. Inline references use the source row `id`, not the source title or text. Use `response` for a plain answer and `response_with_refs` when you want to display citations.

## Managing chat models

List models:

<!-- example conversational_search_show_models -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW CHAT MODELS;
```


<!-- intro -->
##### JSON:

<!-- request JSON -->

```bash
curl -s -X POST 'http://localhost:9308/sql?mode=raw' \
  -H 'Content-Type: text/plain' \
  -d "SHOW CHAT MODELS"
```

<!-- end -->

Describe a model:

<!-- example conversational_search_describe_model -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DESCRIBE CHAT MODEL assistant;
```


<!-- intro -->
##### JSON:

<!-- request JSON -->

```bash
curl -s -X POST 'http://localhost:9308/sql?mode=raw' \
  -H 'Content-Type: text/plain' \
  -d "DESCRIBE CHAT MODEL assistant"
```

<!-- end -->

Drop a model:

<!-- example conversational_search_drop_model -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DROP CHAT MODEL assistant;
```


<!-- intro -->
##### JSON:

<!-- request JSON -->

```bash
curl -s -X POST 'http://localhost:9308/sql?mode=raw' \
  -H 'Content-Type: text/plain' \
  -d "DROP CHAT MODEL assistant"
```

<!-- end -->

Drop safely:

<!-- example conversational_search_drop_model_if_exists -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DROP CHAT MODEL IF EXISTS assistant;
```


<!-- intro -->
##### JSON:

<!-- request JSON -->

```bash
curl -s -X POST 'http://localhost:9308/sql?mode=raw' \
  -H 'Content-Type: text/plain' \
  -d "DROP CHAT MODEL IF EXISTS assistant"
```

<!-- end -->

`SHOW CHAT MODELS` returns `name`, `model`, and `created_at`. `DESCRIBE CHAT MODEL` returns `property` and `value`; stored API keys are shown as `HIDDEN`.

Dropping a chat model also drops that model's conversation history table. Conversation history is stored per model and written with a 30-day TTL.
