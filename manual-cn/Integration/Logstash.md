# 与 Logstash 集成

> 注意：与 Logstash 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保 Buddy 已安装。

[Logstash](https://www.elastic.co/logstash) 是一个日志管理工具，可从各种来源收集数据，实时转换并将其发送到所需的目标。它通常用作 Elasticsearch 的数据管道，Elasticsearch 是一个开源的分析和搜索引擎。

现在，Manticore 支持使用 Logstash 作为处理管道。这允许收集和转换后的数据像发送到 Elasticsearch 一样被发送到 Manticore。目前，支持的版本为 7.6-7.15。

我们来看一个用于索引 `dpkg.log` 的简单 Logstash 配置文件示例，这是 Debian 包管理器的标准日志文件。日志本身的结构很简单，如下所示：

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Logstash 配置

以下是一个 Logstash 配置示例：

```
input {
  file {
    path => ["/var/log/dpkg.log"]
    start_position => "beginning"
    sincedb_path => "/dev/null"
    mode => "read"
    exit_after_read => "true"
   file_completed_action => "log"
   file_completed_log_path => "/dev/null"
  }
}

output {
  elasticsearch {
   index => " dpkg_log"
   hosts => ["http://localhost:9308"]
   ilm_enabled => false
   manage_template => false
  }
}
```

请注意，在继续之前，需要解决一个关键问题：Manticore 不支持 Elasticsearch 的日志模板管理和索引生命周期管理功能。由于这些功能在 Logstash 中默认启用，因此需要在配置中显式禁用。此外，输出配置部分中的 `hosts` 选项必须对应于 Manticore 的 HTTP 监听端口（默认是 localhost:9308）。

## Logstash 结果

在按上述配置调整后，您可以运行 Logstash，dpkg 日志中的数据将被传递到 Manticore 并正确索引。

以下是创建表的结果模式以及插入文档的示例：

```
mysql> DESCRIBE dpkg_log;
+------------------+--------+---------------------+
| Field            | Type   | Properties          |
+------------------+--------+---------------------+
| id               | bigint |                     |
| message          | text   | indexed stored      |
| @version         | text   | indexed stored      |
| @timestamp       | text   | indexed stored      |
| path             | text   | indexed stored      |
| host             | text   | indexed stored      |
+------------------+--------+---------------------+
```

```
mysql> SELECT * FROM dpkg_log LIMIT 1\G

*************************** 1. row ***************************
id: 7280000849080746110
host: logstash-db848f65f-lnlf9
message: 2023-04-12 02:03:21 status unpacked libc-bin:amd64 2.31-0ubuntu9
path: /var/log/dpkg.log
@timestamp: 2023-06-16T09:23:57.405Z
@version: 1
```
