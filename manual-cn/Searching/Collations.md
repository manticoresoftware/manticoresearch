# 排序规则

排序规则主要影响字符串属性的比较。它们定义了字符集编码以及Manticore在涉及字符串属性时执行 `ORDER BY` 或 `GROUP BY` 时用于比较字符串的策略。

在索引期间，字符串属性按原样存储，并没有附加任何字符集或语言信息。当Manticore只需要按字面值存储和返回字符串给调用应用程序时，这是可以的。然而，当你要求Manticore按字符串值排序时，这个请求就变得不明确了。

首先，单字节字符串（如ASCII、ISO-8859-1或Windows-1251）需要与UTF-8字符串（每个字符可能使用可变字节数编码）进行不同的处理。因此，我们需要知道字符集类型，以便将原始字节正确地解释为有意义的字符。

其次，我们还需要知道特定语言的字符串排序规则。例如，按美式英语（en_US本地化规则）排序时，带有分音符的`ï`（小写字母`i`）应该被放在`z`之后。然而，在使用法式法语（fr_FR本地化规则）排序时，它应该位于`i`和`j`之间。还有一些规则可能会完全忽略重音符号，将`ï`和`i`任意混合。

第三，在某些情况下，我们可能需要区分大小写排序，而在其他情况下则需要不区分大小写的排序。

排序规则封装了以下所有内容：字符集、语言规则和区分大小写的规则。Manticore目前提供了四种排序规则：

1. `libc_ci`
2. `libc_cs`
3. `utf8_general_ci`
4. `binary`

前两种排序规则依赖于多个标准C库（libc）调用，因此可以支持系统上安装的任何语言环境。它们分别提供不区分大小写(`_ci`)和区分大小写(`_cs`)的比较。默认情况下，它们使用C语言环境，实际上就是字节比较。要更改此设置，你需要使用 [collation_libc_locale](../Server_settings/Searchd.md#collation_libc_locale) 指令指定一个不同的可用语言环境。系统上可用的语言环境通常可以通过`locale`命令获取：

```bash
$ locale -a
C
en_AG
en_AU.utf8
en_BW.utf8
en_CA.utf8
en_DK.utf8
en_GB.utf8
en_HK.utf8
en_IE.utf8
en_IN
en_NG
en_NZ.utf8
en_PH.utf8
en_SG.utf8
en_US.utf8
en_ZA.utf8
en_ZW.utf8
es_ES
fr_FR
POSIX
ru_RU.utf8
ru_UA.utf8
```

具体的系统语言环境列表可能有所不同。请查阅操作系统文档以安装所需的额外语言环境。

`utf8_general_ci` 和 `binary` 是内置于Manticore中的排序规则。前者是针对UTF-8数据的通用排序规则（不含所谓的语言定制），其行为应类似于MySQL中的`utf8_general_ci`排序规则。后者则是简单的字节比较。

可以通过SQL中的 `SET collation_connection` 语句按会话覆盖排序规则。所有后续的SQL查询将使用此排序规则。否则，所有查询将使用服务器默认的排序规则，或者根据 [collation_server](../Server_settings/Searchd.md#collation_server) 配置指令指定的排序规则。Manticore目前默认使用`libc_ci`排序规则。

排序规则影响所有字符串属性的比较，包括 `ORDER BY` 和 `GROUP BY` 中的比较，因此选择不同的排序规则可能返回不同顺序或分组的结果。需要注意的是，排序规则不会影响全文搜索；对于全文搜索，请使用 [charset_table](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)。

<!-- proofread -->