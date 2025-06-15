# 布尔优化
如果指定了 `OPTION boolean_simplify=1`，查询可以被自动优化。此优化执行的一些转换包括：

* 多余的括号：`((A | B) | C)` 变为 `(A | B | C)`；`((A B) C)` 变为 `(A B C)`
* 多余的 AND NOT：`((A !N1) !N2)` 变为 `(A !(N1 | N2))`
* 通用的 NOT：`((A !N) | (B !N))` 变为 `((A | B) !N)`
* 通用的复合 NOT：`((A !(N AA)) | (B !(N BB)))` 变为 `(((A | B) !N) | (A !AA) | (B !BB))`，如果评估 N 的代价大于评估 A 和 B 之和
* 通用子项：`((A (N | AA)) | (B (N | BB)))` 变为 `(((A | B) N) | (A AA) | (B BB))`，如果评估 N 的代价大于评估 A 和 B 之和
* 通用关键词：`(A | "A B"~N)` 变为 `A`；`("A B" | "A B C")` 变为 `"A B"`；`("A B"~N | "A B C"~N)` 变为 `("A B"~N)`
* 通用短语：`("X A B" | "Y A B")` 变为 `("("X"|"Y") A B")`
* 通用 AND NOT：`((A !X) | (A !Y) | (A !Z))` 变为 `(A !(X Y Z))`
* 通用 OR NOT：`((A !(N | N1)) | (B !(N | N2)))` 变为 `(( (A !N1) | (B !N2) ) !N)`
请注意，优化查询会消耗 CPU 时间，因此对于简单查询或手动优化的查询，使用默认的 `boolean_simplify=0` 值会获得更好的结果。简化通常有利于复杂查询或算法生成的查询。

> 注意：这是一个实验性功能，应谨慎使用。建议验证是否在添加 `OPTION boolean_simplify=1` 后查询返回相同的文档。虽然此优化可以简化和提高复杂或算法生成查询的性能，但也会消耗额外的 CPU 时间。对于更简单或手动优化的查询，默认的 `boolean_simplify=0` 值可能表现更佳。

像 `-dog` 这样可能包含集合中所有文档的查询默认不允许。要允许它们，必须将 `not_terms_only_allowed=1` 指定为[全局设置](../../Server_settings/Searchd.md#not_terms_only_allowed)或[搜索选项](../../Server_settings/Searchd.md#not_terms_only_allowed)。

<!-- proofread -->

