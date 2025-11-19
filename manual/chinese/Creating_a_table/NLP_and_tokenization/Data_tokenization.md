# 数据分词

Manticore 并不会将文本原样存储用于全文搜索。相反，它会将文本拆分成单词（称为词元）并构建多个内部结构以实现快速全文搜索。这些结构包括一个字典，用于快速检查单词是否存在于索引中。其他结构跟踪包含该单词的文档和字段，甚至精确到该单词在字段中的位置。这些都在搜索时用于找到相关结果。

将文本这样拆分和处理的过程称为**分词**。分词在添加数据到索引和执行搜索时都会发生。它既在字符级别也在单词级别工作。

### 字符级分词

在字符级别，只有某些字符被允许。这由 [`charset_table`](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 控制。任何其他字符都会被替换为空格（空格被视为单词分隔符）。`charset_table` 还支持将字符转换为小写或将一个字符替换为另一个字符。它还可以定义被[忽略](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars)、[混合](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars)或作为[短语边界](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary)处理的字符。

### 单词级分词

在单词级别，引擎使用 [`min_word_len`](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len) 设置来决定应索引的最小单词长度（以字符计）。

Manticore 还支持匹配不同形式的单词。例如，要将“car”和“cars”视为同一个单词，可以使用[形态学处理器](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)。

如果你希望不同的单词被视为相同——例如，“USA”和“United States”——你可以使用[词形](../../Creating_a_table/NLP_and_tokenization/Wordforms.md)功能来定义它们。

### 处理常见和噪声词

非常常见的词（如“the”、“and”、“is”）会减慢搜索速度并增加索引大小。你可以使用[停用词](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)将它们过滤掉。这可以使搜索更快，索引更小。

更高级的过滤方法是[二元组](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)，它通过将常见词与不常见词组合创建特殊词元。当涉及常见词的短语搜索时，这可以显著加快速度。

### HTML 内容

如果你正在索引 HTML，通常最好不要将 HTML 标签包含在索引中，因为它们会添加大量不必要的内容。你可以使用[HTML 去除](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#Stripping-HTML-tags)来移除标签，但仍然可以索引某些标签属性或完全跳过特定元素。

### 词元长度限制

请记住，Manticore 有一个**最大词元长度为 42 个字符**的限制。任何超过此长度的单词都会被**截断**。此限制在索引和搜索时都适用，因此确保你的数据和查询考虑到这一点非常重要。
