# SHOW PROFILE

<!-- example SHOW PROFILE -->

`SHOW PROFILE` is an SQL statement that shows a detailed execution profile of the previous SQL statement executed in the current SQL session. Also, profiling must be enabled in the current session **before** running the statement to be instrumented. That can be done with a `SET profiling=1` statement. By default, profiling is disabled to avoid potential performance implications, and therefore the profile will be empty.

* `Status` column briefly describes where exactly (in which state) was the time spent.
* `Duration` column shows the wall clock time, in seconds.
* `Switches` column displays the number of times query engine changed to the given state.   Those are just logical engine state switches and **not** any OS level context switches nor function calls (even though some of the sections can actually map to function calls) and they do **not** have any direct effect on the performance. In a sense, number of switches is just a number of times when the respective instrumentation point was hit.
* `Percent` column shows the percentage of time spent in this state.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SET profiling=1;

SELECT id FROM forum WHERE MATCH('the best') LIMIT 1;

SHOW PROFILE;
```

<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)

+--------+
| id     |
+--------+
| 241629 |
+--------+
1 row in set (0.35 sec)

+--------------+----------+----------+---------+
| Status       | Duration | Switches | Percent |
+--------------+----------+----------+---------+
| unknown      | 0.000557 | 5        | 0.16    |
| net_read     | 0.000016 | 1        | 0.00    |
| local_search | 0.000076 | 1        | 0.02    |
| sql_parse    | 0.000121 | 1        | 0.03    |
| dict_setup   | 0.000003 | 1        | 0.00    |
| parse        | 0.000072 | 1        | 0.02    |
| transforms   | 0.000331 | 2        | 0.10    |
| init         | 0.001945 | 3        | 0.56    |
| read_docs    | 0.001257 | 76       | 0.36    |
| read_hits    | 0.002598 | 186      | 0.75    |
| get_docs     | 0.089328 | 2691     | 25.80   |
| get_hits     | 0.189626 | 2799     | 54.78   |
| filter       | 0.009369 | 2613     | 2.71    |
| rank         | 0.029669 | 7760     | 8.57    |
| sort         | 0.019070 | 2531     | 5.51    |
| finalize     | 0.000001 | 1        | 0.00    |
| clone_attrs  | 0.002009 | 1        | 0.58    |
| aggregate    | 0.000054 | 2        | 0.02    |
| net_write    | 0.000076 | 2        | 0.02    |
| eval_post    | 0.000001 | 1        | 0.00    |
| total        | 0.346179 | 18678    | 0       |
+--------------+----------+----------+---------+
21 rows in set (0.00 sec)
```

<!-- end -->

States in the profile are returned in a prerecorded order that roughly maps (but is **not** identical) to the actual query order.

A list of states may (and will) vary over time, as we refine the states. Here's a brief description of the currently profiled states.

* `unknown`: generic catch-all state. Accounts for both not-yet-instrumented code, or just small miscellaneous tasks that do not really belong in any other state, but are too small to deserve their own state.
* `net_read`: reading the query from the network (that is, the application).
* `io`: generic file IO time.
* `dist_connect`: connecting to remote agents in the distributed index case.
* `sql_parse`: parsing the SQL syntax.
* `dict_setup`: dictionary and tokenizer setup.
* `parse`: parsing the full-text query syntax.
* `transforms`: full-text query transformations (wildcard and other expansions, simplification, etc).
* `init`: initializing the query evaluation.
* `open`: opening the index files.
* `read_docs`: IO time spent reading document lists.
* `read_hits`: IO time spent reading keyword positions.
* `get_docs`: computing the matching documents.
* `get_hits`: computing the matching positions.
* `filter`: filtering the full-text matches.
* `rank`: computing the relevance rank.
* `sort`: sorting the matches.
* `finalize`: finalizing the per-index search result set (last stage expressions, etc).
* `dist_wait`: waiting for the remote results from the agents in the distributed index case.
* `aggregate`: aggregating multiple result sets.
* `net_write`: writing the result set to the network.

## Query profiling in HTTP JSON

You can view the final transformed query tree with all normalized keywords by adding a `"profile":true` property:

```json
{
  "index":"test",
  "profile":true,
  "query":
  {
    "match_phrase": { "_all" : "had grown quite" }
  }
}
```

This feature is somewhat similar to [SHOW PLAN](../../Profiling_and_monitoring/Profiling/Query_plan.md) statement in SQL. The result appears as a profile property in the result set. For example:

```json
"profile":
{
  "query":
  {
    "type": "PHRASE",
    "description": "PHRASE( AND(KEYWORD(had, querypos=1)),  AND(KEYWORD(grown, querypos=2)),  AND(KEYWORD(quite, querypos=3)))",
    "children":
    [
      {
        "type": "AND",
        "description": "AND(KEYWORD(had, querypos=1))",
        "max_field_pos": 0,
        "children":
        [
          {
            "type": "KEYWORD",
            "word": "had",
            "querypos": 1
           }
        ]
      },
      {
        "type": "AND",
        "description": "AND(KEYWORD(grown, querypos=2))",
        "max_field_pos": 0,
        "children":
        [
          {
            "type": "KEYWORD",
            "word": "grown",
            "querypos": 2
          }
        ]
      },
      {
        "type": "AND",
        "description": "AND(KEYWORD(quite, querypos=3))",
        "max_field_pos": 0,
        "children":
        [
          {
            "type": "KEYWORD",
            "word": "quite",
            "querypos": 3
          }
        ]
      }
    ]
  }
}
```

`query` property contains the transformed full-text query tree. Each node contains:

* `type`: node type. Can be AND, OR, PHRASE, KEYWORD etc
* `description`: query subtree for this node shown as a string (in `SHOW PLAN` format)
* `children`: child nodes, if any
* `max_field_pos`: maximum position within a field
* `word`: transformed keyword. Keyword nodes only
* `querypos`: position of this keyword in a query. Keyword nodes only
* `excluded`: keyword excluded from query. Keyword nodes only
* `expanded`: keyword added by prefix expansion. Keyword nodes only
* `field_start`: keyword must occur at the very start of the field. Keyword nodes only
* `field_end`: keyword must occur at the very end of the field. Keyword nodes only
* `boost`: keyword IDF will be multiplied by this. Keyword nodes only
