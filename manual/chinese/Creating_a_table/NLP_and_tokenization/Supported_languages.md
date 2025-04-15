# 支持的语言

Manticore 支持多种语言，通过 `charset_table = non_cont`（默认值）为大多数语言启用基本支持。也可以使用 `non_cjk` 选项，作为 `non_cont` 的别名：`charset_table = non_cjk`。

对于许多语言，Manticore 提供了一个 [停用词](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) 文件，可以用来提高搜索相关性。

此外，少数语言提供高级形态分析，可以通过使用基于字典的词元化或词干算法显著提高搜索相关性，以便进行更好的分割和标准化。

下表列出了所有支持的语言，并指示如何启用：
* 基本支持（“支持”列）
* 停用词（“停用词文件名”列）
* 高级形态分析（“高级形态分析”列）

| 语言 | 支持 | 停用词文件名 | 高级形态分析 | 备注 |
| - | - | - | - | - |
| Afrikaans | charset_table=non_cont | af | - | |
| Arabic | charset_table=non_cont | ar | morphology=stem_ar (阿拉伯语词干); morphology=libstemmer_ar  | |
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
| Chinese using ngrams| ngram_chars=chinese | zh | ngram_chars=1 | 索引更快，但搜索性能可能不如 |
| Croatian | charset_table=non_cont | hr | - | |
| Kurdish | charset_table=non_cont | ckb | - | |
| Czech | charset_table=non_cont | cz | morphology=stem_cz (捷克语词干) | |
| Danish | charset_table=non_cont | da | morphology=libstemmer_da | |
| Dutch | charset_table=non_cont | nl | morphology=libstemmer_nl | |
| English | charset_table=non_cont | en | morphology=lemmatize_en (单一词根形式); morphology=lemmatize_en_all (所有词根形式); morphology=stem_en (波特的英语词干); morphology=stem_enru (波特的英语和俄语词干); morphology=libstemmer_en (来自 libstemmer 的英语)  | |
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
| Korean | ngram_chars=korean | - | ngram_chars=korean ngram_len=1 | 需要基于 ngram 的分词 |
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
| Russian | charset_table=non_cont | ru | morphology=lemmatize_ru (单一词根形式); morphology=lemmatize_ru_all (所有词根形式); morphology=stem_ru (波特的俄语词干); morphology=stem_enru (波特的英语和俄语词干); morphology=libstemmer_ru (来自 libstemmer) | |
| Santali | specify charset_table manually | - | - | |
| Sindhi | specify charset_table manually | - | - | |
| Slovak | charset_table=non_cont | sk | - | |
| Slovenian | charset_table=non_cont | sl | - | |
| Somali | charset_table=non_cont | so | - | |
| Sotho | charset_table=non_cont | st | - | |
| Spanish | charset_table=non_cont | es | morphology=libstemmer_es | |
| Swahili | charset_table=non_cont | sw | - | |
| 瑞典语 | charset_table=non_cont | sv | morphology=libstemmer_sv | |
| 西伦敦语 | 手动指定 charset_table | - | - | |
| 泰米尔语 | 手动指定 charset_table | - | morphology=libstemmer_ta | |
| 泰语 | charset_table=thai | th | - | |
| 土耳其语 | charset_table=non_cont | tr | morphology=libstemmer_tr | |
| 乌克兰语 | charset_table=non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491   | - | morphology=lemmatize_uk_all | 需要[安装](../../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer)英国词形还原器 |
| 約魯巴語 | charset_table=non_cont | yo | - | |
| 祖鲁语 | charset_table=non_cont | zu | - |  |
<!-- proofread -->
