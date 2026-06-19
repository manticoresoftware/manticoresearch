# 会话式搜索

会话式搜索让 Manticore Buddy 能基于已有的向量化表来回答问题。Buddy 先用 KNN 搜索检索最相关的行，再把这些行转换为上下文，最后把上下文和对话历史一起发送给 LLM。

它通过 SQL 进行管理，使用：

* `CREATE CHAT MODEL`
* `SHOW CHAT MODELS`
* `DESCRIBE CHAT MODEL`
* `DROP CHAT MODEL`
* `CALL CHAT`

## 开始之前

你需要一张向量化表和一个 LLM 提供方。下面会说明表的要求。提供方凭据可以在 `CREATE CHAT MODEL` 中通过 `api_key` 设置，也可以通过匹配的环境变量提供，例如 `OPENAI_API_KEY`。

## 工作原理

当 `CALL CHAT` 运行时，Buddy 会按以下顺序构建检索增强答案：

1. Buddy 加载聊天模型。
2. Buddy 加载所提供 conversation UUID 对应的对话历史。如果没有提供 UUID，它会创建一个。
3. Buddy 检查目标表并选择一个 `FLOAT_VECTOR` 字段。
4. LLM 决定如何处理消息：重新搜索、基于之前的搜索上下文回答，或不依赖检索直接回答。
5. 在需要检索时，Buddy 使用选定的向量字段执行 KNN 搜索。
6. Buddy 根据向量字段的 `from='...'` 源字段构建 LLM 上下文。
7. 配置的 LLM 生成答案。
8. Buddy 将用户消息和助手回复保存到对话历史中。

`CALL CHAT` 的第五个参数在内部称为 `fields`，但在会话式搜索里它指的是 `knn(...)` 使用的向量字段，而不是要返回的字段列表。Buddy 会用 `SELECT *` 选出行，然后从 `sources` 载荷中移除向量列，因此响应不会包含大量 embedding 值。

## 表要求

表中必须至少有一个为 [自动嵌入](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29) 配置的 `FLOAT_VECTOR` 字段。向量字段必须包含 `from='...'`，因为 Buddy 会使用这些源字段作为 LLM 上下文。

下面的示例使用 `onnx-models/all-MiniLM-L12-v2-onnx`，它通过推荐的 ONNX 路径运行，不需要 embedding API key。

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

如果 `CALL CHAT` 没有指定向量字段，Buddy 会使用表定义中找到的第一个 `FLOAT_VECTOR` 字段。

## 创建聊天模型

使用 `CREATE CHAT MODEL` 来保存 LLM 提供方、模型 ID 和检索设置。

<!-- example conversational_search_create_model -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE CHAT MODEL assistant (
    model='openai:gpt-4o-mini'
);
```

<!-- end -->

你也可以设置提供方选项和检索限制：

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
<!-- end -->

常见选项：

| 选项 | 必需 | 说明 |
|---|---:|---|
| `model` | 是 | `provider:model` 格式的 LLM 模型 ID。 |
| `description` | 否 | 保存的描述。 |
| `api_key` | 否 | 传递给 `llm` 扩展的提供方 API key。 |
| `base_url` | 否 | 提供方或代理的 base URL。 |
| `timeout` | 否 | LLM 请求超时，`1..65536`。 |
| `retrieval_limit` | 否 | 从 KNN 请求的文档数量，`1..50`；默认值是 `5`。 |
| `max_document_length` | 否 | 单篇文档的上下文长度限制。`0` 表示不截断；`100..65536` 会截断；默认值是 `2000`。 |

聊天模型名称只能包含字母、数字和下划线。

`model` 选项必须使用 `provider:model` 格式：

```sql
model='openai:gpt-4o-mini'
```

如果提供方 key 已经可用在 Buddy 的环境中，则提供方的 `api_key` 是可选的。例如，Docker Compose 服务可以这样传递提供方 key：

```yaml
environment:
  - OPENAI_API_KEY=${OPENAI_API_KEY}
  - OPENROUTER_API_KEY=${OPENROUTER_API_KEY}
```

如果没有在 `CREATE CHAT MODEL` 中设置 `api_key`，`llm` 扩展可以使用匹配的提供方环境变量。只有当你需要让这个模型使用不同的 key 时，才在聊天模型中设置 `api_key`。

## `CALL CHAT` 语法

```sql
CALL CHAT(
    'query',
    'table',
    'model_name',
    'conversation_uuid',
    'vector_field'
);
```

参数只能按位置传递：

| 位置 | 参数 | 必需 | 说明 |
|---:|---|---:|---|
| 1 | `query` | 是 | 用户问题。 |
| 2 | `table` | 是 | 要搜索的表。 |
| 3 | `model_name` | 是 | 聊天模型名称。 |
| 4 | `conversation_uuid` | 否 | 现有对话 ID，或空字符串。 |
| 5 | `fields` / 向量字段 | 否 | `knn(...)` 中使用的 `FLOAT_VECTOR` 字段。 |

表参数必须是普通表标识符，也可以可选地写成 `database.table`。向量字段参数必须是普通字段标识符。

## 提问

使用 `CALL CHAT` 时，传入查询、表和聊天模型。

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

<!-- end -->

要继续同一段对话，请传入相同的 conversation UUID：

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

<!-- end -->

要搜索特定的向量字段，请将其作为第五个参数传入：

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

<!-- end -->

当提供第五个参数时，Buddy 会检查该字段是否存在且类型为 `FLOAT_VECTOR`。如果省略该参数，Buddy 会从 `SHOW CREATE TABLE` 中检测第一个 `FLOAT_VECTOR` 字段。


## 搜索和上下文细节

当 Buddy 需要检索时，它会在选定的向量字段上执行 KNN 搜索，并返回最多 `retrieval_limit` 行。默认距离阈值为 `0.8`。

Buddy 将检索到的行作为 LLM 上下文。同样的行会在 `sources` 中返回，其中包含 `knn_dist`，并移除了 `FLOAT_VECTOR` 列。

`max_document_length` 限制了每个源行可以发送给 LLM 的文本量。使用 `0` 可禁用截断；否则请使用 `100` 到 `65536` 之间的值。

## 响应

`CALL CHAT` 返回一行：

| 列 | 说明 |
|---|---|
| `conversation_uuid` | 现有或生成的对话 ID。 |
| `user_query` | 原始用户查询。 |
| `search_query` | 用于检索的独立搜索查询。 |
| `response` | LLM 答案。 |
| `sources` | 包含检索到的源行的 JSON 字符串。 |

响应结构示例：

```json
{
  "conversation_uuid": "docs-chat-001",
  "user_query": "What is vector search?",
  "search_query": "vector search, embeddings, similarity search",
  "response": "Vector search finds similar items by comparing embeddings...",
  "sources": "[{\"id\":1,\"title\":\"Vector Search\",\"content\":\"...\",\"knn_dist\":0.12}]"
}
```

向量字段不会包含在 `sources` 中。

## 管理聊天模型

列出模型：

<!-- example conversational_search_show_models -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW CHAT MODELS;
```

<!-- end -->

查看模型详情：

<!-- example conversational_search_describe_model -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DESCRIBE CHAT MODEL assistant;
```

<!-- end -->

删除模型：

<!-- example conversational_search_drop_model -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DROP CHAT MODEL assistant;
```

<!-- end -->

安全删除：

<!-- example conversational_search_drop_model_if_exists -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DROP CHAT MODEL IF EXISTS assistant;
```

<!-- end -->

`SHOW CHAT MODELS` 会返回 `name`、`model` 和 `created_at`。`DESCRIBE CHAT MODEL` 会返回 `property` 和 `value`；已保存的 API key 会显示为 `HIDDEN`。

删除聊天模型时也会删除该模型的对话历史表。对话历史按模型单独存储，并以 30 天 TTL 写入。
