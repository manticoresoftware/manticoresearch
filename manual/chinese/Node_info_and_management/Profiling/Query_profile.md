# 查询分析

<!-- example SHOW PROFILE -->

SQL 语句 `SHOW PROFILE` 和 JSON 接口选项 `"profile": true` 都提供了已执行查询的详细执行分析。在 SQL 的情况下，必须在当前会话中**在**运行要分析的语句之前启用分析。这可以通过执行 `SET profiling=1` 语句来完成。默认情况下，分析是禁用的，以防止潜在的性能影响，如果未启用，则分析结果为空。

每个分析结果包含以下字段：
* `Status` 列简要描述了时间花费的具体状态。见下文。
* `Duration` 列显示了实际时间，单位为秒。
* `Switches` 列显示查询引擎切换到给定状态的次数。这些仅是逻辑引擎状态切换，**不是**任何操作系统级别的上下文切换或函数调用（尽管某些部分可能实际上映射到函数调用），并且它们**不会**对性能产生任何直接影响。从某种意义上说，切换次数只是相应分析点被触发的次数。
* `Percent` 列显示在该状态下花费时间的百分比。

分析中的状态按预录顺序返回，该顺序大致映射（但**不**完全相同）到实际查询顺序。

状态列表可能（且会）随着我们对状态的细化而变化。以下是当前分析状态的简要说明。

* `unknown`：通用的捕获状态。用于尚未分析的代码或不属于任何其他状态但又太小而不值得单独列出的杂项任务。
* `net_read`：从网络（即应用程序）读取查询。
* `io`：通用文件 IO 时间。
* `dist_connect`：在分布式表情况下连接远程代理。
* `sql_parse`：解析 SQL 语法。
* `dict_setup`：字典和分词器设置。
* `parse`：解析全文查询语法。
* `transforms`：全文查询转换（通配符和其他扩展，简化等）。
* `init`：初始化查询评估。
* `open`：打开表文件。
* `read_docs`：读取文档列表的 IO 时间。
* `read_hits`：读取关键字位置的 IO 时间。
* `get_docs`：计算匹配的文档。
* `get_hits`：计算匹配的位置。
* `filter`：过滤全文匹配。
* `rank`：计算相关性排名。
* `sort`：对匹配结果排序。
* `finalize`：完成每个表的搜索结果集（最后阶段表达式等）。
* `dist_wait`：在分布式表情况下等待远程代理的结果。
* `aggregate`：聚合多个结果集。
* `net_write`：将结果集写入网络。

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

