# 监控

Manticore 提供了一个 Prometheus 指标端点，并为 Manticore Buddy 提供官方维护的告警规则和 Grafana 仪表板。

## Prometheus 导出器

<!-- example prometheus exporter -->

Manticore Search 内置了 Prometheus 导出器。
要请求指标，请确保暴露 HTTP 端口，然后直接调用 /metrics 端点。

**注意：** 导出器需要启用 **Buddy**。

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

## Prometheus 告警规则

此文件夹包含为 Manticore Search 官方维护的监控资源。

文件：[manticore-alerts.yml](https://github.com/manticoresoftware/manticoresearch-buddy/blob/main/monitoring/prometheus/manticore-alerts.yml)

示例抓取配置：

```yaml
scrape_configs:
  - job_name: manticoresearch
    metrics_path: /metrics
    static_configs:
      - targets:
          - 127.0.0.1:9308
```

#### 告警指南：
* **ManticoreBuddyTargetDown**：Prometheus 无法抓取此目标。表示服务已关闭、不可达或抓取配置错误。
* **ManticoreBuddyRecentlyRestarted**：5 分钟内运行时间保持在 5 分钟以下。表示进程正在重启或波动。
* **ManticoreBuddyMaxedOutErrors**：`maxed_out` 错误计数器增加。表示 Manticore 因限制而拒绝工作。
* **ManticoreBuddySearchLatencyP95High**：p95 搜索延迟在 10 分钟内保持在 500 毫秒以上。大多数用户会感觉搜索变慢。
* **ManticoreBuddySearchLatencyP99High**：p99 搜索延迟在 10 分钟内保持在 1000 毫秒以上。最慢的请求非常慢。
* **ManticoreBuddyWorkQueueBacklog**：工作队列长度在 5 分钟内保持在 100 以上。请求正在堆积。
* **ManticoreBuddyWorkersSaturated**：超过 90% 的工作线程在 10 分钟内处于活动状态。线程已满，延迟将上升。
* **ManticoreBuddyQueryCacheNearLimit**：查询缓存使用率在 10 分钟内保持在 90% 以上。缓存频繁更换，预计查询会变慢。
* **ManticoreBuddyDiskMappedCacheLow**：磁盘映射缓存比率在 15 分钟内保持在 50% 以下。磁盘 I/O 增加，查询变慢。
* **ManticoreBuddyDiskMappedCacheVeryLow**：磁盘映射缓存比率在 15 分钟内保持在 20% 以下。严重的磁盘 I/O，存在高延迟风险。

#### 这些告警存在的原因：
* 可用性和重启检测可以早期发现严重故障和波动。
* 延迟和排队检测可以在错误出现前发现过载。
* 缓存和磁盘映射比率是 I/O 密集型减速的早期预警。
* 工作线程饱和和 maxed_out 错误表明容量限制已被触及。

#### 如果看到告警：
* **TargetDown**：检查服务健康状况、网络、抓取配置和防火墙。
* **RecentlyRestarted**：检查日志、崩溃、内存不足或编排重启。
* **MaxedOutErrors**：检查资源限制、工作线程数和查询并发。
* **延迟/排队/饱和**：减少负载、扩展资源或调整查询/索引。
* **缓存告警**：增加缓存大小或减少缓存频繁更换的查询。
* **磁盘映射缓存** 比率告警：增加 RAM、减少工作集或优化表。

## Grafana 仪表板

文件：[manticore-buddy-dashboard.json](https://github.com/manticoresoftware/manticoresearch-buddy/blob/main/monitoring/grafana/manticore-dashboard.json)

#### 仪表板面板指南：
* **运行时间**：自上次重启以来的时间。如果此值持续下降，服务正在重启或崩溃。
* **版本**：正在运行的 Manticore 精确构建版本。如果部署后行为发生变化，请首先检查此内容。
* **当前连接数**：当前打开的客户端连接数。突然激增表示流量激增；突然下降表示故障或客户端问题。
* **活动工作线程**：当前正在工作的线程数。如果长时间接近总数，服务器已过载。
* **负载（所有队列）**：1/5/15 分钟的平均队列负载。如果三条线都上升，系统正在落后。
* **工作队列长度**：等待的任务数。如果持续增长，请求正在堆积，延迟将上升。
* **每秒命令数**：每秒的搜索/插入/更新等操作数。告诉你流量是否繁重以及类型。
* **搜索延迟（p95/p99）**：95%/99% 的搜索速度都快于此值。如果这些值增长，即使平均值看起来正常，用户也会感觉搜索变慢。
* **查询缓存使用率**：使用的缓存与最大值的比率。如果使用率持续增长到极限，缓存将频繁更换，查询会变慢。
* **查询缓存命中率**：每秒的缓存命中次数。如果流量保持高位而此值下降，缓存没有帮助。
* **磁盘和 RAM 字节**：磁盘和 RAM 上索引数据的大小。快速增长意味着更多的存储压力，通常查询更慢。
* **按表的磁盘映射缓存比率**：每个表在内存中已缓存的比例。低比率意味着更多磁盘读取和更慢的查询。
* **最慢线程**：最慢运行查询所需的时间。如果此值激增，可能存在卡住或非常重的查询。
* **按类型划分的连接**：将连接分为当前、buddy 和 vip。有助于了解哪种协议驱动负载。

#### 如何导入：
1. 打开 Grafana 并进入仪表板 -> 导入。
2. 上传 JSON 文件。
3. 在提示时选择您的 Prometheus 数据源。
