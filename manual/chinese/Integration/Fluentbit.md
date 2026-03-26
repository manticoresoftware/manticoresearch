# 与 Fluent Bit 的集成

> 注意：与 Fluent Bit 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

[Fluent Bit](https://fluentbit.io/) 是一个开源的跨平台日志处理器，可以从许多来源聚合数据并将其发送到多个目的地。你可以将 Fluent Bit 的输出直接输入 Manticore，使收集的数据能够实时搜索。

以下操作指南演示了如何使用 Fluent Bit 和 Manticore 对 Debian 的 `dpkg.log` 进行索引。

## 示例日志结构

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Fluent Bit 配置

创建一个配置文件，例如 `fluent-bit.conf`：

```
[SERVICE]
    flush        1
    daemon       On
    log_level    info

[INPUT]
    name tail
    path /var/log/dpkg.log
    inotify_watcher false
    read_from_head true

[OUTPUT]
    name es
    match *
    host 127.0.0.1
    port 9308
    index dpkg_log
```

- `[SERVICE]` 块以守护进程模式启动 Fluent Bit，这对于基于 Docker 的设置非常方便。如果你想在前台运行，请禁用守护进程标志。
- `inotify_watcher` 已关闭，以避免容器内部的文件通知限制。
- 输出插件 (`name es`) 可以通过端口 `9308` 与 Manticore 的 HTTP 端点通信。
- `index` 定义了当第一批数据到达时 Manticore 自动创建的表名。

使用此配置运行 Fluent Bit，它将跟踪 `dpkg.log`，然后将每一行转发到 Manticore。

## 运行 Fluent Bit

将配置保存为 `fluent-bit.conf`，然后启动 Fluent Bit：

```
fluent-bit -c fluent-bit.conf
```

要在 Docker 中运行，请挂载日志文件（只读）和配置：

```
docker run --rm -v /var/log/dpkg.log:/var/log/dpkg.log:ro \
  -v $(pwd)/fluent-bit.conf:/fluent-bit/etc/fluent-bit.conf:ro \
  fluent/fluent-bit:latest -c /fluent-bit/etc/fluent-bit.conf
```

## Fluent Bit 结果

数据摄入开始后，Manticore 会自动创建 `dpkg_log` 表。以下是表定义和示例数据：

```
mysql> DESCRIBE dpkg_log;
+-------------+--------+----------------+
| Field       | Type   | Properties     |
+-------------+--------+----------------+
| id          | bigint |                |
| @timestamp  | text   | indexed stored |
| log         | text   | indexed stored |
+-------------+--------+----------------+

mysql> SELECT * FROM dpkg_log LIMIT 3\G
*************************** 1. row ***************************
id: 7856533729353662465
@timestamp: 2023-08-04T15:09:21.191Z
log: 2023-06-05 14:03:04 startup archives install
*************************** 2. row ***************************
id: 7856533729353662466
@timestamp: 2023-08-04T15:09:21.191Z
log: 2023-06-05 14:03:04 install base-passwd:amd64 <none> 3.5.47
*************************** 3. row ***************************
id: 7856533729353662467
@timestamp: 2023-08-04T15:09:21.191Z
log: 2023-06-05 14:03:04 status half-installed base-passwd:amd64 3.5.47
```

## 结论

通过这个轻量级的管道，Fluent Bit 处理日志收集和传输，而 Manticore 对事件进行索引以实现快速搜索和分析。这种方法同样适用于其他日志源——只需添加更多输入并重用指向你 Manticore 集群的相同 Elasticsearch 兼容输出。
