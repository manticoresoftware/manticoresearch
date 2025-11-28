# 与Grafana集成

> 注意：与Grafana的集成需要[Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法正常工作，请确保Buddy已安装。

[Grafana](https://grafana.com/)是一个开源的数据可视化和监控平台，它允许您创建交互式仪表板和图表。Manticore Search通过默认的MySQL连接器无缝集成到Grafana，使您能够实时可视化搜索数据、监控性能指标和分析趋势。

目前，支持并测试的Grafana版本为10.0-12.2。

## 先决条件

在将Manticore Search与Grafana集成之前，请确保：

1. Manticore Search（版本6.2.0或更高）已正确安装和配置在您的服务器上。请参考[Manticore Search官方安装指南](../Installation/Installation.md)以获得帮助。
2. Grafana已在您的系统上设置。按照[Grafana官方安装指南](https://grafana.com/docs/grafana/latest/setup-grafana/installation/)进行安装。

## 将Manticore Search连接到Grafana

要将Manticore Search连接到Grafana：

1. 登录您的Grafana仪表板，并点击左侧边栏的"Configuration"（齿轮图标）。
2. 选择"Data Sources"并点击"Add data source"。
3. 从可用数据源列表中选择"MySQL"。
4. 在设置页面，提供以下详细信息：
   - **Name**：数据源的名称（例如："Manticore Search"）
   - **Host**：Manticore Search服务器的主机名或IP地址（带MySQL端口，默认：`localhost:9306`）
   - **Database**：留空或指定您的数据库名称
   - **User**：具有Manticore Search访问权限的用户名（默认：`root`）
   - **Password**：指定用户的密码（默认：空）
5. 点击"Save & Test"以验证连接。

## 创建可视化和仪表板

将Manticore Search连接到Grafana后，您可以创建仪表板和可视化：

1. 在Grafana仪表板中，点击左侧边栏的"+"图标，选择"New dashboard"。
2. 点击"Add visualization"按钮开始配置图表。
3. 选择通过MySQL连接器连接的Manticore Search数据源。
4. 选择要创建的图表类型（例如，时间序列、柱状图、K线图、饼图）。
5. 使用Grafana的查询构建器或编写SQL查询从Manticore Search表中获取数据。
6. 根据需要自定义图表的外观、标签和其他设置。
7. 点击"Apply"将可视化保存到仪表板。

## 示例用例

以下是使用时间序列数据的简单示例。首先，创建一个表并加载示例数据：

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

在Grafana中，您可以创建：
- **时间序列图**：可视化价格变化趋势
- **K线图**：显示金融数据的开盘、最高、最低、收盘价
- **聚合图表**：使用COUNT、AVG、MAX、MIN函数

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

通过Grafana使用Manticore Search时，您可以：

- 对Manticore Search表执行SQL查询
- 使用聚合函数：COUNT、AVG、MAX、MIN
- 应用GROUP BY和ORDER BY操作
- 使用WHERE子句过滤数据
- 通过`information_schema.tables`访问表元数据
- 创建Grafana支持的各种可视化类型

## 限制

- 在通过Grafana使用Manticore Search时，一些高级MySQL功能可能不可用。
- 只能使用Manticore Search支持的功能。请参考[SQL参考](../Searching/Full_text_matching/Basic_usage.md)了解详情。

## 参考资料

欲了解更多信息和详细教程：
- [Grafana集成博客文章](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)
- [Grafana官方文档](https://grafana.com/docs/)
