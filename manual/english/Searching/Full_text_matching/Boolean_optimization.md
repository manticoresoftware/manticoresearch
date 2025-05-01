# Boolean optimization
Queries can be automatically optimized if `OPTION boolean_simplify=1` is specified. Some transformations performed by this optimization include:

* Excess brackets: `((A | B) | C)` becomes `(A | B | C)`; `((A B) C)` becomes `(A B C)`
* Excess AND NOT: `((A !N1) !N2)` becomes `(A !(N1 | N2))`
* Common NOT: `((A !N) | (B !N))` becomes `((A | B) !N)`
* Common Compound NOT: `((A !(N AA)) | (B !(N BB)))` becomes `(((A | B) !N) | (A !AA) | (B !BB))` if the cost of evaluating N is greater than the sum of evaluating A and B
* Common subterm: `((A (N | AA)) | (B (N | BB)))` becomes `(((A | B) N) | (A AA) | (B BB))` if the cost of evaluating N is greater than the sum of evaluating A and B
* Common keywords: `(A | "A B"~N)` becomes `A`; `("A B" | "A B C")` becomes `"A B"`; `("A B"~N | "A B C"~N)` becomes `("A B"~N)`
* Common phrase: `("X A B" | "Y A B")` becomes `("("X"|"Y") A B")`
* Common AND NOT: `((A !X) | (A !Y) | (A !Z))` becomes `(A !(X Y Z))`
* Common OR NOT: `((A !(N | N1)) | (B !(N | N2)))` becomes `(( (A !N1) | (B !N2) ) !N)`
Note that optimizing queries consumes CPU time, so for simple queries or hand-optimized queries, you'll achieve better results with the default `boolean_simplify=0` value. Simplifications often benefit complex queries or algorithmically generated queries.

> NOTE: This is an experimental functionality and should be used with caution. It is recommended to verify that a query returns the same documents with and without adding `OPTION boolean_simplify=1`. While this optimization can simplify and improve the performance of complex or algorithmically generated queries, it also consumes additional CPU time. For simpler or manually optimized queries, the default `boolean_simplify=0` value might yield better results.

Queries like `-dog`, which could potentially include all documents from the collection are not allowed by default. To allow them, you must specify `not_terms_only_allowed=1` either as a [global setting](../../Server_settings/Searchd.md#not_terms_only_allowed) or as a [search option](../../Server_settings/Searchd.md#not_terms_only_allowed).

<!-- proofread -->
