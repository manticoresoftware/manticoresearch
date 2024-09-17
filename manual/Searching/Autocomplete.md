# Autocomplete

Autocomplete (or word completion) predicts and suggests the rest of a word or phrase as a user types. This feature is commonly used in:

- Search boxes on websites
- Query suggestions in search engines
- Text input fields in various applications

Manticore offers a powerful autocomplete feature that functions similarly to Google's autocomplete. It provides suggestions as users type, improving the search experience and helping users find relevant information quickly.

In addition to basic autocomplete functionality, Manticore includes advanced features to enhance the user experience:

1. Spell Correction (Fuzziness): Manticore's autocomplete incorporates spell correction capabilities, allowing it to suggest correct spellings for misspelled queries. This feature uses fuzzy matching algorithms to identify and correct common typos and spelling errors, ensuring users can find what they're looking for even if they make mistakes while typing.

2. Keyboard Layout Autodetection: Manticore employs intelligent logic to automatically detect the user's keyboard layout. This feature is particularly useful for multilingual environments or when users accidentally type in the wrong language. For example, if a user types "ghbdtn" (which is "привет" or "hello" in Russian, typed using an English keyboard layout), Manticore can recognize the intent and suggest the correct word in the appropriate language.

These advanced features make Manticore's autocomplete system more robust and user-friendly, catering to a diverse user base and improving overall search accuracy and efficiency.

Key benefits of autocomplete:

1. Faster query input
2. Reduced typing errors
3. Improved search accuracy
4. Enhanced user experience

Manticore's autocomplete can be customized to fit specific use cases and data sets, making it a versatile tool for various applications.

![Autocomplete](autocomplete.png)

## CALL AUTOCOMPLETE

The primary method for implementing autocomplete in Manticore is the `CALL AUTOCOMPLETE` function. This powerful feature provides intelligent word completion suggestions based on your indexed data.

Before you proceed, ensure that the table you try to use for autocomplete has [infixes](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) enabled.

*Attention:*

We employ smart autochecking for `min_infix_len` in table options, with results cached for 30 seconds. After altering your table, a brief delay may occur (usually unnoticeable). Note that we only cache positive results, so issues may arise when removing or dropping min_infix_len.

### Syntax

```sql
CALL AUTOCOMPLETE('word', 'table', [...options]);
```

Or JSON request with fields "query", "table" and optional "options" sent to `/autocomplete` endpoint.

### Parameters

- `word`: The partial word or phrase to autocomplete
- `table`: The name of the table to use for autocomplete suggestions
- `options`: Additional parameters to customize the autocomplete behavior

### Options

- `layouts`: A comma-separated string of keyboard layout codes to validate and check for spell correction. Available options: us, ru, ua, se, pt, no, it, gr, uk, fr, es, dk, de, ch, br, bg, be. Default: all enabled
- `fuzziness`: 0, 1, or 2 (default: 2). Maximum Levenshtein distance for finding typos. Set to 0 to disable fuzzy matching
- `prepend`: Boolean. If true, adds an asterisk before the last word for prefix expansion (e.g., *word)
- `append`: Boolean. If true, adds an asterisk after the last word for suffix expansion (e.g., word*)
- `expansion_len`: Number of characters to expand in the last word. Default: 10

### Examples

Basic usage:

```sql
mysql> CALL AUTOCOMPLETE('hello', 'comment');
+------------+
| query      |
+------------+
| hello      |
| helio      |
| hell       |
| shell      |
| nushell    |
| powershell |
| well       |
| help       |
+------------+
```

Disabling fuzzy matching:

```sql
mysql> CALL AUTOCOMPLETE('hello', 'comment', 0 as fuzziness);
+-------+
| query |
+-------+
| hello |
+-------+
```

Using JSON request:

```json
{
	"table":"comment",
	"query":"hello"
}
```

```bash
$ curl -sd @request.json localhost:9308/autocomplete | jq
[
  {
    "total": 8,
    "error": "",
    "warning": "",
    "columns": [
      {
        "query": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "query": "hello"
      },
      {
        "query": "helio"
      },
      {
        "query": "hell"
      },
      {
        "query": "shell"
      },
      {
        "query": "nushell"
      },
      {
        "query": "powershell"
      },
      {
        "query": "well"
      },
      {
        "query": "help"
      }
    ]
  }
]
```

## Alternative autocomplete with CALL KEYWORDS in controllable way

While `CALL AUTOCOMPLETE` is the recommended method for most use cases, Manticore also supports other controllable and customizable approaches to implement autocomplete functionality:

##### Autocomplete a sentence
To autocomplete a sentence, you can use [infixed search](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len). You can find endings of a document's field by providing its beginning and:

* using [full-text operators](../Searching/Full_text_matching/Operators.md) `*` to match anything it substitutes
* and optionally `^` to start from the beginning of the field
* and perhaps `""` for phrase matching
* and optionally [highlight the results](../Searching/Highlighting.md) so you don't have to fetch them in full to your application

There is an [article about it in our blog](https://manticoresearch.com/2020/03/31/simple-autocomplete-with-manticore/) and an [interactive course](../Searching/Spell_correction.md#Interactive-course). A quick example is:

* Let's assume you have a document: `My cat loves my dog. The cat (Felis catus) is a domestic species of small carnivorous mammal.`
* Then you can use `^`, `""`, and `*` so as the user is typing, you make queries like: `^"m*"`, `^"my *"`, `^"my c*"`, `^"my ca*"` and so on
* It will find the document, and if you also do [highlighting](../Searching/Highlighting.md), you will get something like: `<b>My cat</b> loves my dog. The cat ( ...`

##### Autocomplete a word
In some cases, all you need is to autocomplete a single word or a couple of words. In this case, you can use `CALL KEYWORDS`.

### CALL KEYWORDS
`CALL KEYWORDS` is available through the SQL interface and offers a way to examine how keywords are tokenized or to obtain the tokenized forms of specific keywords. If the table enables [infixes](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len), it allows you to quickly find possible endings for given keywords, making it suitable for autocomplete functionality.

This is a great alternative to general infixed search, as it provides higher performance since it only needs the table's dictionary, not the documents themselves.

### General syntax
<!-- example keywords -->
```sql
CALL KEYWORDS(text, table [, options])
```
The `CALL KEYWORDS` statement divides text into keywords. It returns the tokenized and normalized forms of the keywords, and if desired, keyword statistics. Additionally, it provides the position of each keyword in the query and all forms of tokenized keywords when the table enables [lemmatizers](../Creating_a_table/NLP_and_tokenization/Morphology.md).

| Parameter | Description |
| - | - |
| text | Text to break down to keywords |
| table | Name of the table from which to take the text processing settings |
| 0/1 as stats | Show statistics of keywords, default is 0 |
| 0/1 as fold_wildcards | Fold wildcards, default is 0 |
| 0/1 as fold_lemmas | Fold morphological lemmas, default is 0 |
| 0/1 as fold_blended | Fold blended words, default is 0 |
| N as expansion_limit | Override [expansion_limit](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expansion_limit) defined in the server configuration, default is 0 (use value from the configuration) |
| docs/hits as sort_mode | Sort output results by either 'docs' or 'hits'. Default no sorting |

The examples show how it works if assuming the user is trying to get an autocomplete for "my cat ...". So on the application side all you need to do is to suggest the user the endings from the column "normalized" for each new word. It often makes sense to sort by hits or docs using `'hits' as sort_mode` or `'docs' as sort_mode`.

<!-- intro -->
##### Examples:

<!-- request Examples -->

```sql
MySQL [(none)]> CALL KEYWORDS('m*', 't', 1 as stats);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | m*        | my         | 1    | 2    |
| 1    | m*        | mammal     | 1    | 1    |
+------+-----------+------------+------+------+

MySQL [(none)]> CALL KEYWORDS('my*', 't', 1 as stats);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | my*       | my         | 1    | 2    |
+------+-----------+------------+------+------+

MySQL [(none)]> CALL KEYWORDS('c*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+-------------+------+------+
| qpos | tokenized | normalized  | docs | hits |
+------+-----------+-------------+------+------+
| 1    | c*        | cat         | 1    | 2    |
| 1    | c*        | carnivorous | 1    | 1    |
| 1    | c*        | catus       | 1    | 1    |
+------+-----------+-------------+------+------+

MySQL [(none)]> CALL KEYWORDS('ca*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+-------------+------+------+
| qpos | tokenized | normalized  | docs | hits |
+------+-----------+-------------+------+------+
| 1    | ca*       | cat         | 1    | 2    |
| 1    | ca*       | carnivorous | 1    | 1    |
| 1    | ca*       | catus       | 1    | 1    |
+------+-----------+-------------+------+------+

MySQL [(none)]> CALL KEYWORDS('cat*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | cat*      | cat        | 1    | 2    |
| 1    | cat*      | catus      | 1    | 1    |
+------+-----------+------------+------+------+
```
<!-- end -->
<!-- example bigram -->
There is a nice trick how you can improve the above algorithm - use [bigram_index](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index). When you have it enabled for the table what you get in it is not just a single word, but each pair of words standing one after another indexed as a separate token.

This allows to predict not just the current word's ending, but the next word too which is especially beneficial for the purpose of autocomplete.

<!-- intro -->
##### Examples:

<!-- request Examples -->

```sql
MySQL [(none)]> CALL KEYWORDS('m*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | m*        | my         | 1    | 2    |
| 1    | m*        | mammal     | 1    | 1    |
| 1    | m*        | my cat     | 1    | 1    |
| 1    | m*        | my dog     | 1    | 1    |
+------+-----------+------------+------+------+

MySQL [(none)]> CALL KEYWORDS('my*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | my*       | my         | 1    | 2    |
| 1    | my*       | my cat     | 1    | 1    |
| 1    | my*       | my dog     | 1    | 1    |
+------+-----------+------------+------+------+

MySQL [(none)]> CALL KEYWORDS('c*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+--------------------+------+------+
| qpos | tokenized | normalized         | docs | hits |
+------+-----------+--------------------+------+------+
| 1    | c*        | cat                | 1    | 2    |
| 1    | c*        | carnivorous        | 1    | 1    |
| 1    | c*        | carnivorous mammal | 1    | 1    |
| 1    | c*        | cat felis          | 1    | 1    |
| 1    | c*        | cat loves          | 1    | 1    |
| 1    | c*        | catus              | 1    | 1    |
| 1    | c*        | catus is           | 1    | 1    |
+------+-----------+--------------------+------+------+

MySQL [(none)]> CALL KEYWORDS('ca*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+--------------------+------+------+
| qpos | tokenized | normalized         | docs | hits |
+------+-----------+--------------------+------+------+
| 1    | ca*       | cat                | 1    | 2    |
| 1    | ca*       | carnivorous        | 1    | 1    |
| 1    | ca*       | carnivorous mammal | 1    | 1    |
| 1    | ca*       | cat felis          | 1    | 1    |
| 1    | ca*       | cat loves          | 1    | 1    |
| 1    | ca*       | catus              | 1    | 1    |
| 1    | ca*       | catus is           | 1    | 1    |
+------+-----------+--------------------+------+------+

MySQL [(none)]> CALL KEYWORDS('cat*', 't', 1 as stats, 'hits' as sort_mode);
+------+-----------+------------+------+------+
| qpos | tokenized | normalized | docs | hits |
+------+-----------+------------+------+------+
| 1    | cat*      | cat        | 1    | 2    |
| 1    | cat*      | cat felis  | 1    | 1    |
| 1    | cat*      | cat loves  | 1    | 1    |
| 1    | cat*      | catus      | 1    | 1    |
| 1    | cat*      | catus is   | 1    | 1    |
+------+-----------+------------+------+------+
```
<!-- end -->

`CALL KEYWORDS` supports distributed tables so no matter how big your data set you can benefit from using it.
<!-- proofread -->
