# Integration with Filebeat

> NOTE: The integration with Filebeat requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

[Filebeat](https://www.elastic.co/beats/filebeat) is a lightweight shipper for forwarding and centralizing log data. Once installed as an agent, it monitors the log files or locations you specify, collects log events, and forwards them for indexing, usually to Elasticsearch or Logstash.

Now, Manticore also supports the use of Filebeat as processing pipelines. This allows the collected and transformed data to be sent to Manticore just like to Elasticsearch. Currently, all the versions >= 7.10  are supported.

## Filebeat configuration

Below is a Filebeat config to work with our example dpkg log:

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

### Configuration for Filebeat versions >= 8.11

Note that Filebeat versions higher than 8.10 have the output compression feature enabled by default. That is why the `compression_level: 0` option must be added to the configuration file to provide compatibility with Manticore:

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
