.. _sphinxql_reference:

SphinxQL reference
=============================

SphinxQL is our SQL dialect that exposes all of the search daemon
functionality using a standard SQL syntax with a few Manticore-specific
extensions. Everything available via the SphinxAPI is also available via
SphinxQL but not vice versa; for instance, writes into RT indexes are
only available via SphinxQL. This chapter documents supported SphinxQL
statements syntax.

.. toctree::
   :glob:

   sphinxql_reference/*