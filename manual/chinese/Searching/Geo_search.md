# 地理搜索

Manticore Search 的最大功能之一是能够将全文搜索与地理位置结合。例如，零售商可以提供一个搜索功能，用户查找产品，结果集可以显示最近有该产品库存的商店，这样用户可以直接去店里取货。旅游网站可以基于限定在某区域的搜索提供结果，并按距某点的距离排序（例如，“搜索酒店附近的博物馆”）。

要执行地理搜索，文档需要包含一对纬度/经度坐标。这些坐标可以作为浮点属性存储。如果文档有多个位置，使用 JSON 属性来存储坐标对可能更方便。



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

如果为纬度和经度属性生成了二级索引，则如果[基于成本的优化器](../Searching/Cost_based_optimizer.md)决定使用它们，则它们可能会自动用于加速地理搜索。

## 执行距离计算

要计算两点之间的距离，可以使用[GEODIST()](../Functions/Geo_spatial_functions.md#GEODIST%28%29)函数。`GEODIST` 需要用前四个参数传入两对坐标。

第五个参数采用简化的 JSON 格式来配置函数的某些方面。默认情况下，`GEODIST` 期望坐标为弧度，但可以添加 `in=degrees` 来允许使用度作为输入。进行地理距离计算的坐标类型（度或弧度）必须与表中存储的类型相同，否则结果将不准确。

计算出来的距离默认单位是米，但通过 `out` 选项可以转换为千米、英尺或英里。最后，默认使用一种名为 `adaptive` 的计算方法。也可以选择基于 `haversine` 算法的替代方法，但后者较慢且精度稍差。

函数的结果——距离——可以用于 `ORDER BY` 子句对结果排序：

```sql
SELECT *, GEODIST(40.7643929, -73.9997683, lat, lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') ORDER BY distance ASC, WEIGHT() DESC;
```

或者限制结果在某点周围某个半径内：

```sql
SELECT *,GEODIST(40.7643929, -73.9997683, lat,lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') AND distance <1000 ORDER BY WEIGHT(), DISTANCE ASC;
```

## 多边形内搜索

另一个地理搜索功能是判断某位置是否位于指定区域内。一个特殊函数构造一个多边形对象，再由另一个函数测试一组坐标是否包含在该多边形内。

有两个函数用于创建多边形：

*   [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) - 创建考虑地球曲率的多边形
*   [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) - 在平面空间创建简单多边形

当区域的边长短于 500 公里（对 3-4 边多边形）时，`POLY2D` 适合用于地理搜索；如果边数更多，则应考虑更低的数值。对于边长较长的区域，为保持结果准确，必须使用 `GEOPOLY2D`。`GEOPOLY2D` 期望坐标为以度为单位的纬度/经度对，使用弧度时结果将是平面空间（类似于 `POLY2D`）。

[CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) 函数接受一个多边形和一组坐标作为输入，若点在多边形内则输出 `1`，否则输出 `0`。

```sql
SELECT *,CONTAINS(GEOPOLY2D(40.76439, -73.9997, 42.21211, -73.999,  42.21211, -76.123, 40.76439, -76.123), 41.5445, -74.973) AS inside FROM myindex WHERE MATCH('...') AND inside=1;
```
<!-- proofread -->

