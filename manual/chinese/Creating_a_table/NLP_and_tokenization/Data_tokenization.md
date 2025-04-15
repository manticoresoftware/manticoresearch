# 数据标记化

Manticore 不会将文本原样存储以进行全文搜索。相反，它提取单词并创建多个结构，以便快速进行全文搜索。从找到的单词中构建一个字典，以便快速查找以确定单词是否存在于索引中。此外，其他结构记录了找到该单词的文档和字段（以及它在字段内的位置）。在执行全文匹配时使用所有这些结构。

划分和分类单词的过程称为标记化。标记化在索引和搜索时都适用，并且操作在字符和单词级别。

在字符级别，引擎只允许某些字符通过。这是由 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 定义的。任何其他字符都将被空格替换（空格被视为默认的单词分隔符）。charset_table 还允许进行映射，例如小写化或简单地用另一个字符替换一个字符。除此之外，可以 [忽略](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars)、[混合](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars)、定义为 [短语边界](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary)。 

在单词级别，基本设置是 [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)，它定义了在索引中接受的最小单词长度（以字符为单位）。一个常见的请求是匹配单数和复数形式的单词。为此，可以使用 [形态处理器](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)。 

进一步而言，我们可能希望将一个单词与另一个单词匹配，因为它们是同义词。为此，可以使用 [词形](../../Creating_a_table/NLP_and_tokenization/Wordforms.md) 功能，它允许将一个或多个单词映射到另一个单词。

非常常见的单词在搜索中可能会产生一些不必要的影响，主要是因为它们的频率，它们需要大量的计算来处理它们的文档/命中列表。可以使用 [停用词](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) 功能将其列入黑名单。这不仅有助于加快查询速度，还有助于减少索引大小。

更高级的黑名单是 [二元组](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)，它允许在一个“二元组”（常见）单词和一个不常见的单词之间创建一个特殊的标记。在短语搜索中使用常见单词时，这可以加速几倍。

在索引 HTML 内容时，重要的是不要对 HTML 标签进行索引，因为它们可能会在索引中引入大量“噪音”。可以使用 [HTML 剥离](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#Stripping-HTML-tags)，并可以配置为剥离，但索引某些标签属性或完全忽略某些 HTML 元素的内容。
<!-- proofread -->
