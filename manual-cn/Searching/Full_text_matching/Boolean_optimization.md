# 布尔优化
如果指定 `OPTION boolean_simplify=1`，查询可以自动优化。此优化执行的一些转换包括：

- 多余的括号：`((A | B) | C)` 变为 `(A | B | C)`；`((A B) C)` 变为 `(A B C)`
- 多余的 AND NOT：`((A !N1) !N2)` 变为 `(A !(N1 | N2))`
- 共同的 NOT：`((A !N) | (B !N))` 变为 `((A | B) !N)`
- 共同的复合 NOT：`((A !(N AA)) | (B !(N BB)))` 变为 `(((A | B) !N) | (A !AA) | (B !BB))`，如果评估 N 的成本大于评估 A 和 B 的成本总和
- 共同的子项：`((A (N | AA)) | (B (N | BB)))` 变为 `(((A | B) N) | (A AA) | (B BB))`，如果评估 N 的成本大于评估 A 和 B 的成本总和
- 共同的关键词：`(A | "A B"~N)` 变为 `A`；`("A B" | "A B C")` 变为 `"A B"`；`("A B"~N | "A B C"~N)` 变为 `("A B"~N)`
- 共同的短语：`("X A B" | "Y A B")` 变为 `("("X"|"Y") A B")`
- 共同的 AND NOT：`((A !X) | (A !Y) | (A !Z))` 变为 `(A !(X Y Z))`
- 共同的 OR NOT：`((A !(N | N1)) | (B !(N | N2)))` 变为 `(( (A !N1) | (B !N2) ) !N)`

请注意，优化查询会消耗 CPU 时间，因此对于简单查询或手动优化的查询，使用默认的 `boolean_simplify=0` 会获得更好的结果。简化通常对复杂查询或算法生成的查询有益。

像 `-dog` 这样的查询，可能会包含集合中的所有文档，默认情况下是不允许的。要允许它们，您必须在 [全局设置](../../Server_settings/Searchd.md#not_terms_only_allowed)或 [搜索选项](../../Server_settings/Searchd.md#not_terms_only_allowed) 中指定 `not_terms_only_allowed=1`。

<!-- proofread -->
