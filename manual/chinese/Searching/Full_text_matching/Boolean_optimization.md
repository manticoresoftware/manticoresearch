# 布尔优化
全文搜索查询在设置了 `OPTION boolean_simplify=1` 时会自动进行优化（此选项默认启用）。此优化所做的一些转换包括：

* 多余的括号：`((A | B) | C)` 变为 `(A | B | C)`；`((A B) C)` 变为 `(A B C)`
* 多余的 AND NOT：`((A !N1) !N2)` 变为 `(A !(N1 | N2))`
* 共有的 NOT：`((A !N) | (B !N))` 变为 `((A | B) !N)`
* 共有的复合 NOT：`((A !(N AA)) | (B !(N BB)))` 变为 `(((A | B) !N) | (A !AA) | (B !BB))`，条件是评估 N 的成本大于评估 A 和 B 成本之和
* 共有的子项：`((A (N | AA)) | (B (N | BB)))` 变为 `(((A | B) N) | (A AA) | (B BB))`，条件是评估 N 的成本大于评估 A 和 B 成本之和
* 共有的关键词：`(A | "A B"~N)` 变为 `A`；`("A B" | "A B C")` 变为 `"A B"`；`("A B"~N | "A B C"~N)` 变为 `("A B"~N)`
* 共有的短语：`("X A B" | "Y A B")` 变为 `("("X"|"Y") A B")`
* 共有的 AND NOT：`((A !X) | (A !Y) | (A !Z))` 变为 `(A !(X Y Z))`
* 共有的 OR NOT：`((A !(N | N1)) | (B !(N | N2)))` 变为 `(( (A !N1) | (B !N2) ) !N)`
请注意，优化查询会消耗 CPU 时间，因此在某些极其简单或手动优化的查询情况下，禁用优化（通过 `boolean_simplify=0`）可能会获得更好的效果。

像 `-dog` 这样的查询，可能会包含集合中的所有文档，默认是不允许的。若要允许此类查询，必须将 `not_terms_only_allowed=1` 设置为[全局设置](../../Server_settings/Searchd.md#not_terms_only_allowed)或者作为[搜索选项](../../Server_settings/Searchd.md#not_terms_only_allowed)指定。

<!-- proofread -->

