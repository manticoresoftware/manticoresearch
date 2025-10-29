# Data tokenization

Manticore doesn't store text exactly as it is for full-text searching. Instead, it breaks the text into words (called tokens) and builds several internal structures to enable fast full-text searches. These structures include a dictionary that helps quickly check if a word exists in the index. Other structures track which documents and fields contain the word, and even where exactly in the field it appears. These are all used during a search to find relevant results.

The process of splitting and handling text like this is called **tokenization**. Tokenization happens both when adding data to the index and when running a search. It works at both the character and word level.

### Character-level tokenization

At the character level, only certain characters are allowed. This is controlled by the [`charset_table`](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Any other characters are replaced with a space (which is treated as a word separator). The `charset_table` also supports things like turning characters into lowercase or replacing one character with another. It can also define characters to be [ignored](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars), [blended](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars), or treated as a [phrase boundary](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary).

### Word-level tokenization

At the word level, the engine uses the [`min_word_len`](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len) setting to decide the minimum word length (in characters) that should be indexed.

Manticore also supports matching words with different forms. For example, to treat "car" and "cars" as the same word, you can use [morphology processors](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology).

If you want different words to be treated as the same—for example, "USA" and "United States" — you can define them using the [word forms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md) feature.

### Handling common and noisy words

Very common words (like "the", "and", "is") can slow down searches and increase index size. You can filter them out using [stop words](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords). This can make searches faster and the index smaller.

A more advanced filtering method is [bigrams](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index), which creates special tokens by combining a common word with an uncommon one. This can significantly speed up phrase searches when common words are involved.

### HTML content

If you're indexing HTML, it's usually best not to include the HTML tags in the index, since they add a lot of unnecessary content. You can use [HTML stripping](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#Stripping-HTML-tags) to remove the tags, but still index certain tag attributes or skip specific elements entirely.

### Token length limit

Keep in mind that Manticore has a **maximum token length of 42 characters**. Any word longer than this will be **truncated**. This limit applies during both indexing and searching, so it's important to ensure your data and queries account for it.
