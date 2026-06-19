# Integration of Manticore with OpenSearch Dashboards

[OpenSearch Dashboards](https://opensearch.org/platform/opensearch-dashboards/) is a visual interface that allows you to explore, visualize, and create dashboards for your log data. It connects to the backend over an HTTP API. Manticore exposes that API on its HTTP listener, so you can point OpenSearch Dashboards at Manticore and use **Discover**, **Visualize**, and **Dashboards** in a similar way to [Kibana](../Integration/Kibana.md). With this integration, you can build charts and dashboards, run ad hoc searches in **Discover**, and keep working with familiar ingestion tools such as [Logstash](../Integration/Logstash.md) and [Filebeat](../Integration/Filebeat.md) to load log and event data for analysis.

## Prerequisites
1. **Download OpenSearch Dashboards**: Ensure you download an OpenSearch Dashboards version compatible with Manticore. Currently, version **3.4.0** is tested and recommended. Other versions may work but could introduce issues. Set [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string) in Manticore to match your OpenSearch Dashboards version (see [Configuration](../Integration/Opensearch_Dashboards.md#Configuration) below).
2. **Verify Manticore**: Ensure your Manticore instance is running and its HTTP API is reachable (default: `http://localhost:9308`).
3. **Manticore Buddy**: Ensure [Manticore Buddy](../Installation/Manticore_Buddy.md) is installed and running. OpenSearch Dashboards integration is implemented by Buddy's **EmulateElastic** plugin, which emulates the Elasticsearch-compatible HTTP API that Dashboards expects (the same code path as [Kibana](../Integration/Kibana.md)). Buddy starts automatically with `searchd` unless you disable it via [`buddy_path`](../Server_settings/Searchd.md#buddy_path).

## Configuration
1. Open the [OpenSearch Dashboards configuration file](https://docs.opensearch.org/3.4/install-and-configure/configuring-dashboards/) (`opensearch_dashboards.yml`; common paths include `config/opensearch_dashboards.yml` in the tarball layout or `/etc/opensearch-dashboards/opensearch_dashboards.yml` on some packages).
2. Set the URL of your Manticore instance:
   ```yaml
   opensearch.hosts: ["http://localhost:9308"]
   ```
   When running OpenSearch Dashboards in Docker, you can set the same value via the `OPENSEARCH_HOSTS` environment variable.

   Since Manticore does not provide the OpenSearch Security plugin, you must disable the security dashboards plugin in OpenSearch Dashboards as well:
   
   - **Docker**: set `DISABLE_SECURITY_DASHBOARDS_PLUGIN=true` in the container environment.
   - **Tarball install**: stop OpenSearch Dashboards, then run: `./bin/opensearch-dashboards-plugin remove securityDashboards`. After that, start OpenSearch Dashboards again.
   
   See [Disabling and enabling the Security plugin](https://docs.opensearch.org/3.4/security/configuration/disable-enable-security/) for details.
3. Start OpenSearch Dashboards and open it in your browser at `http://localhost:5601`. Replace `localhost` with your server's IP or hostname if necessary.

> Note: Manticore must work in [real-time mode](../Read_this_first.md#Real-time-mode-vs-plain-mode) to be integrated with OpenSearch Dashboards.

### Manticore configuration example
Set `kibana_version_string` to the same version as your OpenSearch Dashboards install. OpenSearch Dashboards checks the backend version reported by Manticore and may show warnings or fail to start if they do not match.

```ini
searchd {
    listen = 127.0.0.1:9308:http
    pid_file = /var/run/manticore/searchd.pid
    data_dir = /var/lib/manticore
    kibana_version_string = 3.4.0
}
```

## Supported Features

### Discover
- Use the **Discover** tab to search and filter documents in Manticore tables interactively.

### Visualizations
- Navigate to **Visualizations** to create custom visualizations:
  - Create an **index pattern** that matches a Manticore table name. 
  - Choose a visualization type (e.g., bar chart, line chart, or pie chart) backed by supported aggregations: `terms`, `histogram`, `date_histogram`, `range`, `date_range`, and metric aggregations `max`, `min`, `sum`, `avg`.
  - Configure your visualization, execute it, and explore your data.
  - Save visualizations for future use.

### Dashboards
- Access **Dashboards** to create or view interactive dashboards:
  - Add saved visualizations, filters, or controls for a personalized experience.
  - Interact with your data directly from the dashboard.
  - Save dashboards for future use.

### Management
- Use **Management > Dashboards Management** for index patterns and saved objects (visualizations, dashboards).
- Stack-level requests needed for Dashboards startup are emulated (node version, cluster settings, config objects, index listing). Full OpenSearch cluster administration is not available against Manticore.

## Limitations

- Currently, OpenSearch Dashboards version **3.4.0** is tested and recommended. Other versions may work but could cause issues. You must set [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string) in Manticore to match the OpenSearch Dashboards version you use.
- The following [OpenSearch field types](https://docs.opensearch.org/3.4/mappings/supported-field-types/index/) are not supported:
  - [Geographic field types](https://docs.opensearch.org/3.4/mappings/supported-field-types/geographic/) (`geo_point`, `geo_shape`)
  - [Cartesian field types](https://docs.opensearch.org/3.4/mappings/supported-field-types/xy/) (`xy_point`, `xy_shape`)
  - [Range field types](https://docs.opensearch.org/3.4/mappings/supported-field-types/range/) (`integer_range`, `long_range`, `double_range`, `float_range`, `ip_range`, `date_range`)
  - [Specialized search field types](https://docs.opensearch.org/3.4/mappings/supported-field-types/specialized-search/) (`semantic`, `rank_feature`, `rank_features`, `percolator`, `star_tree`, `derived`)
  - [Vector field types](https://docs.opensearch.org/3.4/mappings/supported-field-types/knn-vector/) (`knn_vector`, `sparse_vector`)
  - [Autocomplete field types](https://docs.opensearch.org/3.4/mappings/supported-field-types/autocomplete/) and [advanced string field types](https://docs.opensearch.org/3.4/mappings/supported-field-types/string/) ([`completion`](https://docs.opensearch.org/3.4/mappings/supported-field-types/completion/), [`search_as_you_type`](https://docs.opensearch.org/3.4/mappings/supported-field-types/search-as-you-type/), [`match_only_text`](https://docs.opensearch.org/3.4/mappings/supported-field-types/match-only-text/), and similar types; plain `text` and `keyword` are supported)
  - [Relational field types](https://docs.opensearch.org/3.4/mappings/supported-field-types/index/#object-field-types) ([`nested`](https://docs.opensearch.org/3.4/mappings/supported-field-types/nested/), [`join`](https://docs.opensearch.org/3.4/mappings/supported-field-types/join/))
- Metric aggregation functions are limited to [those supported by Manticore](../Searching/Grouping.md#Aggregation-functions).
- Nested aggregations (`aggs` inside `aggs`) are not supported.
- Advanced [Dashboards Query Language (DQL)](https://docs.opensearch.org/3.4/dashboards/dql/) features (nested field search, regular expressions, fuzzy search, proximity search, term boosting, and similar) may not work against Manticore.
- The following OpenSearch Dashboards tools and plugins are not supported:
  - [Geospatial](https://docs.opensearch.org/3.4/install-and-configure/plugins/) (map visualizations that depend on geographic field types)
  - [Observability](https://docs.opensearch.org/3.4/observing-your-data/) – Apps for monitoring infrastructure metrics, exploring logs, and trace analytics
  - [Alerting](https://docs.opensearch.org/3.4/observing-your-data/alerting/index/) – Monitors, triggers, and notifications against OpenSearch APIs
  - [Anomaly Detection](https://docs.opensearch.org/3.4/observing-your-data/ad/index/) – Detectors for outliers in time-series data
  - [Security Analytics](https://docs.opensearch.org/3.4/security-analytics/) – SIEM workflows for threat detection and investigation
  - Monitoring:
    - [Index State Management](https://docs.opensearch.org/3.4/im-plugin/ism/index/) – Automated index lifecycle policies
    - [Performance Analyzer](https://docs.opensearch.org/3.4/monitoring-your-cluster/pa/index/) – Cluster performance metrics and root cause analysis
  - [Index Management](https://docs.opensearch.org/3.4/dashboards/im-dashboards/index/) – A UI for managing OpenSearch indexes, templates, aliases, and lifecycle settings
  - [Security plugin](https://docs.opensearch.org/3.4/api-reference/security/index/) flows – not available on Manticore

## Data Ingestion and Exploration
Integrate Manticore with tools like [Logstash](../Integration/Logstash.md), [Filebeat](../Integration/Filebeat.md), [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/), or [Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/) to ingest data from sources like web logs. Once the data is loaded into Manticore, you can explore and visualize it in OpenSearch Dashboards.
