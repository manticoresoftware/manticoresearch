# Integration with Vector.dev

> NOTE: The integration with Fluent Bit requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

[Vector by Datadog](https://vector.dev/) is an open source observability data pipeline that can collect, transform, and route logs or metrics. While Vector can aggregate the data by itself, pairing it with Manticore provides a purpose-built storage and search layer.

The example below shows how to forward Debian's `dpkg.log` through Vector.dev and index it in Manticore.

## Example log structure

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Vector.dev configuration

Create a `vector.toml` similar to the following:

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

- `endpoints` points to Manticore's HTTP interface (port `9308` by default). Adjust it if your instance listens elsewhere.
- The `remap` transform moves Vector's default `timestamp` field to `vec_timestamp`, because `timestamp` is reserved in Manticore.
- `bulk.index` defines the table that will be created automatically when Vector starts sending data.

Start Vector.dev with this configuration and it will tail the log file, transform each event, and forward it directly to Manticore.

## Running Vector.dev

Save the configuration as `vector.toml`, then start the agent:

```
vector --config vector.toml
```

If you run Vector.dev in Docker, mount both the config file and the log directory, for example:

```
docker run --rm -v /var/log/dpkg.log:/var/log/dpkg.log:ro \
  -v $(pwd)/vector.toml:/etc/vector/vector.toml:ro \
  timberio/vector:latest --config /etc/vector/vector.toml
```

## Vector.dev Results

When the pipeline runs, Manticore creates the `dpkg_log` table automatically. Its schema and example documents look like this:

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

## Conclusion

Using Vector.dev with Manticore lets you collect logs from virtually any source, enrich or sanitize them in-flight, and store the results in a search-ready database. This workflow keeps observability pipelines simple while still enabling advanced transformations when needed.
