# 布尔优化
当布尔简化搜索选项（或相应的全局设置[服务器设置](../../Server_settings/Searchd.md#boolean_simplify)）设置为`1`（默认启用）时，全文本查询会自动进行优化。这种优化所做的一些更改包括：

* 过剩括号：`((A | B) | C)` 变为 `(A | B | C)`；`((A B) C)` 变为 `(A B C)`
* 过剩的AND NOT：`((A !N1) !N2)` 变为 `(A !(N1 | N2))`
* 共同的NOT：`((A !N) | (B !N))` 变为 `((A | B) !N)`
* 共同的复合NOT：`((A !(N AA)) | (B !(N BB)))` 变为 `(((A | B) !N) | (A !AA) | (B !BB))` 如果评估N的成本大于评估A和B的成本之和
* 共同的子项：`((A (N | AA)) | (B (N | BB)))` 变为 `(((A | B) N) | (A AA) | (B BB))` 如果评估N的成本大于评估A和B的成本之和
* 共同的关键字：`(A | "A B"~N)` 变为 `A`；`("A B" | "A B C")` 变为 `"A B"`；`("A B"~N | "A B C"~N)` 变为 `("A B"~N)`
* 共同的短语：`("X A B" | "Y A B")` 变为 `("X"|"Y") A B`
* 共同的AND NOT：`((A !X) | (A !Y) | (A !Z))` 变为 `(A !(X Y Z))`
* 共同的OR NOT：`((A !(N | N1)) | (B !(N | N2)))` 变为 `(( (A !N1) | (B !N2) ) !N)`
请注意，优化查询会消耗CPU时间，因此在某些情况下，对于非常简单的查询或手动优化的查询，通过将布尔简化设置为`0`来禁用优化可能会获得更好的结果。

默认情况下，不允许像`-dog`这样的查询，该查询可能包含集合中的所有文档。要允许它们，您必须指定`not_terms_only_allowed=1`作为[全局设置](../../Server_settings/Searchd.md#not_terms_only_allowed)或作为[搜索选项](../../Server_settings/Searchd.md#not_terms_only_allowed)。

<!-- proofread -->

