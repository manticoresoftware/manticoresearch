# Full-text matching

Full-text matching is the core search functionality of Manticore Search. It allows you to search for documents based on the content of their text fields using sophisticated query syntax and operators.

## Getting started

The foundation of full-text search in Manticore is the `MATCH` clause. See [Basic usage](Basic_usage.md) to learn how to perform full-text searches.

## Query syntax and operators

Manticore supports a rich query syntax that allows you to build complex search queries:

* **[Operators](Operators.md)** - Learn about various full-text search operators like phrase search, boolean operators, proximity search, and more
* **[Escaping](Escaping.md)** - Understand how to properly escape special characters in your search queries

## Performance optimization

* **[Search profiling](Profiling.md)** - Analyze and optimize the performance of your full-text queries
* **[Boolean optimization](Boolean_optimization.md)** - Optimize boolean queries for better performance

Full-text search works by tokenizing both your documents during indexing and your search queries during searching. The tokenization process can be configured through various [NLP and tokenization settings](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) when creating your tables.

<!-- proofread -->