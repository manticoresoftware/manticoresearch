# 数据分词

Manticore 并不是直接以全文搜索的形式存储文本。相反，它会将文本切分成单词（称为词元），并构建多个内部结构以实现快速全文搜索。这些结构包括一个词典，用于快速检查词是否存在于索引中。其他结构则跟踪包含该词的文档和字段，甚至词在字段中的具体位置。所有这些都会在搜索时被用来寻找相关结果。

这种切分和处理文本的过程称为**分词（tokenization）**。分词发生在向索引添加数据时以及执行搜索时，既在字符层面也在单词层面进行。

### 字符层分词

在字符层面，只允许某些字符。这个由[`charset_table`](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 控制。所有其他字符都会被替换为空格（空格被视为单词分隔符）。`charset_table` 还支持将字符转为小写或用另一个字符替换它。它还能定义要[忽略](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars)、[混合](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars)或视为[短语边界](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary)的字符。

### 单词层分词

在单词层面，引擎使用[`min_word_len`](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len) 设置来决定应该索引的单词最小长度（以字符计）。

Manticore 还支持匹配不同形式的单词。例如，要将“car”和“cars”视为相同单词，可以使用[形态处理器](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)。

如果希望不同的单词被视为相同——比如“USA”和“United States”——可以用[单词形式](../../Creating_a_table/NLP_and_tokenization/Wordforms.md)功能定义它们。

### 处理常见和噪声词

非常常见的词（如“the”、“and”、“is”）会降低搜索速度并增大索引大小。你可以用[停用词](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)过滤它们。这可以让搜索更快，索引更小。

更高级的过滤方法是[二元组](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)，通过将常见词与不常见词组合创建特殊词元。这能在涉及常见词的短语搜索时显著加快速度。

### HTML 内容

如果你在索引 HTML，通常最好不把 HTML 标签包含进索引，因为它们会增加很多无用内容。你可以用[HTML 剥离](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#Stripping-HTML-tags)去除标签，但仍然索引某些标签属性，或者完全跳过特定元素。

### 词元长度限制

请记住 Manticore 有一个**最大词元长度为 42 个字符**的限制。任何超过该长度的词都会被**截断**。该限制在索引和搜索过程中均适用，所以确保你的数据和查询都考虑到这一点很重要。
