.. _mva_multi-valued_attributes:

MVA (multi-valued attributes)
-----------------------------

MVAs, or multi-valued attributes, are an important special type of
per-document attributes in Manticore. MVAs let you attach sets of numeric
values to every document. That is useful to implement article tags,
product categories, etc. Filtering and group-by (but not sorting) on MVA
attributes is supported.

MVA values can either be unsigned 32-bit integers (UNSIGNED INTEGER) or
signed 64-bit integers (BIGINT).

The set size is not limited, you can have an arbitrary number of values
attached to each document as long as RAM permits (``.spm`` file that
contains the MVA values will be precached in RAM by ``searchd``). The
source data can be taken either from a separate query, or from a
document field; see source type in
:ref:`sql_attr_multi <sql_attr_multi>`.
In the first case the query will have to return pairs of document ID and
MVA values, in the second one the field will be parsed for integer
values. There are absolutely no requirements as to incoming data order;
the values will be automatically grouped by document ID (and internally
sorted within the same ID) during indexing anyway.

When filtering, a document will match the filter on MVA attribute if
*any* of the values satisfy the filtering condition. (Therefore,
documents that pass through exclude filters will not contain any of the
forbidden values.) When grouping by MVA attribute, a document will
contribute to as many groups as there are different MVA values
associated with that document. For instance, if the collection contains
exactly 1 document having a ‘tag’ MVA with values 5, 7, and 11, grouping
on ‘tag’ will produce 3 groups with 'COUNT(\*)‘equal to 1 and
'GROUPBY()’ key values of 5, 7, and 11 respectively. Also note that
grouping by MVA might lead to duplicate documents in the result set:
because each document can participate in many groups, it can be chosen
as the best one in in more than one group, leading to duplicate IDs. PHP
API historically uses ordered hash on the document ID for the resulting
rows; so you'll also need to use
:ref:`SetArrayResult() <set_array_result>` in
order to employ group-by on MVA with PHP API.
