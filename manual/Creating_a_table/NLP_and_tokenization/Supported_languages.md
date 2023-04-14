# Supported languages

Manticore supports a wide range of languages, with basic support enabled for most languages via `charset_table = non_cjk` (which is the default value).

For many languages, Manticore provides a [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) file that can be used to improve search relevance.

Additionally, advanced morphology is available for a few languages that can significantly improve search relevance by using dictionary-based lemmatization or stemming algorithms for better segmentation and normalization.

The table below lists all supported languages and indicates how to enable:
* basic support (column "Supported")
* stopwords (column "Stopwords file name")
* advanced morphology (column "Advanced morphology")

| Language | Supported | Stopwords file name | Advanced morphology | Notes |
| - | - | - | - | - |
| Afrikaans | charset_table=non_cjk | af | - | |
| Arabic | charset_table=non_cjk | ar | morphology=stem_ar (Arabic stemmer); morphology=libstemmer_ar  | |
| Armenian | charset_table=non_cjk | hy | - | |
| Assamese | specify charset_table specify charset_table manually | - | - | |
| Basque | charset_table=non_cjk | eu | - | |
| Bengali | charset_table=non_cjk | bn | - | |
| Bishnupriya | specify charset_table manually | - | - | |
| Buhid | specify charset_table manually | - | - | |
| Bulgarian | charset_table=non_cjk | bg | - | |
| Catalan | charset_table=non_cjk | ca | morphology=libstemmer_ca | |
| Chinese | charset_table=chinese or ngram_chars=chinese | zh | morphology=icu_chinese or ngram_chars=1 correspondingly | ICU dictionary based segmentation is much more accurate than ngram-based |
| Croatian | charset_table=non_cjk | hr | - | |
| Kurdish | charset_table=non_cjk | ckb | - | |
| Czech | charset_table=non_cjk | cz | morphology=stem_cz (Czech stemmer) | |
| Danish | charset_table=non_cjk | da | morphology=libstemmer_da | |
| Dutch | charset_table=non_cjk | nl | morphology=libstemmer_nl | |
| English | charset_table=non_cjk | en | morphology=lemmatize_en (single root form); morphology=lemmatize_en_all (all root forms); morphology=stem_en (Porter's English stemmer); morphology=stem_enru (Porter's English and Russian stemmers); morphology=libstemmer_en (English from libstemmer)  | |
| Esperanto | charset_table=non_cjk | eo | - | |
| Estonian | charset_table=non_cjk | et | - | |
| Finnish | charset_table=non_cjk | fi | morphology=libstemmer_fi | |
| French | charset_table=non_cjk | fr | morphology=libstemmer_fr | |
| Galician | charset_table=non_cjk | gl | - | |
| Garo | specify charset_table manually | - | - | |
| German | charset_table=non_cjk | de | morphology=lemmatize_de (single root form); morphology=lemmatize_de_all (all root forms); morphology=libstemmer_de | |
| Greek | charset_table=non_cjk | el | morphology=libstemmer_el | |
| Hebrew | charset_table=non_cjk | he | - | |
| Hindi | charset_table=non_cjk | hi | morphology=libstemmer_hi | |
| Hmong | specify charset_table manually | - | - | |
| Ho | specify charset_table manually | - | - | |
| Hungarian | charset_table=non_cjk | hu | morphology=libstemmer_hu | |
| Indonesian | charset_table=non_cjk | id | morphology=libstemmer_id | |
| Irish | charset_table=non_cjk | ga | morphology=libstemmer_ga | |
| Italian | charset_table=non_cjk | it | morphology=libstemmer_it | |
| Japanese | ngram_chars=japanese | - | ngram_chars=japanese ngram_len=1 | Requires ngram-based segmentation |
| Komi | specify charset_table manually | - | - | |
| Korean | ngram_chars=korean | - | ngram_chars=korean ngram_len=1 | Requires ngram-based segmentation  |
| Large Flowery Miao | specify charset_table manually | - | - | |
| Latin | charset_table=non_cjk | la | - | |
| Latvian | charset_table=non_cjk | lv | - | |
| Lithuanian | charset_table=non_cjk | lt | morphology=libstemmer_lt | |
| Maba | specify charset_table manually | - | - | |
| Maithili | specify charset_table manually | - | - | |
| Marathi | specify charset_table manually | - | - | |
| Marathi | charset_table=non_cjk | mr | - | |
| Mende | specify charset_table manually | - | - | |
| Mru | specify charset_table manually | - | - | |
| Myene | specify charset_table manually | - | - | |
| Nepali | specify charset_table manually | - | morphology=libstemmer_ne | |
| Ngambay | specify charset_table manually | - | - | |
| Norwegian | charset_table=non_cjk | no | morphology=libstemmer_no | |
| Odia | specify charset_table manually | - | - | |
| Persian | charset_table=non_cjk | fa | - | |
| Polish | charset_table=non_cjk | pl | - | |
| Portuguese | charset_table=non_cjk | pt | morphology=libstemmer_pt | |
| Romanian | charset_table=non_cjk | ro | morphology=libstemmer_ro | |
| Russian | charset_table=non_cjk | ru | morphology=lemmatize_ru (single root form); morphology=lemmatize_ru_all (all root forms); morphology=stem_ru (Porter's Russian stemmer); morphology=stem_enru (Porter's English and Russian stemmers); morphology=libstemmer_ru (from libstemmer) | |
| Santali | specify charset_table manually | - | - | |
| Sindhi | specify charset_table manually | - | - | |
| Slovak | charset_table=non_cjk | sk | - | |
| Slovenian | charset_table=non_cjk | sl | - | |
| Somali | charset_table=non_cjk | so | - | |
| Sotho | charset_table=non_cjk | st | - | |
| Spanish | charset_table=non_cjk | es | morphology=libstemmer_es | |
| Swahili | charset_table=non_cjk | sw | - | |
| Swedish | charset_table=non_cjk | sv | morphology=libstemmer_sv | |
| Sylheti | specify charset_table manually | - | - | |
| Tamil | specify charset_table manually | - | morphology=libstemmer_ta | |
| Thai | charset_table=non_cjk | th | - | |
| Turkish | charset_table=non_cjk | tr | morphology=libstemmer_tr | |
| Ukrainian | charset_table=non_cjk,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491   | - | morphology=lemmatize_uk_all | Requires [installation](../../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) of UK lemmatizer |
| Yoruba | charset_table=non_cjk | yo | - | |
| Zulu | charset_table=non_cjk | zu | - |  |
<!-- proofread -->
