# K 近邻向量搜索

Manticore Search 支持为每个文档添加由机器学习模型生成的嵌入，然后对其执行最近邻搜索。这让你可以构建相似性搜索、推荐、语义搜索，以及基于 NLP 算法的相关性排序等功能，还包括图像、视频和声音搜索。

要将 KNN 向量搜索与全文搜索结合以获得更好的相关性，请参阅 [混合搜索](../Searching/Hybrid_search.md)。

## 什么是嵌入？

嵌入是一种将文本、图像或声音等数据表示为高维空间中向量的方法。这些向量的设计目标是让它们之间的距离能够反映所代表数据的相似度。这个过程通常会使用词嵌入算法（例如 Word2Vec、BERT）处理文本，或使用神经网络处理图像。向量空间具有高维特性，每个向量包含许多分量，因此能够表示对象之间复杂而细微的关系。它们的相似度通过这些向量之间的距离来衡量，通常使用欧氏距离或余弦相似度等方法。

Manticore Search 使用 HNSW 库支持 k 近邻（KNN）向量搜索。该功能属于 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)。

<!-- example KNN -->

### 为 KNN 搜索配置表

要运行 KNN 搜索，首先必须配置你的表。浮点向量和 KNN 搜索仅支持实时表（不支持普通表）。表中需要至少有一个 [float_vector](../Creating_a_table/Data_types.md#Float-vector) 属性，它充当数据向量。你需要指定以下属性：
* `knn_type`：必填设置；目前仅支持 `hnsw`。
* `knn_dims`：必填设置，用于指定被索引向量的维度。
* `hnsw_similarity`：必填设置，用于指定 HNSW 索引使用的距离函数。可接受的值有：
  - `L2` - 平方 L2
  - `IP` - 内积
  - `COSINE` - 余弦相似度

  **注意：** 当使用 `COSINE` 相似度时，向量会在插入时自动归一化。这意味着存储的向量值可能与原始输入值不同，因为它们会被转换为单位向量（数学长度/模长为 1.0 的向量），以便高效计算余弦相似度。该归一化过程会保留向量方向，同时标准化其长度。
* `hnsw_m`：可选设置，定义图中最大出边数。默认值为 16。
* `hnsw_ef_construction`：可选设置，定义构建时间与精度之间的权衡。默认值为 200。

> 注意：在多核主机上，RT chunk 保存、`OPTIMIZE TABLE` / 自动优化 chunk 合并，以及 `ALTER TABLE ... ADD/DROP/REBUILD` KNN 重建期间的 HNSW 图构建默认会并行运行；工作线程数由 searchd 设置 [`knn_parallel_build`](../Server_settings/Searchd.md#knn_parallel_build) 控制（将其设为 `1` 可强制走串行路径）。这只影响构建阶段性能。由于并行 HNSW 构建可能以不同顺序插入向量，生成的图可能不会与串行构建在位级别完全一致。

<!-- intro -->
##### SQL

<!-- request SQL -->
```sql
create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' );
```

<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.01 sec)
```

<!-- request JSON -->
```json
POST /sql?mode=raw -d "create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' )"
```

<!-- response JSON -->

```json
[
  {
    "total": 0,
    "error": "",
    "warning": ""
  }
]
```

<!-- intro -->
##### 普通模式（使用配置文件）- 手动向量：

<!-- request Config -->
```ini
table test_vec {
    type = rt
	...
    rt_attr_float_vector = image_vector
    knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":4,"hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200}]}
}
```

**注意：** 关于普通模式下的自动嵌入，请参见下面的示例，其中展示了如何在 `knn` 配置中使用 `model_name` 和 `from` 参数。

<!-- end -->

<!-- example knn_insert -->

### 插入向量数据

#### 自动嵌入（推荐）

处理向量数据最简单的方法是使用**自动嵌入**。使用此功能时，你创建一个带有 `MODEL_NAME` 和 `FROM` 参数的表，然后只需插入文本数据即可，Manticore 会自动为你生成嵌入。

##### 创建带自动嵌入的表

创建用于自动嵌入的表时，请指定：
- `MODEL_NAME`：要使用的嵌入模型
- `FROM`：用于生成嵌入的字段（留空表示所有文本/字符串字段）
- `API_KEY`：远程模型（OpenAI、Voyage、Jina）必需。API 密钥会在建表期间通过发起真实 API 请求进行验证。
- `API_URL`：可选。自定义 API 端点 URL。如果未指定，则使用默认提供方端点（例如 OpenAI 使用 `https://api.openai.com/v1/embeddings`）。
- `API_TIMEOUT`：可选。API 请求的 HTTP 超时时间，单位为秒。默认值为 10 秒。设为 `'0'` 可使用默认超时。此设置同时适用于建表期间的验证请求和 INSERT 操作期间的嵌入生成。

对于远程模型，`MODEL_NAME` 可以写成两种形式：
- 传统的带提供方前缀形式：`openai/text-embedding-ada-002`、`voyage/voyage-3.5-lite`、`jina/jina-embeddings-v4`
- 用于自定义端点的显式提供方标识形式：`openai:text-embedding-ada-002`、`openai:openai/text-embedding-ada-002`、`voyage:custom-model`、`jina:custom-model`

当你把 `provider:model` 形式与 `API_URL` 一起使用时，冒号前的部分只用于选择请求格式。冒号后的部分会原样发送到远程端点。这对 OpenAI 兼容网关很有用，例如 OpenRouter 或 LiteLLM。

**支持的嵌入模型：**

| 模型类型 | 示例 | 需要 API 密钥 | 说明 |
|------------|---------|-----------------|-------|
| **ONNX（推荐）** | `Xenova/all-MiniLM-L6-v2` | 否 | 来自任何提供 `.onnx` 文件的 Hugging Face 仓库的本地模型。运行在 Manticore 高速的 ONNX Runtime 后端上。浏览列表：[feature-extraction ONNX 模型](https://huggingface.co/Xenova/models?pipeline_tag=feature-extraction&search=minilm)。 |
| **Sentence Transformers** | `sentence-transformers/all-MiniLM-L6-v2` | 否 | 基于 BERT 的本地模型，会自动下载。仍然支持 - 在可用时优先使用上面的 ONNX。 |
| **Qwen** | `Qwen/Qwen3-Embedding-0.6B` | 否 | 本地 Qwen 系列模型 |
| **Llama** | `TinyLlama/TinyLlama-1.1B-Chat-v1.0` | 否 | 本地 Llama 系列模型 |
| **Mistral** | `Locutusque/TinyMistral-248M-v2` | 否 | 本地 Mistral 系列模型 |
| **Gemma** | `h2oai/embeddinggemma-300m` | 否 | 本地 Gemma 系列模型 |
| **OpenAI** | `openai/text-embedding-ada-002` 或 `openai:text-embedding-ada-002` | 是 | `API_KEY='***'` |
| **Voyage** | `voyage/voyage-3.5-lite` 或 `voyage:voyage-3.5-lite` | 是 | `API_KEY='***'` |
| **Jina** | `jina/jina-embeddings-v4` 或 `jina:jina-embeddings-v4` | 是 | `API_KEY='***'` |

**本地模型格式要求：**
- 必须以 `safetensors` 格式保存（仅单文件）
- 支持的系列：Qwen、Llama、Mistral、Gemma
- 已测试模型：`TinyLlama/TinyLlama-1.1B-Chat-v1.0`、`Locutusque/TinyMistral-248M-v2`、`Qwen/Qwen3-Embedding-0.6B`、`h2oai/embeddinggemma-300m`
- 其他 `safetensors` 模型也可能可用，但不作保证

关于配置 `float_vector` 属性的更多信息，请参见[这里](../Creating_a_table/Data_types.md#Float-vector)。

<!-- intro -->
##### SQL：

<!-- request SQL -->

使用本地 [ONNX 模型](https://huggingface.co/Xenova/models?pipeline_tag=feature-extraction&search=minilm) - 推荐（无需 API key）
```sql
CREATE TABLE products (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='Xenova/all-MiniLM-L6-v2' FROM='title'
);
```

使用 sentence-transformers（无需 API key；走 Candle 路径 - 在可用时优先使用上面的 ONNX）
```sql
CREATE TABLE products_st (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title'
);
```

使用 Qwen 本地嵌入（无需 API 密钥）
```sql
CREATE TABLE products_qwen (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='Qwen/Qwen3-Embedding-0.6B' FROM='title' CACHE_PATH='/opt/homebrew/var/manticore/.cache/manticore'
);
```

使用 OpenAI（需要 API_KEY 参数）
```sql
CREATE TABLE products_openai (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description' API_KEY='...'
);
```
使用 OpenAI 搭配自定义 API URL 和超时设置（可选）
```sql
CREATE TABLE products_openai_custom (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai:text-embedding-ada-002' FROM='title,description'
    API_KEY='***' API_URL='https://custom-api.example.com/v1/embeddings' API_TIMEOUT='30'
);
```

使用期望提供方限定模型 ID 的 OpenAI 兼容网关
```sql
CREATE TABLE products_openrouter (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai:openai/text-embedding-ada-002' FROM='title,description'
    API_KEY='***' API_URL='https://openrouter.ai/api/v1/embeddings' API_TIMEOUT='30'
);
```

使用所有文本字段生成嵌入（FROM 为空）
```sql
CREATE TABLE products_all (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='Xenova/all-MiniLM-L6-v2' FROM=''
);
```

<!-- intro -->
##### 普通模式（使用配置文件）：

<!-- request Config -->
```ini
table products {
    type = rt
    path = /path/to/products
    rt_field = title
    rt_field = description
    rt_attr_float_vector = embedding_vector
    knn = {"attrs":[{"name":"embedding_vector","type":"hnsw","hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200,"model_name":"Xenova/all-MiniLM-L6-v2","from":"title"}]}
}
```

在普通模式下使用带 API 密钥的 OpenAI：
```ini
table products_openai {
    type = rt
    path = /path/to/products_openai
    rt_field = title
    rt_field = description
    rt_attr_float_vector = embedding_vector
    knn = {"attrs":[{"name":"embedding_vector","type":"hnsw","hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200,"model_name":"openai/text-embedding-ada-002","from":"title,description","api_key":"your-api-key-here"}]}
}
```

使用所有文本字段（空 FROM）：
```ini
table products_all {
    type = rt
    path = /path/to/products_all
    rt_field = title
    rt_field = description
    rt_attr_float_vector = embedding_vector
    knn = {"attrs":[{"name":"embedding_vector","type":"hnsw","hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200,"model_name":"Xenova/all-MiniLM-L6-v2","from":""}]}
}
```

**普通模式重要说明：**
- 使用 `model_name` 时，**不能**指定 `dims` - 模型会自动决定向量维度。`dims` 和 `model_name` 参数互斥。
- 当**不**使用 `model_name`（手动插入向量）时，**必须**指定 `dims` 来说明向量维度。
- `from` 参数指定用于生成嵌入的字段（以逗号分隔的列表，或留空字符串表示所有文本/字符串字段）。使用 `model_name` 时必须提供此参数。
- 对于基于 API 的模型（OpenAI、Voyage、Jina），请在 knn 配置中包含 `api_key` 参数

<!-- end -->

##### 使用自动嵌入插入数据

<!--
以下示例的数据：

DROP TABLE IF EXISTS products;
CREATE TABLE products(title text, embedding_vector float_vector knn_type='hnsw' hnsw_similarity='l2' model_name='Xenova/all-MiniLM-L6-v2' from='title');
DROP TABLE IF EXISTS products_openai;
CREATE TABLE products_openai(title text, description text, embedding_vector float_vector knn_type='hnsw' hnsw_similarity='l2' model_name='Xenova/all-MiniLM-L6-v2' from='title,description');
-->

<!-- example inserting_embeddings -->

使用自动嵌入时，你可以：

- 省略向量字段，让 Manticore 根据 `FROM` 中列出的字段生成嵌入
- 为某一行显式提供你自己的向量
- 提供 `()` 以跳过生成并存储全零向量

如果你之后运行 `ALTER TABLE ... REBUILD EMBEDDINGS`，当前包含来自 `()` 的零向量的行也会被重新生成。

<!-- intro -->
##### SQL：

<!-- request SQL -->

仅插入文本数据 - 嵌入会自动生成
```sql
INSERT INTO products (title) VALUES
('machine learning artificial intelligence'),
('banana fruit sweet yellow');
```

插入用户提供的向量
```sql
INSERT INTO products (title, embedding_vector) VALUES
('machine learning artificial intelligence', (0.653448,0.192478,0.017971,0.339821));
```

插入多个字段 - 如果 FROM='title,description'，两者都会用于生成嵌入
```sql
INSERT INTO products_openai (title, description) VALUES
('smartphone', 'latest mobile device with advanced features'),
('laptop', 'portable computer for work and gaming');
```

插入空向量（不自动生成；存储零向量）
```sql
INSERT INTO products (title, embedding_vector) VALUES
('no embedding item', ());
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

仅插入文本数据 - 嵌入会自动生成
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title) VALUES ('machine learning artificial intelligence'),('banana fruit sweet yellow')"
```

插入多个字段 - 如果 `FROM='title,description'`，两者都会用于嵌入
```JSON
POST /sql?mode=raw -d "INSERT INTO products_openai (title, description) VALUES ('smartphone', 'latest mobile device with advanced features'), ('laptop', 'portable computer for work and gaming')"
```

插入空向量（文档被排除在向量搜索之外）
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title, embedding_vector) VALUES ('no embedding item', ())"
```

<!-- end -->

##### 使用自动嵌入进行搜索

<!-- example embeddings_search -->
搜索方式相同 - 提供查询文本，Manticore 会生成嵌入并找到相似文档：

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
SELECT id, knn_dist() FROM products WHERE knn(embedding_vector, 'machine learning');
```

<!-- response SQL -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    1 | 0.12345678 |
|    2 | 0.87654321 |
+------+------------+
2 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

使用文本查询和自动嵌入
```json
POST /search
{
    "table": "products",
    "knn": {
        "field": "embedding_vector",
        "query": "machine learning"
    }
}
```

直接使用向量查询
```json
POST /search
{
    "table": "products",
    "knn": {
        "field": "embedding_vector",
        "query": [0.1, 0.2, 0.3, 0.4]
    }
}
```

<!-- response JSON -->

```json
{
    "took": 0,
    "timed_out": false,
    "hits": {
        "total": 2,
        "total_relation": "eq",
        "hits": [
            {
                "_id": 1,
                "_score": 1,
                "_knn_dist": 0.12345678,
                "_source": {
                    "title": "machine learning artificial intelligence"
                }
            },
            {
                "_id": 2,
                "_score": 1,
                "_knn_dist": 0.87654321,
                "_source": {
                    "title": "banana fruit sweet yellow"
                }
            }
        ]
    }
}
```

<!-- end -->

#### 手动插入向量

<!-- example manual_vector -->
或者，你也可以手动插入预先计算好的向量数据，确保它与创建表时指定的维度一致。你也可以插入空向量；这意味着该文档将被排除在向量搜索结果之外。

**重要：** 当使用 `hnsw_similarity='cosine'` 时，向量会在插入时自动归一化为单位向量（数学长度/模长为 1.0 的向量）。这种归一化会保留向量方向，同时标准化其长度，这对于高效计算余弦相似度是必需的。这意味着存储值会与你的原始输入值不同。

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
insert into test values ( 1, 'yellow bag', (0.653448,0.192478,0.017971,0.339821) ), ( 2, 'white bag', (-0.148894,0.748278,0.091892,-0.095406) );
```
<!-- response SQL -->

```sql
Query OK, 2 rows affected (0.00 sec)
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

```json
POST /insert
{
	"table":"test_vec",
	"id":1,
	"doc": 	{ "title" : "yellow bag", "image_vector" : [0.653448,0.192478,0.017971,0.339821] }
}

POST /insert
{
	"table":"test_vec",
	"id":2,
	"doc": 	{ "title" : "white bag", "image_vector" : [-0.148894,0.748278,0.091892,-0.095406] }
}
```

<!-- response JSON -->

```json
{
	"table":"test",
	"id":1,
	"created":true,
	"result":"created",
	"status":201
}

{
	"table":"test",
	"id":2,
	"created":true,
	"result":"created",
	"status":201
}
```

<!-- end -->

<!-- example knn_search -->

### KNN 向量搜索

现在，你可以在 SQL 或 JSON 格式中使用 `knn` 子句执行 KNN 搜索。两种接口支持相同的核心参数，无论你选择哪种格式，都能获得一致的体验：

- SQL: `select ... from <table name> where knn ( <field>, <query vector> [,<options>] )`
- JSON：
  ```
  POST /search
  {
      "table": "<table name>",
      "knn":
      {
          "field": "<field>",
          "query": "<text or vector>",
          "ef": <ef>,
		  "rescore": <rescore>,
		  "oversampling": <oversampling>
      }
  }
  ```

参数如下：
* `field`：包含向量数据的 float vector 属性名称。
* `k`：已弃用选项。请改用查询 `limit`。它用于指定单个 HNSW 索引应返回的文档数量。不过，最终结果中包含的文档总数可能会变化。例如，如果系统处理的是按磁盘 chunk 划分的实时表，每个 chunk 都可能返回 `k` 个文档，从而使总数超过指定的 `k`（因为累计数量为 `num_chunks * k`）。另一方面，如果在请求 `k` 个文档后，其中一些又根据特定属性被过滤掉，那么最终文档数量也可能少于 `k`。需要注意的是，`k` 参数不适用于 ramchunks。在 ramchunks 场景下，检索过程的工作方式不同，因此 `k` 参数对返回文档数量的影响不适用。
* `query`：（推荐参数）搜索查询，可以是：
  - 文本字符串：如果该字段配置了自动嵌入，则会自动转换为嵌入。如果该字段没有自动嵌入，则会返回错误。
  - 向量数组：工作方式与 `query_vector` 相同。
* `query_vector`：（旧参数）作为数字数组的搜索向量。为向后兼容仍然支持。
  **注意：** 同一请求中只能使用 `query` 或 `query_vector` 其中之一，不能同时使用。
* `ef`：搜索过程中使用的动态列表大小，可选。`ef` 越大，搜索越准确，但速度越慢。默认值为 10。
* `rescore`：启用 KNN 重新评分（默认启用）。在 SQL 中设为 `0` 或在 JSON 中设为 `false` 可禁用重新评分。KNN 搜索在使用量化向量完成后（可能伴随过采样），会使用原始（全精度）向量重新计算距离并重新排序结果，以提高排序准确性。
* `oversampling`：设置一个因子（浮点值），在执行 KNN 搜索时将 `k` 乘以该因子，从而使用量化向量检索出比所需更多的候选结果。默认应用 `oversampling=3.0`。如果启用了重新评分，这些候选结果之后可以重新评估。过采样也适用于非量化向量。由于它会增大 `k`，进而影响 HNSW 索引的工作方式，因此可能会使结果精度略有变化。
* `early_termination`：启用或禁用 HNSW 图遍历期间的自适应提前终止。默认启用。设为 SQL 中的 `0` 或 JSON 中的 `false` 可禁用。详情参见[提前终止](../Searching/KNN.md#Early-termination)。

当提供的是文本查询时（因此 Manticore 会在搜索前对字符串进行嵌入），可以在 SQL 中通过 `OPTION embeddings_threads = N` 按查询覆盖嵌入库使用的线程数。该值只会限制此查询的嵌入调用，覆盖全局 [embeddings_threads](../Server_settings/Searchd.md#embeddings_threads) 设置；`0` 表示不设上限。当查询以向量数组形式提供时，此选项不起作用。

文档总是按其与搜索向量的距离排序。你指定的任何附加排序条件都会在这个主排序条件之后应用。若要获取距离，有一个内置函数 [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29)。

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926), { ef=2000, oversampling=3.0, rescore=1 } );
```
<!-- response SQL -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    1 | 0.28146550 |
|    2 | 0.81527930 |
+------+------------+
2 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

```json
POST /search
{
	"table": "test",
	"knn":
	{
		"field": "image_vector",
		"query": [0.286569,-0.031816,0.066684,0.032926],
		"ef": 2000,
		"rescore": true,
		"oversampling": 3.0
	}
}
```

<!-- response JSON -->

```json
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":2,
		"total_relation":"eq",
		"hits":
		[
			{
				"_id": 1,
				"_score":1,
				"_knn_dist":0.28146550,
				"_source":
				{
					"title":"yellow bag",
					"image_vector":[0.653448,0.192478,0.017971,0.339821]
				}
			},
			{
				"_id": 2,
				"_score":1,
				"_knn_dist":0.81527930,
				"_source":
				{
					"title":"white bag",
					"image_vector":[-0.148894,0.748278,0.091892,-0.095406]
				}
			}
		]
	}
}
```

<!-- end -->

<!-- example knn_quantization -->

### 向量量化

HNSW 索引必须完整加载到内存中才能执行 KNN 搜索，这可能导致较高的内存消耗。为了降低内存占用，可以应用标量量化 - 这是一种通过用有限数量的离散值表示每个分量（维度）来压缩高维向量的技术。Manticore 支持 8 位和 1 位量化，这意味着每个向量分量都会从 32 位浮点压缩为 8 位甚至 1 位，分别将内存占用降低 4 倍或 32 倍。这些压缩表示还可以加快距离计算，因为更多的向量分量可以在单条 SIMD 指令中处理。虽然标量量化会引入一定的近似误差，但它通常是在搜索精度与资源效率之间值得接受的权衡。若要获得更高精度，可以将量化与重新评分和过采样结合使用：检索出的候选数量会多于请求数量，然后使用原始 32 位浮点向量重新计算这些候选的距离。

支持的量化类型包括：
* `8bit`：每个向量分量都会量化为 8 位。
* `1bit`：每个向量分量都会量化为 1 位。这里使用非对称量化，查询向量量化为 4 位，存储向量量化为 1 位。与更简单的方法相比，这种方式提供了更高的精度，但会带来一定的性能权衡。
* `1bitsimple`：每个向量分量都会量化为 1 位。该方法比 `1bit` 更快，但通常精度更低。

<!-- intro -->
##### SQL：

<!-- request SQL -->
```sql
create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' quantization='1bit');
```

<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.01 sec)
```

<!-- intro -->
##### JSON：

<!-- request JSON -->
```json
POST /sql?mode=raw -d "create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' quantization='1bit')"
```

<!-- response JSON -->

```json
[
  {
    "total": 0,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- Example knn_similar_docs -->

### 按 id 查找相似文档

> 注意：按 id 查找相似文档需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法工作，请确认已安装 Buddy。

根据唯一 ID 查找与某个文档相似的文档是一个常见任务。例如，当用户查看某个特定条目时，Manticore Search 可以高效地识别并显示在向量空间中与之最相似的一组条目。做法如下：

- SQL: `select ... from <table name> where knn ( <field>, <k>, <document id> )`
- JSON：
  ```
  POST /search
  {
      "table": "<table name>",
      "knn":
      {
          "field": "<field>",
          "doc_id": <document id>,
          "k": <k>
      }
  }
  ```

参数如下：
* `field`：包含向量数据的 float vector 属性名称。
* `k`：表示要返回的文档数量，是分层可导航小世界（HNSW）索引的关键参数。它指定单个 HNSW 索引应返回的文档数量。不过，最终结果中包含的文档总数可能会变化。例如，如果系统处理的是按磁盘 chunk 划分的实时表，每个 chunk 都可能返回 `k` 个文档，从而使总数超过指定的 `k`（因为累计数量为 `num_chunks * k`）。另一方面，如果在请求 `k` 个文档后，其中一些又根据特定属性被过滤掉，那么最终文档数量也可能少于 `k`。需要注意的是，`k` 参数不适用于 ramchunks。在 ramchunks 场景下，检索过程的工作方式不同，因此 `k` 参数对返回文档数量的影响不适用。
* `document id`：用于 KNN 相似度搜索的文档 ID。


<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, 5, 1 );
```
<!-- response SQL -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    2 | 0.81527930 |
+------+------------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

```json
POST /search
{
  "table": "test",
  "knn":
  {
    "field": "image_vector",
    "doc_id": 1,
    "k": 5
  }
}
```

<!-- response JSON -->

```json
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"total_relation":"eq",
		"hits":
		[
			{
				"_id": 2,
				"_score":1643,
				"_knn_dist":0.81527930,
				"_source":
				{
					"title":"white bag",
					"image_vector":[-0.148894,0.748278,0.091892,-0.095406]
				}
			}
		]
	}
}
```

<!-- end -->

<!-- Example knn_filtering -->

### 过滤 KNN 向量搜索结果

Manticore 还支持对 KNN 搜索返回的文档进行额外过滤，可以通过全文匹配、属性过滤，或两者同时进行。

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926) ) and match('white') and id < 10;
```
<!-- response SQL -->

```sql
+------+------------+
| id   | knn_dist() |
+------+------------+
|    2 | 0.81527930 |
+------+------------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

```json
POST /search
{
	"table": "test",
	"knn":
	{
		"field": "image_vector",
		"query": [0.286569,-0.031816,0.066684,0.032926],
		"k": 5
	},
	"query":
	{
		"bool":
		{
			"must":
			[
				{ "match": {"_all":"white"} },
				{ "range": { "id": { "lt": 10 } } }
			]
		}
	}
}
```

<!-- response JSON -->

```json
{
	"took":0,
	"timed_out":false,
	"hits":
	{
		"total":1,
		"total_relation":"eq",
		"hits":
		[
			{
				"_id": 2,
				"_score":1643,
				"_knn_dist":0.81527930,
				"_source":
				{
					"title":"white bag",
					"image_vector":[-0.148894,0.748278,0.091892,-0.095406]
				}
			}
		]
	}
}
```

<!-- end -->

<!-- example knn_filtering_strategies -->

### 过滤策略：预过滤 vs 后过滤

当将 KNN 向量搜索与属性过滤结合时，Manticore 支持两种策略，它们的区别在于过滤条件相对于 HNSW 图遍历的应用时机。

* 预过滤（默认；SQL 中为 `prefilter=1`，JSON 中为 `"prefilter": true`，默认）会把过滤条件直接传入 HNSW 遍历过程。每个候选项在加入结果堆之前都会先检查是否满足过滤条件 - 只有匹配的文档才会计入最终的 `k` 个结果。这减少了无效的距离计算，并保证最终返回恰好 `k` 个匹配文档（前提是确实存在 `k` 个匹配文档）。

* 后过滤（SQL 中为 `prefilter=0`，JSON 中为 `"prefilter": false`）会先在完整数据集上执行 KNN 搜索，然后再对结果应用过滤条件。这种方式安全且可预测：HNSW 图在不受干扰的情况下遍历，过滤器只影响哪些结果会返回给客户端。缺点是图可能会在最终会被丢弃的候选项上耗费精力。当过滤条件非常严格、只匹配极少部分文档时，返回的 `k` 个结果可能会明显少于请求数量，因为大多数 KNN 候选都会被过滤掉。

在内部，Manticore 在预过滤中使用了基于 ACORN-1 的算法。朴素的预过滤只会简单跳过不匹配的节点，这会有丢失连接原本分离部分 HNSW 图的“桥接”节点的风险，从而在过滤条件越来越严格时导致召回率崩溃。ACORN-1 避免了这一点：当某个节点不满足过滤条件时，它的邻居仍会被加入探索队列。这样遍历就可以绕过被过滤掉的节点，并维持图的连通性。当通过过滤条件的文档少于总文档数的 60% 时，会自动启用 ACORN-1 探索。

**自动暴力回退：** 当启用预过滤时，Manticore 会估算在过滤后的子集上执行暴力距离扫描是否比遍历 HNSW 图更便宜。该估算会比较 HNSW 预计访问的节点数与通过过滤的文档数。如果过滤后的集合足够小，直接扫描更快，Manticore 会自动切换到暴力方式，完全跳过 HNSW。这样即使在极高选择性条件下，也能保证正确性和良好性能。

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
-- prefilter (default): filter applied during HNSW traversal (ACORN-1 used automatically)
SELECT id, knn_dist() FROM test
WHERE knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926) )
AND price < 100;

-- postfilter: KNN runs over full dataset, filter applied to results
SELECT id, knn_dist() FROM test
WHERE knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926), { prefilter=0 } )
AND price < 100;
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

```json
// prefilter (default): filter applied during HNSW traversal
POST /search
{
    "table": "test",
    "knn": {
        "field": "image_vector",
        "query": [0.286569,-0.031816,0.066684,0.032926]
    },
    "query": {
        "range": { "price": { "lt": 100 } }
    }
}

// postfilter: filter applied after KNN search
POST /search
{
    "table": "test",
    "knn": {
        "field": "image_vector",
        "query": [0.286569,-0.031816,0.066684,0.032926],
        "prefilter": false
    },
    "query": {
        "range": { "price": { "lt": 100 } }
    }
}
```

<!-- end -->

### 提前终止

默认情况下，Manticore 在 HNSW 图遍历期间使用自适应提前终止算法。它不会始终探索由 `ef` 定义的完整候选集，而是监控新候选项改进结果集的速率，并在该速率持续低于阈值时提前停止。这样可以减少距离计算次数，同时不会显著影响结果质量。

提前终止默认启用，并且当 `k` 为 10 或更少时会自动禁用，因为对于这么小的结果集，该算法的开销不值得。性能收益会随着 `k` 增大而提升 - 结果集越大，通过提前停止就能节省越多距离计算。

请注意，过采样会乘大 HNSW 遍历期间使用的有效 `k`，因此提前终止也会从过采样中受益：更高的有效 `k` 意味着可能跳过更多候选项。

<!-- example knn_early_termination -->
若要显式控制提前终止，请使用 `early_termination` 选项：

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
-- disable early termination
SELECT id, knn_dist() FROM test WHERE knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926), { ef=200, early_termination=0 } );

-- enable early termination explicitly (default)
SELECT id, knn_dist() FROM test WHERE knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926), { ef=200, early_termination=1 } );
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

```json
POST /search
{
    "table": "test",
    "knn":
    {
        "field": "image_vector",
        "query": [0.286569,-0.031816,0.066684,0.032926],
        "ef": 200,
        "early_termination": false
    }
}
```

<!-- end -->

何时应禁用提前终止：
* 当结果集精度至关重要，且你无法接受超出 HNSW 已提供范围的任何近似时。
* 当使用较低的 `k` 值（大约 30 或更少）时，此时提前终止带来的性能收益很小，但可能降低精度。

<!-- proofread -->
