# Гибридный поиск

Гибридный поиск объединяет [полнотекстовый (BM25) поиск](../Searching/Full_text_matching/Basic_usage.md) с [KNN векторным поиском](../Searching/KNN.md) в одном запросе, объединяя результаты с использованием **Reciprocal Rank Fusion (RRF)**. Это позволяет использовать сильные стороны обоих методов извлечения: точность по ключевым словам от BM25 и семантическое понимание от векторного сходства.

Полнотекстовый поиск отлично справляется с точным соответствием ключевых слов и редкими терминами, но упускает концептуально похожий контент. Векторный поиск улавливает семантическое значение, но может быть зашумленным на неоднозначных запросах. Гибридный поиск объединяет оба подхода, поэтому документы, которые хорошо оцениваются по **любому или обоим** сигналам, оказываются на поверхности.

## Reciprocal Rank Fusion (RRF)

RRF — это алгоритм слияния, основанный на рангах. Он работает с **позициями рангов**, а не с исходными оценками, что позволяет избежать необходимости нормализации несовместимых шкал оценок (оценки BM25 неограничены; расстояния KNN имеют другую шкалу).

### Формула

```
RRF_score(d) = SUM over all result sets r:  weight_r / (rank_constant + rank_r(d))
```

Где:
- `d` — это документ
- `rank_r(d)` — это позиция документа (начиная с 1) в наборе результатов `r` (отсортированном по оценке этого извлекателя)
- `rank_constant` — это константа сглаживания (по умолчанию: 60, настраивается через опцию [`rank_constant`](../Searching/Options.md#rank_constant))
- `weight_r` — это необязательный вес для каждого извлекателя (по умолчанию: 1.0)

Если документ не появляется в конкретном наборе результатов, его вклад от этого набора равен 0.

### Параметр rank_constant

- `rank_constant=60` используется по умолчанию.
- Меньшие значения (например, 10) усиливают различия между элементами с наивысшим рангом.
- Более высокие значения (например, 100) распределяют влияние более равномерно по всем рангам.

## Базовый гибридный запрос

Объедините `MATCH(...)` и `KNN(...)` в предложении WHERE с `OPTION fusion_method='rrf'`:

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

Это запускает текстовый поиск и KNN поиск как независимые параллельные подзапросы, а затем объединяет результаты с помощью RRF. Без `fusion_method='rrf'` запрос выполняется как обычный KNN поиск, отфильтрованный по текстовому соответствию (поведение до гибридного поиска).

В этом режиме до гибридного поиска ранжирование KNN по-прежнему имеет приоритет. Если `knn_dist()` доступна и вы не сортируете по ней явно, Manticore добавляет `knn_dist() ASC` в начало порядка сортировки. На практике `ORDER BY weight() DESC` становится вторичным критерием разрешения ничьих, а не глобальной сортировкой по BM25.

### Доступные функции

- [`hybrid_score()`](../Functions/Other_functions.md#HYBRID_SCORE%28%29) — оценка слияния RRF (доступна только в гибридных запросах)
- [`weight()`](../Functions/Searching_and_ranking_functions.md#WEIGHT%28%29) — оценка текстового соответствия BM25
- [`knn_dist()`](../Functions/Other_functions.md#KNN_DIST%28%29) — векторное расстояние (минимум по всем KNN подзапросам, если их несколько)

### Опции

| Опция | Тип | По умолчанию | Описание |
|--------|------|---------|-------------|
| [`fusion_method`](../Searching/Options.md#fusion_method) | строка | *(нет)* | Установите в `'rrf'`, чтобы включить гибридный поиск. Обязательно. |
| [`rank_constant`](../Searching/Options.md#rank_constant) | целое число | 60 | Константа сглаживания в формуле RRF |
| [`window_size`](../Searching/Options.md#window_size) | целое число | 0 (авто) | Сколько результатов извлекает каждый подзапрос перед слиянием. При 0 вычисляется автоматически на основе KNN `k` (с передискретизацией) и `LIMIT` запроса |
| [`fusion_weights`](../Searching/Options.md#fusion_weights) | кортеж | *(все 1.0)* | Веса для каждого подзапроса при оценке RRF |

## Настройка rank_constant

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

## Фильтры атрибутов

Стандартные фильтры WHERE работают вместе с гибридным поиском. Фильтры применяются как к текстовым, так и к KNN подзапросам:

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

## Сортировка

По умолчанию результаты сортируются по `hybrid_score() DESC`. Вы можете переопределить это:

Этот раздел относится к истинным гибридным запросам, т.е. запросам с использованием `OPTION fusion_method='rrf'`. Без `fusion_method='rrf'` запрос, включающий `KNN(...)`, не объединяется и остается с приоритетом KNN, поэтому `ORDER BY weight() DESC` не дает глобально отсортированного по весу набора результатов.

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

## Несоответствующий текст

Если текстовый запрос не соответствует ни одному документу, только результаты KNN вносят вклад в оценку RRF:

```sql
SELECT id, hybrid_score() FROM t
WHERE match('xyznonexistent') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
OPTION fusion_method='rrf';
-- Returns results ranked purely by KNN rank
```

## Несколько KNN поисков

Один гибридный запрос может объединять текстовый поиск с несколькими KNN поисками по разным векторным атрибутам. Все они объединяются через RRF:

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

Несколько KNN поисков без `fusion_method` приводят к ошибке.

## Взвешенное слияние (fusion_weights)

По умолчанию все подзапросы вносят равный вклад (вес 1.0). Чтобы придать разную важность текстовому и KNN поискам, используйте [`fusion_weights`](../Searching/Options.md#fusion_weights) с явными псевдонимами:

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

### Правила псевдонимов

**SQL:**
- Используйте `AS alias` для `MATCH(...)` и `KNN(...)`, чтобы дать им имена. Неявных/псевдонимов по умолчанию нет.
- Пропущенные псевдонимы по умолчанию имеют вес 1.0.
- Ссылка на несуществующий псевдоним приводит к ошибке.

**JSON:**
- `"query"` — это фиксированный псевдоним для полнотекстового подзапроса.
- Псевдонимы KNN устанавливаются через свойство `"name"` для каждой записи KNN.
- Запись KNN с именем `"query"` конфликтует с текстовым псевдонимом и приводит к ошибке.
- Неявные псевдонимы (имена полей без явного `"name"`) не поддерживаются в `fusion_weights`.

### Частичные веса

Вы можете указать веса только для некоторых подзапросов; остальные по умолчанию будут иметь вес 1.0:

```sql
-- Only boost text, KNN searches default to weight 1.0
SELECT id, hybrid_score()
FROM t
WHERE match('machine learning') AS text
  AND knn(vec1, (0.1, 0.1, 0.1, 0.1)) AS dense1
  AND knn(vec2, (1.0, 0.0, 0.0, 0.0)) AS dense2
OPTION fusion_method='rrf', fusion_weights=(text=2.0);
```

## Упрощенный синтаксис: hybrid_match()

Для таблиц с настроенными [авто-эмбеддингами](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29) на атрибуте float_vector, `hybrid_match()` предоставляет сокращенную запись, которая автоматически запускает как текстовый, так и KNN поиск из одной строки запроса:

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

`hybrid_match()` автоматически:
1. Выполняет текстовый запрос как полнотекстовый поиск BM25
2. Генерирует эмбеддинг из той же текстовой строки
3. Выполняет KNN поиск с использованием этого эмбеддинга
4. Объединяет результаты через RRF

**Требование**: Векторный атрибут должен иметь настроенные `model_name` и `from` для [авто-эмбеддингов](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29). Без них `hybrid_match()` возвращает ошибку.

## Упрощенный синтаксис JSON: hybrid

<!-- example hybrid_json -->

Для таблиц с авто-эмбеддингами свойство `"hybrid"` предоставляет сокращенную запись в JSON:

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

Свойство `"hybrid"` не может использоваться вместе с `"knn"`.

## Авто-эмбеддинг KNN в гибридных запросах

Когда векторный атрибут имеет [авто-эмбеддинги](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29), вы можете использовать `"query"` (строка) вместо `"query_vector"` (массив) в объекте `knn`:

```json
POST /search
{
  "table": "ht",
  "knn": { "field": "vec", "query": "machine learning", "k": 5 },
  "query": { "match": { "title": "machine learning" } },
  "options": { "fusion_method": "rrf" }
}
```

Строка автоматически преобразуется в эмбеддинг во время выполнения запроса. Без настроенных авто-эмбеддингов это возвращает ошибку.

## Архитектура выполнения

Внутренне гибридный запрос разделяется на N+1 параллельных подзапросов:

1. Задача 0: Полнотекстовый (BM25) подзапрос (пропускается, если текстовый запрос пуст, чтобы избежать загрязнения RRF результатами полного сканирования)
2. Задачи 1..N: Один KNN подзапрос на каждую запись `knn(...)`

Все подзапросы выполняются параллельно. После завершения всех, RRF-слияние:
1. Собирает ранжированные результаты из каждого подзапроса
2. Для каждого документа накапливает вклады в оценку RRF от каждого подзапроса, в котором он появляется
3. Сортирует по объединенной оценке RRF по убыванию
4. Устанавливает [`knn_dist()`](../Functions/Other_functions.md#KNN_DIST%28%29) равной минимальному расстоянию по всем KNN подзапросам для каждого документа
5. Сохраняет [`weight()`](../Functions/Searching_and_ranking_functions.md#WEIGHT%28%29) из текстового подзапроса

<!-- proofread -->
