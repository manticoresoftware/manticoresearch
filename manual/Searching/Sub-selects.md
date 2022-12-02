# Sub-selects

Manticore supports SELECT subqueries via SQL in the following format:

```sql
SELECT * FROM (SELECT ... ORDER BY cond1 LIMIT X) ORDER BY cond2 LIMIT Y
```

The outer select allows only `ORDER BY` and `LIMIT` clauses. Sub-selects queries currently have 2 usage cases:

1. We have a query with 2 ranking UDFs, one very fast and the other slow and we perform a full-text search with a big match result set. Without subselect the query would look like

    ```sql
    SELECT id,slow_rank() as slow,fast_rank() as fast FROM index 
        WHERE MATCH(‘some common query terms’) ORDER BY fast DESC, slow DESC LIMIT 20 
        OPTION max_matches=1000;
    ```

    With sub-selects the query can be rewritten as:

    ```sql
    SELECT * FROM
        (SELECT id,slow_rank() as slow,fast_rank() as fast FROM index WHERE 
            MATCH(‘some common query terms’)
            ORDER BY fast DESC LIMIT 100 OPTION max_matches=1000)
    ORDER BY slow DESC LIMIT 20;
    ```

    In the initial query the `slow_rank()` UDF is computed for the entire match result set. With SELECT sub-queries only `fast_rank()` is computed for the entire match result set, while `slow_rank()` is only computed for a limited set.

2. The second case comes handy for large result set coming from a distributed table.

    For this query:

    ```sql
    SELECT * FROM my_dist_index WHERE some_conditions LIMIT 50000;
    ```

    If we have 20 nodes, each node can send back to master a number of 50K records, resulting in **20 x 50K = 1M records**, however as the master sends back only 50K (out of 1M), it might be good enough for us for the nodes to send only the top 10K records. With sub-select we can rewrite the query as:

    ```sql
    SELECT * FROM 
         (SELECT * FROM my_dist_index WHERE some_conditions LIMIT 10000) 
     ORDER by some_attr LIMIT 50000;
    ```

    In this case, the nodes receive only the inner query and execute. This means the master will receive only *20x10K=200K records*. The master will take all the records received, reorder them by the `OUTER` clause and return the best 50K records. The sub-select help reducing the traffic between the master and the nodes and also reduce the master's computation time (as it process only 200K instead of 1M).