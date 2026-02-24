# 与 Grafana 的集成

> 注意：与 Grafana 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确保已安装 Buddy。

[Grafana](https://grafana.com/) 是一个开源的数据可视化和监控平台，允许您创建交互式仪表盘和图表。Manticore Search 使用默认的 MySQL 连接器与 Grafana 无缝集成，使您能够实时可视化搜索数据、监控性能指标并分析趋势。

目前测试并支持Grafana版本10.0-12.4。

## 前提条件

在将 Manticore Search 集成到 Grafana 之前，请确保：

1. 您的服务器上已正确安装并配置了 Manticore Search（版本 6.2.0 或更高）。请参阅[官方 Manticore Search 安装指南](../Installation/Installation.md)获取帮助。
2. 您的系统上已搭建 Grafana。请参考[官方 Grafana 安装指南](https://grafana.com/docs/grafana/latest/setup-grafana/installation/)完成安装。

## 连接 Manticore Search 到 Grafana

连接 Manticore Search 到 Grafana 的步骤：

1. 登录 Grafana 仪表盘，在左侧边栏点击“配置”（齿轮图标）。
2. 选择“数据源”，然后点击“添加数据源”。
3. 从可用数据源列表中选择“MySQL”。
4. 在设置页面填写以下信息：
   - **名称**：数据源名称（例如，“Manticore Search”）
   - **主机**：Manticore Search 服务器的主机名或 IP 地址（及 MySQL 端口，默认：`localhost:9306`）
   - **数据库**：留空或指定数据库名
   - **用户**：有权限访问 Manticore Search 的用户名（默认：`root`）
   - **密码**：指定用户的密码（默认为空）
5. 点击“保存并测试”以验证连接。

## 创建可视化和仪表盘

连接 Manticore Search 到 Grafana 后，您可以创建仪表盘和可视化：

1. 在 Grafana 仪表盘左侧边栏点击“+”图标，选择“新建仪表盘”。
2. 点击“添加可视化”按钮，开始配置您的图表。
3. 选择通过 MySQL 连接器连接的 Manticore Search 数据源。
4. 选择您要创建的图表类型（例如，时间序列、条形图、蜡烛图、饼图）。
5. 使用 Grafana 的查询生成器或编写 SQL 查询，从 Manticore Search 表中获取数据。
6. 根据需要自定义图表的外观、标签和其他设置。
7. 点击“应用”，将您的可视化保存至仪表盘。

## 示例用例

以下是一个使用时间序列数据的简单示例。首先，创建表并加载示例数据：

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

加载数据：
```bash
curl -sSL https://gist.githubusercontent.com/donhardman/df109ba6c5e690f73198b95f3768e73f/raw/0fab3aee69d7007fad012f4e97f38901a64831fb/btc_usd_trading.sql | mysql -h0 -P9306
```

在 Grafana 中，您可以创建：
- **时间序列图表**：可视化价格随时间的变化
- **蜡烛图**：显示金融数据的开盘价、最高价、最低价、收盘价
- **聚合图表**：使用 COUNT、AVG、MAX、MIN 函数

示例查询：
```sql
-- Time series query
SELECT time, close FROM btc_usd_trading ORDER BY time;

-- Aggregation query
SELECT DATE(time) as date, AVG(close) as avg_price 
FROM btc_usd_trading 
GROUP BY date 
ORDER BY date;
```

## 支持的功能

通过 Grafana 使用 Manticore Search，您可以：

- 对 Manticore Search 表执行 SQL 查询
- 使用聚合函数：COUNT、AVG、MAX、MIN
- 应用 GROUP BY 和 ORDER BY 操作
- 使用 WHERE 子句过滤数据
- 通过 `information_schema.tables` 访问表元数据
- 创建 Grafana 支持的各种可视化类型

## 限制

- 在通过 Grafana 使用 Manticore Search 时，某些高级 MySQL 功能可能不可用。
- 仅支持 Manticore Search 支持的功能。详细信息请参阅[SQL 参考](../Searching/Full_text_matching/Basic_usage.md)。

## 参考资料

更多信息和详细教程：
- [Grafana 集成博客文章](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)
- [官方 Grafana 文档](https://grafana.com/docs/)
