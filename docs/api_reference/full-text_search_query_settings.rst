Full-text search query settings
-------------------------------

.. _set_field_weights:

SetFieldWeights
~~~~~~~~~~~~~~~

**Prototype:** function SetFieldWeights ( $weights )

Binds per-field weights by name. Parameter must be a hash (associative
array) mapping string field names to integer weights.

Match ranking can be affected by per-field weights. For instance, see
:ref:`search_results_ranking` for an explanation
how phrase proximity ranking is affected. This call lets you specify
what non-default weights to assign to different full-text fields.

The weights must be positive 32-bit integers. The final weight will be a
32-bit integer too. Default weight value is 1. Unknown field names will
be silently ignored.

There is no enforced limit on the maximum weight value at the moment.
However, beware that if you set it too high you can start hitting 32-bit
wraparound issues. For instance, if you set a weight of 10,000,000 and
search in extended mode, then maximum possible weight will be equal to
10 million (your weight) by 1 thousand (internal BM25 scaling factor,
see `search_results_ranking`) by 1 or more
(phrase proximity rank). The result is at least 10 billion that does not
fit in 32 bits and will be wrapped around, producing unexpected results.

.. _set_index_weights:

SetIndexWeights
~~~~~~~~~~~~~~~

**Prototype:** function SetIndexWeights ( $weights )

Sets per-index weights, and enables weighted summing of match weights
across different indexes. Parameter must be a hash (associative array)
mapping string index names to integer weights. Default is empty array
that means to disable weighting summing.

When a match with the same document ID is found in several different
local indexes, by default Manticore simply chooses the match from the index
specified last in the query. This is to support searching through
partially overlapping index partitions.

However in some cases the indexes are not just partitions, and you might
want to sum the weights across the indexes instead of picking one.
``SetIndexWeights()`` lets you do that. With summing enabled, final
match weight in result set will be computed as a sum of match weight
coming from the given index multiplied by respective per-index weight
specified in this call. Ie. if the document 123 is found in index A with
the weight of 2, and also in index B with the weight of 3, and you
called
``SetIndexWeights ( array ( "A"=>100, "B"=>10 ) )``,
the final weight return to the client will be 2\ *100+3*\ 10 = 230.


.. _set_match_mode:

SetMatchMode
~~~~~~~~~~~~

**DEPRECATED**

**Prototype:** function SetMatchMode ( $mode )

Sets full-text query matching mode, as described in :ref:`matching_modes`. Parameter must be a
constant specifying one of the known modes.

**WARNING:** (PHP specific) you **must not** take the matching
mode constant name in quotes, that syntax specifies a string and is
incorrect:

.. code-block:: php


    $cl->SetMatchMode ( "SPH_MATCH_ANY" ); // INCORRECT! will not work as expected
    $cl->SetMatchMode ( SPH_MATCH_ANY ); // correct, works OK

.. _set_ranking_mode:

SetRankingMode
~~~~~~~~~~~~~~

**Prototype:** function SetRankingMode ( $ranker, $rankexpr=“” )

Sets ranking mode (aka ranker). Only available in SPH_MATCH_EXTENDED
matching mode. Parameter must be a constant specifying one of the known
rankers.

By default, in the EXTENDED matching mode Manticore computes two factors
which contribute to the final match weight. The major part is a phrase
proximity value between the document text and the query. The minor part
is so-called BM25 statistical function, which varies from 0 to 1
depending on the keyword frequency within document (more occurrences
yield higher weight) and within the whole index (more rare keywords
yield higher weight).

However, in some cases you'd want to compute weight differently - or
maybe avoid computing it at all for performance reasons because you're
sorting the result set by something else anyway. This can be
accomplished by setting the appropriate ranking mode. The list of the
modes is available in :ref:`search_results_ranking`.

``$rankexpr`` argument lets you specify a ranking formula to use with
the `expression based
ranker <expression_based_ranker_sphrank_expr>`,
that is, when ``$ranker`` is set to SPH_RANK_EXPR. In all other cases,
``$rankexpr`` is ignored.


.. _set_sort_mode:

SetSortMode
~~~~~~~~~~~

**Prototype:** function SetSortMode ( $mode, $sortby=“” )

Set matches sorting mode, as described in :ref:`sorting_modes`. Parameter must be a constant
specifying one of the known modes.

**WARNING:** (PHP specific) you **must not** take the matching
mode constant name in quotes, that syntax specifies a string and is
incorrect:

.. code-block:: php


    $cl->SetSortMode ( "SPH_SORT_ATTR_DESC" ); // INCORRECT! will not work as expected
    $cl->SetSortMode ( SPH_SORT_ATTR_ASC ); // correct, works OK

.. _set_weights:

SetWeights
~~~~~~~~~~

**Prototype:** function SetWeights ( $weights )

Binds per-field weights in the order of appearance in the index.
**DEPRECATED**, use
:ref:`SetFieldWeights() <set_field_weights>`
instead.
