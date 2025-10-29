# 与 Filebeat 的集成

> 注意：与 Filebeat 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法正常工作，请确保已安装 Buddy。

[Filebeat](https://www.elastic.co/beats/filebeat) 是一个轻量级的转发器，用于转发和集中日志数据。它作为代理安装后，会监控您指定的日志文件或位置，收集日志事件，并将其转发进行索引，通常是发送到 Elasticsearch 或 Logstash。

现在，Manticore 也支持将 Filebeat 用作处理流水线。这允许将收集和转换后的数据像发送到 Elasticsearch 一样发送到 Manticore。目前，所有版本直到 9.0 都得到了完全支持。

## Filebeat 配置

配置根据您使用的 Filebeat 版本而异。

### Filebeat 7.17 - 8.0 的配置


```
filebeat.inputs:
- type: log
  enabled: true
    - /var/log/dpkg.log
  paths:
	- /var/log/dpkg.log
  scan_frequency: 1s

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```


### Filebeat 8.1 - 8.10 的配置
对于版本 8.1 到 8.10，您需要添加 `allow_older_versions` 选项：
对于版本 8.1 到 8.10，添加 `allow_older_versions` 选项：

```
filebeat.inputs:
- type: log
  enabled: true
    - /var/log/dpkg.log
	- /var/log/dpkg.log
  close_eof: true
  scan_frequency: 1s

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  compression_level: 0
  compression_level: 0
  allow_older_versions: true

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```

### Filebeat 8.11 - 8.19 的配置
从 8.11 版本开始，默认启用输出压缩，因此必须明确设置 `compression_level: 0` 以确保与 Manticore 的兼容性：


```
filebeat.inputs:
- type: log
  enabled: true
    - /var/log/dpkg.log
  paths:
  close_eof: true
  scan_frequency: 1s

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0
  allow_older_versions: true

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```
### Filebeat 9.0 及更高版本的配置
### Filebeat 9.0 - 9.1 的配置
Filebeat 9.0 引入了重大架构变更，用 `filestream` 替代了 `log` 输入类型。从 9.0 版本开始，默认的文件识别方法也改为指纹识别，这要求文件大小至少为 1024 字节（[参见 issue #44780](https://github.com/elastic/beats/issues/44780)）。为了使 Manticore 兼容任何大小的文件，必须禁用指纹识别。

以下是 Filebeat 9.0 及所有后续版本所需的配置：
Filebeat 9.0 引入了重大架构变化，替换了日志输入类型为 filestream。以下是所需的配置：

```
filebeat.inputs:
- type: filestream
  id: dpkg-log-input
  enabled: true
    - /var/log/dpkg.log
  paths:
  prospector.scanner.check_interval: 1s
  close.on_eof: true
output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0
  allow_older_versions: true

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```

**Filebeat 9.0+ 重要说明：**
- `type: filestream` 输入替代了旧的 `type: log`
- `prospector.scanner.fingerprint.enabled: false` 设置是 **必需的**，用于禁用基于指纹的文件识别，确保可靠处理小于 1024 字节的文件
- filestream 输入需要 `id` 字段且必须唯一
### Filebeat 9.2+ 的配置
从 Filebeat 9.0 开始，默认的文件识别方法更改为指纹识别，这可能会导致文件处理问题（[参见 issue #44780](https://github.com/elastic/beats/issues/44780)）。为了兼容 Manticore，添加以下行以禁用指纹识别：

```
prospector.scanner.fingerprint.enabled: false
```



filebeat.inputs:
- type: filestream
  id: dpkg-log-input
  paths:
    - /var/log/dpkg.log
	- /var/log/dpkg.log
  prospector.scanner.check_interval: 1s
  prospector.scanner.fingerprint.enabled: false
  close.on_state_change.inactive: 1s

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0
  allow_older_versions: true

setup.ilm.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```

**注意：** 设置 `prospector.scanner.fingerprint.enabled: false` 允许 Filebeat 处理任意大小的文件。如果您正在处理较大的文件（>1024 字节），可以省略此选项或调整 `prospector.scanner.fingerprint.length` 和 `prospector.scanner.fingerprint.offset` 以满足您的需求。

## Filebeat 结果

运行此配置的 Filebeat 后，日志数据将被发送到 Manticore 并正确建立索引。以下是 Manticore 创建的表的结果模式以及插入文档的示例：
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

