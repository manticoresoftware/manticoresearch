# 布尔优化
如果指定了 `OPTION boolean_simplify=1` ，查询可以被自动优化。这种优化执行的一些转换包括：

* 多余的括号: `((A | B) | C)` 变为 `(A | B | C)` ; `((A B) C)` 变为 `(A B C)`
* 多余的 AND NOT: `((A !N1) !N2)` 变为 `(A !(N1 | N2))`
* 公共 NOT: `((A !N) | (B !N))` 变为 `((A | B) !N)`
* 公共复合 NOT: `((A !(N AA)) | (B !(N BB)))` 变为 `(((A | B) !N) | (A !AA) | (B !BB))` 如果评估 N 的成本大于评估 A 和 B 的总和
* 公共子项: `((A (N | AA)) | (B (N | BB)))` 变为 `(((A | B) N) | (A AA) | (B BB))` 如果评估 N 的成本大于评估 A 和 B 的总和
* 公共关键字: `(A | "A B"~N)` 变为 `A` ; `("A B" | "A B C")` 变为 `"A B"` ; `("A B"~N | "A B C"~N)` 变为 `("A B"~N)`
* 公共短语: `("X A B" | "Y A B")` 变为 `("("X"|"Y") A B")`
* 公共 AND NOT: `((A !X) | (A !Y) | (A !Z))` 变为 `(A !(X Y Z))`
* 公共 OR NOT: `((A !(N | N1)) | (B !(N | N2)))` 变为 `(( (A !N1) | (B !N2) ) !N)`
注意，优化查询会消耗 CPU 时间，因此对于简单查询或手动优化的查询，您会发现使用默认值 `boolean_simplify=0` 得到更好的结果。简化通常对复杂查询或算法生成的查询有利。

> 注意：这是一个实验性功能，使用时请谨慎。建议验证查询在添加和不添加 `OPTION boolean_simplify=1` 的情况下返回相同的文档。虽然这种优化可以简化并提高复杂或算法生成查询的性能，但它也会消耗额外的 CPU 时间。对于简单或手动优化的查询，默认 `boolean_simplify=0` 值可能会获得更好的结果。

像 `-dog` 这样的查询，可能会包含所有集合中的文档，默认情况下是不允许的。要允许它们，您必须指定 `not_terms_only_allowed=1` ，作为 [全局设置](../../Server_settings/Searchd.md#not_terms_only_allowed) 或作为 [搜索选项](../../Server_settings/Searchd.md#not_terms_only_allowed)。

<!-- proofread -->
