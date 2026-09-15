# 支持的语言

Manticore 支持范围很广的语言，大多数语言默认启用 `charset_table = non_cont` 即可获得基础支持（这也是默认值）。也可以使用 `non_cjk` 这个别名，它等同于 `non_cont`：`charset_table = non_cjk`。

对于许多语言，Manticore 提供了 [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) 文件，可用于提升搜索相关性。

此外，少数语言还支持高级形态学处理；通过基于词典的词形还原或词干提取算法来改善分词和规范化，可以显著提升搜索相关性。

下表列出了所有受支持的语言以及启用方式：
* 基础支持（“Supported” 列）
* stopwords（“Stopwords file name” 列）
* 高级形态学处理（“Advanced morphology” 列）

| 语言 | 支持情况 | stopwords 文件名 | 高级形态学处理 | 备注 |
| - | - | - | - | - |
| 南非荷兰语 | charset_table=non_cont | af | - | |
| 阿拉伯语 | charset_table=non_cont | ar | morphology=stem_ar（阿拉伯语词干提取器）；morphology=libstemmer_ar  | |
| 亚美尼亚语 | charset_table=non_cont | hy | - | |
| 阿萨姆语 | 需要手动指定 charset_table | - | - | |
| 巴斯克语 | charset_table=non_cont | eu | - | |
| 孟加拉语 | charset_table=non_cont | bn | - | |
| Bishnupriya | 需要手动指定 charset_table | - | - | |
| Buhid | 需要手动指定 charset_table | - | - | |
| 保加利亚语 | charset_table=non_cont | bg | - | |
| 加泰罗尼亚语 | charset_table=non_cont | ca | morphology=libstemmer_ca | |
| 使用 [ICU](https://icu.unicode.org/) 的中文 | charset_table=chinese | zh | morphology=icu_chinese | 比使用 ngram 更准确 |
| 使用 [Jieba](https://github.com/fxsjy/jieba) 的中文 | charset_table=chinese | zh | morphology=jieba_chinese，需要安装 `manticore-language-packs` 包 | 比使用 ngram 更准确 |
| 使用 ngram 的中文| ngram_chars=chinese | zh | ngram_chars=1 | 索引更快，但搜索性能可能不如前者 |
| 克罗地亚语 | charset_table=non_cont | hr | - | |
| 库尔德语 | charset_table=non_cont | ckb | - | |
| 捷克语 | charset_table=non_cont | cz | morphology=stem_cz（捷克语词干提取器） | |
| 丹麦语 | charset_table=non_cont | da | morphology=libstemmer_da | |
| 荷兰语 | charset_table=non_cont | nl | morphology=libstemmer_nl | |
| 英语 | charset_table=non_cont | en | morphology=lemmatize_en（单一词根形式）；morphology=lemmatize_en_all（所有词根形式）；morphology=stem_en（Porter 英语词干提取器）；morphology=stem_enru（Porter 英语和俄语词干提取器）；morphology=libstemmer_en（来自 libstemmer 的英语）  | |
| 世界语 | charset_table=non_cont | eo | - | |
| 爱沙尼亚语 | charset_table=non_cont | et | - | |
| 芬兰语 | charset_table=non_cont | fi | morphology=libstemmer_fi | |
| 法语 | charset_table=non_cont | fr | morphology=libstemmer_fr | |
| 加利西亚语 | charset_table=non_cont | gl | - | |
| Garo | 需要手动指定 charset_table | - | - | |
| 德语 | charset_table=non_cont | de | morphology=lemmatize_de（单一词根形式）；morphology=lemmatize_de_all（所有词根形式）；morphology=lemmatize_de_v2（带 sharp-s 规范化的单一词根形式）；morphology=lemmatize_de_v2_all（带 sharp-s 规范化的所有词根形式）；morphology=libstemmer_de | 使用 AOT v2 变体时，请将 charset_table 设为 `non_cont,german` |
| 希腊语 | charset_table=non_cont | el | morphology=libstemmer_el | |
| 希伯来语 | charset_table=non_cont | he | - | |
| 印地语 | charset_table=non_cont | hi | morphology=libstemmer_hi | |
| 苗语 | 需要手动指定 charset_table | - | - | |
| Ho 语 | 需要手动指定 charset_table | - | - | |
| 匈牙利语 | charset_table=non_cont | hu | morphology=libstemmer_hu | |
| 印度尼西亚语 | charset_table=non_cont | id | morphology=libstemmer_id | |
| 爱尔兰语 | charset_table=non_cont | ga | morphology=libstemmer_ga | |
| 意大利语 | charset_table=non_cont | it | morphology=libstemmer_it | |
| 日语 | ngram_chars=japanese | - | ngram_chars=japanese ngram_len=1 | 需要基于 ngram 的分词 |
| 科米语 | 需要手动指定 charset_table | - | - | |
| 韩语 | ngram_chars=korean | - | ngram_chars=korean ngram_len=1 | 需要基于 ngram 的分词  |
| Large Flowery Miao | 需要手动指定 charset_table | - | - | |
| 拉丁语 | charset_table=non_cont | la | - | |
| 拉脱维亚语 | charset_table=non_cont | lv | - | |
| 立陶宛语 | charset_table=non_cont | lt | morphology=libstemmer_lt | |
| Maba | 需要手动指定 charset_table | - | - | |
| Maithili | 需要手动指定 charset_table | - | - | |
| 马拉地语 | 需要手动指定 charset_table | - | - | |
| 马拉地语 | charset_table=non_cont | mr | - | |
| Mende | 需要手动指定 charset_table | - | - | |
| Mru | 需要手动指定 charset_table | - | - | |
| Myene | 需要手动指定 charset_table | - | - | |
| 尼泊尔语 | 需要手动指定 charset_table | - | morphology=libstemmer_ne | |
| Ngambay | 需要手动指定 charset_table | - | - | |
| 挪威语 | charset_table=non_cont | no | morphology=libstemmer_no | |
| 奥里亚语 | 需要手动指定 charset_table | - | - | |
| 波斯语 | charset_table=non_cont | fa | - | |
| 波兰语 | charset_table=non_cont | pl | - | |
| 葡萄牙语 | charset_table=non_cont | pt | morphology=libstemmer_pt | |
| 罗马尼亚语 | charset_table=non_cont | ro | morphology=libstemmer_ro | |
| 俄语 | charset_table=non_cont | ru | morphology=lemmatize_ru（单一词根形式）；morphology=lemmatize_ru_all（所有词根形式）；morphology=stem_ru（Porter 俄语词干提取器）；morphology=stem_enru（Porter 英语和俄语词干提取器）；morphology=libstemmer_ru（来自 libstemmer） | |
| Santali | 需要手动指定 charset_table | - | - | |
| Sindhi | 需要手动指定 charset_table | - | - | |
| 斯洛伐克语 | charset_table=non_cont | sk | - | |
| 斯洛文尼亚语 | charset_table=non_cont | sl | - | |
| 索马里语 | charset_table=non_cont | so | - | |
| 南索托语 | charset_table=non_cont | st | - | |
| 西班牙语 | charset_table=non_cont | es | morphology=libstemmer_es | |
| 斯瓦希里语 | charset_table=non_cont | sw | - | |
| 瑞典语 | charset_table=non_cont | sv | morphology=libstemmer_sv | |
| Sylheti | 需要手动指定 charset_table | - | - | |
| 泰米尔语 | 需要手动指定 charset_table | - | morphology=libstemmer_ta | |
| 泰语 | charset_table=thai | th | - | |
| 土耳其语 | charset_table=non_cont | tr | morphology=libstemmer_tr | |
| 乌克兰语 | charset_table=non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491 | - | morphology=lemmatize_uk（单一词根形式）；morphology=lemmatize_uk_all（所有词根形式） | 覆盖 `charset_table` 以保留 `і`、`ї` 和 `ґ` |
| 越南语 | charset_table=non_cont | - | - | 使用拉丁字母。默认情况下，越南语变音符号（ă、â、ê、ô、ơ、ư、đ 以及声调符号）会自动映射为对应的基础拉丁字符，因此无需额外配置，`tiếng` 可以匹配 `tieng`。 |
| 约鲁巴语 | charset_table=non_cont | yo | - | |
| 祖鲁语 | charset_table=non_cont | zu | - |  |

### 德语 AOT 形态学和 sharp s

默认的 `non_cont` 字符集会在形态学处理前把德语变音符号映射为对应的 ASCII 基础字母，并将 `ß` 映射为 `s`。如果要让德语 AOT 形态学直接接收这些字符，请在 `non_cont` 后面追加内置的 `german` 别名：

```ini
charset_table = non_cont, german
```

该别名会保留 `ä`、`ö`、`ü` 和 `ß`，并将它们的大写形式映射为小写，包括把 `ẞ` 映射为 `ß`。它只会更改分词器的字符映射；不会自动选择任何形态学模式。

| 形态学值 | 行为 |
| - | - |
| `lemmatize_de` | 现有的德语 AOT 单词根行为 |
| `lemmatize_de_all` | 现有的德语 AOT 全词根行为 |
| `lemmatize_de_v2` | 带 sharp s 规范化的单词根行为 |
| `lemmatize_de_v2_all` | 带 sharp s 规范化的全词根行为 |

因此，v2 配置既要包含形态学值，也要包含保留德语字符的字符集，例如：

```ini
morphology = lemmatize_de_v2
charset_table = non_cont, german
```

v2 变体会在词典查找前将 `ß` 和 `ẞ` 规范化为 `ss`，并且也会将生成的词典词元中包含的 `ß` 规范化为 `ss`。使用 v2 后，`Straße`/`Strasse`/`STRAẞE` 以及 `GUSSE`/`GÜSSE`/`GUß`/`GUSS` 这类形式在普通整词搜索中是等价的。这也会刻意消除诸如 `Maße` 与 `Masse` 这类在普通全文匹配中的区别。

启用 `index_exact_words=1` 时，Manticore 还会保存形态学处理之前的分词结果。因此，像 `MATCH('=straße')` 和 `MATCH('=strasse')` 这样的精确词查询可以区分 `ß` 和 `ss` 形式。精确词仍然遵循 `charset_table` 的映射规则：例如启用 `german` 别名时，`Straße` 和 `STRAẞE` 的精确形式都会是 `straße`，因为大小写折叠先于形态学处理发生。

现有表如果继续使用 `lemmatize_de` 或 `lemmatize_de_all`，在 daemon 升级后无需任何操作。将现有表迁移到任一 v2 值会改变其生成的词项，因此在投入使用前，请重建普通表，或将所有文档重新回放到新的 RT 表中。对已填充的 RT 表更改形态学设置，或者使用 merge、attach 或 optimize，都不会转换已索引的词项。现有的 dictionary-settings 检查可能会在适用范围内拒绝 morphology 值不同的表之间执行 attach，但这并不是迁移机制。

两个 v2 值都使用与现有值相同的 `de.pak`；无需更新语言包。早于 v2 选项名称的 daemon 无法加载 v2 表，因此回退 v2 表时必须恢复或重建使用现有形态学值的表。保留 `lemmatize_de` 或 `lemmatize_de_all` 的表不受此限制影响。

<!-- proofread -->
