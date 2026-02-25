# Integration with Grafana

> NOTE: The integration with Grafana requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

[Grafana](https://grafana.com/) is an open-source data visualization and monitoring platform that allows you to create interactive dashboards and charts. Manticore Search integrates seamlessly with Grafana using the default MySQL connector, enabling you to visualize search data, monitor performance metrics, and analyze trends in real-time.

Currently, Grafana versions 10.0-12.4 are tested and supported.

## Prerequisites

Before integrating Manticore Search with Grafana, ensure that:

1. Manticore Search (version 6.2.0 or later) is properly installed and configured on your server. Refer to the [official Manticore Search installation guide](../Installation/Installation.md) for assistance.
2. Grafana is set up on your system. Follow the [official Grafana installation guide](https://grafana.com/docs/grafana/latest/setup-grafana/installation/) for installation instructions.

## Connecting Manticore Search to Grafana

To connect Manticore Search to Grafana:

1. Log in to your Grafana dashboard and click on "Configuration" (the gear icon) in the left sidebar.
2. Select "Data Sources" and click on "Add data source."
3. Choose "MySQL" from the list of available data sources.
4. In the settings page, provide the following details:
   - **Name**: A name for the data source (e.g., "Manticore Search")
   - **Host**: The hostname or IP address of your Manticore Search server (with MySQL port, default: `localhost:9306`)
   - **Database**: Leave empty or specify your database name
   - **User**: The username with access to Manticore Search (default: `root`)
   - **Password**: The password for the specified user (default: empty)
5. Click on "Save & Test" to verify the connection.

## Creating Visualizations and Dashboards

After connecting Manticore Search to Grafana, you can create dashboards and visualizations:

1. In the Grafana dashboard, click on the "+" icon in the left sidebar and select "New dashboard."
2. Click on the "Add visualization" button to start configuring your chart.
3. Choose the Manticore Search data source that you connected via the MySQL connector.
4. Select the type of chart you want to create (e.g., Time Series, Bar chart, Candlestick, Pie chart).
5. Use Grafana's query builder or write an SQL query to fetch data from your Manticore Search tables.
6. Customize the chart's appearance, labels, and other settings as needed.
7. Click "Apply" to save your visualization to the dashboard.

## Example Use Case

Here's a simple example using time-series data. First, create a table and load sample data:

```sql
CREATE TABLE btc_usd_trading (
  id bigint,
  time timestamp,
  open float,
  high float,
  low float,
  close float
);
```

Load the data:
```bash
curl -sSL https://gist.githubusercontent.com/donhardman/df109ba6c5e690f73198b95f3768e73f/raw/0fab3aee69d7007fad012f4e97f38901a64831fb/btc_usd_trading.sql | mysql -h0 -P9306
```

In Grafana, you can create:
- **Time Series chart**: Visualize price changes over time
- **Candlestick chart**: Display open, high, low, close values for financial data
- **Aggregation charts**: Use COUNT, AVG, MAX, MIN functions

Example queries:
```sql
-- Time series query
SELECT time, close FROM btc_usd_trading ORDER BY time;

-- Aggregation query
SELECT DATE(time) as date, AVG(close) as avg_price 
FROM btc_usd_trading 
GROUP BY date 
ORDER BY date;
```

## Supported Features

When working with Manticore Search through Grafana, you can:

- Execute SQL queries against Manticore Search tables
- Use aggregation functions: COUNT, AVG, MAX, MIN
- Apply GROUP BY and ORDER BY operations
- Filter data with WHERE clauses
- Access table metadata via `information_schema.tables`
- Create various visualization types supported by Grafana

## Limitations

- Some advanced MySQL features may not be available when working with Manticore Search through Grafana.
- Only features supported by Manticore Search are available. Refer to the [SQL reference](../Searching/Full_text_matching/Basic_usage.md) for details.

## References

For more information and detailed tutorials:
- [Grafana integration blog post](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)
- [Official Grafana documentation](https://grafana.com/docs/)
