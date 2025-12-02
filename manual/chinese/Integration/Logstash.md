# 与 Logstash 的集成

> 注意：与 Logstash 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果集成不工作，请确保 Buddy 已安装。

[Logstash](https://www.elastic.co/logstash) 是一种日志管理工具，它从各种来源收集数据，实时转换数据，并发送到您想要的目标。它通常用作 Elasticsearch（一个开源分析和搜索引擎）的数据管道。

现在，Manticore 支持使用 Logstash 作为处理管道。这允许收集和转换后的数据像发送给 Elasticsearch 一样发送给 Manticore。目前，支持的版本是 7.6 到 9.2。

让我们来看看一个用于索引 `dpkg.log` 的简单 Logstash 配置文件示例，`dpkg.log` 是 Debian 包管理器的标准日志文件。该日志本身结构简单，如下所示：

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Logstash 配置

下面是一个 Logstash 配置示例：

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

请注意，在继续之前，需要强调一个关键注意事项：Manticore 不支持 Elasticsearch 的日志模板管理和索引生命周期管理功能。由于这些功能在 Logstash 中默认启用，必须在配置中显式禁用它们。此外，output 配置段中的 hosts 选项必须对应 Manticore 的 HTTP 监听端口（默认是 localhost:9308）。

## 针对不同版本的配置

配置根据您使用的 Logstash 版本而有所不同。

### Logstash 7.17 的配置

对于 Logstash 7.17，基本配置相当简单，不需要额外的 ILM 设置：

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
    index => "dpkg_log"
    hosts => ["http://localhost:9308"]
  }
}
```

运行命令：
```bash
logstash -f logstash.conf
```

### Logstash 8.0 - 9.1 的配置

从 8.0 版本开始，ILM（索引生命周期管理）和模板管理默认启用，必须显式禁用以兼容 Manticore：

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
    index => "dpkg_log"
    hosts => ["http://localhost:9308"]
    ilm_enabled => false
    manage_template => false
  }
}
```

对于 9.0 和 9.1 版本，Logstash 需要以超级用户身份运行。在启动前设置环境变量：

```bash
export ALLOW_SUPERUSER=1
logstash -f logstash.conf
```

### Logstash 9.2+ 的配置

从 9.2 版本开始，推荐通过配置文件来配置超级用户设置，而不是使用环境变量，提供了更持久和便于管理的方案。

配置文件（例如 `logstash.conf`）：
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
    index => "dpkg_log"
    hosts => ["http://localhost:9308"]
    ilm_enabled => false
    manage_template => false
  }
}
```

创建 `/etc/logstash/logstash.yml`：
```yaml
allow_superuser: true
```

运行命令：
```bash
logstash --path.settings=/etc/logstash -f logstash.conf
```

## Logstash 结果

按照上述方式调整配置后，您可以运行 Logstash，dpkg 日志中的数据将传递给 Manticore 并正确索引。

下面是创建的表的结果模式以及插入文档的示例：

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

