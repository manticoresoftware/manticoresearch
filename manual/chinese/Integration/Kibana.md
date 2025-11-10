# Manticore 与 Kibana 的集成

[Kibana](https://www.elastic.co/kibana) 是一个可视化界面，允许您探索、可视化并为日志数据创建仪表盘。将 Kibana 与 Manticore Search 集成，可以使 Kibana 可视化加载速度比 Elasticsearch 快最多 3 倍，如此 [演示](https://github.com/manticoresoftware/kibana-demo#manticore-search-kibana-demo) 所示。此集成使用户能够通过交互式仪表盘、自定义可视化和实时搜索功能无缝分析数据。它还通过支持 Logstash 和 Filebeat 等工具简化了处理多样化数据源的过程，实现了流畅的数据摄取，是日志分析工作流的理想选择。

## 前提条件
1. **下载 Kibana**：确保下载与 Manticore 兼容的 Kibana 版本。目前，推荐并测试的版本是 7.6.0。其他 7.x 版本可能可用，但可能会引入问题。不支持 8.x 版本。
2. **验证 Manticore**：确保您的 Manticore 实例正在运行且其 HTTP API 可访问（默认地址：`http://localhost:9308`）。

## 配置
1. 打开 [Kibana 配置文件](https://www.elastic.co/guide/en/kibana/current/settings.html) (`kibana.yml`)。
2. 设置您的 Manticore 实例的 URL：
   ```yaml
   elasticsearch.hosts: ["http://localhost:9308"]
   ```
3. 启动 Kibana 并在浏览器中打开 `http://localhost:5601`。如有必要，将 `localhost` 替换为您的服务器 IP 或主机名。

> 注意：Manticore 在与 Kibana 配合使用时不需要设置认证。同时请注意，Manticore 必须以[实时模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)运行，才能与 Kibana 集成。

### Manticore 配置示例
```searchd {
    listen = 127.0.0.1:9308:http
    pid_file = /var/run/manticore/searchd.pid
    data_dir = /var/lib/manticore
 }

## Supported Features
### Discover
- Use the **Discover** tab in Kibana to search and filter your data interactively.
- Refine your searches using the query bar with simple queries in the [Kibana query language](https://www.elastic.co/guide/en/kibana/current/kuery-query.html).

### Visualizations
- Navigate to **Visualizations** to create custom visualizations:
  - Create a table pattern (it’s called an 'index pattern' in Kibana) if one doesn’t already exist to define your data source.
  - Choose a visualization type (e.g., bar chart, line chart, or pie chart).
  - Configure your visualization, execute it, and explore your data.
  - Save your visualizations for future use.

### Dashboards
- Access **Dashboards** to create or view interactive dashboards:
  - Add visualizations, filters, or controls for a personalized experience.
  - Interact with your data directly from the dashboard.
  - Save dashboards for future use.

### Management
- Go to **Management > Kibana** to customize settings like default time zones and visualization preferences.

## Limitations
- Currently, Kibana version 7.6.0 is tested and recommended. Other 7.x versions may work but could cause issues. Versions 8.x are not supported.
- The following Elasticsearch-specific field types are not supported:
  - [Spatial data types](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#spatial_datatypes)
  - [Structured data types](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#structured-data-types)
  - [Document ranking types](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#document-ranking-types)
  - [Text search types](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#text-search-types) (except for plain 'text')
  - [Relational data types](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#object-types)
- Metric aggregation functions are limited to [those supported by Manticore](../Searching/Grouping.md#Aggregation-functions).
- The following Kibana tools are not supported:
  - [Canvas](https://www.elastic.co/guide/en/kibana/7.6/canvas.html) – A visualization and presentation tool for combining data with colors and images.
  - [Elastic Maps](https://www.elastic.co/guide/en/kibana/7.6/maps.html) – A tool for analyzing geographical data.
  - [Metrics](https://www.elastic.co/guide/en/kibana/7.6/xpack-infra.html) – An app for monitoring infrastructure metrics.
  - [Logs](https://www.elastic.co/guide/en/kibana/7.6/xpack-logs.html) – A console-like display for exploring logs from common services.
  - Monitoring:
    - [Uptime](https://www.elastic.co/guide/en/kibana/7.6/xpack-uptime.html) – Monitors the status of network endpoints via HTTP/S, TCP, and ICMP.
    - [APM (Application Performance Monitoring)](https://www.elastic.co/guide/en/kibana/7.6/xpack-apm.html) – Collects in-depth performance metrics from applications.
    - [SIEM (Security Information and Event Management)](https://www.elastic.co/guide/en/kibana/7.6/xpack-siem.html) – An interactive workspace for security teams to triage events and conduct initial investigations.
    - [ILM (Index lifecycle management)](https://www.elastic.co/guide/en/elasticsearch/reference/7.6/index-lifecycle-management.html) - Automatically manage indices according to performance, resiliency, and retention requirements.
    - [Stack Monitoring](https://www.elastic.co/guide/en/kibana/7.6/xpack-monitoring.html) – Provides visualizations of monitoring data across the Elastic Stack.
  - [Elasticsearch Management](https://www.elastic.co/guide/en/kibana/7.6/management.html) – A UI for managing Elastic Stack objects, including ILM (Index Lifecycle Management), etc.

## Data Ingestion and Exploration
Integrate Manticore with tools like [Logstash](../Integration/Logstash.md), [Filebeat](../Integration/Filebeat.md), [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/), or [Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/) to ingest data from sources like web logs. Once the data is loaded into Manticore, you can explore and visualize it in Kibana.

