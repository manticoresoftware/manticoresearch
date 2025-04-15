# Collations

Collations primarily impact string attribute comparisons. They define both the character set encoding and the strategy Manticore employs for comparing strings when performing `ORDER BY` or `GROUP BY` with a string attribute involved.

String attributes are stored as-is during indexing, and no character set or language information is attached to them. This is fine as long as Manticore only needs to store and return the strings to the calling application verbatim. However, when you ask Manticore to sort by a string value, the request immediately becomes ambiguous.

First, single-byte (ASCII, ISO-8859-1, or Windows-1251) strings need to be processed differently than UTF-8 strings, which may encode each character with a variable number of bytes. Thus, we need to know the character set type to properly interpret the raw bytes as meaningful characters.

Second, we also need to know the language-specific string sorting rules. For example, when sorting according to US rules in the en_US locale, the accented character `ï` (small letter `i` with diaeresis) should be placed somewhere after `z`. However, when sorting with French rules and the fr_FR locale in mind, it should be placed between `i` and `j`. Some other set of rules might choose to ignore accents altogether, allowing `ï` and `i` to be mixed arbitrarily.

Third, in some cases, we may require case-sensitive sorting, while in others, case-insensitive sorting is needed.

Collations encapsulate all of the following: the character set, the language rules, and the case sensitivity. Manticore currently provides four collations:

1. `libc_ci`
2. `libc_cs`
3. `utf8_general_ci`
4. `binary`

The first two collations rely on several standard C library (libc) calls and can thus support any locale installed on your system. They provide case-insensitive (`_ci`) and case-sensitive (`_cs`) comparisons, respectively. By default, they use the C locale, effectively resorting to bytewise comparisons. To change that, you need to specify a different available locale using the [collation_libc_locale](../Server_settings/Searchd.md#collation_libc_locale) directive. The list of locales available on your system can usually be obtained with the `locale` command:

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

The specific list of system locales may vary. Consult your OS documentation to install additional needed locales.

`utf8_general_ci` and `binary` locales are built-in into Manticore. The first one is a generic collation for UTF-8 data (without any so-called language tailoring); it should behave similarly to the `utf8_general_ci` collation in MySQL. The second one is a simple bytewise comparison.

Collation can be overridden via SQL on a per-session basis using the `SET collation_connection` statement. All subsequent SQL queries will use this collation. Otherwise, all queries will use the server default collation or as specified in the [collation_server](../Server_settings/Searchd.md#collation_server) configuration directive. Manticore currently defaults to the `libc_ci` collation.

Collations affect all string attribute comparisons, including those within `ORDER BY` and `GROUP BY`, so differently ordered or grouped results can be returned depending on the collation chosen. Note that collations don't affect full-text searching; for that, use the [charset_table](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table).

<!-- proofread -->