# 令牌过滤器插件

令牌过滤器插件允许您实现根据自定义规则创建令牌的自定义分词器。有两种类型：

* 由表设置中的 [index_token_filter](../../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_token_filter) 声明的索引时分词器
* 由 [token_filter](../../../Searching/Options.md#token_filter) OPTION 指令声明的查询时分词器

在文本处理管道中，令牌过滤器将在基本分词器处理完成后运行（基本分词器处理来自字段或查询的文本并生成令牌）。

## 索引时分词器

索引时分词器由 `indexer` 在将源数据索引到表中时创建，或者由 RT 表在处理 `INSERT` 或 `REPLACE` 语句时创建。

插件声明格式为 `library name:plugin name:optional string of settings`。插件的初始化函数可接受任意设置，这些设置可以以 `option1=value1;option2=value2;..` 的字符串格式传递。

示例：

```ini
index_token_filter = my_lib.so:email_process:field=email;split=.io
```

索引时令牌过滤器的调用工作流程如下：

1.  `XXX_init()` 在 `indexer` 创建令牌过滤器并传入空字段列表后，以及在 indexer 获取表结构及实际字段列表后被调用。它必须返回零表示初始化成功，否则返回错误描述。
2.  `XXX_begin_document` 只在 RT 表的 `INSERT`/`REPLACE` 每个文档时调用。它必须返回零表示调用成功，否则返回错误描述。通过 OPTION `token_filter_options`，可以向该函数传递额外参数/设置。
    ```sql
    INSERT INTO rt (id, title) VALUES (1, 'some text corp@space.io') OPTION token_filter_options='.io'
    ```
3.  `XXX_begin_field` 在处理字段的基本分词器之前对每个字段调用一次，参数为字段编号。
4.  `XXX_push_token` 对基本分词器生成的每个新令牌调用一次，参数为源令牌。它必须返回令牌、由令牌过滤器产生的额外令牌数量及该令牌的位置偏移。
5.  `XXX_get_extra_token` 如果 `XXX_push_token` 指出存在额外令牌，则多次调用。它必须返回额外令牌及其位置偏移。
6.  `XXX_end_field` 在当前字段的源令牌处理完成后调用一次。
7.  `XXX_deinit` 在索引结束时调用。

必须定义以下函数：`XXX_begin_document`、`XXX_push_token` 和 `XXX_get_extra_token`。

## 查询时令牌过滤器

查询时分词器在每次全文搜索调用时，对所涉及的每个表都会创建。

查询时令牌过滤器的调用工作流程如下：

1.  `XXX_init()` 在解析查询之前对每个表调用一次，参数为最大令牌长度和通过 `token_filter` 选项设置的字符串
    ```sql
    SELECT * FROM index WHERE MATCH ('test') OPTION token_filter='my_lib.so:query_email_process:io'
    ```
    它必须返回零表示初始化成功，否则返回错误描述。
2.  `XXX_push_token()` 对基本分词器生成的每个令牌调用一次，参数包括基本分词器生成的令牌、源查询字符串中的原始令牌指针和原始令牌长度。它必须返回令牌及其位置偏移。
3.  `XXX_pre_morph()` 在令牌传递给形态学处理器前调用一次，参数为令牌引用和停用词标志。该函数可能会设置停用词标志，将该令牌标记为停用词。
4.  `XXX_post_morph()` 在令牌经过形态学处理器处理后调用一次，参数为令牌引用和停用词标志。该函数可能会设置停用词标志，将该令牌标记为停用词。它必须返回一个标志，非零表示使用形态学处理前的令牌。
5.  `XXX_deinit()` 在查询处理结束时调用。

缺少这些函数是允许的。

<!-- proofread -->

