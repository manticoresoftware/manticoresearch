# Integration with Apache Superset

> NOTE: The integration with Apache Superset requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

[Apache Superset](https://superset.apache.org/) is a modern data exploration and visualization platform. Manticore Search can be integrated with Apache Superset, allowing users to harness the full potential of Manticore Search for data analysis and visualization.

**Note: This integration works only with Apache Superset 3.x branch and is not yet available for version 4 or newer.**

## Prerequisites

Before integrating Manticore Search with Apache Superset, ensure that:

1. Manticore Search (version 6.2.0 or later) is properly installed and configured on your server. Refer to the [official Manticore Search installation guide](https://manual.manticoresearch.com/Installation/Installation) for assistance.
2. Apache Superset is set up on your system. Follow the [official Apache Superset installation guide](https://superset.apache.org/docs/quickstart) for installation instructions.

## Connecting Manticore Search to Apache Superset

To connect Manticore Search to Apache Superset:

1. Log in to your Apache Superset dashboard.
2. Click on "+" in the top navigation bar and choose "Database Connections".
3. Click "+ DATABASE" to add a new database.
4. Select "MySQL" as the database type.
5. In the configuration page, provide the following details:
   - Database Name: A name for the database (e.g., "Manticore")
   - Host: Your Manticore Search server host
   - Port: The port on which Manticore Search is running
   - Username: Your Manticore Search username
   - Display Name: A display name for the connection
6. Click "Connect" to verify the connection and save.

## Creating Charts and Dashboards

After connecting Manticore Search to Apache Superset, you can create charts and dashboards using your Manticore data:

1. In the Apache Superset dashboard, click on "Dashboards" in the top navigation bar and select "New dashboard".
2. Open the dashboard and click on the "+" icon to add a new chart.
3. Choose the dataset connected to Manticore Search.
4. Select the type of chart you want to create (e.g., bar chart, line chart, pie chart).
5. Use Apache Superset's query builder or write an SQL query to fetch data from your Manticore Search database.
6. Customize the chart's appearance, labels, and other settings as needed.
7. Click "UPDATE CHART" to update the chart.
8. Add a name for the chart and click "Save" to permanently save it in the Dashboard.

## Supported Functionality

When working with Manticore Search through Apache Superset, you can:

- View and query data from Manticore Search tables
- Create various types of visualizations based on your Manticore data
- Build and customize dashboards using these visualizations

## Data Type Handling

Manticore Search data types are mapped to MySQL data types when working with Apache Superset. Refer to the Manticore Search documentation for more details on supported data types and their mappings.

## Limitations

- Some advanced MySQL features may not be available when working with Manticore Search through Apache Superset.
- Certain data manipulation operations might be limited compared to working directly with MySQL databases.

By following these steps and guidelines, you can effectively integrate Manticore Search with Apache Superset for powerful data exploration and visualization capabilities.

## References
Please refer to the following resources for more information on Apache Superset and Manticore Search:
- [Apache Superset Integration](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/)
