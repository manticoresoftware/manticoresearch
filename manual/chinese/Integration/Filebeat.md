# 与 Filebeat 的集成

> 注意：与 Filebeat 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它无法工作，请确保已安装 Buddy。

[Filebeat](https://www.elastic.co/beats/filebeat) 是一个轻量级的发货工具，用于转发和集中日志数据。一旦作为代理安装，它会监视您指定的日志文件或位置，收集日志事件，并将其转发以供索引，通常是到 Elasticsearch 或 Logstash。

现在，Manticore 也支持将 Filebeat 用作处理管道。这允许收集和转换的数据像发送到 Elasticsearch 一样发送到 Manticore。目前，所有版本 >= 7.10 都受支持。

## Filebeat 配置

以下是一个用于我们示例 dpkg 日志的 Filebeat 配置：

```
filebeat.inputs:
- type: filestream
  id: example
  paths:
	- /var/log/dpkg.log

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index:  "dpkg_log"
  allow_older_versions: true

setup.ilm:
  enabled: false

setup.template:
  name: "dpkg_log"
  pattern: "dpkg_log"
```

### Filebeat 版本 >= 8.11 的配置

请注意，版本高于 8.10 的 Filebeat 默认启用了输出压缩功能。这就是为什么必须在配置文件中添加 `compression_level: 0` 选项以提供与 Manticore 的兼容性：

```
filebeat.inputs:
- type: filestream
  id: example
  paths:
	- /var/log/dpkg.log

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index:  "dpkg_log"
  allow_older_versions: true
  compression_level: 0

setup.ilm:
  enabled: false

setup.template:
  name: "dpkg_log"
  pattern: "dpkg_log"
```

## Filebeat 结果

一旦您使用此配置运行 Filebeat，日志数据将发送到 Manticore 并正确索引。以下是 Manticore 创建的表的结果架构及插入文档的示例：

```
mysql> DESCRIBE dpkg_log;
+------------------+--------+--------------------+
| Field            | Type   | Properties         |
+------------------+--------+--------------------+
| id               | bigint |                    |
| @timestamp       | text   | indexed stored     |
| message          | text   | indexed stored     |
| log              | json   |                    |
| input            | json   |                    |
| ecs              | json   |                    |
| host             | json   |                    |
| agent            | json   |                    |
+------------------+--------+--------------------+
```

```
mysql> SELECT * FROM dpkg_log LIMIT 1\G
*************************** 1. row ***************************
id: 7280000849080753116
@timestamp: 2023-06-16T09:27:38.792Z
message: 2023-04-12 02:06:08 status half-installed libhogweed5:amd64 3.5.1+really3.5.1-2
input: {"type":"filestream"}
ecs: {"version":"1.6.0"}
host: {"name":"logstash-db848f65f-lnlf9"}
agent: {"ephemeral_id":"587c2ebc-e7e2-4e27-b772-19c611115996","id":"2e3d985b-3610-4b8b-aa3b-2e45804edd2c","name":"logstash-db848f65f-lnlf9","type":"filebeat","version":"7.10.0","hostname":"logstash-db848f65f-lnlf9"}
log: {"offset":80,"file":{"path":"/var/log/dpkg.log"}}
```
