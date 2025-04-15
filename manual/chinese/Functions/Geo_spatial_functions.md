# 地理空间函数

### GEODIST()
`GEODIST(lat1, lon1, lat2, lon2, [...])` 函数计算由其坐标指定的两点之间的地球表面距离。请注意，默认情况下，纬度和经度必须以 **弧度** 为单位，结果将以 **米** 为单位。您可以对任意四个坐标使用任意表达式。当一对参数直接引用一对属性，而另一对是常量时，会选择优化路径。

`GEODIST()` 还接受一个可选的第5个参数，使您可以轻松地在输入和输出单位之间进行转换，并选择要使用的特定地理距离公式。完整的语法和一些示例如下：

```sql
GEODIST(lat1, lon1, lat2, lon2, { option=value, ... })

GEODIST(40.7643929, -73.9997683, 40.7642578, -73.9994565, {in=degrees, out=feet})

GEODIST(51.50, -0.12, 29.98, 31.13, {in=deg, out=mi})
```

已知选项及其值如下：

* `in = {deg | degrees | rad | radians}`，指定输入单位；
* `out = {m | meters | km | kilometers | ft | feet | mi | miles | yd | yards | in | inch | cm | centimeters | mm | millimeters | NM | nmi | nauticalmiles}`，指定输出单位；
* `method = {adaptive | haversine}`，指定地理距离计算方法。

默认方法是“自适应”。它是一种经过良好优化的实现，在所有时间上都比“哈弗辛”方法更精确 *且* 快得多。

### GEOPOLY2D()
`GEOPOLY2D(lat1,lon1,lat2,lon2,lat3,lon3...)` 创建一个多边形，用于与 [CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) 函数一起使用。该函数通过将多边形细分为更小的部分来考虑地球的弯曲，应该用于更大的区域。对于小区域，可以使用 [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) 函数代替。该函数期望坐标为以度表示的纬度/经度坐标对；如果使用弧度，则与 `POLY2D()` 给出的结果相同。

### POLY2D()
`POLY2D(x1,y1,x2,y2,x3,y3...)` 创建一个多边形，用于与 [CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) 函数一起使用。该多边形假定地球是平的，因此不应太大；对于大区域，应使用 [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) 函数，该函数考虑地球的曲率。

<!-- proofread -->
