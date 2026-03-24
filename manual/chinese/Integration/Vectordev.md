# 与 Vector.dev 的集成

> 注意：与 Fluent Bit 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法正常工作，请确保已安装 Buddy。

[Vector by Datadog](https://vector.dev/) 是一个开源的可观测性数据管道，可以收集、转换和路由日志或指标。虽然 Vector 可以自行聚合数据，但将其与 Manticore 配合使用可提供专用的存储和搜索层。

以下示例展示了如何通过 Vector.dev 转发 Debian 的 `dpkg.log`，并将其索引到 Manticore 中。

## 示例日志结构

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Vector.dev 配置

创建一个类似于以下内容的 `vector.toml`：

```toml
[sources.test_file]
type = "file"
include = [ "/var/log/dpkg.log" ]

[transforms.modify_test_file]
type = "remap"
inputs = [ "test_file" ]
source = """
.vec_timestamp = del(.timestamp)"""

[sinks.manticore]
type = "elasticsearch"
inputs = [ "modify_test_file" ]
endpoints = ["http://127.0.0.1:9308"]
bulk.index = "dpkg_log"
```

- `endpoints` 指向 Manticore 的 HTTP 接口（默认端口为 `9308`）。如果您的实例在其他位置监听，请进行调整。
- `remap` 转换将 Vector 的默认 `timestamp` 字段移动到 `vec_timestamp`，因为 `timestamp` 在 Manticore 中是保留字段。
- `bulk.index` 定义了当 Vector 开始发送数据时将自动创建的表。

使用此配置启动 Vector.dev，它将跟踪日志文件，转换每个事件，并直接将其转发到 Manticore。

## 运行 Vector.dev

将配置保存为 `vector.toml`，然后启动代理：

```
vector --config vector.toml
```

如果在 Docker 中运行 Vector.dev，请挂载配置文件和日志目录，例如：

```
docker run --rm -v /var/log/dpkg.log:/var/log/dpkg.log:ro \
  -v $(pwd)/vector.toml:/etc/vector/vector.toml:ro \
  timberio/vector:latest --config /etc/vector/vector.toml
```

## Vector.dev 结果

当管道运行时，Manticore 会自动创建 `dpkg_log` 表。其模式和示例文档如下所示：

```
mysql> DESCRIBE dpkg_log;
+-----------------+---------+--------------------+
| Field           | Type    | Properties         |
+-----------------+---------+--------------------+
| id              | bigint  |                    |
| file            | text    | indexed stored     |
| host            | text    | indexed stored     |
| message         | text    | indexed stored     |
| source_type     | text    | indexed stored     |
| vec_timestamp   | text    | indexed stored     |
+-----------------+---------+--------------------+

mysql> SELECT * FROM dpkg_log LIMIT 3\G
*************************** 1. row ***************************
id: 7856533729353672195
file: /var/log/dpkg.log
host: logstash-787f68f6f-nhdd2
message: 2023-06-05 14:03:04 startup archives install
source_type: file
vec_timestamp: 2023-08-04T15:32:50.203091741Z
*************************** 2. row ***************************
id: 7856533729353672196
file: /var/log/dpkg.log
host: logstash-787f68f6f-nhdd2
message: 2023-06-05 14:03:04 install base-passwd:amd64 <none> 3.5.47
source_type: file
vec_timestamp: 2023-08-04T15:32:50.203808861Z
*************************** 3. row ***************************
id: 7856533729353672197
file: /var/log/dpkg.log
host: logstash-787f68f6f-nhdd2
message: 2023-06-05 14:03:04 status half-installed base-passwd:amd64 3.5.47
source_type: file
vec_timestamp: 2023-08-04T15:32:50.203814031Z
```

## 结论

将 Vector.dev 与 Manticore 结合使用，您可以从几乎所有来源收集日志，在传输过程中丰富或清理日志，并将结果存储在可搜索的数据库中。此工作流程在保持可观测性管道简单的同时，仍可在需要时启用高级转换。
