# Spell correction

Spell correction also known as:

* Auto correction
* Text correction
* Fixing a spelling error
* Typo tolerance
* "Did you mean?"

and so on is a software functionality that suggests you alternatives to or makes automatic corrections of the text you have typed in. The concept of correcting typed text dates back to the 1960s, when a computer scientist named Warren Teitelman who also invented the "undo" command came up with a philosophy of computing called D.W.I.M., or "Do What I Mean." Rather than programming computers to accept only perfectly formatted instructions, Teitelman said we should program them to recognize obvious mistakes.

The first well known product which provided spell correction functionality was Microsoft Word 6.0 released in 1993.

### How it works
There are few ways how spell correction can be done, but the important thing is that there is no purely programmatic way which will convert your mistyped "ipone" into "iphone" (at least with decent quality). Mostly there has to be a data set the system is based on. The data set can be:

* A dictionary of properly spelled words. In its turn it can be:
  * Based on your real data. The idea here is that mostly in the dictionary made up from your data the spelling is correct and then for each typed word the system just tries to find a word which is most similar to that (we'll speak about how exactly it can be done with Manticore shortly)
  * Or can be based on an external dictionary which has nothing to do with your data. The problem which may arise here is that your data and the external dictionary can be too different: some words may be missing in the dictionary, some words may be missing in your data.
* Not just dictionary-based, but context-aware, e.g. "white ber" would be corrected to "white bear" while "dark ber" - to "dark beer". The context may be not just a neighbour word in your query, but your location, date of time, current sentence's grammar (to let's say change "there" to "their" or not), your search history and virtually any other things that can affect your intent.
* Another classical approach is to use previous search queries as the data set for spell correction. It's even more utilized in [autocomplete](../Searching/Autocomplete.md) functionality, but makes sense for autocorrect too. The idea here is that mostly users are right with spelling, therefore we can use words from their search history as a source of truth even if we don't have the words in our documents nor we use an external dictionary. Context-awareness is also possible here.


Manticore provides commands `CALL QSUGGEST` and `CALL SUGGEST` that can be used for the purpose of automatic spell correction.

### CALL QSUGGEST, CALL SUGGEST

They are both available via SQL only and the general syntax is:
```sql
CALL QSUGGEST(word, table [,options])
CALL SUGGEST(word, table [,options])

options: N as option_name[, M as another_option, ...]
```

These commands provide for a given word all suggestions from the dictionary. They work only on tables with [infixing](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) enabled and [dict=keywords](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict). They return the suggested keywords, Levenshtein distance between the suggested and original keywords and the docs statistics of the suggested keyword.

If the first parameter is not a single word, but multiple, then:
* `CALL QSUGGEST` will return suggestions only for the **last** word, ignoring the rest
* `CALL SUGGEST` will return suggestions only for the **first** word

That's the only difference between them. Several options are supported for customization:

| Option | Description | Default |
| - | - | - |
| limit | Returns N top matches | 5 |
| max_edits | Keeps only dictionary words which Levenshtein distance is less than or equal to N | 4 |
| result_stats | Provides Levenshtein distance and document count of the found words | 1 (enabled) |
| delta_len | Keeps only dictionary words whose length difference is less than N | 3 |
| max_matches | Number of matches to keep | 25 |
| reject | Rejected words are matches that are not better than those already in the match queue. They are put in a rejected queue that gets reset in case one actually can go in the match queue. This parameter defines the size of the rejected queue (as reject*max(max_matched,limit)). If the rejected queue is filled, the engine stops looking for potential matches | 4 |
| result_line | alternate mode to display the data by returning all suggests, distances and docs each per one row | 0 |
| non_char | do not skip dictionary words with non alphabet symbols | 0 (skip such words) |

To show how it works let's create a table and add few documents into it.

```sql
create table products(title text) min_infix_len='2';
insert into products values (0,'Crossbody Bag with Tassel'), (0,'microfiber sheet set'), (0,'Pet Hair Remover Glove');
```
<!-- example single -->
##### Single word example
As you can see we have a mistype in "crossb**U**dy" which gets corrected to the "crossbody". In addition to that by default `CALL SUGGEST/QSUGGEST` return:

* `distance` - the Levenshtein distance which means how many edits they had to make to convert the given word to the suggestion
* `docs` - and the number of docs that have this word

To disable these stats display you can use option `0 as result_stats`.


<!-- intro -->
##### Example:

<!-- request Example -->

```sql
call suggest('crossbudy', 'products');
```
<!-- response Example -->

```sql
+-----------+----------+------+
| suggest   | distance | docs |
+-----------+----------+------+
| crossbody | 1        | 1    |
+-----------+----------+------+
```
<!-- end -->
<!-- example first -->
##### CALL SUGGEST takes only the first word
If the first parameter is not a single word, but multiple, then `CALL SUGGEST` will return suggestions only for the first word.


<!-- intro -->
##### Example:

<!-- request Example -->

```sql
call suggest('bagg with tasel', 'products');
```
<!-- response Example -->

```sql
+---------+----------+------+
| suggest | distance | docs |
+---------+----------+------+
| bag     | 1        | 1    |
+---------+----------+------+
```
<!-- end -->
<!-- example last -->
##### CALL QSUGGEST takes only the last word
If the first parameter is not a single word, but multiple, then `CALL SUGGEST` will return suggestions only for the last word.


<!-- intro -->
##### Example:

<!-- request Example -->

```sql
CALL QSUGGEST('bagg with tasel', 'products');
```
<!-- response Example -->

```sql
+---------+----------+------+
| suggest | distance | docs |
+---------+----------+------+
| tassel  | 1        | 1    |
+---------+----------+------+
```
<!-- end -->
<!-- example result_line -->
##### Different display mode
Using `1 as result_line` in the options turns on alternate mode to display the data by returning all suggests, distances and docs each per one row.

<!-- intro -->
##### Example:

<!-- request Example -->

```sql
CALL QSUGGEST('bagg with tasel', 'products', 1 as result_line);
```
<!-- response Example -->

```sql
+----------+--------+
| name     | value  |
+----------+--------+
| suggests | tassel |
| distance | 1      |
| docs     | 1      |
+----------+--------+
```
<!-- end -->

### Interactive course

[This interactive course](https://play.manticoresearch.com/didyoumean/) demonstrates online how it works on a web page and provides different examples.

![Typical flow with Manticore and a database](didyoumean.png){.scale-0.5}
