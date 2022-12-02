# Token filter plugins

Token filter plugins let you implement a custom tokenizer that makes tokens according to custom rules. There are two type:

* Index-time tokenizer declared by [index_token_filter](../../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_token_filter) in table settings
* query-time tokenizer declared by [token_filter](../../../Searching/Options.md#token_filter) OPTION directive

In the text processing pipeline, the token filters will run after the base tokenizer processing occurs (which processes the text from field or query and creates tokens out of them).

## Index-time tokenizer

Index-time tokenizer gets created by `indexer` on indexing source data into a table or by an RT table on processing `INSERT` or `REPLACE` statements.

Plugin is declared as `library name:plugin name:optional string of settings`. The init functions of the plugin can accept arbitrary settings that can be passed as a string in format `option1=value1;option2=value2;..`.

Example:

```ini
index_token_filter = my_lib.so:email_process:field=email;split=.io
```

The call workflow for index-time token filter is as follows:

1.  `XXX_init()` gets called right after `indexer` creates token filter with empty fields list then after indexer got table schema with actual fields list. It must return zero for successful initialization or error description otherwise.
2.  `XXX_begin_document` gets called only for RT table `INSERT`/`REPLACE` for every document. It must return zero for successful call or error description otherwise. Using OPTION `token_filter_options` additional parameters/settings can be passed to the function.
    ```sql
    INSERT INTO rt (id, title) VALUES (1, 'some text corp@space.io') OPTION token_filter_options='.io'
    ```
3.  `XXX_begin_field` gets called once for each field prior to processing field with base tokenizer with field number as its parameter.
4.  `XXX_push_token` gets called once for each new token produced by base tokenizer with source token as its parameter. It must return token, count of extra tokens made by token filter and delta position for token.
5.  `XXX_get_extra_token` gets called multiple times in case `XXX_push_token` reports extra tokens. It must return token and delta position for that extra token.
6.  `XXX_end_field` gets called once right after source tokens from current field get over.
7.  `XXX_deinit` gets called in the very end of indexing.

The following functions are mandatory to be defined: `XXX_begin_document` and `XXX_push_token` and `XXX_get_extra_token`.

## query-time token filter

Query-time tokenizer gets created on search each time full-text invoked by every table involved.

The call workflow for query-time token filter is as follows:

1.  `XXX_init()` gets called once per table prior to parsing query with parameters - max token length and string set by `token_filter` option
    ```sql
    SELECT * FROM index WHERE MATCH ('test') OPTION token_filter='my_lib.so:query_email_process:io'
    ```
    It must return zero for successful initialization or error description otherwise.
2.  `XXX_push_token()` gets called once for each new token produced by base tokenizer with parameters: token produced by base tokenizer, pointer to raw token at source query string and raw token length. It must return token and delta position for token.
3.  `XXX_pre_morph()` gets called once for token right before it got passed to morphology processor with reference to token and stopword flag. It might set stopword flag to mark token as stopword.
4.  `XXX_post_morph()` gets called once for token after it processed by morphology processor with reference to token and stopword flag. It might set stopword flag to mark token as stopword. It must return flag non-zero value of which means to use token prior to morphology processing.
5.  `XXX_deinit()` gets called in the very end of query processing.

Absence of any of the functions is tolerated.
