# Integration of Manticore with OpenSearch Dashboards

[OpenSearch Dashboards](https://opensearch.org/platform/opensearch-dashboards/) is the open-source analytics and visualization UI for OpenSearch. It grew out of the same Kibana-era codebase (roughly Elasticsearch 7.10) and talks to the cluster over an Elasticsearch-compatible HTTP API. Manticore exposes that style of API on its HTTP listener, so you can point OpenSearch Dashboards at Manticore and use **Discover**, **Visualize**, and **Dashboards** in a similar way to [Kibana](Kibana.md).

Most day-to-day behavior, limitations on field types and aggregations, and ingestion paths mirror what is already documented for Kibana. Treat the [Kibana integration guide](Kibana.md) as the detailed reference for supported and unsupported features; this page focuses on what is specific to OpenSearch Dashboards.

## Prerequisites

1. **Install OpenSearch Dashboards** from the [OpenSearch downloads](https://opensearch.org/downloads.html) or your distribution’s packages. Pick a version whose default client expectations are close to the Elasticsearch 7.x line; Manticore’s compatibility work is described in terms of that family (the [Kibana](Kibana.md) page documents **Kibana 7.6.0** as tested). Newer OpenSearch Dashboards releases may require tuning (see [Version string](#version-string) below).
2. **Verify Manticore** is running and the HTTP API is reachable (by default `http://localhost:9308` if you follow the example below).

## Configuration

1. Edit OpenSearch Dashboards’ config file (name and path depend on install; common locations are `config/opensearch_dashboards.yml` in the tarball layout or `/etc/opensearch-dashboards/opensearch_dashboards.yml` on some packages). See the official guide: [Configuring OpenSearch Dashboards](https://docs.opensearch.org/latest/install-and-configure/configuring-dashboards/).
2. Set the backend URL to your Manticore HTTP endpoint, for example:

   ```yaml
   opensearch.hosts: ["http://localhost:9308"]
   ```

   Use the host and port where `searchd` listens for HTTP. If Manticore uses HTTPS, use `https://` and adjust `opensearch.ssl.*` settings as described in the OpenSearch documentation.
3. Start OpenSearch Dashboards and open the UI (default URL is typically `http://localhost:5601`). Replace `localhost` with your server’s hostname or IP if needed.

> **Notes**
>
> - Manticore does not require authentication for this integration unless you add it yourself; leave OpenSearch Dashboards auth settings unset unless your environment needs them.
> - Manticore must run in [real-time mode](../Read_this_first.md#Real-time-mode-vs-plain-mode) for this workflow.

### Manticore configuration example

```ini
searchd {
    listen = 127.0.0.1:9308:http
    pid_file = /var/run/manticore/searchd.pid
    data_dir = /var/lib/manticore
}
```

### Version string

For a given Dashboards version, check its compatibility notes below, then set [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string) in `searchd` to match that expected server version—not an arbitrary number. If it does not, you may see unsupported-version warnings or UI failures. The same option is used for Kibana and OpenSearch Dashboards.

## Using the UI

### Discover

Use **Discover** to search and filter documents. The query bar behavior depends on your OpenSearch Dashboards version. Unsupported query features will fail in the same way as with Kibana against Manticore.

### Visualizations and dashboards

Create **index patterns** (or equivalent data connections in newer releases) that match your Manticore tables, then build visualizations and assemble **Dashboards** as you would against OpenSearch or Elasticsearch. Unsupported aggregation types and field mappings are the same class of limits as in the [Kibana](../Integration/Kibana.md) guide.

### Management

Use the **Management** section of OpenSearch Dashboards for stack settings (index patterns, saved objects, advanced options). Exact menu labels vary by version; follow the upstream documentation for your install.

## Limitations

- Unsupported Elasticsearch field types, aggregation limits, and UI apps described for [Kibana](Kibana.md) apply in spirit to OpenSearch Dashboards: Manticore implements a subset of the HTTP API, not the full Elastic or OpenSearch Server surface.

- Currently, OpenSearch Dashboards **3.4.0** is tested and recommended. Other Dashboards versions may work but could cause issues. Untested Dashboards majors are not supported; if you try one anyway, you must align [`kibana_version_string`](../Server_settings/Searchd.md#kibana_version_string) with what that release expects. ([Version string](../Integration/Opensearch_Dashboards.md#version-string)).

- Features that require a real OpenSearch cluster (security plugin flows, alerting against OpenSearch APIs you have not mapped in Manticore, etc.) are outside what Manticore provides.

## Data ingestion and exploration

Ingest with [Logstash](../Integration/Logstash.md), [Filebeat](../Integration/Filebeat.md), [Fluent Bit](../Integration/Fluentbit.md), [Vector.dev](../Integration/Vectordev.md), or other Elasticsearch-compatible sinks, then explore the data in OpenSearch Dashboards.
