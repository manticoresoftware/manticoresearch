# Boolean optimization
Full-text queries are automatically optimized when the `boolean_simplify` search option (or the corresponding [global setting](../../Server_settings/Searchd.md#boolean_simplify)) is set to `1` (enabled by default). Some of the changes made by this optimization include:

* Excess brackets: `((A | B) | C)` becomes `(A | B | C)`; `((A B) C)` becomes `(A B C)`
* Excess AND NOT: `((A !N1) !N2)` becomes `(A !(N1 | N2))`
* Common NOT: `((A !N) | (B !N))` becomes `((A | B) !N)`
* Common Compound NOT: `((A !(N AA)) | (B !(N BB)))` becomes `(((A | B) !N) | (A !AA) | (B !BB))` if the cost of evaluating N is greater than the sum of evaluating A and B
* Common subterm: `((A (N | AA)) | (B (N | BB)))` becomes `(((A | B) N) | (A AA) | (B BB))` if the cost of evaluating N is greater than the sum of evaluating A and B
* Common keywords: `(A | "A B"~N)` becomes `A`; `("A B" | "A B C")` becomes `"A B"`; `("A B"~N | "A B C"~N)` becomes `("A B"~N)`
* Common phrase: `("X A B" | "Y A B")` becomes `("("X"|"Y") A B")`
* Common AND NOT: `((A !X) | (A !Y) | (A !Z))` becomes `(A !(X Y Z))`
* Common OR NOT: `((A !(N | N1)) | (B !(N | N2)))` becomes `(( (A !N1) | (B !N2) ) !N)`
Note that optimizing queries consumes CPU time, so in some cases for very simple queries or hand-optimized queries, you may achieve better results by disabling optimization with `boolean_simplify=0`.

Queries like `-dog`, which could potentially include all documents from the collection are not allowed by default. To allow them, you must specify `not_terms_only_allowed=1` either as a [global setting](../../Server_settings/Searchd.md#not_terms_only_allowed) or as a [search option](../../Server_settings/Searchd.md#not_terms_only_allowed).

<!-- proofread -->
