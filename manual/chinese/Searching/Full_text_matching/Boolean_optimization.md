# 布尔优化
全文查询在设置了 `boolean_simplify` 搜索选项（或对应的 [全局设置](../../Server_settings/Searchd.md#boolean_simplify)）为 `1`（默认启用）时，会自动进行优化。此优化所做的一些更改包括：

* 多余的括号：`((A | B) | C)` 变为 `(A | B | C)`；`((A B) C)` 变为 `(A B C)`
* 多余的 AND NOT：`((A !N1) !N2)` 变为 `(A !(N1 | N2))`
* 公共 NOT：`((A !N) | (B !N))` 变为 `((A | B) !N)`
* 共同复合 NOT：`((A !(N AA)) | (B !(N BB)))` 如果评估 N 的代价大于评估 A 和 B 之和，变为 `(((A | B) !N) | (A !AA) | (B !BB))`
* 共同子项：`((A (N | AA)) | (B (N | BB)))` 如果评估 N 的代价大于评估 A 和 B 之和，变为 `(((A | B) N) | (A AA) | (B BB))`
* 共同关键字：`(A | "A B"~N)` 变为 `A`；`("A B" | "A B C")` 变为 `"A B"`；`("A B"~N | "A B C"~N)` 变为 `("A B"~N)`
* 共同短语：`("X A B" | "Y A B")` 变为 `("("X"|"Y") A B")`
* 共同 AND NOT：`((A !X) | (A !Y) | (A !Z))` 变为 `(A !(X Y Z))`
* 共同 OR NOT：`((A !(N | N1)) | (B !(N | N2)))` 变为 `(( (A !N1) | (B !N2) ) !N)`
注意，优化查询会消耗 CPU 时间，因此在某些非常简单的查询或手工优化的查询中，禁用优化（`boolean_simplify=0`）可能获得更好的结果。

像 `-dog` 这样的查询，默认不允许因为它可能包含集合中的所有文档。要允许它们，必须在 [全局设置](../../Server_settings/Searchd.md#not_terms_only_allowed)或[搜索选项](../../Server_settings/Searchd.md#not_terms_only_allowed)中指定 `not_terms_only_allowed=1`。

<!-- proofread -->

