# Manticore 与 OpenSearch Dashboards 的集成

[OpenSearch Dashboards](https://opensearch.org/platform/opensearch-dashboards/) 是 OpenSearch 的开源分析和可视化用户界面。它源自同一时期的 Kibana 代码库（大致为 Elasticsearch 7.10），通过与 Elasticsearch 兼容的 HTTP API 与集群通信。Manticore 在其 HTTP 监听器上暴露了这种风格的 API，因此你可以将 OpenSearch Dashboards 指向 Manticore，并以类似于 [Kibana](Kibana.md) 的方式使用 **Discover**、**Visualize** 和 **Dashboards**。

大多数日常行为、字段类型和聚合的限制以及数据摄入路径与 Kibana 的现有文档中描述的内容一致。将 [Kibana 集成指南](Kibana.md) 视为支持和不支持功能的详细参考；本页面专注于 OpenSearch Dashboards 的特定内容。

## 先决条件

1. **安装 OpenSearch Dashboards**，从 [OpenSearch 下载页面](https://opensearch.org/downloads.html) 或你的发行版的软件包中安装。选择一个默认客户端期望值接近 Elasticsearch 7.x 系列的版本；Manticore 的兼容性工作以该系列进行描述（[Kibana](Kibana.md) 页面记录了测试的 **Kibana 7.6.0**）。较新的 OpenSearch Dashboards 发布版本可能需要调整（请参见下方的 [版本字符串](#version-string)）。
2. **验证 Manticore** 是否正在运行且 HTTP API 可访问（默认为 `http://localhost:9308`，如果你遵循下面的示例）。

## 配置

1. 编辑 OpenSearch Dashboards 的配置文件（名称和路径取决于安装方式；常见位置是 tarball 布局中的 `config/opensearch_dashboards.yml` 或某些软件包中的 `/etc/opensearch-dashboards/opensearch_dashboards.yml`）。请参阅官方指南：[配置 OpenSearch Dashboards](https://docs.opensearch.org/latest/install-and-configure/configuring-dashboards/)。
2. 将后端 URL 设置为你的 Manticore HTTP 端点，例如：

   ```yaml
   opensearch.hosts: ["http://localhost:9308"]
   ```

   使用 `searchd` 监听 HTTP 的主机和端口。如果 Manticore 使用 HTTPS，请使用 `https://` 并根据 OpenSearch 文档调整 `opensearch.ssl.*` 设置。
3. 启动 OpenSearch Dashboards 并打开 UI（默认 URL 通常是 `http://localhost:5601`）。如果需要，请将 `localhost` 替换为你的服务器的主机名或 IP。

> **注意事项**
>
> - 除非你自己添加，否则 Manticore 不需要为此集成进行身份验证；除非你的环境需要，否则请将 OpenSearch Dashboards 身份验证设置留空。
> - 此工作流程中，Manticore 必须以 [实时模式](../Read_this_first.md#Real-time-mode-vs-plain-mode) 运行。

### Manticore 配置示例

```ini
searchd {
    listen = 127.0.0.1:9308:http
    pid_file = /var/run/manticore/searchd.pid
    data_dir = /var/lib/manticore
}
```

### 版本字符串

对于某个 Dashboards 版本，请先查看其下方的兼容性说明，然后在 `searchd` 中设置 [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string) 以匹配该预期服务器版本——而不是任意数字。如果不匹配，你可能会看到不支持版本的警告或 UI 故障。Kibana 和 OpenSearch Dashboards 使用相同的选项。

## 使用 UI

### Discover

使用 **Discover** 搜索和过滤文档。查询栏的行为取决于你的 OpenSearch Dashboards 版本。不支持的查询功能将以与 Kibana 对 Manticore 的方式相同的方式失败。

### 可视化和仪表板

创建匹配你的 Manticore 表的 **索引模式**（或较新版本中的等效数据连接），然后像对 OpenSearch 或 Elasticsearch 一样构建可视化并组装 **仪表板**。不支持的聚合类型和字段映射与 [Kibana](../Integration/Kibana.md) 指南中的限制属于同一类。

### 管理

使用 OpenSearch Dashboards 的 **管理** 部分进行堆栈设置（索引模式、已保存对象、高级选项）。确切的菜单标签因版本而异；请遵循你安装的上游文档。

## 限制

- [Kibana](Kibana.md) 中描述的不支持的 Elasticsearch 字段类型、聚合限制和 UI 应用在精神上适用于 OpenSearch Dashboards：Manticore 实现了 HTTP API 的一部分，而不是完整的 Elastic 或 OpenSearch 服务器表面。

- 目前，OpenSearch Dashboards **3.4.0** 是经过测试并推荐的版本。其他 Dashboards 版本可能可以工作，但可能会导致问题。未经过测试的 Dashboards 主版本不被支持；如果你仍然尝试使用其中一个版本，你必须将 [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string) 与该版本所期望的值对齐。（[版本字符串](../Integration/Opensearch_Dashboards.md#version-string)）。

- 需要真实 OpenSearch 集群的功能（安全插件流程、针对你未在 Manticore 中映射的 OpenSearch API 的警报等）超出了 Manticore 提供的范围。

## 数据摄入和探索

使用 [Logstash](../Integration/Logstash.md)、[Filebeat](../Integration/Filebeat.md)、[Fluent Bit](../Integration/Fluentbit.md)、[Vector.dev](../Integration/Vectordev.md) 或其他与 Elasticsearch 兼容的接收器进行摄入，然后在 OpenSearch Dashboards 中探索数据。
