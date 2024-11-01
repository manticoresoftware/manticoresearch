# 数据标记化

Manticore 不会直接存储文本以进行全文搜索。相反，它会提取单词并创建多种结构，以实现快速的全文搜索。通过找到的单词，构建一个字典，从而快速查找某个单词是否存在于索引中。此外，其他结构会记录文档和字段中找到的单词（以及它在字段中的位置）。在执行全文匹配时，这些信息都会被使用。

标记化的过程就是对单词进行划分和分类。标记化在索引和搜索时都会应用，并且在字符和单词层面上进行操作。

在字符层面，引擎只允许某些字符通过。这由 [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) 定义。其他字符会被替换为空格（这被视为默认的单词分隔符）。charset_table 还允许进行映射，例如将字符转换为小写或简单地用一个字符替换另一个字符。此外，字符可以被 [忽略](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars)、[混合](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars)，或被定义为 [短语边界](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary)。

在单词层面，基本设置是 [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)，它定义了被接受到索引中的最小单词长度（字符数）。一个常见的请求是匹配单数和复数形式的单词。为此，可以使用 [形态处理器](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)。

进一步说，我们可能希望将一个单词与另一个单词进行匹配，因为它们是同义词。为此，可以使用 [词形](../../Creating_a_table/NLP_and_tokenization/Wordforms.md) 功能，它允许将一个或多个单词映射到另一个单词。

非常常见的单词可能会对搜索产生一些不必要的影响，主要是因为它们的频率，处理它们的文档/命中列表需要大量计算。可以使用 [停用词](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) 功能将其列入黑名单。这不仅有助于加速查询，还能减小索引大小。

一种更高级的黑名单是 [双字母组合](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)，它允许在“常见”单词和不常见单词之间创建特殊标记。这在常见单词用于短语搜索时，可以加速数倍。

在索引 HTML 内容时，重要的是不要索引 HTML 标签，因为它们可能会在索引中引入大量“噪音”。可以使用 [HTML 剥离](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#Stripping-HTML-tags) 功能，并可以配置为剥离，但索引某些标签属性或完全忽略某些 HTML 元素的内容。

<!-- proofread -->
