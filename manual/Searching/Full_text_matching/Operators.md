# Full text operators

The query string can contain certain operators that allow telling the conditions of how the words from the query string should be matched.

### Boolean operators

#### AND operator

There always is implicit `AND` operator, so "hello world" means that both "hello" and "world" must be present in matching document.

```sql
hello  world
```

#### OR operator

`OR` operator precedence is higher than AND, so `looking for cat | dog | mouse` means `looking for ( cat | dog | mouse )`
 and not `(looking for cat) | dog | mouse`.

```sql
hello | world
```

### MAYBE operator

```sql
hello MAYBE world
```

`MAYBE` operator works much like operator `|` but doesn't return documents which match only right subtree expression.

### Negation operator

```sql
hello -world
hello !world
```

The negation operator enforces a rule for a word to not exist.

Queries having **only** negations are **not** supported by default in Manticore Search. There's the server option [not_terms_only_allowed](../../Server_settings/Searchd.md#not_terms_only_allowed) to enable it.

### Field search operator

```sql
@title hello @body world
```

Field limit operator limits subsequent searching to a given field. Normally, query will fail with an error message if given field name does not exist in the searched table. However, that can be suppressed by specifying `@@relaxed` option at the very beginning of the query:

```sql
@@relaxed @nosuchfield my query
```

This can be helpful when searching through heterogeneous tables with different schemas.

Field position limit additionally restricts the searching to first N position within given field (or fields). For example, `@body [50] hello` will not match the documents where the keyword `hello` occurs at position 51 and below in the body.

```sql
@body[50] hello
```

Multiple-field search operator:

```sql
@(title,body) hello world
```

Ignore field search operator (will ignore any matches of 'hello world' from field 'title'):

```sql
@!title hello world
```

Ignore multiple-field search operator (if we have fields title,
    subject and body then `@!(title)` is equivalent to `@(subject,body)`):

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

The phrase operator requires the words to be next to each other.

The phrase search operator may include a `match any term` modifier. Terms within the phrase operator are position significant. When the 'match any term' modifier is implemented, the position of the subsequent terms from that phrase query will be shifted. Therefore, 'match any' has no impact on search performance.

```sql
"exact * phrase * * for terms"
```

###  Proximity search operator

```sql
"hello world"~10
```

Proximity distance is specified in words, adjusted for word count, and applies to all words within quotes. For instance, `"cat dog mouse"~5` query means that there must be less than 8-word span which contains all 3 words, ie. `CAT aaa bbb ccc DOG eee fff MOUSE` document will not match this query, because this span is exactly 8 words long.

###  Quorum matching operator

```sql
"the world is a wonderful place"/3
```

Quorum matching operator introduces a kind of fuzzy matching. It will only match those documents that pass a given threshold of given words. The example above (`"the world is a wonderful place"/3`) will match all documents that have at least 3 of the 6 specified words. Operator is limited to 255 keywords. Instead of an absolute number, you can also specify a number between 0.0 and 1.0 (standing for 0% and 100%), and Manticore will match only documents with at least the specified percentage of given words. The same example above could also have been written `"the world is a wonderful place"/0.5` and it would match documents with at least 50% of the 6 words.

### Strict order operator

```sql
aaa << bbb << ccc
```

Strict order operator (aka operator "before") will match the document only if its argument keywords occur in the document exactly in the query order. For instance, `black << cat` query (without quotes) will match the document "black and white cat" but not the "that cat was black" document. Order operator has the lowest priority. It can be applied both to just keywords and more complex expressions, ie. this is a valid query:

```sql
(bag of words) << "exact phrase" << red|green|blue
```

### Exact form modifier

```sql
raining =cats and =dogs
="exact phrase"
```

Exact form keyword modifier will match the document only if the keyword occurred in exactly the specified form. The default behaviour is to match the document if the stemmed/lemmatized keyword matches. For instance, "runs" query will match both the document that contains "runs" and the document that contains "running", because both forms stem to just "run" while `=runs` query will only match the first document. Exact form operator requires [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) option to be enabled.

Another use case is to avoid [expanding](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords) a keyword to its `*keyword*` form. I.e. with `index_exact_words=1` + `expand_keywords=1/star` `bcd` will find a document containing `abcde`, but `=bcd` will not.

This is a modifier that affects the keyword and thus can be used within operators such as phrase, proximity, and quorum operators. It is possible to apply an exact form modifier to the phrase operator. In this case it internally adds the exact form modifier to all terms in the phrase.

### Wildcard operators

```sql
nation* *nation* *national
```

Requires [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) for prefix (expansion in trail) and/or sufix (expansion in head). If only prefixing is wanted, [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) can be used instead.

The search will try to find all the expansions of the wildcarded tokens and each expansion is recorded as a matched hit. The number of expansions for a token can be controlled with [expansion_limit](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) table setting. Wildcarded tokens can have a big impact on the query search time, especially when tokens have short length. In such cases is desired to use the expansion limit.

The wildcard operator can be automatically applied if [expand_keywords](../../Searching/Options.md#expand_keywords) table setting is used.

In addition, the following inline wildcard operators are supported:

* `?` can match any(one) character: `t?st` will match `test`, but not `teast`
* `%` can match zero or one character : `tes%` will match `tes` or `test`, but not `testing`

The inline operators require `dict=keywords` and infixing enabled.


### Field-start and field-end modifier

```sql
^hello world$
```

Field-start and field-end keyword modifiers will make the keyword match only if it occurred at the very start or the very end of a fulltext field, respectively. For instance, the query `"^hello world$"` (with quotes and thus combining phrase operator and start/end modifiers) will only match documents that contain at least one field that has exactly these two keywords.

### IDF boost modifier

```sql
boosted^1.234 boostedfieldend$^1.234
```

The boost modifier increases the word IDF score by the specified factor in ranking scores that use IDF in their formula. It does not affect the matching in any way.  

### NEAR operator

```sql
hello NEAR/3 world NEAR/4 "my test"
```

Operator `NEAR` is a generalized version of the proximity operator. The syntax is `NEAR/N`, it is case-sensitive, and no spaces are allowed between the `NEAR` keywords, the slash sign, and the distance value.

The original proximity operator only works on sets of keywords. `NEAR` is more generic and can accept arbitrary subexpressions as its two arguments, matching the document when both subexpressions are found within N words of each other, no matter in which order. `NEAR` is left associative and has the same (lowest) precedence as [BEFORE](../../Searching/Full_text_matching/Operators.md#Strict-order-operator).

You should also note how `one NEAR/7 two NEAR/7 three` is not really equivalent to `"one two three"~7`. The difference here is that the proximity operator allows for up to 6 non-matching words between all the 3 matching words, but the version with `NEAR` is less restrictive: it would allow for up to 6 words between `one` and `two` and then for up to 6 more between that two-word matching and `three`.

### NOTNEAR operator

```sql
Church NOTNEAR/3 street
```
Operator `NOTNEAR` is a negative assertion. It matches the document when left argument exists and either there is no right argument in document or right argument is distance away from left matched argument's end. The distance is specified in words. The syntax is `NOTNEAR/N`, it is case-sensitive, and no spaces are allowed between the `NOTNEAR` keyword, the slash sign, and the distance value. Both arguments of this operator might be terms or any operators or group of operators.

### SENTENCE and PARAGRAPH operators

```sql
all SENTENCE words SENTENCE "in one sentence"
```


```sql
"Bill Gates" PARAGRAPH "Steve Jobs"
```
`SENTENCE` and `PARAGRAPH` operators matches the document when both its arguments are within the same sentence or the same paragraph of text, respectively. The arguments can be either keywords, or phrases, or the instances of the same operator.

The order of the arguments within the sentence or paragraph does not matter. These operators only work on tables built with [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp) (sentence and paragraph indexing feature) enabled, and revert to a mere AND otherwise. Refer to the [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp) directive documentation for the notes on what's considered a sentence and a paragraph.


### ZONE limit operator

```sql
ZONE:(h3,h4)

only in these titles
```

`ZONE limit` operator is quite similar to field limit operator, but restricts matching to a given in-field zone or a list of zones. Note that the subsequent subexpressions are not required to match in a single contiguous span of a given zone, and may match in multiple spans. For instance, `(ZONE:th hello world)` query will match this example document:

```html
<th>Table 1. Local awareness of Hello Kitty brand.</th>
.. some table data goes here ..
<th>Table 2. World-wide brand awareness.</th>
```

`ZONE` operator affects the query until the next field or `ZONE` limit operator, or the closing parenthesis. It only works on the tables built with zones support (see [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)) and will be ignored otherwise.

### ZONESPAN limit operator

```sql
ZONESPAN:(h2)

only in a (single) title
```

`ZONESPAN` limit operator is similar to the `ZONE` operator, but requires the match to occur in a single contiguous span. In the example above, `ZONESPAN:th hello world` would not match the document, since "hello" and "world" do not occur within the same span.
