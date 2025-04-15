# Data tokenization

Manticore doesn't store text as is for performing full-text searching on it. Instead, it extracts words and creates several structures that allow fast full-text searching. From the found words, a dictionary is built, which allows a quick look to discover if the word is present or not in the index. In addition, other structures record the documents and fields in which the word was found (as well as the position of it inside a field). All these are used when a full-text match is performed.

The process of demarcating and classifying words is called tokenization. The tokenization is applied at both indexing and searching, and it operates at the character and word level.

On the character level, the engine allows only certain characters to pass. This is defined by the [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table). Anything else is replaced with a whitespace (which is considered the default word separator). The charset_table also allows mappings, such as lowercasing or simply replacing one character with another. Besides that, characters can be [ignored](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars), [blended](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars), defined as a [phrase boundary](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary). 

At the word level, the base setting is the [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len) which defines the minimum word length in characters to be accepted in the index. A common request is to match singular with plural forms of words. For this, [morphology processors](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) can be used. 

Going further, we might want a word to be matched as another one because they are synonyms. For this, the [word forms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md) feature can be used, which allows one or more words to be mapped to another one.

Very common words can have some unwanted effects on searching, mostly because of their frequency they require lots of computing to process their doc/hit lists. They can be blacklisted with the [stop words](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) functionality. This helps not only in speeding up queries but also in decreasing the index size.

A more advanced blacklisting is [bigrams](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index), which allows creating a special token between a "bigram" (common) word and an uncommon word. This can speed up several times when common words are used in phrase searches.

In case of indexing HTML content, it's important not to index the HTML tags, as they can introduce a lot of "noise" in the index. [HTML stripping](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#Stripping-HTML-tags) can be used and can be configured to strip, but index certain tag attributes or completely ignore the content of certain HTML elements.
<!-- proofread -->
