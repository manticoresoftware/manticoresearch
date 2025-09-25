# Полнотекстовые операторы

Строка запроса может включать конкретные операторы, которые определяют условия того, как слова из строки запроса должны сопоставляться.

### Булевы операторы

#### Оператор AND

Неявный логический оператор AND всегда присутствует, поэтому "hello world" означает, что должны быть найдены одновременно и "hello", и "world" в совпадающем документе.

```sql
hello  world
```

Примечание: Явного оператора `AND` нет.

#### Оператор OR

Логический оператор OR `|` имеет более высокий приоритет, чем AND, поэтому `looking for cat | dog | mouse` означает `looking for (cat | dog | mouse)`, а не `(looking for cat) | dog | mouse`.

```sql
hello | world
```

Примечание: Оператора `OR` нет. Пожалуйста, используйте вместо него `|`.

### Оператор MAYBE

```sql
hello MAYBE world
```

Оператор `MAYBE` функционирует аналогично оператору `|`, но не возвращает документы, которые совпадают только с выражением правого поддерева.

### Оператор отрицания

```sql
hello -world
hello !world
```

Оператор отрицания задаёт правило, при котором слово не должно присутствовать.

Запросы, содержащие **только** отрицания, **по умолчанию** не поддерживаются. Для включения этой возможности используйте параметр сервера [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed).

### Оператор поиска по полю

```sql
@title hello @body world
```

Оператор ограничения поля ограничивает последующие поиски указанным полем. По умолчанию запрос завершится ошибкой, если указанное имя поля отсутствует в искомой таблице. Однако это поведение можно подавить, указав опцию `@@relaxed` в начале запроса:

```sql
@@relaxed @nosuchfield my query
```

Это полезно при поиске по неоднородным таблицам с разными схемами.

Ограничения позиции поля дополнительно сужают поиск до первых N позиций в указанном поле (или полях). Например, `@body [50] hello` не совпадёт с документами, где ключевое слово `hello` встречается на 51-й или более поздней позиции в тексте.

```sql
@body[50] hello
```

Оператор поиска по нескольким полям:

```sql
@(title,body) hello world
```

Оператор игнорирования поиска по полю (игнорирует любые совпадения 'hello world' из поля 'title'):

```sql
@!title hello world
```

Оператор игнорирования поиска по нескольким полям (если есть поля 'title', 'subject' и 'body', то `@!(title)` эквивалентен `@(subject,body)`):

```sql
@!(title,body) hello world
```

Оператор поиска по всем полям:

```sql
@* hello
```

### Оператор поиска фразы

```sql
"hello world"
```

Оператор фразы требует, чтобы слова шли подряд друг за другом.

Оператор поиска фразы может содержать модификатор `match any term` (совпадение с любым термином). Внутри оператора фразы слова значимы по позициям. При применении модификатора 'match any term' позиции последующих слов в этой фразе будут смещены. В результате модификатор 'match any' не влияет на производительность поиска.

```sql
"exact * phrase * * for terms"
```

Вы также можете использовать оператор OR внутри кавычек. Оператор OR (`|`) должен быть заключён в скобки `()`, когда используется внутри фраз. Каждая опция проверяется на одной позиции, и фраза совпадает, если любая из опций подходит для этой позиции.

**Правильные примеры** (со скобками):
```sql
"( a | b ) c"
"( ( a b c ) | d ) e"
"man ( happy | sad ) but all ( ( as good ) | ( as fast ) )"
```

**Неправильные примеры** (без скобок — работать не будут):
```sql
"a | b c"
"happy | sad"
```

###  Оператор поиска по близости

```sql
"hello world"~10
```

Расстояние близости измеряется в словах, учитывая количество слов, и применяется ко всем словам в кавычках. Например, запрос `"cat dog mouse"~5` означает, что должен быть интервал менее 8 слов, содержащий все 3 слова. Следовательно, документ с `CAT aaa bbb ccc DOG eee fff MOUSE` не совпадёт с этим запросом, так как интервал ровно 8 слов.

Вы также можете использовать оператор OR внутри поиска по близости. Оператор OR (`|`) должен быть заключён в скобки `()`, когда используется внутри поиска по близости. Каждая опция проверяется отдельно.

**Правильный пример** (со скобками):
```sql
"( two | four ) fish chips"~5
```

**Неправильный пример** (без скобок — не будет работать):
```sql
"two | four fish chips"~5
```

###  Оператор кворума

```sql
"the world is a wonderful place"/3
```

Оператор кворума вводит тип нечеткого совпадения. Он совпадает только с теми документами, которые соответствуют заданному порогу из указанных слов. В приведённом выше примере (`"the world is a wonderful place"/3`) он совпадёт со всеми документами, содержащими не менее 3 из 6 заданных слов. Оператор ограничен 255 ключевыми словами. Вместо абсолютного числа можно указать значение от 0.0 до 1.0 (от 0% до 100%), и Manticore совпадёт только с документами, содержащими как минимум указанный процент от данных слов. Тот же пример можно записать как `"the world is a wonderful place"/0.5`, и он совпадёт с документами, содержащими как минимум 50% из 6 слов.

Оператор кворума поддерживает оператор OR (`|`). Оператор OR (`|`) должен быть заключён в скобки `()` при использовании внутри кворума. Для совпадения считается только одно слово из каждой группы OR.

**Правильные примеры** (со скобками):
```sql
"( ( a b c ) | d ) e f g"/0.5
"happy ( sad | angry ) man"/2
```

**Неправильный пример** (без скобок — не будет работать):
```sql
"a b c | d e f g"/0.5
```

### Оператор строгого порядка

```sql
aaa << bbb << ccc
```

Оператор строгого порядка (также известный как оператор "before") совпадает с документом только если ключевые слова аргумента появляются в документе именно в том порядке, который задан в запросе. Например, запрос `black << cat` совпадёт с документом "black and white cat", но не с документом "that cat was black". Оператор порядка имеет самый низкий приоритет. Его можно применять как к отдельным ключевым словам, так и к более сложным выражениям. Например, это валидный запрос:

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### Модификатор точной формы

```sql
raining =cats and =dogs
="exact phrase"
```

Модификатор точной формы ключевого слова совпадает с документом только если ключевое слово встречается в точной форме, указанной в запросе. По умолчанию документ считается совпадающим, если совпадает слово после стемминга/лемматизации. Например, запрос "runs" совпадёт как с документом, содержащим "runs", так и с документом, содержащим "running", так как обе формы сводятся к "run". Однако запрос `=runs` совпадёт только с первым документом. Модификатор точной формы требует включённой опции [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words).

Another use case is to prevent [expanding](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords) a keyword to its `*keyword*` form. For example, with `index_exact_words=1` + `expand_keywords=1/star`, `bcd` will find a document containing `abcde`, but `=bcd` will not.

As a modifier affecting the keyword, it can be used within operators such as phrase, proximity, and quorum operators. Applying an exact form modifier to the phrase operator is possible, and in this case, it internally adds the exact form modifier to all terms in the phrase.

### Wildcard operators

```sql
nation* *nation* *national
```

Requires [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) for prefix (expansion in trail) and/or suffix (expansion in head). If only prefixing is desired, [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) can be used instead.

The search will attempt to find all expansions of the wildcarded tokens, and each expansion is recorded as a matched hit. The number of expansions for a token can be controlled with the [expansion_limit](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) table setting. Wildcarded tokens can have a significant impact on query search time, especially when tokens have short lengths. In such cases, it is desirable to use the expansion limit.

The wildcard operator can be automatically applied if the [expand_keywords](../../Searching/Options.md#expand_keywords) table setting is used.

In addition, the following inline wildcard operators are supported:

* `?` can match any single character: `t?st` will match `test`, but not `teast`
* `%` can match zero or one character: `tes%` will match `tes` or `test`, but not `testing`

The inline operators require `dict=keywords` (enabled by default) and prefixing/infixing enabled.

### REGEX operator

```sql
REGEX(/t.?e/)
```

Requires the [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) or [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) and [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords options to be set (which is a default).

Similarly to the [wildcard operators](../../Searching/Full_text_matching/Operators.md#Wildcard-operators), the REGEX operator attempts to find all tokens matching the provided pattern, and each expansion is recorded as a matched hit. Note, this can have a significant impact on query search time, as the entire dictionary is scanned, and every term in the dictionary undergoes matching with the REGEX pattern.

The patterns should adhere to the [RE2 syntax](https://github.com/google/re2/wiki/Syntax). The REGEX expression delimiter is the first symbol after the open bracket. In other words, all text between the open bracket followed by the delimiter and the delimiter and the closed bracket is considered as a RE2 expression.
Please note that the terms stored in the dictionary undergo `charset_table` transformation, meaning that for example, REGEX may not be able to match uppercase characters if all characters are lowercased according to the `charset_table` (which happens by default). To successfully match a term using a REGEX expression, the pattern must correspond to the entire token. To achieve partial matching, place `.*` at the beginning and/or end of your pattern.

```sql
REGEX(/.{3}t/)
REGEX(/t.*\d*/)
```

### Field-start and field-end modifier

```sql
^hello world$
```

Field-start and field-end keyword modifiers ensure that a keyword only matches if it appears at the very beginning or the very end of a full-text field, respectively. For example, the query `"^hello world$"` (enclosed in quotes to combine the phrase operator with the start/end modifiers) will exclusively match documents containing at least one field with these two specific keywords.

### IDF boost modifier

```sql
boosted^1.234 boostedfieldend$^1.234
```

The boost modifier raises the word [IDF](../../Searching/Options.md#idf)_score by the indicated factor in ranking scores that incorporate IDF into their calculations. It does not impact the matching process in any manner.

### NEAR operator

```sql
hello NEAR/3 world NEAR/4 "my test"
```

The `NEAR` operator is a more generalized version of the proximity operator. Its syntax is `NEAR/N`, which is case-sensitive and does not allow spaces between the `NEAR` keywords, slash sign, and distance value.

While the original proximity operator works only on sets of keywords, `NEAR` is more versatile and can accept arbitrary subexpressions as its two arguments. It matches a document when both subexpressions are found within N words of each other, regardless of their order. `NEAR` is left-associative and shares the same (lowest) precedence as [BEFORE](../../Searching/Full_text_matching/Operators.md#Strict-order-operator).

It is important to note that `one NEAR/7 two NEAR/7 three` is not exactly equivalent to `"one two three"~7`. The key difference is that the proximity operator allows up to 6 non-matching words between all three matching words, while the version with `NEAR` is less restrictive: it permits up to 6 words between `one` and `two`, and then up to 6 more between that two-word match and `three`.

### NOTNEAR operator

```sql
Church NOTNEAR/3 street
```
The `NOTNEAR` operator serves as a negative assertion. It matches a document when the left argument is present and either the right argument is absent from the document or the right argument is a specified distance away from the end of the left matched argument. The distance is denoted in words. The syntax is `NOTNEAR/N`, which is case-sensitive and does not permit spaces between the `NOTNEAR` keyword, slash sign, and distance value. Both arguments of this operator can be terms or any operators or group of operators.

### SENTENCE and PARAGRAPH operators

```sql
all SENTENCE words SENTENCE "in one sentence"
```


```sql
"Bill Gates" PARAGRAPH "Steve Jobs"
```
Операторы `SENTENCE` и `PARAGRAPH` соответствуют документу, когда оба их аргумента находятся в одном и том же предложении или в одном и том же абзаце текста, соответственно. Эти аргументы могут быть ключевыми словами, фразами или экземплярами того же оператора.

Порядок аргументов внутри предложения или абзаца не имеет значения. Эти операторы работают только с таблицами, построенными с включённой функцией [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp) (индексация по предложениям и абзацам), и в противном случае сводятся к простой операции AND. Для информации о том, что считается предложением и абзацем, обратитесь к документации директивы [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp).


### Оператор ограничения ZONE

```sql
ZONE:(h3,h4)

only in these titles
```

Оператор ограничения `ZONE` очень похож на оператор ограничения поля, но ограничивает совпадения указанной зоной внутри поля или списком зон. Важно отметить, что последующие подвыражения не обязаны совпадать в одном непрерывном фрагменте данной зоны и могут совпадать через несколько фрагментов. Например, запрос `(ZONE:th hello world)` будет соответствовать следующему примерному документу:

```html
<th>Table 1. Local awareness of Hello Kitty brand.</th>
.. some table data goes here ..
<th>Table 2. World-wide brand awareness.</th>
```

Оператор `ZONE` действует до следующего оператора ограничения поля или `ZONE`, либо до закрывающей скобки. Он работает исключительно с таблицами, построенными с поддержкой зон (см. [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)), и будет игнорироваться в противном случае.

### Оператор ограничения ZONESPAN

```sql
ZONESPAN:(h2)

only in a (single) title
```

Оператор ограничения `ZONESPAN` похож на оператор `ZONE`, но требует, чтобы совпадение происходило в одном непрерывном фрагменте. В приведённом ранее примере `ZONESPAN:th hello world` не будет соответствовать документу, так как слова "hello" и "world" не появляются в одном срезе.

<!-- proofread -->

