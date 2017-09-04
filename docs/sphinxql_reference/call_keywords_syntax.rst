.. _call_keywords_syntax:

CALL KEYWORDS syntax
--------------------

.. code-block:: mysql


    CALL KEYWORDS(text, index [, options])

CALL KEYWORDS statement splits text into particular keywords. It returns
tokenized and normalized forms of the keywords, and, optionally, keyword
statistics. It also returns the position of each keyword in the query
and all forms of tokenized keywords in the case that lemmatizers were
used.

``text`` is the text to break down to keywords. ``index`` is the name of
the index from which to take the text processing settings. ``options``,
is an optional boolean parameter that specifies whether to return
document and hit occurrence statistics. ``options`` can also accept
parameters for configuring folding depending on tokenization settings:

-  ``stats`` - show statistics of keywords, default is 0

-  ``fold_wildcards`` - fold wildcards, default is 1

-  ``fold_lemmas`` - fold morphological lemmas, default is 0

-  ``fold_blended`` - fold blended words, default is 0

-  ``expansion_limit`` - override expansion_limit defined in
   configuration, default is 0 (use value from configuration)

.. code-block:: mysql


    call keywords(
        'que*',
        'myindex',
        1 as fold_wildcards,
        1 as fold_lemmas,
        1 as fold_blended,
        1 as expansion_limit,
        1 as stats);

Default values to match previous CALL KEYWORDS output are:

.. code-block:: mysql


    call keywords(
        'que*',
        'myindex',
        1 as fold_wildcards,
        0 as fold_lemmas,
        0 as fold_blended,
        0 as expansion_limit,
        0 as stats);

