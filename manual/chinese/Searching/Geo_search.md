# 地理搜索

Manticore Search 最伟大的特点之一是将全文搜索与地理位置结合的能力。例如，零售商可以提供一种搜索方式，让用户寻找产品，而结果集可以指示出库存中该产品最近的商店，以便用户可以到店内提货。一个旅游网站可以提供基于搜索限制在特定区域的结果，并按距离某个点的远近进行排序（例如，“搜索酒店附近的博物馆”）。

要执行地理搜索，文档需要包含纬度/经度坐标对。坐标可以作为浮点属性存储。如果文档有多个位置，可以方便地使用 JSON 属性来存储坐标对。


```ini
table myrt
{
    ...
    rt_attr_float = lat
    rt_attr_float = lon
    ...
}
```

坐标可以存储为度或弧度。

如果为纬度和经度属性生成了二级索引，那么如果 [基于成本的优化器](../Searching/Cost_based_optimizer.md) 决定使用它们时，它们可能会自动用于加快地理搜索的速度。

## 执行距离计算

要找出两点之间的距离，可以使用 [GEODIST()](../Functions/Geo_spatial_functions.md#GEODIST%28%29) 函数。`GEODIST` 需要作为前四个参数的两个坐标对。

第五个参数可以使用简化的 JSON 格式来配置函数的某些方面。默认情况下，`GEODIST` 期望坐标以弧度表示，但可以添加 `in=degrees` 来允许使用度作为输入。我们执行地理距离的坐标必须与存储在表中的坐标类型（度或弧度）相同；否则，结果将具有误导性。

默认情况下，计算得出的距离以米为单位，但通过 `out` 选项，可以转换为公里、英尺或英里。最后，默认使用一种称为 `adaptive` 的计算方法。基于 `haversine` 算法的替代方法可用；然而，这种方法较慢且精确度较低。

函数的结果 - 距离 - 可以在 `ORDER BY` 子句中用于对结果进行排序：

```sql
SELECT *, GEODIST(40.7643929, -73.9997683, lat, lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') ORDER BY distance ASC, WEIGHT() DESC;
```

或者限制结果在某个点周围的半径区域内：

```sql
SELECT *,GEODIST(40.7643929, -73.9997683, lat,lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') AND distance <1000 ORDER BY WEIGHT(), DISTANCE ASC;
```

## 在多边形中搜索

另一个地理搜索功能是能够确定某个位置是否在指定区域内。一个特殊的函数构建一个多边形对象，然后被另一个函数用来测试一组坐标是否包含在该多边形内。

有两个函数可用于创建多边形：

*   [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) - 创建一个考虑地球曲率的多边形
*   [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) - 在平面空间中创建一个简单的多边形

当区域的边长小于 500 公里（对于 3-4 边的多边形；对于边数更多的多边形，应该考虑更小的值）时，`POLY2D` 适合用于地理搜索。对于边长较长的区域，使用 `GEOPOLY2D` 是必要的，以保持结果的准确性。`GEOPOLY2D` 期望坐标作为度的纬度/经度对；使用弧度将导致平面空间中的结果（类似于 `POLY2D`）。

[CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) 函数将多边形和一组坐标作为输入，如果点在多边形内部，则输出 `1`，否则输出 `0`。

```sql
SELECT *,CONTAINS(GEOPOLY2D(40.76439, -73.9997, 42.21211, -73.999,  42.21211, -76.123, 40.76439, -76.123), 41.5445, -74.973) AS inside FROM myindex WHERE MATCH('...') AND inside=1;
```
<!-- proofread -->

