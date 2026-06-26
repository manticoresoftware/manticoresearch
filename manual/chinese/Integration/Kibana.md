# Manticore 与 Kibana 的集成

[Kibana](https://www.elastic.co/kibana) 是一个可视化界面，可让你探索、可视化并为日志数据创建仪表板。将 Kibana 与 Manticore Search 集成后，相比 Elasticsearch，Kibana 可视化内容的加载速度最高可提升 3 倍，正如这个 [demo](https://github.com/manticoresoftware/kibana-demo#manticore-search-kibana-demo) 所示。这个集成让用户能够通过交互式仪表板、自定义可视化和实时搜索能力无缝分析数据。它还通过支持 Logstash 和 Filebeat 等工具简化了多样化数据源的处理，从而实现更流畅的数据摄取，因此非常适合日志分析工作流。

## 先决条件
1. **下载 Kibana**：确保下载与 Manticore 兼容的 Kibana 版本。目前已测试并推荐 7.6.0 版本。其他 7.x 版本可能可用，但可能会引入问题。不支持 8.x 版本。
2. **验证 Manticore**：确保你的 Manticore 实例正在运行，并且其 HTTP API 可访问（默认：`http://localhost:9308`）。

## 配置
1. 打开 [Kibana 配置文件](https://www.elastic.co/guide/en/kibana/current/settings.html)（`kibana.yml`）。
2. 设置你的 Manticore 实例 URL：
   ```yaml
   elasticsearch.hosts: ["http://localhost:9308"]
   ```
3. 启动 Kibana，并在浏览器中打开 `http://localhost:5601`。如有需要，请将 `localhost` 替换为你服务器的 IP 或主机名。

> 注意：Manticore 与 Kibana 配合使用时不需要配置身份验证。另请注意，Manticore 必须以 [实时模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) 运行，才能与 Kibana 集成。

### Manticore 配置示例

```searchd {
    listen = 127.0.0.1:9308:http
    pid_file = /var/run/manticore/searchd.pid
    data_dir = /var/lib/manticore
 }
```

## 支持的功能
### Discover
- 在 Kibana 的 **Discover** 选项卡中交互式搜索和筛选你的数据。
- 使用查询栏和 [Kibana 查询语言](https://www.elastic.co/guide/en/kibana/current/kuery-query.html) 中的简单查询来优化搜索。

### 可视化
- 转到 **Visualizations** 创建自定义可视化：
  - 如果还不存在，请创建一个表模式（在 Kibana 中称为 'index pattern'）来定义你的数据源。
  - 选择一种可视化类型（例如柱状图、折线图或饼图）。
  - 配置你的可视化，执行它，并探索你的数据。
  - 将可视化保存以便日后使用。

### 仪表板
- 进入 **Dashboards** 创建或查看交互式仪表板：
  - 添加可视化、筛选器或控件，以获得个性化体验。
  - 直接从仪表板与数据交互。
  - 将仪表板保存以便日后使用。

### 管理
- 转到 **Management > Kibana** 自定义默认时区和可视化偏好等设置。

## 限制
- 目前已测试并推荐 Kibana 7.6.0 版本。其他 7.x 版本可能可用，但可能会引发问题。不支持 8.x 版本。
- 不支持以下 Elasticsearch 特有的字段类型：
  - [空间数据类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#spatial_datatypes)
  - [结构化数据类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#structured-data-types)
  - [文档排序类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#document-ranking-types)
  - [文本搜索类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#text-search-types)（不包括纯 `'text'`）
  - [关系数据类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#object-types)
- 指标聚合函数仅限于 [Manticore 支持的函数](../Searching/Grouping.md#Aggregation-functions)。
- 不支持以下 Kibana 工具：
  - [Canvas](https://www.elastic.co/guide/en/kibana/7.6/canvas.html) – 用于将数据与颜色和图像结合起来进行可视化和展示的工具。
  - [Elastic Maps](https://www.elastic.co/guide/en/kibana/7.6/maps.html) – 用于分析地理数据的工具。
  - [Metrics](https://www.elastic.co/guide/en/kibana/7.6/xpack-infra.html) – 用于监控基础设施指标的应用。
  - [Logs](https://www.elastic.co/guide/en/kibana/7.6/xpack-logs.html) – 类似控制台的界面，用于浏览常见服务的日志。
  - 监控：
    - [Uptime](https://www.elastic.co/guide/en/kibana/7.6/xpack-uptime.html) – 通过 HTTP/S、TCP 和 ICMP 监控网络端点状态。
    - [APM (Application Performance Monitoring)](https://www.elastic.co/guide/en/kibana/7.6/xpack-apm.html) – 收集应用程序的深入性能指标。
    - [SIEM (Security Information and Event Management)](https://www.elastic.co/guide/en/kibana/7.6/xpack-siem.html) – 为安全团队提供的交互式工作区，用于分诊事件并开展初步调查。
    - [ILM (Index lifecycle management)](https://www.elastic.co/guide/en/elasticsearch/reference/7.6/index-lifecycle-management.html) - 根据性能、可靠性和保留要求自动管理索引。
    - [Stack Monitoring](https://www.elastic.co/guide/en/kibana/7.6/xpack-monitoring.html) – 提供整个 Elastic Stack 中监控数据的可视化。
  - [Elasticsearch Management](https://www.elastic.co/guide/en/kibana/7.6/management.html) – 用于管理 Elastic Stack 对象的界面，包括 ILM (Index Lifecycle Management) 等。

## 数据摄取与探索
将 Manticore 与 [Logstash](../Integration/Logstash.md)、[Filebeat](../Integration/Filebeat.md)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) 或 [Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/) 等工具集成，以从 Web 日志等来源摄取数据。数据加载到 Manticore 后，你就可以在 Kibana 中对其进行探索和可视化。

