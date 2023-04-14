# Boolean optimization
Queries can be automatically optimized if `OPTION boolean_simplify=1` is specified. Some transformations performed by this optimization include:

* Excess brackets: ((A | B) | C) becomes (A | B | C); ((A B) C) becomes (A B C)
* Excess AND NOT: ((A !N1) !N2) becomes (A !(N1 | N2))
* Common NOT: ((A !N) | (B !N)) becomes ((A | B) !N)
* Common Compound NOT: ((A !(N AA)) | (B !(N BB))) becomes (((A | B) !N) | (A !AA) | (B !BB)) if the cost of evaluating N is greater than the sum of evaluating A and B
* Common subterm: ((A (N | AA)) | (B (N | BB))) becomes (((A | B) N) | (A AA) | (B BB)) if the cost of evaluating N is greater than the sum of evaluating A and B
* Common keywords: (A | "A B"~N) becomes A; ("A B" | "A B C") becomes "A B"; ("A B"~N | "A B C"~N) becomes ("A B"~N)
* Common phrase: ("X A B" | "Y A B") becomes ("("X"|"Y") A B")
* Common AND NOT: ((A !X) | (A !Y) | (A !Z)) becomes (A !(X Y Z))
* Common OR NOT: ((A !(N | N1)) | (B !(N | N2))) becomes (( (A !N1) | (B !N2) ) !N)
Note that optimizing queries consumes CPU time, so for simple queries or hand-optimized queries, you'll achieve better results with the default `boolean_simplify=0` value. Simplifications often benefit complex queries or algorithmically generated queries.

Queries like "-dog," which implicitly include all documents from the collection, cannot be evaluated. This is due to both technical and performance reasons. Technically, Manticore does not always maintain a list of all IDs. Performance-wise, evaluating such queries could take a long time when the collection is massive (e.g., 10-100M documents).
 
<!-- proofread -->