# 数据标记化

Manticore 不会将文本原样存储以进行全文搜索。相反，它会将文本拆分成单词（称为标记），并构建多个内部结构以实现快速全文搜索。这些结构包括一个字典，用于快速检查某个单词是否存在于索引中。其他结构则跟踪包含该单词的文档和字段，甚至该单词在字段中的具体位置。这些都在搜索时用于查找相关结果。

这种拆分和处理文本的过程称为**标记化**。标记化既发生在向索引添加数据时，也发生在运行搜索时。它同时在字符和单词级别进行。

### 字符级标记化

在字符级别，只有特定字符被允许。此行为由[`charset_table`](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 控制。其它所有字符都会被替换为空格（空格被视为单词分隔符）。`charset_table` 还支持将字符转换为小写字母或将一个字符替换为另一个字符。它还可以定义被[忽略](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars)、[混合](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars)或视为[短语边界](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary)的字符。

### 单词级标记化

在单词级别，引擎使用[`min_word_len`](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len) 设置来决定应索引的单词最小长度（以字符计）。

Manticore 还支持不同形式的单词匹配。例如，为了将“car”和“cars”视为同一单词，可以使用[形态处理器](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)。

如果您希望不同的词被视为相同——例如，“USA”和“United States”——您可以使用[word forms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md) 功能进行定义。

### 处理常见和噪声词

非常常见的词（如“the”，“and”，“is”）会减慢搜索速度并增加索引大小。您可以使用[停用词](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)将它们过滤掉。这可以使搜索更快，索引更小。

更高级的过滤方法是[二元词组](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)，它通过将一个常见词和一个不常见词组合起来创建特殊标记。当常见词出现时，这能显著加快短语搜索。

### HTML 内容

如果您正在索引 HTML，通常最好不要将 HTML 标签包含在索引中，因为它们添加了很多不必要的内容。您可以使用[HTML 去除](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#Stripping-HTML-tags)功能去除标签，但仍能索引某些标签属性或完全跳过特定元素。

### 标记长度限制

请记住 Manticore 的**最大标记长度为 42 个字符**。任何超过此长度的单词都将被**截断**。此限制在索引和搜索时均适用，因此确保您的数据和查询考虑到这一点非常重要。
