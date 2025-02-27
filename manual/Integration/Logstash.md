# Integration with Logstash

> NOTE: The integration with Logstash requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

[Logstash](https://www.elastic.co/logstash) is a log management tool that collects data from a variety of sources, transforms it on the fly, and sends it to your desired destination. It is often used as a data pipeline for Elasticsearch, an open-source analytics and search engine.

Now, Manticore supports the use of Logstash as a processing pipeline. This allows the collected and transformed data to be sent to Manticore just like to Elasticsearch. Currently, the versions 7.6-7.15  are supported.  

Let’s examine a simple example of a Logstash config file used for indexing `dpkg.log`, a standard log file of the Debian package manager. The log itself has a simple structure, as shown below:

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Logstash configuration

Here is an example Logstash configuration:

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

Note that, before proceeding further, one crucial caveat needs to be addressed: Manticore does not support Log Template Management and the Index Lifecycle Management features of Elasticsearch. As these features are enabled by default in Logstash, they need to be explicitly disabled in the config. Additionally, the hosts option in the output config section must correspond to Manticore’s HTTP listen port (default is localhost:9308).

## Logstash results

After adjusting the config as described, you can run Logstash, and the data from the dpkg log will be passed to Manticore and properly indexed.

Here is the resulting schema of the created table and an example of the inserted document:

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
