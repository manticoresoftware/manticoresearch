# 查询配置文件

<!-- example SHOW PROFILE -->

SQL 语句 `SHOW PROFILE` 和 JSON 接口选项 `"profile": true` 都能提供已执行查询的详细执行配置文件。对于 SQL，必须在当前会话**运行被度量的语句之前**启用分析。这可以通过执行 `SET profiling=1` 来完成。默认情况下，分析是禁用的，以避免潜在的性能影响，若未启用则会得到空的配置文件。

每个分析结果包含以下字段：
* `Status` 列简要描述了花费时间的具体状态。见下文。
* `Duration` 列显示墙钟时间，单位为秒。
* `Switches` 列显示查询引擎切换到指定状态的次数。这仅仅是逻辑引擎状态切换，**非**任何操作系统层面的上下文切换或函数调用（尽管某些部分可能实际上对应函数调用），且它们**不会**直接影响性能。从某种意义上讲，切换次数只是对应度量点被触发的次数。
* `Percent` 列显示在该状态下花费的时间百分比。

配置文件中的状态按照预先录制的顺序返回，该顺序大致对应（但**不**完全等同于）实际查询执行顺序。

随着我们对状态的不断完善，状态列表可能（且会）发生变化。以下是当前度量状态的简要说明。

* `unknown`：通用的捕获状态。涵盖尚未被度量的代码或不属于其他状态的零星小任务，但这些任务又小得不足以单独设立状态。
* `net_read`：从网络（如应用程序）读取查询。
* `io`：通用文件 IO 时间。
* `dist_connect`：连接到分布式表中的远程代理。
* `sql_parse`：解析 SQL 语法。
* `dict_setup`：词典和分词器设置。
* `parse`：解析全文本查询语法。
* `transforms`：全文本查询转换（通配符及其他扩展、简化等）。
* `init`：初始化查询评估。
* `open`：打开表文件。
* `read_docs`：读取文档列表的 IO 时间。
* `read_hits`：读取关键词位置的 IO 时间。
* `get_docs`：计算匹配文档。
* `get_hits`：计算匹配位置。
* `filter`：过滤全文本匹配项。
* `rank`：计算相关性排名。
* `sort`：对匹配项进行排序。
* `finalize`：最终确定每个表的搜索结果集（最终阶段表达式等）。
* `dist_wait`：等待分布式表中远程代理返回结果。
* `aggregate`：汇总多个结果集。
* `net_write`：将结果集写回网络。

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

