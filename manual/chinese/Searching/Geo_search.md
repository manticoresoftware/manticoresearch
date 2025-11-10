# 地理搜索

Manticore Search 最强大的功能之一是能够将全文搜索与地理位置结合起来。例如，零售商可以提供一种搜索，用户查找某个产品，结果集可以显示最近的有该产品库存的商店，这样用户就可以去店里取货。旅游网站可以基于限定区域的搜索提供结果，并按距离某点的远近排序（例如，“搜索酒店附近的博物馆”）。

要执行地理搜索，文档需要包含一对纬度/经度坐标。坐标可以存储为浮点属性。如果文档有多个位置，使用 JSON 属性存储坐标对可能更方便。



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

如果为纬度和经度属性生成了二级索引，且[基于成本的优化器](../Searching/Cost_based_optimizer.md)决定使用它们，则它们可能会自动用于加速地理搜索。

## 执行距离计算

要计算两点之间的距离，可以使用[GEODIST()](../Functions/Geo_spatial_functions.md#GEODIST%28%29)函数。`GEODIST` 需要两个坐标对作为前四个参数。

第五个参数是简化的 JSON 格式，可以配置函数的某些方面。默认情况下，`GEODIST` 期望坐标为弧度，但可以添加 `in=degrees` 以允许使用度作为输入。用于计算地理距离的坐标必须与表中存储的坐标类型（度或弧度）相同，否则结果会产生误导。

计算出的距离默认单位为米，但通过 `out` 选项，可以转换为千米、英尺或英里。最后，默认使用一种称为 `adaptive` 的计算方法。还有一种基于 `haversine` 算法的替代方法，但该方法较慢且精度较低。

函数的结果——距离——可以用于 `ORDER BY` 子句对结果进行排序：

```sql
SELECT *, GEODIST(40.7643929, -73.9997683, lat, lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') ORDER BY distance ASC, WEIGHT() DESC;
```

或者限制结果在某点的半径范围内：

```sql
SELECT *,GEODIST(40.7643929, -73.9997683, lat,lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') AND distance <1000 ORDER BY WEIGHT(), DISTANCE ASC;
```

## 多边形内搜索

另一个地理搜索功能是判断某个位置是否在指定区域内。一个特殊函数构造多边形对象，另一个函数用来测试一组坐标是否包含在该多边形内。

有两个函数可用于创建多边形：

*   [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) - 创建考虑地球曲率的多边形
*   [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) - 创建平面空间中的简单多边形

`POLY2D` 适用于边长小于 500 公里的区域（对于 3-4 边的多边形；边数更多时应考虑更低的值）。对于边长较长的区域，必须使用 `GEOPOLY2D` 以保持结果准确。`GEOPOLY2D` 期望坐标为度的纬度/经度对；使用弧度将得到平面空间的结果（类似于 `POLY2D`）。

[CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) 接受一个多边形和一组坐标作为输入，如果点在多边形内则输出 `1`，否则输出 `0`。

```sql
SELECT *,CONTAINS(GEOPOLY2D(40.76439, -73.9997, 42.21211, -73.999,  42.21211, -76.123, 40.76439, -76.123), 41.5445, -74.973) AS inside FROM myindex WHERE MATCH('...') AND inside=1;
```
<!-- proofread -->

