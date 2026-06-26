# 地理搜索

Manticore Search 最伟大的功能之一是能够结合全文搜索与地理定位。例如，零售商可以提供一个搜索功能，用户可以查找产品，结果集可以指示最近的商店是否有库存，这样用户可以去店内取货。旅游网站可以根据特定区域进行搜索，并按距离某个点的距离对结果进行排序（例如，“搜索酒店附近的博物馆”）。

要执行地理搜索，文档需要包含经纬度坐标对。这些坐标可以存储为浮点数属性。如果文档中有多个位置，使用 JSON 属性存储坐标对可能会更方便。



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

如果为纬度和经度属性生成了二级索引，那么在 [成本基优化器](../Searching/Cost_based_optimizer.md) 决定使用它们的情况下，它们可能会自动用于加快地理搜索速度。

## 执行距离计算

要找出两个点之间的距离，可以使用 [GEODIST()](../Functions/Geo_spatial_functions.md#GEODIST%28%29) 函数。`GEODIST` 需要作为前四个参数的两个坐标对。

`GEODIST` 的第五个参数可以以简化 JSON 格式配置某些方面。默认情况下，`GEODIST` 假定坐标为弧度，但可以添加 `in=degrees` 以允许使用度作为输入。我们执行地理距离的坐标类型必须与表中存储的坐标类型相同；否则，结果会误导人。

计算出的距离默认以米为单位，但可以通过 `out` 选项将其转换为千米、英尺或英里。最后，默认使用一种称为 `adaptive` 的计算方法。基于 `haversine` 算法的另一种方法可用，但这种方法较慢且不够精确。

函数的结果——距离——可以用于 `ORDER BY` 子句以对结果进行排序：

```sql
SELECT *, GEODIST(40.7643929, -73.9997683, lat, lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') ORDER BY distance ASC, WEIGHT() DESC;
```

或者限制结果为某个点周围的圆形区域：

```sql
SELECT *,GEODIST(40.7643929, -73.9997683, lat,lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') AND distance <1000 ORDER BY WEIGHT(), DISTANCE ASC;
```

## 在多边形中搜索

另一个地理搜索功能是确定某个位置是否在指定区域内。有一个特殊函数构建一个多边形对象，然后使用另一个函数测试一组坐标是否包含在该多边形内。

有两种函数可用于创建多边形：

*   [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) - 考虑地球曲率创建多边形
*   [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) - 在平面空间中创建简单多边形

`POLY2D` 适用于多边形的边长小于 500km（对于 3-4 边的多边形；对于更多边的多边形，应考虑较低的值）的地理搜索。对于边长较长的区域，必须使用 `GEOPOLY2D` 以保持结果的准确性。`GEOPOLY2D` 预期坐标为度数的纬度/经度对；使用弧度将导致平面空间中的结果（类似于 `POLY2D`）。

[CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) 接受一个多边形和一组坐标作为输入，并输出 `1` 如果点在多边形内，否则输出 `0`。

```sql
SELECT *,CONTAINS(GEOPOLY2D(40.76439, -73.9997, 42.21211, -73.999,  42.21211, -76.123, 40.76439, -76.123), 41.5445, -74.973) AS inside FROM myindex WHERE MATCH('...') AND inside=1;
```
<!-- proofread -->

