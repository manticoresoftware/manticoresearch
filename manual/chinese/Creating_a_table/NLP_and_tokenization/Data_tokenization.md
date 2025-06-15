# 数据分词

Manticore 不会直接存储文本本身来执行全文搜索。相反，它会提取单词并创建多个结构，以便快速进行全文搜索。从提取的单词中构建字典，该字典允许快速判断某个单词是否存在于索引中。此外，其他结构记录了包含该单词的文档和字段（以及它在字段内的位置）。当执行全文匹配时，都会使用这些结构。

将单词划分和分类的过程称为分词。分词在索引和搜索时都会应用，并且在字符和单词级别上进行操作。

在字符级别，搜索引擎只允许通过某些特定的字符。这由[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)定义。其他字符都会被替换为空白（默认视为空格作为单词分隔符）。charset_table 还支持映射功能，例如小写转换或简单地将一个字符替换为另一个字符。除此之外，字符还可以被[忽略](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars)、[混合](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars)，或定义为[短语边界](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary)。

在单词级别，基础设置是[min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)，它定义了被接受进入索引的单词最小字符长度。一个常见需求是匹配单数和复数形式的单词。为此，可以使用[morphology processors](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)。

进一步来说，我们希望因为同义词的关系，将一个单词匹配为另一个单词。可以使用[word forms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md)功能，该功能允许将一个或多个单词映射到另一个单词。

非常常见的单词可能对搜索产生一些不良影响，主要是因为它们出现频率极高，处理它们的文档/命中列表需要大量计算资源。这些词可以通过[停用词](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)功能列入黑名单。这样不仅可以加快查询速度，也可以减小索引大小。

更高级的黑名单功能是[双字模型](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)，它允许在一个“常见”的词和一个不常见词之间创建特殊的标记。当在短语搜索中使用常见词时，这可以将搜索速度提升数倍。

在索引 HTML 内容时，重要的是不要索引 HTML 标签，因为它们会在索引中引入大量“噪音”。可以使用[HTML 过滤](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#Stripping-HTML-tags)，它可以被配置为过滤标签，但仍索引某些标签属性，或完全忽略某些 HTML 元素的内容。
<!-- proofread -->

