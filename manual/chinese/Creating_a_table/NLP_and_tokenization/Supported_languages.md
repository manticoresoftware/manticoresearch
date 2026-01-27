# 支持的语言

Manticore 支持多种语言，大多数语言通过 `charset_table = non_cont`（默认值）启用基本支持。也可以使用 `non_cjk` 选项，它是 `non_cont` 的别名：`charset_table = non_cjk`。

对于许多语言，Manticore 提供了[停用词](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)文件，可用于提升搜索相关性。

此外，对少数语言提供了高级形态学功能，通过使用基于词典的词形还原或词干算法进行更好的分词和规范化，显著提升搜索相关性。

下表列出了所有支持的语言，并指明如何启用：
* 基本支持（“Supported” 列）
* 停用词文件（“Stopwords file name” 列）
* 高级形态学（“Advanced morphology” 列）

| 语言 | 支持 | 停用词文件名 | 高级词形处理 | 备注 |
| - | - | - | - | - |
| 南非荷兰语 | charset_table=non_cont | af | - | |
| 阿拉伯语 | charset_table=non_cont | ar | morphology=stem_ar（阿拉伯语词干提取器）；morphology=libstemmer_ar  | |
| 亚美尼亚语 | charset_table=non_cont | hy | - | |
| 阿萨姆语 | 手动指定 charset_table | - | - | |
| 巴斯克语 | charset_table=non_cont | eu | - | |
| 孟加拉语 | charset_table=non_cont | bn | - | |
| 比什努普里亚语 | 手动指定 charset_table | - | - | |
| 布希德语 | 手动指定 charset_table | - | - | |
| 保加利亚语 | charset_table=non_cont | bg | - | |
| 加泰罗尼亚语 | charset_table=non_cont | ca | morphology=libstemmer_ca | |
| 使用 [ICU](https://icu.unicode.org/) 的中文 | charset_table=chinese | zh | morphology=icu_chinese | 比使用ngrams更准确 |
| 使用 [Jieba](https://github.com/fxsjy/jieba) 的中文 | charset_table=chinese | zh | morphology=jieba_chinese，需要包 `manticore-language-packs` | 比使用ngrams更准确 |
| 使用ngrams的中文 | ngram_chars=chinese | zh | ngram_chars=1 | 索引速度更快，但搜索性能可能不如 |
| 克罗地亚语 | charset_table=non_cont | hr | - | |
| 库尔德语 | charset_table=non_cont | ckb | - | |
| 捷克语 | charset_table=non_cont | cz | morphology=stem_cz（捷克语词干提取器） | |
| 丹麦语 | charset_table=non_cont | da | morphology=libstemmer_da | |
| 荷兰语 | charset_table=non_cont | nl | morphology=libstemmer_nl | |
| 英语 | charset_table=non_cont | en | morphology=lemmatize_en（单根形式）；morphology=lemmatize_en_all（所有根形式）；morphology=stem_en（Porter英语词干提取器）；morphology=stem_enru（Porter英语和俄语词干提取器）；morphology=libstemmer_en（来自libstemmer的英语）  | |
| 世界语 | charset_table=non_cont | eo | - | |
| 爱沙尼亚语 | charset_table=non_cont | et | - | |
| 芬兰语 | charset_table=non_cont | fi | morphology=libstemmer_fi | |
| 法语 | charset_table=non_cont | fr | morphology=libstemmer_fr | |
| 加利西亚语 | charset_table=non_cont | gl | - | |
| 加罗语 | 手动指定 charset_table | - | - | |
| 德语 | charset_table=non_cont | de | morphology=lemmatize_de（单根形式）；morphology=lemmatize_de_all（所有根形式）；morphology=libstemmer_de | |
| 希腊语 | charset_table=non_cont | el | morphology=libstemmer_el | |
| 希伯来语 | charset_table=non_cont | he | - | |
| 印地语 | charset_table=non_cont | hi | morphology=libstemmer_hi | |
| 老挝语 | 手动指定 charset_table | - | - | |
| 霍语 | 手动指定 charset_table | - | - | |
| 匈牙利语 | charset_table=non_cont | hu | morphology=libstemmer_hu | |
| 印度尼西亚语 | charset_table=non_cont | id | morphology=libstemmer_id | |
| 爱尔兰语 | charset_table=non_cont | ga | morphology=libstemmer_ga | |
| 意大利语 | charset_table=non_cont | it | morphology=libstemmer_it | |
| 日语 | ngram_chars=japanese | - | ngram_chars=japanese ngram_len=1 | 需要基于ngram的分词 |
| 科米语 | 手动指定 charset_table | - | - | |
| 韩语 | ngram_chars=korean | - | ngram_chars=korean ngram_len=1 | 需要基于ngram的分词  |
| 大花苗语 | 手动指定 charset_table | - | - | |
| 拉丁语 | charset_table=non_cont | la | - | |
| 拉脱维亚语 | charset_table=non_cont | lv | - | |
| 立陶宛语 | charset_table=non_cont | lt | morphology=libstemmer_lt | |
| 马巴语 | 手动指定 charset_table | - | - | |
| 马提利语 | 手动指定 charset_table | - | - | |
| 马拉地语 | 手动指定 charset_table | - | - | |
| 马拉地语 | charset_table=non_cont | mr | - | |
| 门德语 | 手动指定 charset_table | - | - | |
| 马鲁语 | 手动指定 charset_table | - | - | |
| 美尼语 | 手动指定 charset_table | - | - | |
| 尼泊尔语 | 手动指定 charset_table | - | morphology=libstemmer_ne | |
| 甘巴伊语 | 手动指定 charset_table | - | - | |
| 挪威语 | charset_table=non_cont | no | morphology=libstemmer_no | |
| 奥里亚语 | 手动指定 charset_table | - | - | |
| 波斯语 | charset_table=non_cont | fa | - | |
| 波兰语 | charset_table=non_cont | pl | - | |
| 葡萄牙语 | charset_table=non_cont | pt | morphology=libstemmer_pt | |
| 罗马尼亚语 | charset_table=non_cont | ro | morphology=libstemmer_ro | |
| 俄语 | charset_table=non_cont | ru | morphology=lemmatize_ru（单根形式）；morphology=lemmatize_ru_all（所有根形式）；morphology=stem_ru（Porter俄语词干提取器）；morphology=stem_enru（Porter英语和俄语词干提取器）；morphology=libstemmer_ru（来自libstemmer） | |
| 桑塔利语 | 手动指定 charset_table | - | - | |
| 信德语 | 手动指定 charset_table | - | - | |
| 斯洛伐克语 | charset_table=non_cont | sk | - | |
| 斯洛文尼亚语 | charset_table=non_cont | sl | - | |
| 索马里语 | charset_table=non_cont | so | - | |
| 塞索托语 | charset_table=non_cont | st | - | |
| 西班牙语 | charset_table=non_cont | es | morphology=libstemmer_es | |
| 斯瓦希里语 | charset_table=non_cont | sw | - | |
| 瑞典语 | charset_table=non_cont | sv | morphology=libstemmer_sv | |
| 锡尔赫提语 | 手动指定 charset_table | - | - | |
| 泰米尔语 | 手动指定 charset_table | - | morphology=libstemmer_ta | |
| 泰语 | charset_table=thai | th | - | |
| 土耳其语 | charset_table=non_cont | tr | morphology=libstemmer_tr | |
| 乌克兰语 | charset_table=non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491   | - | morphology=lemmatize_uk_all | 需要安装 [UK词形还原器](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) |
| 越南语 | charset_table=non_cont | - | - | 使用拉丁字母。越南语变音符号（ă, â, ê, ô, ơ, ư, đ 和声调符号）默认会自动映射到其基本拉丁字符，因此无需额外配置，“tiếng” 会匹配 “tieng” |
| 约鲁巴语 | charset_table=non_cont | yo | - | |
| 祖鲁语 | charset_table=non_cont | zu | - |  |
<!-- proofread -->

