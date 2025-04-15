# 排序规则

排序规则主要影响字符串属性的比较。它们定义了字符集编码和Manticore在执行涉及字符串属性的`ORDER BY`或`GROUP BY`时用于比较字符串的策略。

字符串属性在索引过程中原样存储，并且没有附加字符集或语言信息。这没问题，只要Manticore只需要将字符串逐字存储并返回给调用应用程序。然而，当你要求Manticore按字符串值排序时，请求立即变得模糊不清。

首先，单字节（ASCII，ISO-8859-1或Windows-1251）字符串需要与UTF-8字符串不同的处理，后者可能会用可变字节数对每个字符进行编码。因此，我们需要知道字符集类型，以便正确解释原始字节为有意义的字符。

其次，我们还需要知道特定语言的字符串排序规则。例如，当根据美国规则在en_US区域进行排序时，带有变音符号的字符`ï`（小写字母`i`带有分音符）应放在`z`之后。但是，当考虑法国语法和fr_FR区域的排序时，它应放在`i`和`j`之间。某些其他规则可能会选择完全忽略变音符号，从而允许`ï`和`i`随意混合。

第三，在某些情况下，我们可能需要区分大小写的排序，而在其他情况下则需要不区分大小写的排序。

排序规则封装了以下所有内容：字符集、语言规则和大小写敏感性。Manticore当前提供四种排序规则：

1. `libc_ci`
2. `libc_cs`
3. `utf8_general_ci`
4. `binary`

前两个排序规则依赖于几个标准C库（libc）调用，因此可以支持您系统上安装的任何区域。它们分别提供不区分大小写（`_ci`）和区分大小写（`_cs`）的比较。默认情况下，它们使用C区域，有效地转为逐字节比较。要更改这一点，您需要使用[collation_libc_locale](../Server_settings/Searchd.md#collation_libc_locale)指令指定不同的可用区域。您系统上可用区域的列表通常可以通过`locale`命令获得：

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

系统区域的具体列表可能会有所不同。请查阅您的操作系统文档以安装所需的其他区域。

`utf8_general_ci`和`binary`区域是内置于Manticore中的。第一个是适用于UTF-8数据的通用排序规则（没有所谓的语言调整）；它的行为应类似于MySQL中的`utf8_general_ci`排序规则。第二个是简单的逐字节比较。

排序规则可以通过SQL按会话逐个覆盖，使用`SET collation_connection`语句。所有后续的SQL查询将使用此排序规则。否则，所有查询将使用服务器默认的排序规则或在[collation_server](../Server_settings/Searchd.md#collation_server)配置指令中指定的规则。Manticore当前默认为`libc_ci`排序规则。

排序规则影响所有字符串属性的比较，包括那些在`ORDER BY`和`GROUP BY`中的比较，因此可以根据所选择的排序规则返回不同顺序或分组的结果。请注意，排序规则不影响全文搜索；要进行全文搜索，请使用[charset_table](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)。

<!-- proofread -->
