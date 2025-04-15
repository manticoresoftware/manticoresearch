# Token filter plugins

Token filter plugins allow you to implement a custom tokenizer that creates tokens according to custom rules. There are two types:

* Index-time tokenizer declared by [index_token_filter](../../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_token_filter) in table settings
* Query-time tokenizer declared by [token_filter](../../../Searching/Options.md#token_filter) OPTION directive

In the text processing pipeline, token filters will run after the base tokenizer processing occurs (which processes the text from fields or queries and creates tokens out of them).

## Index-time tokenizer

Index-time tokenizer is created by `indexer` when indexing source data into a table or by an RT table when processing `INSERT` or `REPLACE` statements.

Plugin is declared as `library name:plugin name:optional string of settings`. The init functions of the plugin can accept arbitrary settings that can be passed as a string in the format `option1=value1;option2=value2;..`.

Example:

```ini
index_token_filter = my_lib.so:email_process:field=email;split=.io
```

The call workflow for index-time token filter is as follows:

1.  `XXX_init()` gets called right after `indexer` creates token filter with an empty fields list and then after indexer gets the table schema with the actual fields list. It must return zero for successful initialization or an error description otherwise.
2.  `XXX_begin_document` gets called only for RT table `INSERT`/`REPLACE` for every document. It must return zero for a successful call or an error description otherwise. Using OPTION `token_filter_options`, additional parameters/settings can be passed to the function.
    ```sql
    INSERT INTO rt (id, title) VALUES (1, 'some text corp@space.io') OPTION token_filter_options='.io'
    ```
3.  `XXX_begin_field` gets called once for each field prior to processing the field with the base tokenizer, with the field number as its parameter.
4.  `XXX_push_token` gets called once for each new token produced by the base tokenizer, with the source token as its parameter. It must return the token, count of extra tokens made by the token filter, and delta position for the token.
5.  `XXX_get_extra_token` gets called multiple times in case `XXX_push_token` reports extra tokens. It must return the token and delta position for that extra token.
6.  `XXX_end_field` gets called once right after the source tokens from the current field are processed.
7.  `XXX_deinit` gets called at the very end of indexing.

The following functions are mandatory to be defined: `XXX_begin_document`, `XXX_push_token`, and `XXX_get_extra_token`.

## query-time token filter

Query-time tokenizer gets created on search each time full-text is invoked by every table involved.

The call workflow for query-time token filter is as follows:

1.  `XXX_init()` gets called once per table prior to parsing the query with parameters - max token length and a string set by the `token_filter` option
    ```sql
    SELECT * FROM index WHERE MATCH ('test') OPTION token_filter='my_lib.so:query_email_process:io'
    ```
    It must return zero for successful initialization or error description otherwise.
2.  `XXX_push_token()` gets called once for each new token produced by the base tokenizer with parameters: token produced by the base tokenizer, pointer to raw token at source query string, and raw token length. It must return the token and delta position for the token.
3.  `XXX_pre_morph()` gets called once for the token right before it gets passed to the morphology processor with a reference to the token and stopword flag. It might set the stopword flag to mark the token as a stopword.
4.  `XXX_post_morph()` gets called once for the token after it is processed by the morphology processor with a reference to the token and stopword flag. It might set the stopword flag to mark the token as a stopword. It must return a flag, the non-zero value of which means to use the token prior to morphology processing.
5.  `XXX_deinit()` gets called at the very end of query processing.

Absence of the functions is tolerated.

<!-- proofread -->