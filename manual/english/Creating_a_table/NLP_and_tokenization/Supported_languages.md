# Supported languages

Manticore supports a wide range of languages, with basic support enabled for most languages via `charset_table = non_cont` (which is the default value). The `non_cjk` option which is an alias for `non_cont` can be used as well: `charset_table = non_cjk`.

For many languages, Manticore provides a [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) file that can be used to improve search relevance.

Additionally, advanced morphology is available for a few languages that can significantly improve search relevance by using dictionary-based lemmatization or stemming algorithms for better segmentation and normalization.

The table below lists all supported languages and indicates how to enable:
* basic support (column "Supported")
* stopwords (column "Stopwords file name")
* advanced morphology (column "Advanced morphology")

| Language | Supported | Stopwords file name | Advanced morphology | Notes |
| - | - | - | - | - |
| Afrikaans | charset_table=non_cont | af | - | |
| Arabic | charset_table=non_cont | ar | morphology=stem_ar (Arabic stemmer); morphology=libstemmer_ar  | |
| Armenian | charset_table=non_cont | hy | - | |
| Assamese | specify charset_table specify charset_table manually | - | - | |
| Basque | charset_table=non_cont | eu | - | |
| Bengali | charset_table=non_cont | bn | - | |
| Bishnupriya | specify charset_table manually | - | - | |
| Buhid | specify charset_table manually | - | - | |
| Bulgarian | charset_table=non_cont | bg | - | |
| Catalan | charset_table=non_cont | ca | morphology=libstemmer_ca | |
| Chinese using [ICU](https://icu.unicode.org/) | charset_table=chinese | zh | morphology=icu_chinese | More accurate than using ngrams |
| Chinese using [Jieba](https://github.com/fxsjy/jieba) | charset_table=chinese | zh | morphology=jieba_chinese, requires package `manticore-language-packs` | More accurate than using ngrams |
| Chinese using ngrams| ngram_chars=chinese | zh | ngram_chars=1 | Faster indexing, but the search performance might not be as good |
| Croatian | charset_table=non_cont | hr | - | |
| Kurdish | charset_table=non_cont | ckb | - | |
| Czech | charset_table=non_cont | cz | morphology=stem_cz (Czech stemmer) | |
| Danish | charset_table=non_cont | da | morphology=libstemmer_da | |
| Dutch | charset_table=non_cont | nl | morphology=libstemmer_nl | |
| English | charset_table=non_cont | en | morphology=lemmatize_en (single root form); morphology=lemmatize_en_all (all root forms); morphology=stem_en (Porter's English stemmer); morphology=stem_enru (Porter's English and Russian stemmers); morphology=libstemmer_en (English from libstemmer)  | |
| Esperanto | charset_table=non_cont | eo | - | |
| Estonian | charset_table=non_cont | et | - | |
| Finnish | charset_table=non_cont | fi | morphology=libstemmer_fi | |
| French | charset_table=non_cont | fr | morphology=libstemmer_fr | |
| Galician | charset_table=non_cont | gl | - | |
| Garo | specify charset_table manually | - | - | |
| German | charset_table=non_cont | de | morphology=lemmatize_de (single root form); morphology=lemmatize_de_all (all root forms); morphology=lemmatize_de_v2 (single root form with sharp-s normalization); morphology=lemmatize_de_v2_all (all root forms with sharp-s normalization); morphology=libstemmer_de | Use charset_table=non_cont,german with the AOT v2 variants |
| Greek | charset_table=non_cont | el | morphology=libstemmer_el | |
| Hebrew | charset_table=non_cont | he | - | |
| Hindi | charset_table=non_cont | hi | morphology=libstemmer_hi | |
| Hmong | specify charset_table manually | - | - | |
| Ho | specify charset_table manually | - | - | |
| Hungarian | charset_table=non_cont | hu | morphology=libstemmer_hu | |
| Indonesian | charset_table=non_cont | id | morphology=libstemmer_id | |
| Irish | charset_table=non_cont | ga | morphology=libstemmer_ga | |
| Italian | charset_table=non_cont | it | morphology=libstemmer_it | |
| Japanese | ngram_chars=japanese | - | ngram_chars=japanese ngram_len=1 | Requires ngram-based segmentation |
| Komi | specify charset_table manually | - | - | |
| Korean | ngram_chars=korean | - | ngram_chars=korean ngram_len=1 | Requires ngram-based segmentation  |
| Large Flowery Miao | specify charset_table manually | - | - | |
| Latin | charset_table=non_cont | la | - | |
| Latvian | charset_table=non_cont | lv | - | |
| Lithuanian | charset_table=non_cont | lt | morphology=libstemmer_lt | |
| Maba | specify charset_table manually | - | - | |
| Maithili | specify charset_table manually | - | - | |
| Marathi | specify charset_table manually | - | - | |
| Marathi | charset_table=non_cont | mr | - | |
| Mende | specify charset_table manually | - | - | |
| Mru | specify charset_table manually | - | - | |
| Myene | specify charset_table manually | - | - | |
| Nepali | specify charset_table manually | - | morphology=libstemmer_ne | |
| Ngambay | specify charset_table manually | - | - | |
| Norwegian | charset_table=non_cont | no | morphology=libstemmer_no | |
| Odia | specify charset_table manually | - | - | |
| Persian | charset_table=non_cont | fa | - | |
| Polish | charset_table=non_cont | pl | - | |
| Portuguese | charset_table=non_cont | pt | morphology=libstemmer_pt | |
| Romanian | charset_table=non_cont | ro | morphology=libstemmer_ro | |
| Russian | charset_table=non_cont | ru | morphology=lemmatize_ru (single root form); morphology=lemmatize_ru_all (all root forms); morphology=stem_ru (Porter's Russian stemmer); morphology=stem_enru (Porter's English and Russian stemmers); morphology=libstemmer_ru (from libstemmer) | |
| Santali | specify charset_table manually | - | - | |
| Sindhi | specify charset_table manually | - | - | |
| Slovak | charset_table=non_cont | sk | - | |
| Slovenian | charset_table=non_cont | sl | - | |
| Somali | charset_table=non_cont | so | - | |
| Sotho | charset_table=non_cont | st | - | |
| Spanish | charset_table=non_cont | es | morphology=libstemmer_es | |
| Swahili | charset_table=non_cont | sw | - | |
| Swedish | charset_table=non_cont | sv | morphology=libstemmer_sv | |
| Sylheti | specify charset_table manually | - | - | |
| Tamil | specify charset_table manually | - | morphology=libstemmer_ta | |
| Thai | charset_table=thai | th | - | |
| Turkish | charset_table=non_cont | tr | morphology=libstemmer_tr | |
| Ukrainian | charset_table=non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491 | - | morphology=lemmatize_uk (single root form); morphology=lemmatize_uk_all (all root forms) | Override `charset_table` to preserve `і`, `ї`, and `ґ` |
| Vietnamese | charset_table=non_cont | - | - | Uses Latin script. Vietnamese diacritics (ă, â, ê, ô, ơ, ư, đ, and tone marks) are automatically mapped to their base Latin characters by default, so "tiếng" matches "tieng" without additional configuration. |
| Yoruba | charset_table=non_cont | yo | - | |
| Zulu | charset_table=non_cont | zu | - |  |

### German AOT morphology and sharp s

The default `non_cont` charset maps German umlauts to their ASCII base letters and maps `ß` to `s` before morphology runs. To let German AOT morphology receive these characters, add the built-in `german` alias after `non_cont`:

```ini
charset_table = non_cont, german
```

The alias preserves `ä`, `ö`, `ü`, and `ß`, and maps their uppercase variants to lowercase, including `ẞ` to `ß`. It changes only the tokenizer's character mappings; it does not select a morphology mode automatically.

| Morphology value | Behavior |
| - | - |
| `lemmatize_de` | Existing single-root German AOT behavior |
| `lemmatize_de_all` | Existing all-roots German AOT behavior |
| `lemmatize_de_v2` | Single-root behavior with sharp-s normalization |
| `lemmatize_de_v2_all` | All-roots behavior with sharp-s normalization |

A v2 configuration therefore includes both the morphology value and a charset that retains German characters, for example:

```ini
morphology = lemmatize_de_v2
charset_table = non_cont, german
```

The v2 variants normalize `ß` and `ẞ` to `ss` before dictionary lookup and also normalize `ß` contained in generated dictionary lemmas to `ss`. With v2, forms such as `Straße`/`Strasse`/`STRAẞE` and `GUSSE`/`GÜSSE`/`GUß`/`GUSS` are equivalent in ordinary whole-word searches. This intentionally also removes distinctions such as `Maße` versus `Masse` for ordinary full-text matching.

With `index_exact_words=1`, Manticore also stores the tokenized form from before morphology. Exact-word queries such as `MATCH('=straße')` and `MATCH('=strasse')` can therefore distinguish the `ß` and `ss` forms. Exact words still follow `charset_table` mappings: with the `german` alias, for example, `Straße` and `STRAẞE` both have the exact form `straße` because case is folded before morphology.

Existing tables that keep `lemmatize_de` or `lemmatize_de_all` require no action after a daemon upgrade. Migrating an existing table to either v2 value changes its generated terms, so rebuild a plain table or replay all documents into a new RT table before serving it. Changing the morphology of a populated RT table, or using merge, attach, or optimize, does not convert already indexed terms. Existing dictionary-settings checks can reject an attach between tables with different morphology values where those checks apply, but they are not a migration mechanism.

Both v2 values use the same `de.pak` as the existing values; no language-pack update is required. Daemons that predate the v2 option names cannot load a v2 table, so rolling back a v2 table requires restoring or rebuilding a table with an existing morphology value. Tables that keep `lemmatize_de` or `lemmatize_de_all` are unaffected by this limitation.

<!-- proofread -->
