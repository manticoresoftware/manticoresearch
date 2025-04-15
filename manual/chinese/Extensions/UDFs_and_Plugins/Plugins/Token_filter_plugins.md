# 令牌过滤插件

令牌过滤插件允许您实现一个自定义的分词器，根据自定义规则创建令牌。其类型有两种：

* 索引时令牌器，声明于 [index_token_filter](../../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_token_filter) 表设置中
* 查询时令牌器，声明于 [token_filter](../../../Searching/Options.md#token_filter) 选项指令中

在文本处理管道中，令牌过滤器将在基础令牌处理之后运行（基础令牌处理处理来自字段或查询的文本并从中创建令牌）。

## 索引时令牌器

索引时令牌器由 `indexer` 在将源数据索引到表中时创建，或者在处理 `INSERT` 或 `REPLACE` 语句时由 RT 表创建。

插件声明为 `library name:plugin name:optional string of settings`。插件的初始化函数可以接受任意设置，这些设置可以作为字符串传递，格式为 `option1=value1;option2=value2;..`。

示例：

```ini
index_token_filter = my_lib.so:email_process:field=email;split=.io
```

索引时令牌过滤器的调用工作流程如下：

1.  `XXX_init()` 在 `indexer` 创建具有空字段列表的令牌过滤器后立即被调用，然后在索引器获取具有实际字段列表的表架构后被调用。它必须返回零以成功初始化，否则返回错误描述。
2.  `XXX_begin_document` 仅在 RT 表 `INSERT`/`REPLACE` 每个文档上被调用。它必须返回零以表示成功调用，否则返回错误描述。使用选项 `token_filter_options`，可以将附加参数/设置传递给函数。
    ```sql
    INSERT INTO rt (id, title) VALUES (1, 'some text corp@space.io') OPTION token_filter_options='.io'
    ```
3.  `XXX_begin_field` 在处理字段之前为每个字段调用一次，参数为字段编号。
4.  `XXX_push_token` 在基础令牌器生成每个新令牌时调用一次，参数为源令牌。它必须返回令牌、令牌过滤器生成的额外令牌的计数和令牌的增量位置。
5.  `XXX_get_extra_token` 在 `XXX_push_token` 报告额外令牌的情况下被多次调用。它必须返回该额外令牌及其增量位置。
6.  `XXX_end_field` 在处理当前字段的源令牌之后立即调用一次。
7.  `XXX_deinit` 在索引的最后调用。

以下函数是必需定义的：`XXX_begin_document`、`XXX_push_token` 和 `XXX_get_extra_token`。

## 查询时令牌过滤器

查询时令牌器在每个涉及的表上每次调用全文搜索时创建。

查询时令牌过滤器的调用工作流程如下：

1.  `XXX_init()` 在解析查询之前每个表调用一次，参数是最大令牌长度和由 `token_filter` 选项设置的字符串
    ```sql
    SELECT * FROM index WHERE MATCH ('test') OPTION token_filter='my_lib.so:query_email_process:io'
    ```
    它必须返回零以成功初始化，否则返回错误描述。
2.  `XXX_push_token()` 在基础令牌器生成每个新令牌时调用一次，参数为：基础令牌器生成的令牌、指向源查询字符串中原始令牌的指针和原始令牌长度。它必须返回令牌及其增量位置。
3.  `XXX_pre_morph()` 在令牌被传递给形态处理器之前调用一次，参数为令牌引用和停用词标志。它可以设置停用词标志以将令牌标记为停用词。
4.  `XXX_post_morph()` 在令牌被形态处理器处理后调用一次，参数为令牌引用和停用词标志。它可以设置停用词标志以将令牌标记为停用词。它必须返回一个标志，其非零值表示在形态处理之前使用该令牌。
5.  `XXX_deinit()` 在查询处理的最后调用。

缺少函数是可以容忍的。

<!-- proofread -->
