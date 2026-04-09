# Filebeat与集成

> NOTE: 与Filebeat的集成需要[Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装Buddy。

[Filebeat](https://www.elastic.co/beats/filebeat) 是一个轻量级的转发器，用于转发和集中日志数据。安装并作为代理运行后，它会监控您指定的日志文件或位置，收集日志事件，并将它们转发进行索引，通常到Elasticsearch或Logstash。

现在，Manticore 也支持使用 Filebeat 作为处理管道。这允许收集和转换后的数据像发送到 Elasticsearch 一样发送到 Manticore。目前支持的版本为 7.17-9.3。

## Filebeat配置

配置取决于您使用的Filebeat版本。

### Filebeat 7.17, 8.0, 8.1的配置

> **重要**: Filebeat版本7.17.0, 8.0.0和8.1.0与glibc 2.35+（用于Ubuntu 22.04及更高版本的发行版）存在已知问题。这些版本可能会因“致命glibc错误：rseq注册失败”而崩溃。要修复此问题，请按以下所示添加`seccomp`配置。

```yaml
filebeat.inputs:
- type: log
  enabled: true
  paths:
    - /var/log/dpkg.log
  close_eof: true
  scan_frequency: 1s

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0
  allow_older_versions: true  # Required for 8.1

# Fix for glibc 2.35+ compatibility (Ubuntu 22.04+)
seccomp:
  default_action: allow
  syscalls:
    - action: allow
      names:
        - rseq

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```

**参考**: [Issue #30576](https://github.com/elastic/beats/issues/30576)，[PR #30620](https://github.com/elastic/beats/pull/30620)


### Filebeat 8.1 - 8.10的配置

对于版本8.1到8.10，您需要添加`allow_older_versions`选项：

```
filebeat.inputs:
- type: log
  enabled: true
  paths:
    - /var/log/dpkg.log
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

### Filebeat 8.11 - 8.19的配置

从版本8.11开始，输出压缩默认启用，因此您必须显式设置`compression_level: 0`以与Manticore兼容：

```
filebeat.inputs:
- type: log
  enabled: true
  paths:
    - /var/log/dpkg.log
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

### Filebeat 9.0+的配置

Filebeat 9.0引入了主要的架构更改，用`filestream`替换`log`输入类型。从版本9.0开始，文件识别方法也已更改，使用指纹法，这需要文件至少为1024字节（请参阅[issue #44780](https://github.com/elastic/beats/issues/44780)）。为了与任何大小的文件兼容，您必须禁用指纹识别。

以下是Filebeat 9.0及更高版本所需的配置：

```
filebeat.inputs:
- type: filestream
  id: dpkg-log-input
  enabled: true
  paths:
    - /var/log/dpkg.log
  prospector.scanner.check_interval: 1s
  prospector.scanner.fingerprint.enabled: false

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

**Filebeat 9.0+的重要注意事项:**
- `type: filestream`输入替换旧的`type: log`
- `prospector.scanner.fingerprint.enabled: false`设置是**必需的**，以禁用基于指纹的文件识别，确保可靠处理小于1024字节的文件
- `id`字段是filestream输入所需的，并且必须是唯一的

## Filebeat结果

一旦使用此配置运行Filebeat，日志数据将发送到Manticore并正确索引。以下是Manticore创建的表的结构示例以及插入的文档示例：

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
