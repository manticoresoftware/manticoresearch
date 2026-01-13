# Integration with Fluent Bit

> NOTE: The integration with Fluent Bit requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

[Fluent Bit](https://fluentbit.io/) is an open source, cross-platform log processor that can aggregate data from many sources and ship it to multiple destinations. You can feed Fluent Bit outputs directly into Manticore to make the collected data searchable in real time.

The following walkthrough indexes Debian's `dpkg.log` with Fluent Bit and Manticore.

## Example log structure

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Fluent Bit configuration

Create a configuration file such as `fluent-bit.conf`:

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

- The `[SERVICE]` block starts Fluent Bit in daemon mode, which is convenient for Docker-based setups. Disable the daemon flag if you want to run it in the foreground.
- `inotify_watcher` is turned off to avoid file notification limitations inside containers.
- The output plugin (`name es`) can talk to Manticore's HTTP endpoint on port `9308`.
- `index` defines the automatic table name Manticore creates when the first batch arrives.

Run Fluent Bit with this configuration and it will tail `dpkg.log`, then forward each line to Manticore.

## Running Fluent Bit

Store the configuration as `fluent-bit.conf`, then launch Fluent Bit:

```
fluent-bit -c fluent-bit.conf
```

To run it in Docker, mount the log file (read-only) and the configuration:

```
docker run --rm -v /var/log/dpkg.log:/var/log/dpkg.log:ro \
  -v $(pwd)/fluent-bit.conf:/fluent-bit/etc/fluent-bit.conf:ro \
  fluent/fluent-bit:latest -c /fluent-bit/etc/fluent-bit.conf
```

## Fluent Bit results

After ingestion begins, Manticore automatically creates the `dpkg_log` table. Here is the table definition and sample data:

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

## Conclusion

With this lightweight pipeline, Fluent Bit handles log collection and delivery while Manticore indexes the events for fast search and analytics. The approach works equally well for other log sources—just add more inputs and reuse the same Elasticsearch-compatible output pointed at your Manticore cluster.
