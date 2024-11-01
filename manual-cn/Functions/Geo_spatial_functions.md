# 地理空间函数

### GEODIST()
`GEODIST(lat1, lon1, lat2, lon2, [...])` 函数计算由坐标指定的两个点之间的地球距离。请注意，默认情况下，纬度和经度必须以**弧度**表示，结果将以**米**为单位。您可以对四个坐标中的任意一个使用任意表达式。当一对参数直接引用一对属性，而另一对参数是常量时，将选择优化路径。

`GEODIST()` 还接受可选的第五个参数，允许您轻松在输入和输出单位之间进行转换，并选择要使用的具体地理距离公式。完整的语法及几个示例如下：

```sql
GEODIST(lat1, lon1, lat2, lon2, { option=value, ... })

GEODIST(40.7643929, -73.9997683, 40.7642578, -73.9994565, {in=degrees, out=feet})

GEODIST(51.50, -0.12, 29.98, 31.13, {in=deg, out=mi})
```

已知的选项及其值为：

- `in = {deg | degrees | rad | radians}`，指定输入单位；
- `out = {m | meters | km | kilometers | ft | feet | mi | miles}`，指定输出单位；
- `method = {adaptive | haversine}`，指定地理距离计算方法。

默认方法是 "adaptive"。这是一个优化良好的实现，在任何时候都比 "haversine" 更精确且更快。

### GEOPOLY2D()
`GEOPOLY2D(lat1,lon1,lat2,lon2,lat3,lon3...)` 创建一个多边形，配合 [CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS()) 函数使用。该函数通过将多边形细分为更小的多边形来考虑地球的曲率，适用于较大区域的操作。对于较小的区域，可以使用 [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D())函数。该函数要求坐标以度为单位的纬度/经度对输入；如果使用弧度，将得到与 `POLY2D()` 相同的结果。

### POLY2D()
`POLY2D(x1,y1,x2,y2,x3,y3...)` 创建一个多边形，配合 [CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS()) 函数使用。此多边形假设地球是平的，因此不应用于太大的区域；对于较大的区域，应该使用 [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D())，该函数考虑到了地球的曲率。

<!-- proofread -->
