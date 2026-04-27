# Integration with Filebeat

> NOTE: The integration with Filebeat requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

[Filebeat](https://www.elastic.co/beats/filebeat) is a lightweight shipper for forwarding and centralizing log data. Once installed as an agent, it monitors the log files or locations you specify, collects log events, and forwards them for indexing, usually to Elasticsearch or Logstash.

Now, Manticore also supports the use of Filebeat as processing pipelines. This allows the collected and transformed data to be sent to Manticore just like to Elasticsearch. Currently, versions 7.17-9.3 are supported.

## Filebeat configuration

Configuration varies depending on which version of Filebeat you're using.

### Configuration for Filebeat 7.17, 8.0, 8.1

> **Important**: Filebeat versions 7.17.0, 8.0.0, and 8.1.0 have a known issue with glibc 2.35+ (used in Ubuntu 22.04 and newer distributions). These versions may crash with "Fatal glibc error: rseq registration failed". To fix this, add the `seccomp` configuration as shown below.

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

**References**: [Issue #30576](https://github.com/elastic/beats/issues/30576), [PR #30620](https://github.com/elastic/beats/pull/30620)


### Configuration for Filebeat 8.1 - 8.10

For versions 8.1 through 8.10, you need to add the `allow_older_versions` option:

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

### Configuration for Filebeat 8.11 - 8.19

From version 8.11, output compression is enabled by default, so you must explicitly set `compression_level: 0` for compatibility with Manticore:

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

### Configuration for Filebeat 9.0+

Filebeat 9.0 introduces a major architecture change, replacing the `log` input type with `filestream`. Starting from version 9.0, the default file identification method also changed to fingerprint, which requires files to be at least 1024 bytes ([see issue #44780](https://github.com/elastic/beats/issues/44780)). For Manticore compatibility with files of any size, you must disable fingerprinting.

Here's the required configuration for Filebeat 9.0 and all later versions:

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

**Important notes for Filebeat 9.0+:**
- The `type: filestream` input replaces the older `type: log`
- The `prospector.scanner.fingerprint.enabled: false` setting is **required** to disable fingerprint-based file identification, ensuring reliable processing of files smaller than 1024 bytes
- The `id` field is required for filestream inputs and must be unique

## Filebeat results

Once you run Filebeat with this configuration, log data will be sent to Manticore and properly indexed. Here is the resulting schema of the table created by Manticore and an example of the inserted document:

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
