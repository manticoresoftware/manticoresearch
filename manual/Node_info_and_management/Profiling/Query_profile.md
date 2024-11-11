# Query profile

<!-- example SHOW PROFILE -->

The SQL `SHOW PROFILE` statement and the `"profile": true` JSON interface option both provide a detailed execution profile of the executed query. In the case of SQL, profiling must be enabled in the current session **before** running the statement to be instrumented. This can be accomplished with the `SET profiling=1` statement. By default, profiling is disabled to prevent potential performance implications, resulting in an empty profile if not enabled.

Each profiling result includes the following fields:
* `Status` column briefly describes the specific state where the time was spent. See below.
* `Duration` column shows the wall clock time, in seconds.
* `Switches` column displays the number of times the query engine changed to the given state. These are merely logical engine state switches and **not** any OS level context switches or function calls (although some sections might actually map to function calls), and they do **not** have any direct effect on performance. In a sense, the number of switches is just the number of times the respective instrumentation point was hit.
* `Percent` column shows the percentage of time spent in this state.

States in the profile are returned in a prerecorded order that roughly maps (but is **not** identical) to the actual query order.

The list of states may (and will) change over time as we refine the states. Here's a brief description of the currently profiled states.

* `unknown`: generic catch-all state. Accounts for not-yet-instrumented code or small miscellaneous tasks that don't really belong in any other state but are too small to warrant their own state.
* `net_read`: reading the query from the network (i.e., the application).
* `io`: generic file IO time.
* `dist_connect`: connecting to remote agents in the distributed table case.
* `sql_parse`: parsing the SQL syntax.
* `dict_setup`: dictionary and tokenizer setup.
* `parse`: parsing the full-text query syntax.
* `transforms`: full-text query transformations (wildcard and other expansions, simplification, etc.).
* `init`: initializing the query evaluation.
* `open`: opening the table files.
* `read_docs`: IO time spent reading document lists.
* `read_hits`: IO time spent reading keyword positions.
* `get_docs`: computing the matching documents.
* `get_hits`: computing the matching positions.
* `filter`: filtering the full-text matches.
* `rank`: computing the relevance rank.
* `sort`: sorting the matches.
* `finalize`: finalizing the per-table search result set (last stage expressions, etc.).
* `dist_wait`: waiting for remote results from agents in the distributed table case.
* `aggregate`: aggregating multiple result sets.
* `net_write`: writing the result set to the network.

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

<!-- request JSON -->

```json
POST /search
{
  "table": "test",
  "profile": true,
  "query":
  {
    "match_phrase": { "_all" : "had grown quite" }
  }
}
```

<!-- response JSON -->

```json
 "profile": {
    "query": [
      {
        "status": "unknown",
        "duration": 0.000141,
        "switches": 8,
        "percent": 2.17
      },
      {
        "status": "local_df",
        "duration": 0.000870,
        "switches": 1,
        "percent": 13.40
      },
      {
        "status": "local_search",
        "duration": 0.001038,
        "switches": 2,
        "percent": 15.99
      },
      {
        "status": "setup_iter",
        "duration": 0.000154,
        "switches": 14,
        "percent": 2.37
      },
      {
        "status": "dict_setup",
        "duration": 0.000026,
        "switches": 3,
        "percent": 0.40
      },
      {
        "status": "parse",
        "duration": 0.000205,
        "switches": 3,
        "percent": 3.15
      },
      {
        "status": "transforms",
        "duration": 0.000974,
        "switches": 4,
        "percent": 15.01
      },
      {
        "status": "init",
        "duration": 0.002931,
        "switches": 20,
        "percent": 45.16
      },
      {
        "status": "get_docs",
        "duration": 0.000007,
        "switches": 7,
        "percent": 0.10
      },
      {
        "status": "rank",
        "duration": 0.000002,
        "switches": 14,
        "percent": 0.03
      },
      {
        "status": "finalize",
        "duration": 0.000013,
        "switches": 7,
        "percent": 0.20
      },
      {
        "status": "aggregate",
        "duration": 0.000128,
        "switches": 1,
        "percent": 1.97
      },
      {
        "status": "total",
        "duration": 0.006489,
        "switches": 84,
        "percent": 100.00
      }
    ]
  }
```
<!-- end -->

<!-- proofread -->
