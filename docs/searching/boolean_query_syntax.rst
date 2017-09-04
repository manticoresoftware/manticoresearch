.. _boolean_query_syntax:

Boolean query syntax
--------------------

Boolean queries allow the following special operators to be used:

-  operator OR:

.. code-block:: mysql

       hello | world

-  operator NOT:

.. code-block:: mysql


       hello -world
       hello !world

-  grouping:

.. code-block:: mysql

       ( hello world )

Here's an example query which uses all these operators:

Example 5.1. Boolean query example
                                  

.. code-block:: mysql


    ( cat -dog ) | ( cat -mouse)

There always is implicit AND operator, so “hello world” query actually
means “hello & world”.

OR operator precedence is higher than AND, so “looking for cat \| dog \|
mouse” means “looking for ( cat \| dog \| mouse )” and *not* “(looking
for cat) \| dog \| mouse”.

Queries may be automatically optimized if OPTION boolean_simplify=1 is
specified. Some transformations performed by this optimization include:

-  Excess brackets: ((A \| B) \| C) becomes ( A \| B \| C ); ((A B) C)
   becomes ( A B C )

-  Excess AND NOT: ((A !N1) !N2) becomes (A !(N1 \| N2))

-  Common NOT: ((A !N) \| (B !N)) becomes ((A\|B) !N)

-  Common Compound NOT: ((A !(N AA)) \| (B !(N BB))) becomes (((A\|B)
   !N) \| (A !AA) \| (B !BB)) if the cost of evaluating N is greater
   than the added together costs of evaluating A and B

-  Common subterm: ((A (N \| AA)) \| (B (N \| BB))) becomes (((A\|B) N)
   \| (A AA) \| (B BB)) if the cost of evaluating N is greater than the
   added together costs of evaluating A and B

-  Common keywords: (A \| “A B”~N) becomes A; (“A B” \| “A B C”) becomes
   “A B”; (“A B”~N \| “A B C”~N) becomes (“A B”~N)

-  Common phrase: (“X A B” \| “Y A B”) becomes ((“X\|Y”) “A B”)

-  Common AND NOT: ((A !X) \| (A !Y) \| (A !Z)) becomes (A !(X Y Z))

-  Common OR NOT: ((A !(N \| N1)) \| (B !(N \| N2))) becomes (( (A !N1)
   \| (B !N2) ) !N)

Note that optimizing the queries consumes CPU time, so for simple
queries -or for hand-optimized queries- you'll do better with the
default boolean_simplify=0 value. Simplifications are often better for
complex queries, or algorithmically generated queries.

Queries like “-dog”, which implicitly include all documents from the
collection, can not be evaluated. This is both for technical and
performance reasons. Technically, Manticore does not always keep a list of
all IDs. Performance-wise, when the collection is huge (ie. 10-100M
documents), evaluating such queries could take very long.
