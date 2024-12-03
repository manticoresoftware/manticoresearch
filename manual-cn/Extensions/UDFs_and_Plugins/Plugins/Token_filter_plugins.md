# Token 过滤器插件

Token 过滤器插件允许您根据自定义规则实现自定义的分词器。分为两种类型：

- 索引时分词器，由表设置中的 [index_token_filter](../../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_token_filter) 声明
- 查询时分词器，由 [token_filter](../../../Searching/Options.md#token_filter) OPTION 指令声明

在文本处理流程中，token 过滤器将在基础分词器处理之后运行（基础分词器处理字段或查询的文本，并将其转换为 token）。

## 索引时分词器

索引时分词器由 `indexer` 在将源数据索引到表中时创建，或由 RT 表在处理 `INSERT` 或 `REPLACE` 语句时创建。

插件的声明格式为 `库名:插件名:可选的设置字符串`。插件的初始化函数可以接受任意的设置，这些设置可以通过 `option1=value1;option2=value2;..` 格式的字符串传递。

示例：

```ini
index_token_filter = my_lib.so:email_process:field=email;split=.io
```

索引时 token 过滤器的调用流程如下：

1.  `XXX_init()` 在 `indexer` 创建 token 过滤器后立即被调用，此时字段列表为空；之后，当 indexer 获取表架构并包含实际字段列表时再次调用。如果初始化成功则返回零，否则返回错误描述。
2.  `XXX_begin_document` 仅在 RT 表的 `INSERT`/`REPLACE` 操作时为每个文档调用一次。如果调用成功则返回零，否则返回错误描述。可以通过 OPTION `token_filter_options` 传递额外的参数/设置给函数。
    
    ```sql
    INSERT INTO rt (id, title) VALUES (1, 'some text corp@space.io') OPTION token_filter_options='.io'
    ```
3.  `XXX_begin_field` 在处理每个字段之前调用一次，传递字段编号作为参数。
4.  `XXX_push_token` 在基础分词器生成每个新 token 时调用一次，将源 token 作为参数传递。它必须返回 token、由 token 过滤器生成的额外 token 数量，以及 token 的位置增量。
5.  如果 `XXX_push_token` 报告了额外 token，则会多次调用 `XXX_get_extra_token`。它必须返回额外 token 及其位置增量。
6.  在当前字段的源 token 处理完成后，`XXX_end_field` 会被调用一次。
7.  在索引结束时，`XXX_deinit` 会被调用。

以下函数必须定义：`XXX_begin_document`、`XXX_push_token` 和 `XXX_get_extra_token`。

## 查询时分词器

查询时分词器在每次搜索时为每个涉及的表创建，并在每次调用全文搜索时执行。

查询时 token 过滤器的调用流程如下：

1.  在解析查询之前，每个表会调用一次 `XXX_init()`，并传递最大 token 长度和由 `token_filter` 选项设置的字符串作为参数。
    ```sql
    SELECT * FROM index WHERE MATCH ('test') OPTION token_filter='my_lib.so:query_email_process:io'
    ```
    如果初始化成功则返回零，否则返回错误描述。
2.  基础分词器生成每个新 token 时，`XXX_push_token()` 被调用一次，参数包括：基础分词器生成的 token、源查询字符串中的原始 token 指针，以及原始 token 长度。它必须返回 token 和 token 的位置增量。
3.  在 token 传递给形态学处理器之前，`XXX_pre_morph()` 会被调用一次，并传递 token 和停用词标志的引用。可以设置停用词标志，将该 token 标记为停用词。
4.  在 token 经过形态学处理器处理之后，`XXX_post_morph()` 会被调用一次，并传递 token 和停用词标志的引用。可以设置停用词标志，将该 token 标记为停用词。它必须返回一个标志，非零值表示使用形态学处理之前的 token。
5.  在查询处理结束时，`XXX_deinit()` 会被调用。

函数的缺失是允许的。

<!-- proofread -->