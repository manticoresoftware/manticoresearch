# 地理空间函数

### GEODIST()
`GEODIST(lat1, lon1, lat2, lon2, [...])` 函数计算由其坐标指定的两点之间的地球表面距离。请注意，默认情况下，纬度和经度必须都是**弧度**，结果将以**米**为单位。你可以对任意四个坐标使用任意表达式。当一对参数直接引用一对属性，而另一对是常数时，将选择优化路径。

`GEODIST()` 还接受一个可选的第5个参数，允许你轻松在输入和输出单位之间转换，并选择使用特定的地理距离公式。完整语法及部分示例如下：

```sql
GEODIST(lat1, lon1, lat2, lon2, { option=value, ... })

GEODIST(40.7643929, -73.9997683, 40.7642578, -73.9994565, {in=degrees, out=feet})

GEODIST(51.50, -0.12, 29.98, 31.13, {in=deg, out=mi})
```

已知的选项及其取值为：

* `in = {deg | degrees | rad | radians}`，指定输入单位；
* `out = {m | meters | km | kilometers | ft | feet | mi | miles | yd | yards | in | inch | cm | centimeters | mm | millimeters | NM | nmi | nauticalmiles}`，指定输出单位；
* `method = {adaptive | haversine}`，指定地理距离计算方法。

默认方法是“adaptive”。这是一种经过良好优化的实现，始终比“haversine”更精确*且*更快。

### GEOPOLY2D()
`GEOPOLY2D(lat1,lon1,lat2,lon2,lat3,lon3...)` 创建一个多边形，用于 [CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) 函数。此函数通过将多边形分割成更小的多边形，考虑了地球的曲率，适合用于较大区域。对于小区域，可以使用 [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) 函数。该函数期望坐标为纬度/经度度数对；若使用弧度，则结果与 `POLY2D()` 相同。

### POLY2D()
`POLY2D(x1,y1,x2,y2,x3,y3...)` 创建一个多边形，用于 [CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) 函数。该多边形假设地球是平的，因此不应过大；对于大面积，应使用考虑地球曲率的 [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数。

<!-- proofread -->

