# K近邻向量搜索

Manticore Search 支持将机器学习模型生成的嵌入向量添加到每个文档中，然后对它们进行最近邻搜索。这使您可以基于NLP算法构建相似性搜索、推荐、语义搜索和相关性排序等功能，包括图像、视频和声音搜索等。

要将KNN向量搜索与全文搜索结合以获得更好的相关性，请参阅[混合搜索](../Searching/Hybrid_search.md)。

## 什么是嵌入？

嵌入是一种表示数据的方法 - 例如文本、图像或声音 - 作为高维空间中的向量。这些向量的构造是为了确保它们之间的距离反映了它们所代表的数据的相似性。此过程通常使用诸如词嵌入（例如Word2Vec、BERT）用于文本或神经网络用于图像的算法。向量空间的高维特性，每个向量有多个分量，允许表示项目之间复杂而细致的关系。它们的相似性通过这些向量之间的距离来衡量，通常使用欧几里得距离或余弦相似度等方法进行测量。

Manticore Search 使用 HNSW 库启用 k-最近邻（KNN）向量搜索。此功能是 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 的一部分。

<!-- example KNN -->

### 为 KNN 搜索配置表

要运行 KNN 搜索，您必须首先配置您的表。浮点向量和 KNN 搜索仅支持实时表（不支持普通表）。表需要至少一个 [float_vector](../Creating_a_table/Data_types.md#Float-vector) 属性，作为数据向量。您需要指定以下属性：
* `knn_type`：必填设置；目前仅支持 `hnsw`。
* `knn_dims`：必填设置，指定要索引的向量的维度。
* `hnsw_similarity`：必填设置，指定 HNSW 索引使用的距离函数。可接受的值为：
  - `L2` - 平方 L2
  - `IP` - 点积
  - `COSINE` - 余弦相似度

  **注意：** 使用 `COSINE` 相似度时，插入时向量会自动归一化。这意味着存储的向量值可能与原始输入值不同，因为它们将被转换为单位向量（具有数学长度/幅度为 1.0 的向量），以实现高效的余弦相似度计算。这种归一化保留了向量的方向，同时标准化了其长度。
* `hnsw_m`：可选设置，定义图中出边的最大数量。默认值为 16。
* `hnsw_ef_construction`：可选设置，定义构建时间/准确性权衡。默认值为 200。

> 注意：在 RT 块保存期间，HNSW 图构建，`OPTIMIZE TABLE` / 自动优化块合并，以及 `ALTER TABLE ... ADD/DROP/REBUILD` KNN 重建默认在多核主机上并行运行；工作线程数由 [`knn_parallel_build`](../Server_settings/Searchd.md#knn_parallel_build) searchd 设置控制（将其设置为 `1` 以强制串行路径）。这仅影响构建时间性能。由于并行 HNSW 构建可能会以不同顺序插入向量，因此生成的图可能与串行构建不完全相同。

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

**注意：** 有关普通模式中的自动嵌入，请参阅下面的示例，该示例展示了如何在 `knn` 配置中使用 `model_name` 和 `from` 参数。

<!-- end -->

<!-- example knn_insert -->

### 插入向量数据

#### 自动嵌入（推荐）

使用向量数据最简单的方法是使用 **自动嵌入**。使用此功能时，您创建一个带有 `MODEL_NAME` 和 `FROM` 参数的表，然后只需插入您的文本数据 - Manticore 会为您自动生成嵌入。

##### 创建带有自动嵌入的表

创建自动嵌入表时，请指定：
- `MODEL_NAME`：使用的嵌入模型
- `FROM`：用于生成嵌入的字段（留空表示所有文本/字符串字段）
- `API_KEY`：对于远程模型（OpenAI、Voyage、Jina）是必需的。在表创建期间通过实际 API 请求验证 API 密钥。
- `API_URL`：可选。自定义 API 端点 URL。如果未指定，则使用默认提供程序端点（例如，OpenAI 的 `https://api.openai.com/v1/embeddings`）。
- `API_TIMEOUT`：可选。API 请求的 HTTP 超时时间（以秒为单位）。默认值为 10 秒。设置为 `'0'` 以使用默认超时。适用于表创建期间的验证请求和插入操作期间的嵌入生成。

**支持的嵌入模型：**

| 模型类型 | 示例 | 需要 API 密钥 | 说明 |
|------------|---------|-----------------|-------|
| **Sentence Transformers** | `sentence-transformers/all-MiniLM-L6-v2` | 否 | 本地基于 BERT 的模型，自动下载 |
| **Qwen** | `Qwen/Qwen3-Embedding-0.6B` | 否 | 本地 Qwen 家族模型 |
| **Llama** | `TinyLlama/TinyLlama-1.1B-Chat-v1.0` | 否 | 本地 Llama 家族模型 |
| **Mistral** | `Locutusque/TinyMistral-248M-v2` | 否 | 本地 Mistral 家族模型 |
| **Gemma** | `h2oai/embeddinggemma-300m` | 否 | 本地 Gemma 家族模型 |
| **OpenAI** | `openai/text-embedding-ada-002` | 是 | `API_KEY='<OPENAI_API_KEY>'` |
| **Voyage** | Voyage AI 模型 | 是 | `API_KEY='<VOYAGE_API_KEY>'` |
| **Jina** | Jina AI 模型 | 是 | `API_KEY='<JINA_API_KEY>'` |

**本地模型格式要求：**
- 必须以 `safetensors` 格式保存（仅单文件）
- 支持的家族：Qwen、Llama、Mistral、Gemma
- 测试过的模型：`TinyLlama/TinyLlama-1.1B-Chat-v1.0`、`Locutusque/TinyMistral-248M-v2`、`Qwen/Qwen3-Embedding-0.6B`、`h2oai/embeddinggemma-300m`
- 其他 `safetensors` 模型可能也有效，但不保证

有关设置 `float_vector` 属性的更多信息，请参见 [此处](../Creating_a_table/Data_types.md#Float-vector)。

<!-- intro -->
##### SQL:

<!-- request SQL -->

使用 sentence-transformers（无需 API 密钥）
```sql
CREATE TABLE products (
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

使用 OpenAI 与自定义 API URL 和超时（可选）
```sql
CREATE TABLE products_openai_custom (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description'
    API_KEY='...' API_URL='https://custom-api.example.com/v1/embeddings' API_TIMEOUT='30'
);
```

使用所有文本字段进行嵌入（FROM 为空）
```sql
CREATE TABLE products_all (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM=''
);
```

<!-- intro -->
##### Plain mode (使用配置文件):

<!-- request Config -->
```ini
table products {
    type = rt
    path = /path/to/products
    rt_field = title
    rt_field = description
    rt_attr_float_vector = embedding_vector
    knn = {"attrs":[{"name":"embedding_vector","type":"hnsw","hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200,"model_name":"sentence-transformers/all-MiniLM-L6-v2","from":"title"}]}
}
```

在 plain mode 中使用 OpenAI：
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

使用所有文本字段（FROM 为空）：
```ini
table products_all {
    type = rt
    path = /path/to/products_all
    rt_field = title
    rt_field = description
    rt_attr_float_vector = embedding_vector
    knn = {"attrs":[{"name":"embedding_vector","type":"hnsw","hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200,"model_name":"sentence-transformers/all-MiniLM-L6-v2","from":""}]}
}
```

**plain mode 的重要说明：**
- 当使用 `model_name` 时，**不得**指定 `dims` - 模型会自动确定向量维度。`dims` 和 `model_name` 参数是互斥的。
- 当 **不**使用 `model_name`（手动插入向量）时，**必须**指定 `dims` 以指示向量维度。
- `from` 参数指定用于生成嵌入的字段（逗号分隔列表，或空字符串表示所有文本/字符串字段）。当使用 `model_name` 时，此参数是必需的。
- 对于基于 API 的模型（OpenAI、Voyage、Jina），在 knn 配置中包含 `api_key` 参数

<!-- end -->

##### 使用自动嵌入插入数据

<!--
以下示例的数据：

DROP TABLE IF EXISTS products;
CREATE TABLE products(title text, embedding_vector float_vector knn_type='hnsw' hnsw_similarity='l2' model_name='sentence-transformers/all-MiniLM-L6-v2' from='title');
DROP TABLE IF EXISTS products_openai;
CREATE TABLE products_openai(title text, description text, embedding_vector float_vector knn_type='hnsw' hnsw_similarity='l2' model_name='sentence-transformers/all-MiniLM-L6-v2' from='title,description');
-->

<!-- example inserting_embeddings -->

使用自动嵌入时，您可以：

- 省略向量字段，让 Manticore 从 `FROM` 中列出的字段生成嵌入
- 为某一行显式提供自己的向量
- 提供 `()` 以跳过生成并存储全零向量

如果您之后运行 `ALTER TABLE ... REBUILD EMBEDDINGS`，当前通过 `()` 生成的零向量行也会被重新生成。

<!-- intro -->
##### SQL:

<!-- request SQL -->

仅插入文本数据 - 自动生成嵌入
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

插入多个字段 - 如果 FROM='title,description'，两者都会用于嵌入
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
##### JSON:

<!-- request JSON -->

仅插入文本数据 - 自动生成嵌入
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title) VALUES ('machine learning artificial intelligence'),('banana fruit sweet yellow')"
```

插入多个字段 - 如果 FROM='title,description'，两者都会用于嵌入  
```JSON
POST /sql?mode=raw -d "INSERT INTO products_openai (title, description) VALUES ('smartphone', 'latest mobile device with advanced features'), ('laptop', 'portable computer for work and gaming')"
```

插入空向量（文档将被排除在向量搜索之外）
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title, embedding_vector) VALUES ('no embedding item', ())"
```

<!-- end -->

##### 使用自动嵌入进行搜索

<!-- example embeddings_search -->
搜索方式相同 - 提供查询文本，Manticore 会生成嵌入并查找相似文档：

<!-- intro -->
##### SQL:

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
##### JSON:

<!-- request JSON -->

使用文本查询与自动嵌入
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

#### 手动向量插入

<!-- example manual_vector -->
或者，您可以手动插入预计算的向量数据，确保其与创建表时指定的维度匹配。您也可以插入空向量；这意味着文档将被排除在向量搜索结果之外。

**重要：** 当使用 `hnsw_similarity='cosine'` 时，插入时向量会自动归一化为单位向量（数学长度/幅度为 1.0 的向量）。这种归一化保留了向量的方向，同时标准化其长度，这是高效余弦相似度计算所必需的。这意味着存储的值将与您原始输入值不同。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
insert into test values ( 1, 'yellow bag', (0.653448,0.192478,0.017971,0.339821) ), ( 2, 'white bag', (-0.148894,0.748278,0.091892,-0.095406) );
```
<!-- response SQL -->

```sql
Query OK, 2 rows affected (0.00 sec)
```

<!-- intro -->
##### JSON:

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
	"_id":1,
	"created":true,
	"result":"created",
	"status":201
}

{
	"table":"test",
	"_id":2,
	"created":true,
	"result":"created",
	"status":201
}
```

<!-- end -->

<!-- example knn_search -->

### KNN 向量搜索

现在，您可以使用 SQL 或 JSON 格式中的 `knn` 子句执行 KNN 搜索。两种接口支持相同的本质参数，无论您选择哪种格式，都能确保一致的体验：

- SQL: `select ... from <table name> where knn ( <field>, <query vector> [,<options>] )`
- JSON:
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

参数包括：
* `field`：这是包含向量数据的浮点向量属性的名称。
* `k`：已弃用的选项。请改用查询 `limit`。它曾用于指定单个 HNSW 索引应返回的文档数量。然而，最终结果中包含的文档实际数量可能会有所不同。例如，如果系统处理的是划分为磁盘块的实时表，每个块可能返回 `k` 个文档，导致总数超过指定的 `k`（因为累计数量将是 `num_chunks * k`）。另一方面，如果在请求 `k` 个文档后，根据特定属性过滤掉一些文档，最终文档数量可能少于 `k`。需要注意的是，参数 `k` 不适用于 ramchunks。在 ramchunks 的上下文中，检索过程的工作方式不同，因此 `k` 参数对返回文档数量的影响不适用。
* `query`：（推荐参数）搜索查询，可以是：
  - 文本字符串：如果字段已配置自动嵌入，则会自动转换为嵌入。如果字段未配置自动嵌入，将返回错误。
  - 向量数组：与 `query_vector` 的工作方式相同。
* `query_vector`：（旧版参数）作为数字数组的搜索向量。仍为向后兼容性提供支持。
  **注意**：在同一个请求中使用 `query` 或 `query_vector`，不要同时使用两者。
* `ef`：搜索期间使用的动态列表的可选大小。较高的 `ef` 会导致更准确但更慢的搜索。默认值为 10。
* `rescore`：启用 KNN 重排序（默认启用）。在 SQL 中设置为 `0` 或在 JSON 中设置为 `false` 以禁用重排序。在使用量化向量完成 KNN 搜索（可能有过采样）后，距离将使用原始（全精度）向量重新计算，结果将重新排序以提高排名准确性。
* `oversampling`：在执行 KNN 搜索时，设置一个乘数因子（浮点值），导致使用量化向量检索的候选对象数量超过所需数量。默认应用 `oversampling=3.0`。如果启用了重排序，这些候选对象可以稍后重新评估。过采样也适用于非量化向量。由于它会增加 `k`，这会影响 HNSW 索引的工作方式，可能会导致结果准确性的小幅变化。
* `early_termination`：启用或禁用 HNSW 图遍历期间的自适应早期终止。默认启用。在 SQL 中设置为 `0` 或在 JSON 中设置为 `false` 以禁用。有关详细信息，请参阅 [早期终止](../Searching/KNN.md#Early-termination)。

文档始终按其与搜索向量的距离进行排序。您指定的任何其他排序条件将在此主排序条件之后应用。要获取距离，有一个内置函数称为 [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29)。

<!-- intro -->
##### SQL:

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
##### JSON:

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

HNSW 索引需要完全加载到内存中才能执行 KNN 搜索，这可能导致显著的内存消耗。为了减少内存使用，可以应用标量量化——一种通过用有限数量的离散值表示每个组件（维度）来压缩高维向量的技术。Manticore 支持 8 位和 1 位量化，这意味着每个向量组件从 32 位浮点数压缩为 8 位甚至 1 位，分别减少内存使用量 4 倍或 32 倍。这些压缩表示还允许更快的距离计算，因为可以在单个 SIMD 指令中处理更多向量组件。尽管标量量化会引入一些近似误差，但通常这是在搜索准确性和资源效率之间值得权衡的。为了获得更好的准确性，量化可以与重排序和过采样结合使用：检索的候选对象数量超过请求的数量，并使用原始 32 位浮点向量重新计算这些候选对象的距离。

支持的量化类型包括：
* `8bit`：每个向量组件量化为 8 位。
* `1bit`：每个向量组件量化为 1 位。使用非对称量化，查询向量量化为 4 位，存储向量量化为 1 位。这种方法比简单方法提供更高的精度，但有一些性能权衡。
* `1bitsimple`：每个向量组件量化为 1 位。此方法比 `1bit` 快，但通常精度较低。

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' quantization='1bit');
```

<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.01 sec)
```

<!-- intro -->
##### JSON:

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

### 通过 ID 查找相似文档

> 注意：通过 ID 查找相似文档需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

根据特定文档的唯一 ID 查找相似文档是一项常见任务。例如，当用户查看某个特定项目时，Manticore Search 可以高效地识别并显示在向量空间中与该项目最相似的项目列表。以下是实现方法：

- SQL：`select ... from <table name> where knn ( <field>, <k>, <document id> )`
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

参数包括：
* `field`：这是包含向量数据的浮点向量属性的名称。
* `k`：这表示要返回的文档数量，是分层可导航小世界（HNSW）索引的关键参数。它指定单个HNSW索引应返回的文档数量。然而，最终结果中包含的实际文档数量可能会有所不同。例如，如果系统处理的是被划分为磁盘块的实时表，每个块可能返回`k`个文档，导致总数超过指定的`k`（因为累积数量将是`num_chunks * k`）。另一方面，如果在请求`k`个文档后，根据特定属性过滤掉一些文档，最终的文档数量可能少于`k`。需要注意的是，参数`k`不适用于ramchunks。在ramchunks的上下文中，检索过程的运作方式不同，因此`k`参数对返回文档数量的影响不适用。
* `document id`：KNN相似性搜索的文档ID。


<!-- intro -->
##### SQL:

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
##### JSON:

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

### 过滤KNN向量搜索结果

Manticore还支持通过全文匹配、属性过滤或两者的组合来进一步过滤KNN搜索返回的文档。

<!-- intro -->
##### SQL:

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
##### JSON:

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

### 过滤策略：预过滤 vs. 后过滤

在将KNN向量搜索与属性过滤结合使用时，Manticore支持两种策略，它们的区别在于过滤相对于HNSW图遍历的应用时机。

* 预过滤（默认；SQL中为`prefilter=1`或JSON中为`"prefilter": true`（默认））将过滤器直接传递到HNSW遍历过程中。在将候选文档添加到结果堆之前，会检查其是否符合过滤条件——只有匹配的文档才会贡献到最终的`k`个结果中。这减少了浪费的距离计算，并保证恰好返回`k`个匹配文档（假设存在`k`个匹配文档）。

* 后过滤（SQL中为`prefilter=0`或JSON中为`"prefilter": false`）首先在完整数据集上执行KNN搜索，然后对结果应用过滤器。这种方法是安全且可预测的：HNSW图的遍历不受干扰，过滤器仅影响返回给客户端的结果。缺点是图可能会在最终被丢弃的候选文档上耗费精力。当过滤器匹配的文档比例很小时，返回的`k`个结果可能显著少于请求的数量，因为大多数KNN候选文档会失败过滤。

内部而言，Manticore使用基于ACORN-1的算法进行预过滤。一种简单的预过滤方法仅跳过不匹配的节点，这可能会丢失连接HNSW图中其他分离部分的“桥接”节点，导致随着过滤器变得更严格，召回率崩溃。ACORN-1避免了这种情况：当节点不通过过滤时，其邻居仍会被添加到探索队列中。这使遍历能够绕过被过滤的节点并保持图的连通性。当通过过滤的文档少于总数的60%时，ACORN-1探索会自动激活。

**自动暴力扫描回退：** 当启用预过滤时，Manticore会估算是否比遍历HNSW图更便宜地对过滤后的子集执行暴力距离扫描。估算会比较HNSW预期访问的节点数与通过过滤的文档数。如果过滤后的集合足够小，直接扫描会更快，Manticore会自动切换到暴力扫描，完全跳过HNSW。这确保了即使在极端选择性下也能保证正确性和良好性能。

<!-- intro -->
##### SQL:

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
##### JSON:

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

默认情况下，Manticore在HNSW图遍历期间使用自适应提前终止算法。它不会始终探索由`ef`定义的完整候选集，而是监控新候选对结果集的改进速度，并在该速度持续低于阈值时提前停止。这减少了距离计算的数量，而不会显著影响结果质量。

默认启用提前终止，并且当`k`小于或等于10时会自动禁用，因为在这种小结果集上算法的开销不值得。性能收益随着`k`的增大而增加——结果集越大，通过提前终止节省的距离计算越多。

请注意，过采样会乘以HNSW遍历期间使用的有效`k`，因此提前终止也能从过采样中受益：更高的有效`k`意味着更多可能跳过的候选。

<!-- example knn_early_termination -->
要显式控制提前终止，请使用`early_termination`选项：

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
-- disable early termination
SELECT id, knn_dist() FROM test WHERE knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926), { ef=200, early_termination=0 } );

-- enable early termination explicitly (default)
SELECT id, knn_dist() FROM test WHERE knn ( image_vector, (0.286569,-0.031816,0.066684,0.032926), { ef=200, early_termination=1 } );
```

<!-- intro -->
##### JSON:

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

何时禁用提前终止：
* 当结果集的精度至关重要，且您无法承受HNSW本身之外的任何近似。
* 当使用较低的`k`值（约30或更少）时，提前终止提供的性能收益较小，但可能会降低精度。

<!-- proofread -->
