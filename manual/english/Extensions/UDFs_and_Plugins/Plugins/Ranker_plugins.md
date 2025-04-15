# Ranker plugins 

Ranker plugins let you implement a custom ranker that receives all the occurrences of the keywords matched in the document, and computes a  `WEIGHT()` value. They can be called as follows:

```sql
SELECT id, attr1 FROM test WHERE match('hello') OPTION ranker=myranker('option1=1');
```
The call workflow proceeds as follows:

1. `XXX_init()` is invoked once per query per table, at the very beginning. Several query-wide options are passed to it via a `SPH_RANKER_INIT` structure, including the user options strings (for instance, "option1=1" in the example above).
2. `XXX_update()` is called multiple times for each matched document, with every matched keyword occurrence provided as its parameter, a `SPH_RANKER_HIT` structure. The occurrences within each document are guaranteed to be passed in ascending order of `hit->hit_pos` values.
3. `XXX_finalize()` is called once for each matched document when there are no more keyword occurrences. It must return the `WEIGHT()` value. This function is the only mandatory one.
4. `XXX_deinit()` is invoked once per query, at the very end.


<!-- proofread -->