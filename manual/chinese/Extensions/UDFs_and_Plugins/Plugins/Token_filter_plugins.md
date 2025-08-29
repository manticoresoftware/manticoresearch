# 令牌过滤器插件

令牌过滤器插件允许您实现一个自定义的分词器，根据自定义规则创建令牌。有两种类型：

* 通过表设置中的 [index_token_filter](../../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_token_filter) 声明的索引时分词器
* 通过 [token_filter](../../../Searching/Options.md#token_filter) OPTION 指令声明的查询时分词器

在文本处理管道中，令牌过滤器将在基础分词器处理之后运行（基础分词器处理来自字段或查询的文本并从中创建令牌）。

## 索引时分词器

索引时分词器由 `indexer` 在将源数据索引到表中时创建，或由 RT 表在处理 `INSERT` 或 `REPLACE` 语句时创建。

插件声明格式为 `library name:plugin name:optional string of settings`。插件的初始化函数可以接受任意设置，这些设置以字符串形式传递，格式为 `option1=value1;option2=value2;..`。

示例：

```ini
index_token_filter = my_lib.so:email_process:field=email;split=.io
```

索引时令牌过滤器的调用流程如下：

1.  `XXX_init()` 在 `indexer` 创建令牌过滤器时被调用，此时字段列表为空，随后在 `indexer` 获取实际字段列表和表模式后再次调用。它必须返回零表示初始化成功，否则返回错误描述。
2.  `XXX_begin_document` 仅在 RT 表的 `INSERT`/`REPLACE` 中为每个文档调用。它必须返回零表示调用成功，否则返回错误描述。可以使用 OPTION `token_filter_options` 将额外参数/设置传递给该函数。
    ```sql
    INSERT INTO rt (id, title) VALUES (1, 'some text corp@space.io') OPTION token_filter_options='.io'
    ```
3.  `XXX_begin_field` 在处理字段的基础分词器前，为每个字段调用一次，参数为字段编号。
4.  `XXX_push_token` 为基础分词器生成的每个新令牌调用一次，参数为源令牌。它必须返回令牌、令牌过滤器生成的额外令牌计数，以及该令牌的位置增量。
5.  `XXX_get_extra_token` 在 `XXX_push_token` 报告额外令牌时会被多次调用。它必须返回该额外令牌及其位置增量。
6.  `XXX_end_field` 在当前字段的源令牌处理完后调用一次。
7.  `XXX_deinit` 在索引的最后被调用。

以下函数是必定义的：`XXX_begin_document`、`XXX_push_token` 和 `XXX_get_extra_token`。

## 查询时令牌过滤器

查询时令牌过滤器在每次执行全文搜索时为涉及的每个表创建。

查询时令牌过滤器的调用流程如下：

1.  `XXX_init()` 在每个表解析查询之前调用一次，参数为最大令牌长度和由 `token_filter` 选项设置的字符串
    ```sql
    SELECT * FROM index WHERE MATCH ('test') OPTION token_filter='my_lib.so:query_email_process:io'
    ```
    它必须返回零表示初始化成功，否则返回错误描述。
2.  `XXX_push_token()` 为基础分词器生成的每个新令牌调用一次，参数包括基础分词器生成的令牌、指向源查询字符串中原始令牌的指针和原始令牌长度。它必须返回令牌和该令牌的位置增量。
3.  `XXX_pre_morph()` 在令牌传递给形态学处理器之前调用一次，参数为令牌引用和停用词标志。它可以设置停用词标志，将令牌标记为停用词。
4.  `XXX_post_morph()` 在令牌经过形态学处理器后调用一次，参数为令牌引用和停用词标志。它可以设置停用词标志，将令牌标记为停用词。它必须返回一个标志，非零值表示使用形态学处理之前的令牌。
5.  `XXX_deinit()` 在查询处理的最后被调用。

缺少这些函数是被允许的。

<!-- proofread -->

