# Manticore与Kibana的集成

[Kibana](https://www.elastic.co/kibana) 是一个可视化界面，允许您探索、可视化和创建日志数据的仪表板。将Kibana与Manticore Search集成，可以使Kibana可视化的加载速度比Elasticsearch快最多3倍，正如这个[演示](https://github.com/manticoresoftware/kibana-demo#manticore-search-kibana-demo)所示。此集成使用户能够无缝地使用交互式仪表板、自定义可视化和实时搜索功能分析数据。它还通过支持像Logstash和Filebeat这样的工具简化了对多样数据源的处理，从而成为日志分析工作流的理想选择。

## 先决条件
1. **下载Kibana**：确保下载与Manticore兼容的Kibana版本。目前，推荐使用测试过的7.6.0版本。其他7.x版本可能有效，但可能会引入问题。8.x版本不受支持。
2. **验证Manticore**：确保您的Manticore实例正在运行，并且其HTTP API可达（默认：`http://localhost:9308`）。

## 配置
1. 打开[Kibana配置文件](https://www.elastic.co/guide/en/kibana/current/settings.html) (`kibana.yml`)。
2. 设置您的Manticore实例的URL：
   ```yaml
   elasticsearch.hosts: ["http://localhost:9308"]
   ```
3. 启动Kibana，并在浏览器中打开`http://localhost:5601`。如果需要，将`localhost`替换为您的服务器IP或主机名。

> 注意：在与Kibana配合使用时，Manticore不需要进行身份验证设置。

## 支持的功能
### 发现
- 使用Kibana中的**发现**选项卡以交互方式搜索和过滤您的数据。
- 使用查询栏通过简单查询精炼搜索，使用[Kibana查询语言](https://www.elastic.co/guide/en/kibana/current/kuery-query.html)。

### 可视化
- 转到**可视化**以创建自定义可视化：
  - 如果还不存在，则创建一个表模式（在Kibana中称为“索引模式”）以定义您的数据源。
  - 选择可视化类型（例如，柱状图、折线图或饼图）。
  - 配置您的可视化，执行它并探索您的数据。
  - 保存您的可视化以供将来使用。

### 仪表板
- 访问**仪表板**以创建或查看交互式仪表板：
  - 添加可视化、过滤器或控件，以获得个性化体验。
  - 直接从仪表板与您的数据进行交互。
  - 保存仪表板以供将来使用。

### 管理
- 转到**管理 > Kibana**以自定义设置，如默认时区和可视化偏好设置。

## 限制
- 当前推荐使用测试过的Kibana版本7.6.0。其他7.x版本可能有效，但可能会导致问题。8.x版本不受支持。
- 以下特定于Elasticsearch的字段类型不受支持：
  - [空间数据类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#spatial_datatypes)
  - [结构化数据类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#structured-data-types)
  - [文档排名类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#document-ranking-types)
  - [文本搜索类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#text-search-types)（除了普通“文本”）
  - [关系数据类型](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#object-types)
- 指标聚合函数仅限于[Manticore支持的那些](../Searching/Grouping.md#Aggregation-functions)。
- 以下Kibana工具不受支持：
  - [Canvas](https://www.elastic.co/guide/en/kibana/7.6/canvas.html) - 一个可视化和演示工具，用于将数据与颜色和图像结合。
  - [Elastic Maps](https://www.elastic.co/guide/en/kibana/7.6/maps.html) - 一个用于分析地理数据的工具。
  - [Metrics](https://www.elastic.co/guide/en/kibana/7.6/xpack-infra.html) - 用于监控基础设施指标的应用程序。
  - [Logs](https://www.elastic.co/guide/en/kibana/7.6/xpack-logs.html) - 类似控制台的显示，用于探索常见服务的日志。
  - 监控：
    - [Uptime](https://www.elastic.co/guide/en/kibana/7.6/xpack-uptime.html) - 通过HTTP/S、TCP和ICMP监控网络端点的状态。
    - [APM（应用性能监控）](https://www.elastic.co/guide/en/kibana/7.6/xpack-apm.html) - 从应用程序中收集深入的性能指标。
    - [SIEM（安全信息和事件管理）](https://www.elastic.co/guide/en/kibana/7.6/xpack-siem.html) - 为安全团队提供的交互式工作区，用于对事件进行分类和进行初步调查。
    - [ILM（索引生命周期管理）](https://www.elastic.co/guide/en/elasticsearch/reference/7.6/index-lifecycle-management.html) - 根据性能、弹性和保留要求自动管理索引。
    - [堆栈监控](https://www.elastic.co/guide/en/kibana/7.6/xpack-monitoring.html) - 提供跨Elastic Stack的监控数据可视化。
  - [Elasticsearch管理](https://www.elastic.co/guide/en/kibana/7.6/management.html) - 用于管理Elastic Stack对象的UI，包括ILM（索引生命周期管理）等。

## 数据进入和探索
将Manticore与[Logstash](../Integration/Logstash.md)、[Filebeat](../Integration/Filebeat.md)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/)或[Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/)等工具集成，以从网络日志等来源摄取数据。一旦数据加载到Manticore中，您可以在Kibana中探索和可视化它。
