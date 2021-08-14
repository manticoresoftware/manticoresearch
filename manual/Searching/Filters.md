# Filters

## WHERE

`WHERE` is an SQL clause which works for both fulltext matching and additional filtering. The following operators are available:

* [Comparison operators](../Searching/Expressions.md#Comparison-operators) `<, > <=, >=, =, <>, BETWEEN, IN, IS NULL`
* [Boolean operators](../Searching/Full_text_matching/Operators.md#Boolean-operators) `AND, OR, NOT`

`MATCH('query')` is supported and maps to [fulltext query](../Searching/Full_text_matching/Operators.md). 

`{col_name | expr_alias} [NOT] IN @uservar` condition syntax is supported. Refer to [SET](../Server_settings/Setting_variables_online.md#SET) syntax for a description of global user variables.

## HTTP

JSON queries have two distinct entities: fulltext queries and filters. Both can be organised in a tree (using a bool query), but for now filters work only for the root element of the query. For example:

```json
{
  "index":"test",
  "query": { "range": { "price": { "lte": 11 } } }
}
```

Here's an example of several filters in a `bool` query:

```json
{
  "index": "test1",
  "query":
  {
    "bool":
    {
      "must":
      [
        { "match" : { "_all" : "product" } },
        { "range": { "price": { "gte": 500, "lte": 1000 } } },
      ],
      "must_not":
      {
        "range": { "revision": { "lt": 15 } }
      }
    }
  }
}
```

This is a fulltext query that matches all the documents containing product in any field. These documents must have a price greater or equal than 500 (`gte`) and less or equal than 1000 (`lte`). All of these documents must not have a revision less than 15 (`lt`).

### bool query

A bool query matches documents matching boolean combinations of other queries and/or filters. Queries and filters must be specified in "must", "should" or "must_not" sections. Example:

```json
{
  "index":"test",
  "query":
  {
    "bool":
    {
      "must":
      [
        { "match": {"_all":"keyword"} },
        { "range": { "int_col": { "gte": 14 } } }
      ]
    }
  }
}
```

### must
Queries and filters specified in the "must" section must match the documents. If several fulltext queries or filters are specified, all of them. This is the equivalent of AND queries in SQL.


### should
Queries and filters specified in the `should` section should match the documents. If some queries are specified in `must` or `must_not`, `should` queries are ignored. On the other hand, if there are no queries other than `should`, then at least one of these queries must match a document for it to match the bool query. This is the equivalent of `OR` queries.

### must_not
Queries and filters specified in the `must_not` section must not match the documents. If several queries are specified under `must_not`, the document matches if none of them match.

Example:

```json
{
  "index": "test1",
  "query":
  {
    "bool":
    {
      "must":
      {
        "match" : { "_all" : "product" }
      },
      "must_not":
      [
        { "match": {"_all":"phone"} },
        { "range": { "price": { "gte": 500 } } }
      ]
    }
  }
}
```

### Queries in SQL format
Queries in SQL format (`query_string`) can also be used in bool queries. Example:

```json
{
  "index": "test1",
  "query":
  {
    "bool":
    {
      "must":
      [
        { "query_string" : "product" },
        { "query_string" : "good" }
      ]
    }
  }
}
```

### Equality filters
Equality filters are the simplest filters that work with integer, float and string attributes. Example:

```json
{
  "index":"test1",
  "query":
  {
    "equals": { "price": 500 }
  }
}
```

### Set filters
Set filters check if attribute value is equal to any of the values in the specified set. Example:

```json
{
  "index":"test1",
  "query": 
  {
    "in": 
    {
      "price": [1,10,100]
    }
  }
}
```

Set filters support integer, string and multi-value attributes.


### Range filters
Range filters match documents that have attribute values within a specified range. Example:

```json
{
  "index":"test1",
  "query":
  {
    "range":
    {
      "price":
      {
        "gte": 500,
        "lte": 1000
      }
    }
  }
}
```

Range filters support the following properties:

##### gte
Value must be greater than or equal to

##### gt
value must be greater than

##### lte
value must be less than or equal to

##### lt
value must be less

### Geo distance filters

`geo_distance` filters are used to filter the documents that are within a specific distance from a geo location.

Example:

```json
{
  "index":"test",
  "query":
  {
    "geo_distance":
    {
      "location_anchor": {"lat":49, "lon":15},
      "location_source": {"attr_lat, attr_lon"},
      "distance_type": "adaptive",
      "distance":"100 km"
    }
  }
}
```

##### location_anchor
Specifies the pin location, in degrees. Distances are calculated from this point.

##### location_source
Specifies the attributes that contain latitude and longitude.

##### distance_type
Specifies distance calculation function. Can be either adaptive or haversine. adaptive is faster and more precise, for more details see `GEODIST()`. Optional, defaults to adaptive.

##### distance
Specifies the maximum distance from the pin locations. All documents within this distance match. The distance can be specified in various units. If no unit is specified, the distance is assumed to be in meters. Here is a list of supported distance units:

* Meter: `m` or `meters`
* Kilometer: `km` or `kilometers`
* Centimeter: `cm` or `centimeters`
* Millimeter: `mm` or `millimeters`
* Mile: `mi` or `miles`
* Yard: `yd` or `yards`
* Feet: `ft` or `feet`
* Inch: `in` or `inch`
* Nautical mile: `NM`, `nmi` or `nauticalmiles`

`location_anchor` and `location_source` properties accept the following latitude/longitude formats:

* an object with lat and lon keys: `{ "lat":"attr_lat", "lon":"attr_lon" }`
* a string of the following structure: `"attr_lat,attr_lon"`
* an array with the latitude and longitude in the following order: `[attr_lon, attr_lat]`

Latitude and longitude are specified in degrees.

`geo_distance` can be used as a filter in bool queries along with matches or other attribute filters:

```json
{
  "index": "geodemo",
  "query": {
    "bool": {
      "must": [
        {
          "match": {
            "*": "station"
          }
        },
        {
          "equals": {
            "state_code": "ENG"
          }
        },
        {
          "geo_distance": {
            "distance_type": "adaptive",
            "location_anchor": {
              "lat": 52.396,
              "lon": -1.774
            },
            "location_source": "latitude_deg,longitude_deg",
            "distance": "10000 m"
          }
        }
      ]
    }
  }
}
```



