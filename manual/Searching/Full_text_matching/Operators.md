# Full text operators

The query string can include specific operators that define the conditions for how the words from the query string should be matched.

### Boolean operators

#### AND operator

An implicit `AND` operator is always present, so "hello world" implies that both "hello" and "world" must be found in the matching document.

```sql
hello  world
```

#### OR operator

The `OR` operator has a higher precedence than AND, so `looking for cat | dog | mouse` means `looking for (cat | dog | mouse)` rather than `(looking for cat) | dog | mouse`.

```sql
hello | world
```

### MAYBE operator

```sql
hello MAYBE world
```

The `MAYBE` operator functions similarly to the `|` operator, but it does not return documents that match only the right subtree expression.

### Negation operator

```sql
hello -world
hello !world
```

The negation operator enforces a rule for a word to not exist.

Queries containing **only** negations are **not** supported by default. To enable, use the server option [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed).

### Field search operator

```sql
@title hello @body world
```

The field limit operator restricts subsequent searches to a specified field. By default, the query will fail with an error message if the given field name does not exist in the searched table. However, this behavior can be suppressed by specifying the `@@relaxed` option at the beginning of the query:

```sql
@@relaxed @nosuchfield my query
```

This can be useful when searching through heterogeneous tables with different schemas.

Field position limits additionally constrain the search to the first N positions within a given field (or fields). For example, `@body [50] hello` will not match documents where the keyword `hello` appears at position 51 or later in the body.

```sql
@body[50] hello
```

Multiple-field search operator:

```sql
@(title,body) hello world
```

Ignore field search operator (ignores any matches of 'hello world' from the 'title' field):

```sql
@!title hello world
```

Ignore multiple-field search operator (if there are fields 'title', 'subject', and 'body', then `@!(title)` is equivalent to `@(subject,body)`):

```sql
@!(title,body) hello world
```

All-field search operator:

```sql
@* hello
```

### Phrase search operator

```sql
"hello world"
```

The phrase operator mandates that the words be adjacent to each other.

The phrase search operator can incorporate a `match any term` modifier. Within the phrase operator, terms are positionally significant. When the 'match any term' modifier is employed, the positions of the subsequent terms in that phrase query will be shifted. As a result, the 'match any' modifier does not affect search performance.

```sql
"exact * phrase * * for terms"
```

###  Proximity search operator

```sql
"hello world"~10
```

Proximity distance is measured in words, accounting for word count, and applies to all words within quotes. For example, the query `"cat dog mouse"~5` indicates that there must be a span of fewer than 8 words containing all 3 words. Therefore, a document with `CAT aaa bbb ccc DOG eee fff MOUSE` will not match this query, as the span is exactly 8 words long.

###  Quorum matching operator

```sql
"the world is a wonderful place"/3
```

The quorum matching operator introduces a type of fuzzy matching. It will match only those documents that meet a given threshold of specified words. In the example above (`"the world is a wonderful place"/3`), it will match all documents containing at least 3 of the 6 specified words. The operator is limited to 255 keywords. Instead of an absolute number, you can also provide a value between 0.0 and 1.0 (representing 0% and 100%), and Manticore will match only documents containing at least the specified percentage of given words. The same example above could also be expressed as `"the world is a wonderful place"/0.5`, and it would match documents with at least 50% of the 6 words.

### Strict order operator

```sql
aaa << bbb << ccc
```

The strict order operator (also known as the "before" operator) matches a document only if its argument keywords appear in the document precisely in the order specified in the query. For example, the query `black << cat` will match the document "black and white cat" but not the document "that cat was black". The order operator has the lowest priority. It can be applied to both individual keywords and more complex expressions. For instance, this is a valid query:

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### Exact form modifier

```sql
raining =cats and =dogs
="exact phrase"
```

The exact form keyword modifier matches a document only if the keyword appears in the exact form specified. By default, a document is considered a match if the stemmed/lemmatized keyword matches. For instance, the query "runs" will match both a document containing "runs" and one containing "running", because both forms stem to just "run". However, the `=runs` query will only match the first document. The exact form operator requires the [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) option to be enabled.

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

The inline operators require `dict=keywords` and infixing enabled.


### Field-start and field-end modifier

```sql
^hello world$
```

Field-start and field-end keyword modifiers ensure that a keyword only matches if it appears at the very beginning or the very end of a full-text field, respectively. For example, the query `"^hello world$"` (enclosed in quotes to combine the phrase operator with the start/end modifiers) will exclusively match documents containing at least one field with these two specific keywords.

### IDF boost modifier

```sql
boosted^1.234 boostedfieldend$^1.234
```

The boost modifier raises the word IDF score by the indicated factor in ranking scores that incorporate IDF into their calculations. It does not impact the matching process in any manner.

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
The `SENTENCE` and `PARAGRAPH` operators match a document when both of their arguments are within the same sentence or the same paragraph of text, respectively. These arguments can be keywords, phrases, or instances of the same operator.

The order of the arguments within the sentence or paragraph is irrelevant. These operators function only with tables built with [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp) (sentence and paragraph indexing feature) enabled and revert to a simple AND operation otherwise. For information on what constitutes a sentence and a paragraph, refer to the [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp) directive documentation.


### ZONE limit operator

```sql
ZONE:(h3,h4)

only in these titles
```

The `ZONE limit` operator closely resembles the field limit operator but limits matching to a specified in-field zone or a list of zones. It is important to note that subsequent subexpressions do not need to match within a single continuous span of a given zone and may match across multiple spans. For example, the query `(ZONE:th hello world)` will match the following sample document:

```html
<th>Table 1. Local awareness of Hello Kitty brand.</th>
.. some table data goes here ..
<th>Table 2. World-wide brand awareness.</th>
```

The `ZONE` operator influences the query until the next field or `ZONE` limit operator, or until the closing parenthesis. It functions exclusively with tables built with zone support (refer to [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)) and will be disregarded otherwise.

### ZONESPAN limit operator

```sql
ZONESPAN:(h2)

only in a (single) title
```

The `ZONESPAN` limit operator resembles the `ZONE` operator but mandates that the match occurs within a single continuous span. In the example provided earlier, `ZONESPAN:th hello world` would not match the document, as "hello" and "world" do not appear within the same span.

<!-- proofread -->