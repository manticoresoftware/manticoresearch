# 与 Filebeat 集成

> 注意：与 Filebeat 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保 Buddy 已安装。

[Filebeat](https://www.elastic.co/beats/filebeat) 是一个轻量级的日志数据传输和集中化工具。一旦作为代理安装后，它会监控您指定的日志文件或位置，收集日志事件并将其转发以进行索引，通常发送到 Elasticsearch 或 Logstash。

现在，Manticore 也支持使用 Filebeat 作为处理管道。这允许收集和转换后的数据像发送到 Elasticsearch 一样被发送到 Manticore。目前，支持的所有版本为 7.10 及以上。

## Filebeat 配置

以下是与我们示例的 dpkg 日志配合使用的 Filebeat 配置：

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

请注意，Filebeat 版本高于 8.10 的默认启用了输出压缩功能。因此，必须在配置文件中添加 `compression_level: 0` 选项，以确保与 Manticore 的兼容性：

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

一旦您使用该配置运行 Filebeat，日志数据将被发送到 Manticore 并正确地被索引。以下是 Manticore 创建的表的最终模式以及插入文档的示例：

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
