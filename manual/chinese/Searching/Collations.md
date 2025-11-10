# 排序规则

排序规则主要影响字符串属性的比较。它们定义了字符集编码以及 Manticore 在执行涉及字符串属性的 `ORDER BY` 或 `GROUP BY` 时用于比较字符串的策略。

字符串属性在索引时按原样存储，并且不附加任何字符集或语言信息。只要 Manticore 仅需将字符串逐字存储并返回给调用应用程序，这种方式是可行的。然而，当你要求 Manticore 按字符串值排序时，请求立即变得模糊不清。

首先，单字节（ASCII、ISO-8859-1 或 Windows-1251）字符串需要与 UTF-8 字符串不同地处理，后者可能用可变字节数编码每个字符。因此，我们需要知道字符集类型，以便正确地将原始字节解释为有意义的字符。

其次，我们还需要知道特定语言的字符串排序规则。例如，在 en_US 区域设置中按美国规则排序时，带变音符的字符 `ï`（带分音符的小写字母 `i`）应放在 `z` 之后的某处。然而，在考虑法语规则和 fr_FR 区域设置时，它应放在 `i` 和 `j` 之间。其他规则集可能会完全忽略变音符，使 `ï` 和 `i` 可以任意混合。

第三，在某些情况下，我们可能需要区分大小写的排序，而在其他情况下，则需要不区分大小写的排序。

排序规则封装了以下所有内容：字符集、语言规则和大小写敏感性。Manticore 目前提供四种排序规则：

1. `libc_ci`
2. `libc_cs`
3. `utf8_general_ci`
4. `binary`

前两种排序规则依赖于多个标准 C 库（libc）调用，因此可以支持系统上安装的任何区域设置。它们分别提供不区分大小写（`_ci`）和区分大小写（`_cs`）的比较。默认情况下，它们使用 C 区域设置，实际上是按字节比较。要更改此设置，需要使用 [collation_libc_locale](../Server_settings/Searchd.md#collation_libc_locale) 指令指定不同的可用区域设置。系统上可用的区域设置列表通常可以通过 `locale` 命令获得：

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

具体的系统区域设置列表可能有所不同。请查阅操作系统文档以安装所需的额外区域设置。

`utf8_general_ci` 和 `binary` 区域设置内置于 Manticore。第一种是针对 UTF-8 数据的通用排序规则（没有所谓的语言定制）；其行为应类似于 MySQL 中的 `utf8_general_ci` 排序规则。第二种是简单的按字节比较。

排序规则可以通过 SQL 在每个会话基础上使用 `SET collation_connection` 语句覆盖。所有后续的 SQL 查询将使用该排序规则。否则，所有查询将使用服务器默认排序规则，或使用 [collation_server](../Server_settings/Searchd.md#collation_server) 配置指令中指定的排序规则。Manticore 目前默认使用 `libc_ci` 排序规则。

排序规则影响所有字符串属性的比较，包括 `ORDER BY` 和 `GROUP BY` 中的比较，因此根据所选排序规则，可能返回不同排序或分组的结果。请注意，排序规则不影响全文搜索；全文搜索请使用 [charset_table](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)。

<!-- proofread -->

