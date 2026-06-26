# Manticore 与 OpenSearch Dashboards 的集成

[OpenSearch Dashboards](https://opensearch.org/platform/opensearch-dashboards/) 是一个可视化界面，可用于浏览、可视化并为日志数据创建仪表板。它通过 HTTP API 连接到后端。Manticore 会在其 HTTP 监听端点上提供该 API，因此你可以将 OpenSearch Dashboards 指向 Manticore，并以类似于 [Kibana](../Integration/Kibana.md) 的方式使用 **Discover**、**Visualize** 和 **Dashboards**。借助此集成，你可以构建图表和仪表板，在 **Discover** 中执行即席搜索，并继续使用 [Logstash](../Integration/Logstash.md) 和 [Filebeat](../Integration/Filebeat.md) 等熟悉的采集工具来加载日志和事件数据进行分析。

## 前提条件
1. **下载 OpenSearch Dashboards**：请确保下载与 Manticore 兼容的 OpenSearch Dashboards 版本。目前，**3.4.0** 版本已测试并推荐使用。其他版本可能可用，但可能会引入问题。请在 Manticore 中将 [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string) 设置为与你的 OpenSearch Dashboards 版本一致（参见下方 [配置](../Integration/Opensearch_Dashboards.md#Configuration)）。
2. **验证 Manticore**：请确保你的 Manticore 实例正在运行，并且其 HTTP API 可访问（默认：`http://localhost:9308`）。
3. **Manticore Buddy**：请确保 [Manticore Buddy](../Installation/Manticore_Buddy.md) 已安装并正在运行。OpenSearch Dashboards 集成由 Buddy 的 **EmulateElastic** 插件实现，该插件模拟 Dashboards 期望的 Elasticsearch 兼容 HTTP API（与 [Kibana](../Integration/Kibana.md) 走的是同一代码路径）。除非你通过 [`buddy_path`](../Server_settings/Searchd.md#buddy_path) 禁用它，否则 Buddy 会随 `searchd` 自动启动。

## 配置
1. 打开 [OpenSearch Dashboards 配置文件](https://docs.opensearch.org/3.4/install-and-configure/configuring-dashboards/)（`opensearch_dashboards.yml`；常见路径包括 tar 包布局中的 `config/opensearch_dashboards.yml`，或某些软件包中的 `/etc/opensearch-dashboards/opensearch_dashboards.yml`）。
2. 设置你的 Manticore 实例 URL：
   ```yaml
   opensearch.hosts: ["http://localhost:9308"]
   ```
   在 Docker 中运行 OpenSearch Dashboards 时，你也可以通过 `OPENSEARCH_HOSTS` 环境变量设置相同的值。

   由于 Manticore 不提供 OpenSearch Security 插件，你还需要在 OpenSearch Dashboards 中禁用 security dashboards 插件：
   
   - **Docker**：在容器环境中设置 `DISABLE_SECURITY_DASHBOARDS_PLUGIN=true`。
   - **Tar 包安装**：先停止 OpenSearch Dashboards，然后运行：`./bin/opensearch-dashboards-plugin remove securityDashboards`。之后重新启动 OpenSearch Dashboards。
   
   有关详细信息，请参阅[禁用和启用 Security 插件](https://docs.opensearch.org/3.4/security/configuration/disable-enable-security/)。
3. 启动 OpenSearch Dashboards，并在浏览器中打开 `http://localhost:5601`。如有需要，请将 `localhost` 替换为你的服务器 IP 或主机名。

> 注意：要与 OpenSearch Dashboards 集成，Manticore 必须工作在[实时模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)下。

### Manticore 配置示例
将 `kibana_version_string` 设置为与你安装的 OpenSearch Dashboards 相同的版本。OpenSearch Dashboards 会检查 Manticore 返回的后端版本，如果版本不匹配，可能会显示警告或无法启动。

```ini
searchd {
    listen = 127.0.0.1:9308:http
    pid_file = /var/run/manticore/searchd.pid
    data_dir = /var/lib/manticore
    kibana_version_string = 3.4.0
}
```

## 支持的功能

### Discover
- 使用 **Discover** 选项卡可在 Manticore 表中交互式搜索和筛选文档。

### 可视化
- 转到 **Visualizations** 以创建自定义可视化：
  - 创建一个与 Manticore 表名匹配的 **索引模式**。
  - 选择一种可视化类型（例如柱状图、折线图或饼图），其底层依赖于受支持的聚合：`terms`、`histogram`、`date_histogram`、`range`、`date_range`，以及指标聚合 `max`、`min`、`sum`、`avg`。
  - 配置你的可视化，执行它，并浏览你的数据。
  - 保存可视化以便日后使用。

### 仪表板
- 进入 **Dashboards** 以创建或查看交互式仪表板：
  - 添加已保存的可视化、筛选器或控件，获得个性化体验。
  - 直接在仪表板中与你的数据交互。
  - 保存仪表板以便日后使用。

### 管理
- 使用 **Management > Dashboards Management** 管理索引模式和已保存对象（可视化、仪表板）。
- Dashboards 启动所需的堆栈级请求会被模拟（node 版本、集群设置、配置对象、索引列表）。针对 Manticore 无法进行完整的 OpenSearch 集群管理。

## 限制

- 目前，OpenSearch Dashboards **3.4.0** 版本已测试并推荐使用。其他版本可能可用，但可能会引发问题。你必须在 Manticore 中将 [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string) 设置为与你使用的 OpenSearch Dashboards 版本一致。
- 以下 [OpenSearch 字段类型](https://docs.opensearch.org/3.4/mappings/supported-field-types/index/) 不受支持：
  - [地理字段类型](https://docs.opensearch.org/3.4/mappings/supported-field-types/geographic/) (`geo_point`, `geo_shape`)
  - [笛卡尔字段类型](https://docs.opensearch.org/3.4/mappings/supported-field-types/xy/) (`xy_point`, `xy_shape`)
  - [范围字段类型](https://docs.opensearch.org/3.4/mappings/supported-field-types/range/) (`integer_range`, `long_range`, `double_range`, `float_range`, `ip_range`, `date_range`)
  - [专用搜索字段类型](https://docs.opensearch.org/3.4/mappings/supported-field-types/specialized-search/) (`semantic`, `rank_feature`, `rank_features`, `percolator`, `star_tree`, `derived`)
  - [向量字段类型](https://docs.opensearch.org/3.4/mappings/supported-field-types/knn-vector/) (`knn_vector`, `sparse_vector`)
  - [自动完成字段类型](https://docs.opensearch.org/3.4/mappings/supported-field-types/autocomplete/) 和 [高级字符串字段类型](https://docs.opensearch.org/3.4/mappings/supported-field-types/string/)（[`completion`](https://docs.opensearch.org/3.4/mappings/supported-field-types/completion/)、[`search_as_you_type`](https://docs.opensearch.org/3.4/mappings/supported-field-types/search-as-you-type/)、[`match_only_text`](https://docs.opensearch.org/3.4/mappings/supported-field-types/match-only-text/)，以及类似类型；普通 `text` 和 `keyword` 受支持）
  - [关系字段类型](https://docs.opensearch.org/3.4/mappings/supported-field-types/index/#object-field-types) ([`nested`](https://docs.opensearch.org/3.4/mappings/supported-field-types/nested/)、[`join`](https://docs.opensearch.org/3.4/mappings/supported-field-types/join/))
- 指标聚合函数仅限于 [Manticore 支持的函数](../Searching/Grouping.md#Aggregation-functions)。
- 不支持嵌套聚合（`aggs` 里面再嵌套 `aggs`）。
- 高级 [Dashboards Query Language (DQL)](https://docs.opensearch.org/3.4/dashboards/dql/) 功能（嵌套字段搜索、正则表达式、模糊搜索、邻近搜索、词项提升等）在 Manticore 上可能无法正常工作。
- 以下 OpenSearch Dashboards 工具和插件不受支持：
  - [Geospatial](https://docs.opensearch.org/3.4/install-and-configure/plugins/)（依赖地理字段类型的地图可视化）
  - [Observability](https://docs.opensearch.org/3.4/observing-your-data/) – 用于监控基础设施指标、浏览日志和追踪分析的应用
  - [Alerting](https://docs.opensearch.org/3.4/observing-your-data/alerting/index/) – 面向 OpenSearch API 的监视器、触发器和通知
  - [Anomaly Detection](https://docs.opensearch.org/3.4/observing-your-data/ad/index/) – 用于检测时序数据异常值的探测器
  - [Security Analytics](https://docs.opensearch.org/3.4/security-analytics/) – 用于威胁检测和调查的 SIEM 工作流
  - 监控：
    - [Index State Management](https://docs.opensearch.org/3.4/im-plugin/ism/index/) – 自动化索引生命周期策略
    - [Performance Analyzer](https://docs.opensearch.org/3.4/monitoring-your-cluster/pa/index/) – 集群性能指标和根因分析
  - [Index Management](https://docs.opensearch.org/3.4/dashboards/im-dashboards/index/) – 用于管理 OpenSearch 索引、模板、别名和生命周期设置的界面
  - [Security plugin](https://docs.opensearch.org/3.4/api-reference/security/index/) 流程 - 在 Manticore 上不可用

## 数据采集与探索
将 Manticore 与 [Logstash](../Integration/Logstash.md)、[Filebeat](../Integration/Filebeat.md)、[Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) 或 [Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/) 等工具集成，即可从 Web 日志等来源采集数据。数据加载到 Manticore 之后，你就可以在 OpenSearch Dashboards 中对其进行探索和可视化。
