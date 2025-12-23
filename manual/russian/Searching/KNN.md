# Поиск по векторам K-ближайших соседей

Manticore Search поддерживает возможность добавления эмбеддингов, сгенерированных моделями машинного обучения, к каждому документу и последующего поиска ближайших соседей по ним. Это позволяет создавать функции, такие как поиск по похожести, рекомендации, семантический поиск и ранжирование релевантности на основе алгоритмов обработки естественного языка, а также, помимо прочего, поиск по изображениям, видео и звуку.

## Что такое эмбеддинг?

Эмбеддинг — это метод представления данных — таких как текст, изображения или звук — в виде векторов в пространстве высокой размерности. Эти векторы создаются так, чтобы расстояние между ними отражало степень сходства представленных данных. Этот процесс обычно использует алгоритмы, такие как эмбеддинги слов (например, Word2Vec, BERT) для текста или нейронные сети для изображений. Высокая размерность векторного пространства, с большим числом компонент на вектор, позволяет отображать сложные и тонкие взаимосвязи между объектами. Их сходство оценивается по расстоянию между векторами, часто измеряемому с помощью таких методов, как евклидово расстояние или косинусная близость.

Manticore Search реализует поиск по векторам K-ближайших соседей (KNN) с использованием библиотеки HNSW. Эта функциональность является частью [Manticore Columnar Library](https://github.com/manticoresoftware/columnar).

<!-- example KNN -->

### Настройка таблицы для поиска KNN

Чтобы выполнять поиск KNN, сначала необходимо настроить таблицу. Векторы с плавающей точкой и поиск KNN поддерживаются только в таблицах реального времени (не в обычных таблицах). Таблица должна содержать как минимум один атрибут [float_vector](../Creating_a_table/Data_types.md#Float-vector), который служит вектором данных. Нужно указать следующие свойства:
* `knn_type`: Обязательный параметр; в настоящее время поддерживается только `hnsw`.
* `knn_dims`: Обязательный параметр, указывающий количество измерений в индексируемых векторах.
* `hnsw_similarity`: Обязательный параметр, указывающий функцию расстояния, используемую индексом HNSW. Допустимые значения:
  - `L2` — Квадрат евклидова расстояния
  - `IP` — Внутреннее произведение
  - `COSINE` — Косинусная близость
  
  **Примечание:** При использовании косинусной близости (`COSINE`) векторы автоматически нормализуются при вставке. Это означает, что хранимые векторы могут отличаться от исходных значений, так как они преобразуются в единичные векторы (векторы с длиной/модулем 1.0) для эффективного вычисления косинусной близости. Такая нормализация сохраняет направление вектора, стандартизируя его длину.
* `hnsw_m`: Необязательный параметр, определяющий максимальное количество исходящих связей в графе. По умолчанию — 16.
* `hnsw_ef_construction`: Необязательный параметр, регулирующий компромисс между временем построения и точностью.

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
##### Обычный режим (с использованием конфигурационного файла):

<!-- request Config -->
```ini
table test_vec {
    type = rt
	...
    rt_attr_float_vector = image_vector
    knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":4,"hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200}]}
}
```

<!-- end -->

<!-- example knn_insert -->

### Вставка векторных данных

#### Автоматические эмбеддинги (рекомендуется)

Самый простой способ работы с векторными данными — использовать **автоматические эмбеддинги**. С этой функцией вы создаете таблицу с параметрами `MODEL_NAME` и `FROM`, а затем просто вставляете текстовые данные — Manticore автоматически сгенерирует эмбеддинги для вас.

##### Создание таблицы с автоматическими эмбеддингами

При создании таблицы для автоматических эмбеддингов укажите:
- `MODEL_NAME`: модель эмбеддинга для использования
- `FROM`: какие поля использовать для генерации эмбеддинга (пустое значение означает все текстовые/строковые поля)

**Поддерживаемые модели эмбеддингов:**
- **Sentence Transformers**: Любая [подходящая модель BERT на Hugging Face](https://huggingface.co/sentence-transformers/models) (например, `sentence-transformers/all-MiniLM-L6-v2`) — не требуется API ключ. Manticore загружает модель при создании таблицы.
- **OpenAI**: Модели эмбеддингов OpenAI, например, `openai/text-embedding-ada-002` — требует параметр `API_KEY='<OPENAI_API_KEY>'`
- **Voyage**: Модели эмбеддингов Voyage AI — требует параметр `API_KEY='<VOYAGE_API_KEY>'`
- **Jina**: Модели эмбеддингов Jina AI — требует параметр `API_KEY='<JINA_API_KEY>'`

Более подробную информацию о настройке атрибута `float_vector` можно найти [здесь](../Creating_a_table/Data_types.md#Float-vector).

<!-- intro -->
##### SQL:

<!-- request SQL -->

Использование sentence-transformers (API ключ не нужен)
```sql
CREATE TABLE products (
    title TEXT, 
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' 
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title'
);
```

Использование OpenAI (требуется параметр API_KEY)
```sql
CREATE TABLE products_openai (
    title TEXT,
    description TEXT, 
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description' API_KEY='...'
);
```

Использование всех текстовых полей для эмбеддингов (параметр FROM пустой)
```sql
CREATE TABLE products_all (
    title TEXT,
    description TEXT,
    embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2'
    MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM=''
);
```


<!-- intro -->
##### JSON:

<!-- request JSON -->

Использование sentence-transformers (API ключ не нужен)
```json
POST /sql?mode=raw -d "CREATE TABLE products ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title')"
```

Использование OpenAI (требуется параметр API_KEY)
```json
POST /sql?mode=raw -d "CREATE TABLE products_openai ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description' API_KEY='...')"
```

Использование всех текстовых полей для эмбеддингов (параметр FROM пустой)
```json
POST /sql?mode=raw -d "CREATE TABLE products_all ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='')"
```

<!-- end -->

##### Вставка данных с автоматическими эмбеддингами

<!-- example inserting_embeddings -->

При использовании автоматических эмбеддингов **не указывайте поле вектора** в операторе INSERT. Эмбеддинги генерируются автоматически из текстовых полей, указанных в параметре `FROM`.

<!-- intro -->
##### SQL:

<!-- request SQL -->

Вставить только текстовые данные — эмбеддинги сгенерируются автоматически
```sql
INSERT INTO products (title) VALUES 
('machine learning artificial intelligence'),
('banana fruit sweet yellow');
```

Вставить несколько полей — оба использованы для эмбеддинга, если FROM='title,description'  
```sql
INSERT INTO products_openai (title, description) VALUES
('smartphone', 'latest mobile device with advanced features'),
('laptop', 'portable computer for work and gaming');
```

Вставить пустой вектор (документ исключен из векторного поиска)
```sql
INSERT INTO products (title, embedding_vector) VALUES 
('no embedding item', ());
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

Вставить только текстовые данные — эмбеддинги сгенерируются автоматически
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title) VALUES ('machine learning artificial intelligence'),('banana fruit sweet yellow')"
```

Вставить несколько полей — оба использованы для эмбеддинга, если FROM='title,description'  
```JSON
POST /sql?mode=raw -d "INSERT INTO products_openai (title, description) VALUES ('smartphone', 'latest mobile device with advanced features'), ('laptop', 'portable computer for work and gaming')"
```

Вставить пустой вектор (документ исключен из векторного поиска)
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title, embedding_vector) VALUES ('no embedding item', ())"
```

<!-- end -->

##### Поиск с использованием автоматических эмбеддингов

<!-- example embeddings_search -->
Поиск работает так же — предоставьте текст запроса, и Manticore сгенерирует эмбеддинги и найдет похожие документы:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, knn_dist() FROM products WHERE knn(embedding_vector, 3, 'machine learning');
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

Использование текстового запроса с автоматическими эмбеддингами
```json
POST /search
{
    "table": "products",
    "knn": {
        "field": "embedding_vector",
        "query": "machine learning",
        "k": 3
    }
}
```

Использование векторного запроса напрямую
```json
POST /search
{
    "table": "products",
    "knn": {
        "field": "embedding_vector",
        "query": [0.1, 0.2, 0.3, 0.4],
        "k": 3
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

#### Manual Vector Insertion

<!-- example manual_vector -->
Alternatively, you can manually insert pre-computed vector data, ensuring it matches the dimensions you specified when creating the table. You can also insert an empty vector; this means that the document will be excluded from vector search results.

**Important:** When using `hnsw_similarity='cosine'`, vectors are automatically normalized upon insertion to unit vectors (vectors with a mathematical length/magnitude of 1.0). This normalization preserves the direction of the vector while standardizing its length, which is required for efficient cosine similarity calculations. This means the stored values will differ from your original input values.

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

### KNN vector search

Now, you can perform a KNN search using the `knn` clause in either SQL or JSON format. Both interfaces support the same essential parameters, ensuring a consistent experience regardless of the format you choose:

- SQL: `select ... from <table name> where knn ( <field>, <k>, <query vector> [,<options>] )`
- JSON:
  ```
  POST /search
  {
      "table": "<table name>",
      "knn":
      {
          "field": "<field>",
          "query": "<text or vector>",
          "k": <k>,
          "ef": <ef>,
		  "rescore": <rescore>,
		  "oversampling": <oversampling>
      }
  }
  ```

The parameters are:
* `field`: This is the name of the float vector attribute containing vector data.
* `k`: This represents the number of documents to return and is a key parameter for Hierarchical Navigable Small World (HNSW) indexes. It specifies the quantity of documents that a single HNSW index should return. However, the actual number of documents included in the final results may vary. For instance, if the system is dealing with real-time tables divided into disk chunks, each chunk could return `k` documents, leading to a total that exceeds the specified `k` (as the cumulative count would be `num_chunks * k`). On the other hand, the final document count might be less than `k` if, after requesting `k` documents, some are filtered out based on specific attributes. It's important to note that the parameter `k` does not apply to ramchunks. In the context of ramchunks, the retrieval process operates differently, and thus, the `k` parameter's effect on the number of documents returned is not applicable.
* `query`: (Recommended parameter) The search query, which can be either:
  - Text string: Automatically converted to embeddings if the field has auto-embeddings configured. Will return an error if the field doesn't have auto-embeddings.
  - Vector array: Works the same as `query_vector`.
* `query_vector`: (Legacy parameter) The search vector as an array of numbers. Still supported for backward compatibility.
  **Note:** Use either `query` or `query_vector`, not both in the same request.
* `ef`: optional size of the dynamic list used during the search. A higher `ef` leads to more accurate but slower search.
* `rescore`: Enables KNN rescoring (disabled by default). Set to `1` in SQL or `true` in JSON to enable rescoring. After the KNN search is completed using quantized vectors (with possible oversampling), distances are recalculated with the original (full-precision) vectors and results are re-sorted to improve ranking accuracy.
* `oversampling`: Sets a factor (float value) by which `k` is multiplied when executing the KNN search, causing more candidates to be retrieved than needed using quantized vectors. No oversampling is applied by default. These candidates can be re-evaluated later if rescoring is enabled. Oversampling also works with non-quantized vectors. Since it increases `k`, which affects how the HNSW index works, it may cause a small change in result accuracy.

Documents are always sorted by their distance to the search vector. Any additional sorting criteria you specify will be applied after this primary sort condition. For retrieving the distance, there is a built-in function called [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29).

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926), { ef=2000, oversampling=3.0, rescore=1 } );
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
		"k": 5,
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

### Vector quantization

HNSW indexes need to be fully loaded into memory to perform KNN search, which can lead to significant memory consumption. To reduce memory usage, scalar quantization can be applied - a technique that compresses high-dimensional vectors by representing each component (dimension) with a limited number of discrete values. Manticore supports 8-bit and 1-bit quantization, meaning each vector component is compressed from a 32-bit float to 8 bits or even 1 bit, reducing memory usage by 4x or 32x, respectively. These compressed representations also allow for faster distance calculations, as more vector components can be processed in a single SIMD instruction. Although scalar quantization introduces some approximation error, it is often a worthwhile trade-off between search accuracy and resource efficiency. For even better accuracy, quantization can be combined with rescoring and oversampling: more candidates are retrieved than requested, and distances for these candidates are recalculated using the original 32-bit float vectors.

Supported quantization types include:
* `8bit`: Each vector component is quantized to 8 bits.
* `1bit`: Each vector component is quantized to 1 bit. Asymmetric quantization is used, with query vectors quantized to 4 bits and stored vectors to 1 bit. This approach offers greater precision than simpler methods, though with some performance trade-off.
* `1bitsimple`: Each vector component is quantized to 1 bit. This method is faster than `1bit`, but typically less accurate.

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

### Найти похожие документы по id

> ПРИМЕЧАНИЕ: Для поиска похожих документов по id требуется [Manticore Buddy](../Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

Поиск документов, похожих на конкретный, на основе его уникального ID — распространённая задача. Например, когда пользователь просматривает определённый элемент, Manticore Search может эффективно определить и отобразить список элементов, наиболее похожих на него в векторном пространстве. Вот как вы можете это сделать:

- SQL: `select ... from <table name> where knn ( <field>, <k>, <document id> )`
- JSON:
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

Параметры:
* `field`: Это имя атрибута с плавающим вектором, содержащего векторные данные.
* `k`: Представляет количество возвращаемых документов и является ключевым параметром для индексов Hierarchical Navigable Small World (HNSW). Указывает количество документов, которые должен вернуть один HNSW-индекс. Однако фактическое число документов в итоговых результатах может варьироваться. Например, если система обрабатывает таблицы в реальном времени, разделённые на дисковые чанки, каждый чанк может вернуть `k` документов, что приведёт к общему числу, превышающему указанное `k` (поскольку суммарное количество будет `num_chunks * k`). С другой стороны, итоговое число документов может быть меньше `k`, если после запроса `k` документов некоторые отфильтровываются по определённым атрибутам. Важно отметить, что параметр `k` не применяется к ramchunks. В случае ramchunks процесс выборки происходит иначе, поэтому эффект параметра `k` на количество возвращаемых документов не применяется.
* `document id`: ID документа для поиска сходства KNN.


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

### Фильтрация результатов поиска KNN по вектору

Manticore также поддерживает дополнительную фильтрацию документов, возвращаемых KNN-поиском, либо с помощью полнотекстового поиска, фильтров по атрибутам, или и того, и другого.

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
		"k": 5,
		"filter":
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

<!-- proofread -->

