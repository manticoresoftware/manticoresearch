# Monitoring

Manticore provides a Prometheus metrics endpoint and officially maintained alert rules and Grafana dashboards for Manticore Buddy.

## Prometheus Exporter

<!-- example prometheus exporter -->

Manticore Search has a built-in Prometheus exporter.
To request metrics, make sure the HTTP port is exposed and simply call the /metrics endpoint.

**Note:** The exporter requires **Buddy** to be enabled.

<!-- intro -->
##### HTTP:

<!-- request http -->

```go
curl -s 0:9308/metrics
```
<!-- response http -->

```http
# HELP manticore_uptime_seconds Time in seconds since start
# TYPE manticore_uptime_seconds counter
manticore_uptime_seconds 25
# HELP manticore_connections_count Connections count since start
# TYPE manticore_connections_count gauge
manticore_connections_count 55
# HELP manticore_maxed_out_error_count Count of maxed_out errors since start
# TYPE manticore_maxed_out_error_count counter
manticore_maxed_out_error_count 0
# HELP manticore_version Manticore Search version
# TYPE manticore_version gauge
manticore_version {version="0.0.0 c88e811b2@25060409 (columnar 5.0.1 59c7092@25060304) (secondary 5.0.1 59c7092@25060304) (knn 5.0.1 59c7092@25060304) (embeddings 1.0.0) (buddy v3.28.6-7-g14ee10)"} 1
# HELP manticore_mysql_version Manticore Search version
# TYPE manticore_mysql_version gauge
manticore_mysql_version {version="0.0.0 c88e811b2@25060409 (columnar 5.0.1 59c7092@25060304) (secondary 5.0.1 59c7092@25060304) (knn 5.0.1 59c7092@25060304) (embeddings 1.0.0)"} 1
# HELP manticore_command_search_count Count of search queries since start
# TYPE manticore_command_search_count counter
manticore_command_search_count 1
......
```

<!-- end -->

## Prometheus alerting rules

This folder contains officially maintained monitoring assets for Manticore Search.

File: [manticore-alerts.yml](https://github.com/manticoresoftware/manticoresearch-buddy/blob/main/monitoring/prometheus/manticore-alerts.yml)

Example scrape config:

```yaml
scrape_configs:
  - job_name: manticoresearch
    metrics_path: /metrics
    static_configs:
      - targets:
          - 127.0.0.1:9308
```

#### Alert guide:
* **ManticoreBuddyTargetDown**: Prometheus cannot scrape this target. Means the service is down, unreachable, or the scrape config is wrong.
* **ManticoreBuddyRecentlyRestarted**: Uptime stays under 5 minutes for 5 minutes. Means the process is restarting or flapping.
* **ManticoreBuddyMaxedOutErrors**: The `maxed_out` error counter increases. Means Manticore is rejecting work due to limits.
* **ManticoreBuddySearchLatencyP95High**: p95 search latency stays above 500 ms for 10 minutes. Most users feel slow searches.
* **ManticoreBuddySearchLatencyP99High**: p99 search latency stays above 1000 ms for 10 minutes. The slowest requests are very slow.
* **ManticoreBuddyWorkQueueBacklog**: Work queue length stays above 100 for 5 minutes. Requests are piling up.
* **ManticoreBuddyWorkersSaturated**: More than 90% of workers are active for 10 minutes. Threads are maxed, so latency will rise.
* **ManticoreBuddyQueryCacheNearLimit**: Query cache stays above 90% usage for 10 minutes. Cache churn likely, expect slower queries.
* **ManticoreBuddyDiskMappedCacheLow**: Disk mapped cache ratio stays below 50% for 15 minutes. More disk IO, slower queries.
* **ManticoreBuddyDiskMappedCacheVeryLow**: Disk mapped cache ratio stays below 20% for 15 minutes. Severe disk IO, high latency risk.

#### Why these alerts exist:
* Availability and restarts catch hard outages and flapping early.
* Latency and queueing detect overload before errors appear.
* Cache and disk-mapped ratio are early warnings for IO-bound slowdowns.
* Worker saturation and maxed_out errors indicate capacity limits are being hit.

#### If you see an alert:
* **TargetDown**: check service health, network, scrape config, and firewall.
* **RecentlyRestarted**: check logs, crashes, OOM, or restarts from orchestration.
* **MaxedOutErrors**: check resource limits, worker count, and query concurrency.
* **Latency/Queue/Saturation**: reduce load, scale resources, or tune queries/indexes.
* **Cache alerts**: increase cache size or reduce cache-churning queries.
* **Disk mapped cache** ratio alerts: add RAM, reduce working set, or optimize tables.

## Grafana dashboard

File: [manticore-buddy-dashboard.json](https://github.com/manticoresoftware/manticoresearch-buddy/blob/main/monitoring/grafana/manticore-dashboard.json)

#### Dashboard panel guide:
* **Uptime**: Time since the last restart. If this keeps dropping, the service is restarting or crashing.
* **Version**: The exact Manticore build running. If behavior changes after a deploy, check this first.
* **Current Connections**: How many client connections are open right now. A sudden spike = traffic surge; a sudden drop = outage or client issues.
* **Active Workers**: How many worker threads are busy. If this is near the total for a long time, the server is overloaded.
* **Load (All Queues)**: Average queue load over 1/5/15 minutes. If all three lines climb, the system is falling behind.
* **Work Queue Length**: How many tasks are waiting. If this keeps growing, requests are piling up and latency will rise.
* **Commands per Second**: How many searches/inserts/updates/etc. per second. Tells you if traffic is heavy and what kind.
* **Search Latency (p95/p99)**: 95%/99% of searches are faster than this. If these grow, users feel slow searches even if averages look fine.
* **Query Cache Usage**: How much cache is used vs the max. If usage keeps growing to the limit, the cache will churn and queries slow down.
* **Query Cache Hit Rate**: How many cache hits per second. If this goes down while traffic stays high, the cache is not helping.
* **Disk and RAM Bytes**: Size of indexed data on disk and RAM. Rapid growth means more storage pressure and usually slower queries.
* **Disk Mapped Cache Ratio by Table**: How much of each table is already cached in memory. Low ratios mean more disk reads and slower queries.
* **Slowest Thread**: How long the slowest running query takes. If this spikes, there may be stuck or very heavy queries.
* **Connections by Type**: Splits connections into current, buddy, and vip. Helpful to see which protocol is driving load.

#### How to import:
1. Open Grafana and go to Dashboards -> Import.
2. Upload the JSON file.
3. Select your Prometheus datasource when prompted.
