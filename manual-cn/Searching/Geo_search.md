# 地理（Geo）搜索

Manticore Search 的一个强大功能是能够将全文搜索与地理位置结合起来。例如，零售商可以提供一个搜索功能，用户可以查找产品，结果集可以显示最近有该产品库存的商店，用户可以到店取货。旅游网站可以基于搜索限制在某个区域内的结果，并按照距离某个点的远近排序（例如，“搜索酒店附近的博物馆”）。

要进行地理搜索，文档需要包含一对经纬度坐标。这些坐标可以作为浮点属性存储。如果文档有多个位置，使用 JSON 属性来存储坐标对可能更为方便。

```ini
table myrt
{
    ...
    rt_attr_float = lat
    rt_attr_float = lon
    ...
}
```

坐标可以以度或弧度存储。

如果为经纬度属性生成了辅助索引，[基于成本的优化器](../Searching/Cost_based_optimizer.md) 可能会自动使用它们来加速地理搜索。

## 执行距离计算

要计算两个点之间的距离，可以使用 [GEODIST()](../Functions/Geo_spatial_functions.md#GEODIST()) 函数。`GEODIST` 需要两对坐标作为前四个参数。

在简化的 JSON 格式中，第五个参数可以配置函数的某些方面。默认情况下，`GEODIST` 期望以弧度为单位的坐标，但可以通过添加 `in=degrees` 来允许以度为单位输入。如果要进行地理距离计算的坐标与表中存储的坐标类型不同（度或弧度），结果将会误导。

默认情况下，计算的距离以米为单位，但可以通过 `out` 选项将其转换为公里、英尺或英里。最后，默认情况下使用一种名为 `adaptive` 的计算方法。也可以选择基于 `haversine` 算法的替代方法，尽管这种方法较慢且不太精确。

函数的结果——距离——可以在 `ORDER BY` 子句中使用以按距离排序结果：

```sql
SELECT *, GEODIST(40.7643929, -73.9997683, lat, lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') ORDER BY distance ASC, WEIGHT() DESC;
```

或者可以限制结果在一个点的径向区域内：

```sql
SELECT *,GEODIST(40.7643929, -73.9997683, lat,lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') AND distance <1000 ORDER BY WEIGHT(), DISTANCE ASC;
```

## 在多边形中搜索

另一个地理搜索功能是确定一个位置是否位于指定区域内。一个特殊函数用于构造多边形对象，随后另一个函数用来测试一组坐标是否包含在该多边形内。

有两个函数可用于创建多边形：

- [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D()) - 创建一个考虑地球曲率的多边形
- [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D()) - 创建一个平面空间中的简单多边形

`POLY2D` 适用于边长小于500公里的区域的地理搜索（对于拥有3-4条边的多边形；对于多边形边数较多的情况，应该考虑更小的值）。对于边长较长的区域，需要使用 `GEOPOLY2D` 以确保准确的结果。`GEOPOLY2D` 期望使用以度为单位的纬度/经度对；使用弧度会导致平面空间中的结果（类似于 `POLY2D`）。

[CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS()) 接收一个多边形和一组坐标作为输入，如果点在多边形内则输出 `1`，否则输出 `0`。

```sql
SELECT *,CONTAINS(GEOPOLY2D(40.76439, -73.9997, 42.21211, -73.999,  42.21211, -76.123, 40.76439, -76.123), 41.5445, -74.973) AS inside FROM myindex WHERE MATCH('...') AND inside=1;
```
<!-- proofread -->