# Manticore 与 Kibana 的集成

[Kibana](https://www.elastic.co/kibana) 是一个可视化界面，允许您探索、可视化并为日志数据创建仪表板。将 Kibana 与 Manticore Search 集成后，相较于 Elasticsearch，Kibana 可视化加载速度可提升最多 3 倍，如此[演示](https://github.com/manticoresoftware/kibana-demo#manticore-search-kibana-demo)所示。此集成使用户能够使用交互仪表板、自定义可视化和实时搜索功能，无缝地分析数据。此外，它通过支持 Logstash 和 Filebeat 等工具简化多样数据源的处理，实现数据摄取的流畅化，是日志分析工作流的理想选择。

## 前提条件
1. **下载 Kibana**：确保下载安装的 Kibana 版本与 Manticore 兼容。目前推荐并测试的版本为 7.6.0。其他 7.x 版本可能能用，但可能存在问题。不支持 8.x 版本。
2. **验证 Manticore**：确保 Manticore 实例正在运行且其 HTTP API 可访问（默认地址：`http://localhost:9308`）。

## 配置
1. 打开[Kibana 配置文件](https://www.elastic.co/guide/en/kibana/current/settings.html)（`kibana.yml`）。
2. 设置 Manticore 实例的 URL：
   ```yaml
   elasticsearch.hosts: ["http://localhost:9308"]
   ```
3. 启动 Kibana，并在浏览器中访问 `http://localhost:5601`。如有必要，将 `localhost` 替换为服务器的 IP 或主机名。

> 注意：Manticore 在与 Kibana 配合使用时无需进行身份验证设置。另请注意，Manticore 必须以[实时模式](../Read_this_first#Real-time-mode-vs-plain-mode)工作，才能与 Kibana 集成。

### Manticore 配置示例
```searchd {
    listen = 127.0.0.1:9308:http
    pid_file = /var/run/manticore/searchd.pid
    data_dir = /var/lib/manticore
 }

## 支持的功能
### 发现（Discover）
- 使用 Kibana 中的 **发现** 选项卡，交互式地搜索和过滤数据。
- 使用查询栏和简单查询语法（[Kibana 查询语言](https://www.elastic.co/guide/en/kibana/current/kuery-query.html)）细化搜索。

### 可视化（Visualizations）
- 转到 **可视化**，创建自定义可视化：
  - 如果没有现成的表模式（Kibana 里叫“索引模式”），则创建一个以定义数据源。
  - 选择可视化类型（如柱状图、折线图或饼图）。
  - 配置并执行可视化，探索数据。
  - 保存可视化以便将来使用。

### 仪表板（Dashboards）
- 访问 **仪表板**，创建或查看交互式仪表板：
  - 添加可视化、过滤器或控件，实现个性化体验。
  - 保存仪表板以便将来使用。

### 管理（Management）
- 前往 **管理 > Kibana**，自定义默认时区和可视化偏好等设置。

## 限制
- 目前仅测试推荐使用 Kibana 7.6.0 版本。其他 7.x 版本可能能用但可能引发问题。8.x 版本不支持。
- 目前，推荐使用并测试过的是 Kibana 版本 7.6.0。其他 7.x 版本可能可用，但可能会 

导致问题。8.x 版本不受支持。
- 下列 Elasticsearch 特定字段类型不受支持：
  - [空间数据类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#spatial_datatypes)
  - [结构化数据类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#structured-data-types)
  - [文档排名类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#document-ranking-types)
  - [文本搜索类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#text-search-types)（除纯文本“text”外）
  - [关系数据类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#object-types)
- 度量聚合函数仅限于[Manticore支持的聚合函数](../Searching/Grouping.md#Aggregation-functions)。
- 下列 Kibana 工具不支持：
  - [Canvas](https://www.elastic.co/guide/en/kibana/7.6/canvas.html) – 一个用于结合数据与颜色和图像的可视化及展示工具。
  - [Elastic 地图](https://www.elastic.co/guide/en/kibana/7.6/maps.html) – 用于分析地理数据的工具。
  - [指标（Metrics）](https://www.elastic.co/guide/en/kibana/7.6/xpack-infra.html) – 用于监控基础设施指标的应用。
  - [日志（Logs）](https://www.elastic.co/guide/en/kibana/7.6/xpack-logs.html) – 类似控制台的界面，用于探索常见服务的日志。
  - 监控：
    - [正常运行时间（Uptime）](https://www.elastic.co/guide/en/kibana/7.6/xpack-uptime.html) – 通过 HTTP/S、TCP 和 ICMP 监控网络端点状态。
    - [APM（应用性能监控）](https://www.elastic.co/guide/en/kibana/7.6/xpack-apm.html) – 收集应用的详尽性能指标。
    - [SIEM（安全信息和事件管理）](https://www.elastic.co/guide/en/kibana/7.6/xpack-siem.html) – 安全团队用于事件分级和初步调查的交互工作空间。
    - [ILM（索引生命周期管理）](https://www.elastic.co/guide/en/elasticsearch/reference/7.6/index-lifecycle-management.html) - 根据性能、弹性和保留需求自动管理索引。
    - [堆栈监控（Stack Monitoring）](https://www.elastic.co/guide/en/kibana/7.6/xpack-monitoring.html) – 提供 Elastic Stack 监控数据的可视化。
  - [Elasticsearch 管理](https://www.elastic.co/guide/en/kibana/7.6/management.html) – 管理 Elastic Stack 对象（包括 ILM）等的用户界面。

## 数据摄取与探索
将 Manticore 与[Logstash](../Integration/Logstash.md)、[Filebeat](../Integration/Filebeat.md)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/)或[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/)等工具集成，从 Web 日志等数据源摄取数据。数据加载入 Manticore 后，您即可在 Kibana 中进行探索和可视化。

