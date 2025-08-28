# 支持的语言

Manticore 支持多种语言，大多数语言通过 `charset_table = non_cont`（默认值）启用基本支持。`non_cjk` 选项是 `non_cont` 的别名，也可以使用：`charset_table = non_cjk`。

对于许多语言，Manticore 提供了一个[停用词](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)文件，可用于提高搜索的相关性。

此外，少数语言提供了高级形态学支持，通过使用基于字典的词形还原或词干提取算法来实现更好的分词和规范化，从而显著提升搜索的相关性。

下表列出了所有支持的语言，并说明如何启用：
* 基本支持（“Supported”列）
* 停用词（“Stopwords file name”列）
* 高级形态学（“Advanced morphology”列）

| Language | Supported | Stopwords file name | Advanced morphology | Notes |
| - | - | - | - | - |
| Afrikaans | charset_table=non_cont | af | - | |
| Arabic | charset_table=non_cont | ar | morphology=stem_ar (阿拉伯语词干提取器); morphology=libstemmer_ar  | |
| Armenian | charset_table=non_cont | hy | - | |
| Assamese | specify charset_table specify charset_table manually | - | - | |
| Basque | charset_table=non_cont | eu | - | |
| Bengali | charset_table=non_cont | bn | - | |
| Bishnupriya | specify charset_table manually | - | - | |
| Buhid | specify charset_table manually | - | - | |
| Bulgarian | charset_table=non_cont | bg | - | |
| Catalan | charset_table=non_cont | ca | morphology=libstemmer_ca | |
| Chinese using [ICU](https://icu.unicode.org/) | charset_table=chinese | zh | morphology=icu_chinese | 比使用 ngrams 更准确 |
| Chinese using [Jieba](https://github.com/fxsjy/jieba) | charset_table=chinese | zh | morphology=jieba_chinese, requires package `manticore-language-packs` | 比使用 ngrams 更准确 |
| Chinese using ngrams| ngram_chars=chinese | zh | ngram_chars=1 | 索引更快，但搜索性能可能不如前两者 |
| Croatian | charset_table=non_cont | hr | - | |
| Kurdish | charset_table=non_cont | ckb | - | |
| Czech | charset_table=non_cont | cz | morphology=stem_cz (捷克语词干提取器) | |
| Danish | charset_table=non_cont | da | morphology=libstemmer_da | |
| Dutch | charset_table=non_cont | nl | morphology=libstemmer_nl | |
| English | charset_table=non_cont | en | morphology=lemmatize_en (单一词根形式); morphology=lemmatize_en_all (所有词根形式); morphology=stem_en (Porter 英语词干提取器); morphology=stem_enru (Porter 英语和俄语词干提取器); morphology=libstemmer_en (libstemmer 英语)  | |
| Esperanto | charset_table=non_cont | eo | - | |
| Estonian | charset_table=non_cont | et | - | |
| Finnish | charset_table=non_cont | fi | morphology=libstemmer_fi | |
| French | charset_table=non_cont | fr | morphology=libstemmer_fr | |
| Galician | charset_table=non_cont | gl | - | |
| Garo | specify charset_table manually | - | - | |
| German | charset_table=non_cont | de | morphology=lemmatize_de (单一词根形式); morphology=lemmatize_de_all (所有词根形式); morphology=libstemmer_de | |
| Greek | charset_table=non_cont | el | morphology=libstemmer_el | |
| Hebrew | charset_table=non_cont | he | - | |
| Hindi | charset_table=non_cont | hi | morphology=libstemmer_hi | |
| Hmong | specify charset_table manually | - | - | |
| Ho | specify charset_table manually | - | - | |
| Hungarian | charset_table=non_cont | hu | morphology=libstemmer_hu | |
| Indonesian | charset_table=non_cont | id | morphology=libstemmer_id | |
| Irish | charset_table=non_cont | ga | morphology=libstemmer_ga | |
| Italian | charset_table=non_cont | it | morphology=libstemmer_it | |
| Japanese | ngram_chars=japanese | - | ngram_chars=japanese ngram_len=1 | 需要基于 ngram 的分词 |
| Komi | specify charset_table manually | - | - | |
| Korean | ngram_chars=korean | - | ngram_chars=korean ngram_len=1 | 需要基于 ngram 的分词  |
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
| Russian | charset_table=non_cont | ru | morphology=lemmatize_ru (单一词根形式); morphology=lemmatize_ru_all (所有词根形式); morphology=stem_ru (Porter 俄语词干提取器); morphology=stem_enru (Porter 英语和俄语词干提取器); morphology=libstemmer_ru (libstemmer) | |
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
| Ukrainian | charset_table=non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491   | - | morphology=lemmatize_uk_all | 需要安装 [installation](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) 的乌克兰词形还原器 |
| Yoruba | charset_table=non_cont | yo | - | |
| Zulu | charset_table=non_cont | zu | - |  |
<!-- proofread -->

