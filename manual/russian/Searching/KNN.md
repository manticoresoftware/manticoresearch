# K-nearest neighbor vector search

Manticore Search поддерживает возможность добавления эмбеддингов, сгенерированных моделями машинного обучения, к каждому документу, а затем выполнения поиска ближайших соседей по ним. Это позволяет создавать такие функции, как поиск по сходству, рекомендации, семантический поиск и ранжирование релевантности на основе NLP-алгоритмов, а также поиски по изображениям, видео и звукам.

## Что такое эмбеддинг?

Эмбеддинг — это метод представления данных — таких как текст, изображения или звук — в виде векторов в пространстве высокой размерности. Эти векторы создаются так, чтобы расстояние между ними отражало сходство представленных данных. Этот процесс обычно использует алгоритмы, такие как представления слов (например, Word2Vec, BERT) для текста или нейронные сети для изображений. Высокая размерность векторного пространства, с множеством компонентов на вектор, позволяет представлять сложные и тонкие отношения между элементами. Их сходство оценивается по расстоянию между этими векторами, часто с использованием таких методов, как евклидово расстояние или косинусное сходство.

Manticore Search обеспечивает поиск k-ближайших соседей (KNN) по векторам с использованием библиотеки HNSW. Эта функциональность входит в состав [Manticore Columnar Library](https://github.com/manticoresoftware/columnar).

<!-- example KNN -->

### Настройка таблицы для KNN поиска

Чтобы выполнять KNN поиски, сначала необходимо настроить таблицу. Векторные числа с плавающей точкой и KNN поиск поддерживаются только в таблицах реального времени (не в обычных таблицах). В таблице должен быть хотя бы один атрибут типа [float_vector](../Creating_a_table/Data_types.md#Float-vector), который служит вектором данных. Нужно задать следующие свойства:
* `knn_type`: обязательная настройка; на данный момент поддерживается только `hnsw`.
* `knn_dims`: обязательная настройка, которая указывает размерность индексируемых векторов.
* `hnsw_similarity`: обязательная настройка, указывающая функцию расстояния, используемую индексом HNSW. Допустимые значения:
  - `L2` — квадрат евклидова расстояния
  - `IP` — внутреннее произведение
  - `COSINE` — косинусное сходство
  
  **Примечание:** При использовании `COSINE` сходства векторы автоматически нормализуются при вставке. Это означает, что хранимые значения векторов могут отличаться от исходных, так как они будут преобразованы в единичные векторы (векторы с математической длиной/модулем 1.0) для эффективного вычисления косинусного сходства. Такая нормализация сохраняет направление вектора при стандартизации его длины.
* `hnsw_m`: необязательная настройка, определяющая максимальное количество исходящих связей в графе. По умолчанию 16.
* `hnsw_ef_construction`: необязательная настройка, которая задаёт компромисс между временем построения и точностью.

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
POST /sql?mode=raw -d "create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' );"
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
##### Plain mode (использование конфигурационного файла):

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

Самый простой способ работы с векторными данными — использовать **автоматические эмбеддинги**. С этой функцией вы создаёте таблицу с параметрами `MODEL_NAME` и `FROM`, а затем просто вставляете текстовые данные — Manticore автоматически генерирует эмбеддинги.

##### Создание таблицы с автоматическими эмбеддингами

При создании таблицы для автоматических эмбеддингов укажите:
- `MODEL_NAME`: модель эмбеддинга для использования
- `FROM`: какие поля использовать для генерации эмбеддингов (пустое значение означает все текстовые/строковые поля)

**Поддерживаемые модели эмбеддингов:**
- **Sentence Transformers**: Любая [подходящая модель на базе BERT из Hugging Face](https://huggingface.co/sentence-transformers/models) (например, `sentence-transformers/all-MiniLM-L6-v2`) — ключ API не требуется. Manticore скачивает модель при создании таблицы.
- **OpenAI**: модели эмбеддингов OpenAI, такие как `openai/text-embedding-ada-002` — требуется параметр `API_KEY='<OPENAI_API_KEY>'`
- **Voyage**: модели эмбеддингов Voyage AI — требуется параметр `API_KEY='<VOYAGE_API_KEY>'`
- **Jina**: модели эмбеддингов Jina AI — требуется параметр `API_KEY='<JINA_API_KEY>'`

Дополнительную информацию о настройке атрибута `float_vector` можно найти [здесь](../Creating_a_table/Data_types.md#Float-vector).

<!-- intro -->
##### SQL:

<!-- request SQL -->

Использование sentence-transformers (ключ API не нужен)
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

Использование всех текстовых полей для эмбеддингов (FROM пустой)
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

Использование sentence-transformers (ключ API не требуется)
```json
POST /sql?mode=raw -d "CREATE TABLE products ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='title');"
```

Использование OpenAI (требуется параметр API_KEY)
```json
POST /sql?mode=raw -d "CREATE TABLE products_openai ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='openai/text-embedding-ada-002' FROM='title,description' API_KEY='...');"
```

Использование всех текстовых полей для эмбеддингов (FROM пуст)
```json
POST /sql?mode=raw -d "CREATE TABLE products_all ( title TEXT, description TEXT, embedding_vector FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='sentence-transformers/all-MiniLM-L6-v2' FROM='');"
```

<!-- end -->

##### Вставка данных с автоматическими эмбеддингами

<!-- example inserting_embeddings -->

При использовании автоматических эмбеддингов **не указывайте** поле вектора в вашем операторе INSERT. Эмбеддинги генерируются автоматически из текстовых полей, указанных в параметре `FROM`.

<!-- intro -->
##### SQL:

<!-- request SQL -->

Вставка только текстовых данных — эмбеддинги сгенерируются автоматически
```sql
INSERT INTO products (title) VALUES 
('machine learning artificial intelligence'),
('banana fruit sweet yellow');
```

Вставка нескольких полей — оба используются для эмбеддинга, если FROM='title,description'  
```sql
INSERT INTO products_openai (title, description) VALUES
('smartphone', 'latest mobile device with advanced features'),
('laptop', 'portable computer for work and gaming');
```

Вставка пустого вектора (документ исключается из векторного поиска)
```sql
INSERT INTO products (title, embedding_vector) VALUES 
('no embedding item', ());
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

Вставка только текстовых данных - векторные представления генерируются автоматически
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title) VALUES ('machine learning artificial intelligence'),('banana fruit sweet yellow');"
```

Вставка нескольких полей - оба используются для создания векторных представлений, если FROM='title,description'  
```JSON
POST /sql?mode=raw -d "INSERT INTO products_openai (title, description) VALUES ('smartphone', 'latest mobile device with advanced features'), ('laptop', 'portable computer for work and gaming');"
```

Вставка пустого вектора (документ исключается из поиска по векторам)
```JSON
POST /sql?mode=raw -d "INSERT INTO products (title, embedding_vector) VALUES ('no embedding item', ());"
```

<!-- end -->

##### Поиск с автоматическими эмбеддингами

<!-- example embeddings_search -->
Поиск выполняется так же — введите текст запроса, и Manticore сгенерирует эмбеддинги и найдёт похожие документы:

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

Поиск с текстовым запросом с автоматическими эмбеддингами
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

Поиск с прямым векторным запросом
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

#### Ручная вставка векторов

<!-- example manual_vector -->
В качестве альтернативы, вы можете вручную вставлять заранее вычисленные векторные данные, при этом они должны соответствовать размерности, заданной при создании таблицы. Также можно вставить пустой вектор; это означает, что документ будет исключён из результатов векторного поиска.

**Важно:** При использовании `hnsw_similarity='cosine'` векторы автоматически нормализуются при вставке в единичные векторы (векторы с математической длиной/модулем 1.0). Эта нормализация сохраняет направление вектора, стандартизируя его длину, что необходимо для эффективных вычислений косинусного сходства. Это означает, что сохранённые значения будут отличаться от ваших исходных значений.

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

### Поиск KNN по векторам

Теперь вы можете выполнить поиск KNN, используя клаузу `knn` в формате SQL или JSON. Оба интерфейса поддерживают одинаковые основные параметры, обеспечивая единообразный опыт работы вне зависимости от выбранного формата:

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

Параметры:
* `field`: Имя атрибута с плавающей точкой, содержащего векторные данные.
* `k`: Количество документов для возврата — ключевой параметр для индексов Hierarchical Navigable Small World (HNSW). Он задаёт количество документов, которые должен вернуть один индекс HNSW. Однако фактическое количество документов в итоговом результате может меняться. Например, если система работает с таблицами в реальном времени, разбитыми на дисковые чанки, каждый чанк может вернуть `k` документов, что приводит к общему числу, превышающему заданное `k` (так как сумма будет `num_chunks * k`). С другой стороны, итоговое количество документов может быть меньше `k`, если после запроса `k` документов некоторые из них отфильтровываются по определённым атрибутам. Важно отметить, что параметр `k` не применяется к ramchunks. В контексте ramchunks процесс выборки работает иначе, и поэтому эффект параметра `k` на число возвращаемых документов неприменим.
* `query`: (Рекомендуемый параметр) Поисковый запрос, который может быть:
  - строкой текста: Автоматически преобразуется в эмбеддинги, если для поля настроены автоэмбеддинги. В противном случае будет возвращена ошибка.
  - массивом векторов: Работает так же, как `query_vector`.
* `query_vector`: (Устаревший параметр) Вектор поиска в виде массива чисел. Поддерживается для обратной совместимости.
  **Примечание:** Используйте либо `query`, либо `query_vector`, но не оба параметра в одном запросе.
* `ef`: необязательный размер динамического списка, используемого во время поиска. Более высокое значение `ef` увеличивает точность, но замедляет поиск.
* `rescore`: Включает пересчёт рейтинга KNN (по умолчанию выключено). Установите `1` в SQL или `true` в JSON для активации пересчёта. После завершения поиска KNN с использованием квантизованных векторов (с возможным овэрсэмплингом) расстояния пересчитываются с применением исходных (полной точности) векторов, а результаты переупорядочиваются для улучшения точности ранжирования.
* `oversampling`: Устанавливает множитель (число с плавающей точкой), на который умножается `k` при выполнении поиска KNN, что приводит к выборке большего числа кандидатов, чем требуется, с использованием квантизованных векторов. По умолчанию овэрсэмплинг не применяется. Эти кандидаты могут быть повторно оценены, если включён пересчёт рейтинга. Овэрсэмплинг также работает с неквантизованными векторами. Поскольку он увеличивает `k`, что влияет на работу индекса HNSW, это может привести к небольшим изменениям в точности результатов.

Документы всегда сортируются по расстоянию до искомого вектора. Любые дополнительные критерии сортировки будут применены после основной сортировки. Для получения расстояния существует встроенная функция [knn_dist()](../Functions/Other_functions.md#KNN_DIST%28%29).

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

### Квантование векторов

Индексы HNSW должны быть полностью загружены в память для выполнения поиска KNN, что может привести к значительному расходу памяти. Чтобы уменьшить использование памяти, может применяться скалярное квантование — метод сжатия многомерных векторов путём представления каждого компонента (измерения) ограниченным числом дискретных значений. Manticore поддерживает 8-битное и 1-битное квантование, то есть каждый компонент вектора сжимается с 32-битного числа с плавающей точкой до 8 или даже 1 бита, сокращая расход памяти соответственно в 4 или 32 раза. Эти сжатые представления также позволяют ускорить вычисление расстояний, так как больше компонентов вектора может обрабатываться в одной SIMD-инструкции. Хотя скалярное квантование вносит некоторую аппроксимационную ошибку, это часто приемлемый компромисс между точностью поиска и эффективностью ресурсов. Для ещё большей точности квантование можно сочетать с пересчётом рейтинга и овэрсэмплингом: выбирается больше кандидатов, чем запрашивается, и для них пересчитываются расстояния с использованием исходных 32-битных векторов.

Поддерживаемые типы квантования:
* `8bit`: Каждый компонент вектора квантуется в 8 бит.
* `1bit`: Каждый компонент вектора квантуется в 1 бит. Используется асимметричное квантование — векторы запроса квантуются в 4 бита, а сохранённые векторы — в 1 бит. Такой подход обеспечивает большую точность, чем более простые методы, пусть и за счёт некоторого снижения производительности.
* `1bitsimple`: Каждый компонент вектора квантуется в 1 бит. Этот метод быстрее, чем `1bit`, но обычно менее точен.

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
POST /sql?mode=raw -d "create table test ( title text, image_vector float_vector knn_type='hnsw' knn_dims='4' hnsw_similarity='l2' quantization='1bit');"
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

### Поиск похожих документов по id

> ПРИМЕЧАНИЕ: Поиск похожих документов по id требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

Нахождение документов, похожих на конкретный по его уникальному идентификатору, является распространённой задачей. Например, когда пользователь просматривает определённый элемент, Manticore Search может эффективно определить и показать список элементов, которые наиболее похожи на него в векторном пространстве. Вот как это можно сделать:

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
* `field`: Это имя атрибута с плавающей точкой, содержащего векторные данные.
* `k`: Это количество документов для возврата и ключевой параметр для индексов Hierarchical Navigable Small World (HNSW). Он указывает количество документов, которые должен вернуть один индекс HNSW. Однако фактическое количество документов в итоговых результатах может варьироваться. Например, если система работает с таблицами в реальном времени, разбитыми на дисковые чанки, каждый чанк может вернуть `k` документов, что приведёт к общему количеству, превышающему заданное `k` (поскольку суммарное количество будет `num_chunks * k`). С другой стороны, конечное количество документов может быть меньше `k`, если после запроса `k` документов некоторые из них отфильтровываются по определённым атрибутам. Важно отметить, что параметр `k` не применяется к ramchunks. В контексте ramchunks процесс поиска работает иначе, и, соответственно, параметр `k` не влияет на количество возвращаемых документов.
* `document id`: Идентификатор документа для поиска похожих документов методом KNN.


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

### Фильтрация результатов KNN векторного поиска

Manticore также поддерживает дополнительную фильтрацию документов, возвращаемых поиском KNN, как с помощью полнотекстового поиска, так и фильтров по атрибутам, или обоих способов.

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

