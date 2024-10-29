# Geo search

One of the greatest features of Manticore Search is the ability to combine full-text searching with geo-location. For example, a retailer can offer a search where a user looks for a product, and the result set can indicate the closest shop that has the product in stock, so the user can go in-store and pick it up. A travel site can provide results based on a search limited to a certain area and have the results sorted by the distance from a point (for example, 'search museums near a hotel'). 

To perform geo-searching, a document needs to contain pairs of latitude/longitude coordinates. The coordinates can be stored as float attributes. If the document has multiple locations, it may be convenient to use a JSON attribute to store coordinate pairs.



```ini
table myrt
{
    ...
    rt_attr_float = lat
    rt_attr_float = lon
    ...
}
```

The coordinates can be stored as degrees or radians.

If secondary indexes are generated for latitude and longitude attributes, they may automatically be used to speed up geo searches if the [Cost based optimizer](../Searching/Cost_based_optimizer.md) decides to use them.

## Performing distance сalculation

To find out the distance between two points, the [GEODIST()](../Functions/Geo_spatial_functions.md#GEODIST%28%29) function can be used. `GEODIST` requires two pairs of coordinates as its first four parameters.

The 5th parameter in a simplified JSON format can configure certain aspects of the function. By default, `GEODIST` expects coordinates to be in radians, but `in=degrees` can be added to allow using degrees as input. The coordinates for which we perform the geo distance must have the same type (degrees or radians) as the ones stored in the table; otherwise, results will be misleading.

The calculated distance is by default in meters, but with the `out` option, it can be transformed to kilometers, feet, or miles. Lastly, by default, a calculation method called `adaptive` is used. An alternative method based on the `haversine` algorithm is available; however, this one is slower and less precise.

The result of the function - the distance - can be used in the`ORDER BY` clause to sort the results:

```sql
SELECT *, GEODIST(40.7643929, -73.9997683, lat, lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') ORDER BY distance ASC, WEIGHT() DESC;
```

Or to limit the results to a radial area around the point:

```sql
SELECT *,GEODIST(40.7643929, -73.9997683, lat,lon, {in=degrees, out=miles}) AS distance FROM myindex WHERE MATCH('...') AND distance <1000 ORDER BY WEIGHT(), DISTANCE ASC;
```

## Searching in polygons

Another geo search feature is the ability to determine if a location is within a specified area. A special function constructs a polygon object, which is then used by another function to test whether a set of coordinates is contained within that polygon or not.

There are two functions available for creating the polygon:

*   [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) - creates a polygon that takes into account the Earth's curvature
*   [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) - creates a simple polygon in flat space

`POLY2D` is suitable for geo searches when the area has sides shorter than 500km (for polygons with 3-4 sides; for polygons with more sides, lower values should be considered). For areas with longer sides, using `GEOPOLY2D` is required to maintain accurate results. `GEOPOLY2D` expects coordinates as latitude/longitude pairs in degrees; using radians will yield results in flat space (similar to `POLY2D`).

[CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29) takes a polygon and a set of coordinates as input and outputs `1` if the point is inside the polygon or `0` otherwise.

```sql
SELECT *,CONTAINS(GEOPOLY2D(40.76439, -73.9997, 42.21211, -73.999,  42.21211, -76.123, 40.76439, -76.123), 41.5445, -74.973) AS inside FROM myindex WHERE MATCH('...') AND inside=1;
```
<!-- proofread -->