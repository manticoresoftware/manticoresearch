# 对话式搜索

对话式搜索在现有向量化表上添加了检索增强聊天功能。它搜索表，从匹配文档中构建上下文，并使用大型语言模型（LLM）根据该上下文和当前对话历史回答问题。

对话式搜索由 `ConversationalSearch` Buddy 插件提供，并通过 SQL 命令进行管理：

- `CREATE CHAT MODEL`
- `SHOW CHAT MODELS`
- `DESCRIBE CHAT MODEL`
- `DROP CHAT MODEL`
- `CALL CHAT`

> 注意：对话式搜索需要安装 [Manticore Buddy](../Installation/Manticore_Buddy.md) 和 `ConversationalSearch` 插件以及 `llm` PHP 扩展。提供商支持取决于安装的 `llm` 扩展。

## 工作原理

在查询时，`CALL CHAT`：

1. 加载聊天模型配置。
2. 加载提供的对话 UUID 的对话历史，或在未提供时创建新的 UUID。
3. 检查目标表并选择一个 `FLOAT_VECTOR` 字段。
4. 将用户消息与 LLM 路由，以决定是否：
   - 运行新搜索
   - 从之前的搜索上下文中回答
   - 在不需要检索时直接回答
5. 使用选定的向量字段运行 KNN 搜索。
6. 从选定的向量字段的 `from='...'` 源字段构建提示上下文。
7. 使用配置的 LLM 生成最终答案。
8. 将用户和助手消息存储在对话历史中。

`CALL CHAT` 中的 `fields` 参数是一个历史名称。它表示用于 KNN 的 `FLOAT_VECTOR` 字段，而不是要返回的字段。搜索结果通过 `SELECT *` 选择，但向量列会从返回的 `sources` 负载中移除。

## 表要求

被搜索的表必须至少包含一个带有自动嵌入源字段的 `FLOAT_VECTOR` 字段：

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

`from='title,content'` 有两个原因很重要：

- Manticore 使用它为向量字段构建嵌入。
- 对话式搜索使用相同的字段来构建发送到 LLM 的文本上下文。

对于多个向量字段，每个向量字段可以使用不同的源字段：

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

如果 `CALL CHAT` 没有指定向量字段，Buddy 会使用 `SHOW CREATE TABLE` 检测到的第一个 `FLOAT_VECTOR` 字段。

有关向量字段和自动嵌入的更多信息，请参见 [K-最近邻向量搜索](../Searching/KNN.md)。

## 创建聊天模型

使用 `CREATE CHAT MODEL` 定义 LLM 和检索设置。

最小模型：

<!-- example conversational_search_create_chat_model_minimal -->

<!-- request SQL -->

```sql
CREATE CHAT MODEL assistant (
	model='openai:gpt-4o-mini'
);
```

<!-- end -->

带有提供商选项和检索设置的模型：

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

常见选项：

| 选项 | 必需 | 描述 |
|---|---:|---|
| `model` | 是 | 以 `provider:model` 格式表示的 LLM 模型 ID |
| `description` | 否 | 存储的描述 |
| `api_key` | 否 | 传递给 `llm` 扩展的提供商 API 密钥 |
| `base_url` | 否 | 提供商或代理的基础 URL |
| `timeout` | 否 | LLM 请求超时 |
| `retrieval_limit` | 否 | 从 KNN 请求的文档数量，范围是 `1` 到 `50` |
| `max_document_length` | 否 | 每个文档的上下文限制；`0` 禁用截断 |

`model` 以 `provider:model` 格式验证，例如：

```sql
model='openai:gpt-4o-mini'
```


提供商支持来自安装的 `llm` [PHP 扩展](https://github.com/manticoresoftware/llm-php-ext)。Buddy 将提供商选项如 `api_key`、`base_url` 和 `timeout` 转发到该扩展。

当提供商密钥在 Buddy 的环境中可用时，`api_key` 是可选的。例如，Docker Compose 服务可以这样暴露提供商密钥：


```yaml
environment:
  - OPENAI_API_KEY=${OPENAI_API_KEY}
  - OPENROUTER_API_KEY=${OPENROUTER_API_KEY}
```

如果 `CREATE CHAT MODEL` 中未设置 `api_key`，`llm` 扩展可以使用匹配的提供商环境变量。仅当您希望模型配置覆盖环境时，才传递 `api_key`。

## 提问

基本调用：

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

继续对话：

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

使用特定的向量字段进行 KNN：

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

当提供第五个参数时，Buddy 会验证该字段是否存在且为 `FLOAT_VECTOR`。如果省略，Buddy 会从 `SHOW CREATE TABLE` 检测第一个 `FLOAT_VECTOR` 字段。

### CALL CHAT 语法

```sql
CALL CHAT(
	'query',
	'table',
	'model_name_or_uuid',
	'conversation_uuid',
	'vector_field'
);
```

参数仅按位置：

| 位置 | 参数 | 必需 | 描述 |
|---:|---|---:|---|
| 1 | `query` | 是 | 用户问题 |
| 2 | `table` | 是 | 要搜索的表 |
| 3 | `model_name_or_uuid` | 是 | 聊天模型名称或 UUID |
| 4 | `conversation_uuid` | 否 | 现有对话 ID，或空字符串 |
| 5 | `fields` / 向量字段 | 否 | 在 `knn(...)` 中使用的 `FLOAT_VECTOR` 字段 |

第五个参数在内部存储为 `fields` 以兼容当前解析器，但它必须是单个向量字段名称。

## 搜索和上下文细节

KNN 搜索使用此形状：

```sql
SELECT *, knn_dist() AS knn_dist
FROM <table>
WHERE knn(<vector_field>, <k>, '<search query>')
  AND knn_dist < <threshold>
LIMIT <retrieval_limit>
```

行为：

- `retrieval_limit` 控制最终的 `LIMIT`。
- 当前默认 KNN 阈值为 `0.8`。
- 如果对话路由包含排除查询，Buddy 首先使用更严格的 KNN 阈值查找匹配的 ID，并从主搜索中排除这些 ID。
- 最终的 `sources` 包括正常结果字段和 `knn_dist`。
- `FLOAT_VECTOR` 列从 `sources` 中移除，以避免返回大型嵌入负载。
- LLM 上下文仅从源结果中构建。

`max_document_length` 每个源文档适用：

- `0` 禁用截断。
- `100..65536` 将每个文档上下文截断为这么多字符。
- 较小的值减少提示大小。
- 较大的值保留更多源文本但增加标记使用量。

## 响应

`CALL CHAT` 返回一行包含以下列：

| 列 | 描述 |
|---|---|
| `conversation_uuid` | 现有或生成的对话ID |
| `user_query` | 原始用户查询 |
| `search_query` | 用于检索的独立搜索查询 |
| `response` | LLM 答案 |
| `sources` | 包含检索源行的 JSON 字符串 |

示例响应结构：

```json
{
  "conversation_uuid": "docs-chat-001",
  "user_query": "What is vector search?",
  "search_query": "What is vector search and how does it use embeddings to find similar documents?",
  "response": "Vector search finds similar items by comparing embeddings...",
  "sources": "[{\"id\":1,\"title\":\"Vector Search\",\"content\":\"...\",\"knn_dist\":0.12}]"
}
```

`sources` 中故意省略了向量字段。

## 模型管理

列出模型：

<!-- example conversational_search_show_chat_models -->

<!-- request SQL -->

```sql
SHOW CHAT MODELS;
```

<!-- end -->

描述模型：

<!-- example conversational_search_describe_chat_model -->

<!-- request SQL -->

```sql
DESCRIBE CHAT MODEL assistant;
```

<!-- end -->

通过 UUID 描述：

<!-- example conversational_search_describe_chat_model_uuid -->

<!-- request SQL -->

```sql
DESCRIBE CHAT MODEL '550e8400-e29b-41d4-a716-446655440000';
```

<!-- end -->

删除模型：

<!-- example conversational_search_drop_chat_model -->

<!-- request SQL -->

```sql
DROP CHAT MODEL assistant;
```

<!-- end -->

安全删除：

<!-- example conversational_search_drop_chat_model_if_exists -->

<!-- request SQL -->

```sql
DROP CHAT MODEL IF EXISTS assistant;
```

<!-- end -->

## 完整示例

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

## 故障排除

`表 '<table>' 没有 FLOAT_VECTOR 字段`

该表不包含向量字段。在使用 `CALL CHAT` 之前，请添加一个 `FLOAT_VECTOR` 字段。

`FLOAT_VECTOR 字段 '<field>' 在表 '<table>' 中未找到`

第五个 `CALL CHAT` 参数命名了一个在目标表中不是向量字段的字段。

`FLOAT_VECTOR 字段 '<field>' 没有自动嵌入的源字段`

该向量字段没有 `from='...'`。添加源字段，以便 Buddy 知道使用哪些文本字段作为 LLM 上下文。

`搜索查询必须包含至少一个术语`

路由的独立搜索查询为空。这通常意味着输入查询为空或路由 LLM 返回了无效的搜索文本。
