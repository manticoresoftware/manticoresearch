# Диалоговый поиск

Диалоговый поиск добавляет чат с усилением ретриваля над существующей векторизованной таблицей. Он выполняет поиск в таблице, формирует контекст из соответствующих документов и запрашивает большую языковую модель (LLM) для ответа, используя этот контекст и текущую историю диалога.

Диалоговый поиск предоставляется плагином Buddy `ConversationalSearch` и управляется командами SQL:

- `CREATE CHAT MODEL`
- `SHOW CHAT MODELS`
- `DESCRIBE CHAT MODEL`
- `DROP CHAT MODEL`
- `CALL CHAT`

> ВНИМАНИЕ: Диалоговый поиск требует [Manticore Buddy](../Installation/Manticore_Buddy.md) с плагином `ConversationalSearch` и установленным PHP расширением `llm`. Поддержка провайдеров зависит от установленного расширения `llm`.

## Как это работает

Во время выполнения запроса `CALL CHAT`:

1. Загружает конфигурацию модели чата.
2. Загружает историю диалога для предоставленного UUID диалога или создает новый UUID, если он не предоставлен.
3. Проверяет целевой таблицу и выбирает поле `FLOAT_VECTOR`.
4. Направляет пользовательское сообщение с LLM, чтобы определить, следует:
   - выполнить новый поиск
   - ответить из предыдущего контекста поиска
   - ответить без поиска, когда ретриваль не требуется
5. Выполняет KNN поиск с выбранным векторным полем.
6. Формирует контекст промта из исходных полей `from='...'` выбранного векторного поля.
7. Генерирует окончательный ответ с настроенной LLM.
8. Сохраняет сообщения пользователя и помощника в истории диалога.

Аргумент `fields` в `CALL CHAT` — это историческое название. Он означает, какой поле `FLOAT_VECTOR` использовать для KNN, а не какие поля возвращать. Результаты поиска выбираются с помощью `SELECT *`, но векторные столбцы удаляются из возвращаемого payload `sources`.

## Требования к таблице

Таблица для поиска должна содержать хотя бы одно поле `FLOAT_VECTOR` с исходными полями для авто-эмбеддинга:

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

`from='title,content'` важно по двум причинам:

- Manticore использует его для формирования эмбеддингов для векторного поля.
- Диалоговый поиск использует те же поля для формирования текстового контекста, отправляемого в LLM.

При наличии нескольких векторных полей каждое векторное поле может использовать разные исходные поля:

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

Если `CALL CHAT` не указывает векторное поле, Buddy использует первое обнаруженное поле `FLOAT_VECTOR` из `SHOW CREATE TABLE`.

Для получения дополнительной информации о векторных полях и авто-эмбеддингах см. [K-ближайший сосед векторный поиск](../Searching/KNN.md).

## Создание модели чата

Используйте `CREATE CHAT MODEL` для определения LLM и настроек ретриваля.

Минимальная модель:

<!-- example conversational_search_create_chat_model_minimal -->

<!-- request SQL -->

```sql
CREATE CHAT MODEL assistant (
	model='openai:gpt-4o-mini'
);
```

<!-- end -->

Модель с параметрами провайдера и настройками ретриваля:

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

Общие параметры:

| Параметр | Обязательно | Описание |
|---|---:|---|
| `model` | Да | ID модели LLM в формате `provider:model` |
| `description` | Нет | Сохраняемое описание |
| `api_key` | Нет | Ключ API провайдера, передаваемый расширению `llm` |
| `base_url` | Нет | Базовый URL провайдера или прокси |
| `timeout` | Нет | Таймаут запроса к LLM |
| `retrieval_limit` | Нет | Количество документов, запрашиваемых из KNN, от `1` до `50` |
| `max_document_length` | Нет | Ограничение контекста на документ; `0` отключает обрезку |

`model` проверяется как `provider:model`, например:

```sql
model='openai:gpt-4o-mini'
```


Поддержка провайдеров осуществляется через установленное расширение `llm` [PHP](https://github.com/manticoresoftware/llm-php-ext). Buddy передает параметры провайдера, такие как `api_key`, `base_url` и `timeout`, этому расширению.

`api_key` является опциональным, когда ключ провайдера доступен в окружении Buddy. Например, сервис Docker Compose может предоставлять ключи провайдера следующим образом:


```yaml
environment:
  - OPENAI_API_KEY=${OPENAI_API_KEY}
  - OPENROUTER_API_KEY=${OPENROUTER_API_KEY}
```

Если `api_key` не установлен в `CREATE CHAT MODEL`, расширение `llm` может использовать соответствующую переменную окружения провайдера. Передавайте `api_key` только тогда, когда хотите, чтобы конфигурация модели переопределяла окружение.

## Задание вопросов

Базовый вызов:

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

Продолжение диалога:

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

Использование конкретного векторного поля для KNN:

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

Когда предоставлен пятый аргумент, Buddy проверяет, что поле существует и является `FLOAT_VECTOR`. Если он опущен, Buddy обнаруживает первое поле `FLOAT_VECTOR` из `SHOW CREATE TABLE`.

### Синтаксис CALL CHAT

```sql
CALL CHAT(
	'query',
	'table',
	'model_name_or_uuid',
	'conversation_uuid',
	'vector_field'
);
```

Аргументы только позиционные:

| Позиция | Аргумент | Обязательно | Описание |
|---:|---|---:|---|
| 1 | `query` | Да | Пользовательский вопрос |
| 2 | `table` | Да | Таблица для поиска |
| 3 | `model_name_or_uuid` | Да | Имя модели чата или UUID |
| 4 | `conversation_uuid` | Нет | ID существующего диалога или пустая строка |
| 5 | `fields` / векторное поле | Нет | Поле `FLOAT_VECTOR`, используемое в `knn(...)` |

Пятый аргумент хранится внутри как `fields` для совместимости с текущим парсером, но он должен быть именем одного векторного поля.

## Детали поиска и контекста

KNN поиск использует эту форму:

```sql
SELECT *, knn_dist() AS knn_dist
FROM <table>
WHERE knn(<vector_field>, <k>, '<search query>')
  AND knn_dist < <threshold>
LIMIT <retrieval_limit>
```

Поведение:

- `retrieval_limit` контролирует окончательный `LIMIT`.
- Текущий порог KNN по умолчанию — `0.8`.
- Если маршрут диалога включает запрос исключения, Buddy сначала находит соответствующие ID с более строгим порогом KNN и исключает эти ID из основного поиска.
- Окончательный `sources` включает обычные поля результатов и `knn_dist`.
- Столбцы `FLOAT_VECTOR` удаляются из `sources`, чтобы избежать возврата больших payload эмбеддингов.
- Контекст LLM формируется только из исходных результатов.

`max_document_length` применяется к каждому исходному документу:

- `0` отключает обрезку.
- `100..65536` обрезает контекст каждого документа до указанного количества символов.
- Меньшие значения уменьшают размер промта.
- Большие значения сохраняют больше исходного текста, но увеличивают использование токенов.

## Ответ

`CALL CHAT` возвращает одну строку со следующими столбцами:

| Столбец | Описание |
|---|---|
| `conversation_uuid` | Существующий или сгенерированный идентификатор диалога |
| `user_query` | Исходный запрос пользователя |
| `search_query` | Автономный поисковый запрос, использованный для извлечения |
| `response` | Ответ LLM |
| `sources` | JSON-строка, содержащая извлеченные строки источников |

Пример структуры ответа:

```json
{
  "conversation_uuid": "docs-chat-001",
  "user_query": "What is vector search?",
  "search_query": "What is vector search and how does it use embeddings to find similar documents?",
  "response": "Vector search finds similar items by comparing embeddings...",
  "sources": "[{\"id\":1,\"title\":\"Vector Search\",\"content\":\"...\",\"knn_dist\":0.12}]"
}
```

Векторные поля намеренно отсутствуют в `sources`.

## Управление моделями

Список моделей:

<!-- example conversational_search_show_chat_models -->

<!-- request SQL -->

```sql
SHOW CHAT MODELS;
```

<!-- end -->

Описание модели:

<!-- example conversational_search_describe_chat_model -->

<!-- request SQL -->

```sql
DESCRIBE CHAT MODEL assistant;
```

<!-- end -->

Описание по UUID:

<!-- example conversational_search_describe_chat_model_uuid -->

<!-- request SQL -->

```sql
DESCRIBE CHAT MODEL '550e8400-e29b-41d4-a716-446655440000';
```

<!-- end -->

Удаление модели:

<!-- example conversational_search_drop_chat_model -->

<!-- request SQL -->

```sql
DROP CHAT MODEL assistant;
```

<!-- end -->

Безопасное удаление:

<!-- example conversational_search_drop_chat_model_if_exists -->

<!-- request SQL -->

```sql
DROP CHAT MODEL IF EXISTS assistant;
```

<!-- end -->

## Полный пример

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

## Устранение неполадок

`Table '<table>' has no FLOAT_VECTOR field`

Таблица не содержит векторного поля. Добавьте поле `FLOAT_VECTOR` перед использованием с `CALL CHAT`.

`FLOAT_VECTOR field '<field>' not found in table '<table>'`

Пятый аргумент `CALL CHAT` указывает на поле, которое не является векторным полем в целевой таблице.

`FLOAT_VECTOR field '<field>' has no auto-embedding source fields`

Векторное поле не имеет `from='...'`. Добавьте исходные поля, чтобы Buddy знал, какие текстовые поля использовать для контекста LLM.

`Search query must contain at least one term`

Перенаправленный автономный поисковый запрос пуст. Обычно это означает, что входной запрос был пустым или маршрутизирующая LLM вернула недопустимый поисковый текст.
