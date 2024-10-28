# Expressions in search

Manticore enables the use of arbitrary arithmetic expressions through both SQL and HTTP, incorporating attribute values, internal attributes (document ID and relevance weight), arithmetic operations, several built-in functions, and user-defined functions. Below is the complete reference list for quick access.

## Arithmetic operators
```sql
+, -, *, /, %, DIV, MOD
```

Standard arithmetic operators are available. Arithmetic calculations involving these operators can be executed in three different modes:

1. using single-precision, 32-bit IEEE 754 floating point values (default),
2. using signed 32-bit integers,
3. using 64-bit signed integers.

The expression parser automatically switches to integer mode if no operations result in a floating point value. Otherwise, it uses the default floating point mode. For example, a+b will be computed using 32-bit integers if both arguments are 32-bit integers; or using 64-bit integers if both arguments are integers but one of them is 64-bit; or in floats otherwise. However, `a/b` or `sqrt(a)` will always be computed in floats, as these operations return a non-integer result. To avoid this, you can use `IDIV(a,b)` or a `DIV b` form. Additionally, `a*b` will not automatically promote to 64-bit when arguments are 32-bit. To enforce 64-bit results, use [BIGINT()](../Functions/Type_casting_functions.md#BIGINT%28%29), but note that if non-integer operations are present, BIGINT() will simply be ignored.

## Comparison operators
```sql
<, > <=, >=, =, <>
```

The comparison operators return 1.0 when the condition is true and 0.0 otherwise. For example, `(a=b)+3` evaluates to 4 when attribute `a` is equal to attribute `b`, and to 3 when `a` is not. Unlike MySQL, the equality comparisons (i.e., `=` and `<>` operators) include a small equality threshold (1e-6 by default). If the difference between the compared values is within the threshold, they are considered equal.

The `BETWEEN` and `IN` operators, in the case of multi-value attributes, return true if at least one value matches the condition (similar to [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)). The `IN` operator does not support JSON attributes. The `IS (NOT) NULL` operator is supported only for JSON attributes.

## Boolean operators
```sql
AND, OR, NOT
```

Boolean operators (AND, OR, NOT) behave as expected. They are left-associative and have the lowest priority compared to other operators. NOT has higher priority than AND and OR but still less than any other operator. AND and OR share the same priority, so using parentheses is recommended to avoid confusion in complex expressions.

## Bitwise operators
```sql
&, |
```

These operators perform bitwise AND and OR respectively. The operands must be of integer types.

## Functions:
* [ABS()](../Functions/Mathematical_functions.md#ABS%28%29)
* [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29)
* [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)
* [ATAN2()](../Functions/Mathematical_functions.md#ATAN2%28%29)
* [BIGINT()](../Functions/Type_casting_functions.md#BIGINT%28%29)
* [BITDOT()](../Functions/Mathematical_functions.md#BITDOT%28%29)
* [BM25F()](../Functions/Searching_and_ranking_functions.md#BM25F%28%29)
* [CEIL()](../Functions/Mathematical_functions.md#CEIL%28%29)
* [CONCAT()](../Functions/String_functions.md#CONCAT%28%29)
* [CONTAINS()](../Functions/Arrays_and_conditions_functions.md#CONTAINS%28%29)
* [COS()](../Functions/Mathematical_functions.md#COS%28%29)
* [CRC32()](../Functions/Mathematical_functions.md#CRC32%28%29)
* [DATE_HISTOGRAM()](../Functions/Date_and_time_functions.md#DATE_HISTOGRAM%28%29)
* [DATE_RANGE()](../Functions/Date_and_time_functions.md#DATE_RANGE%28%29)
* [DAY()](../Functions/Date_and_time_functions.md#DAY%28%29)
* [DOUBLE()](../Functions/Type_casting_functions.md#DOUBLE%28%29)
* [EXP()](../Functions/Mathematical_functions.md#EXP%28%29)
* [FIBONACCI()](../Functions/Mathematical_functions.md#FIBONACCI%28%29)
* [FLOOR()](../Functions/Mathematical_functions.md#FLOOR%28%29)
* [GEODIST()](../Functions/Geo_spatial_functions.md#GEODIST%28%29)
* [GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29)
* [GREATEST()](../Functions/Mathematical_functions.md#GREATEST%28%29)
* [HOUR()](../Functions/Date_and_time_functions.md#HOUR%28%29)
* [HISTOGRAM()](../Functions/Arrays_and_conditions_functions.md#HISTOGRAM%28%29)
* [IDIV()](../Functions/Mathematical_functions.md#IDIV%28%29)
* [IF()](../Functions/Arrays_and_conditions_functions.md#IF%28%29)
* [IN()](../Functions/Arrays_and_conditions_functions.md#IN%28%29)
* [INDEXOF()](../Functions/Arrays_and_conditions_functions.md#INDEXOF%28%29)
* [INTEGER()](../Functions/Type_casting_functions.md#INTEGER%28%29)
* [INTERVAL()](../Functions/Arrays_and_conditions_functions.md#INTERVAL%28%29)
* [LAST_INSERT_ID()](../Functions/Other_functions.md#LAST_INSERT_ID%28%29)
* [LEAST()](../Functions/Mathematical_functions.md#LEAST%28%29)
* [LENGTH()](../Functions/Arrays_and_conditions_functions.md#LENGTH%28%29)
* [LN()](../Functions/Mathematical_functions.md#LN%28%29)
* [LOG10()](../Functions/Mathematical_functions.md#LOG10%28%29)
* [LOG2()](../Functions/Mathematical_functions.md#LOG2%28%29)
* [MAX()](../Functions/Mathematical_functions.md#MAX%28%29)
* [MIN()](../Functions/Mathematical_functions.md#MIN%28%29)
* [MINUTE()](../Functions/Date_and_time_functions.md#MINUTE%28%29)
* [MIN_TOP_SORTVAL()](../Functions/Searching_and_ranking_functions.md#MIN_TOP_SORTVAL%28%29)
* [MIN_TOP_WEIGHT()](../Functions/Searching_and_ranking_functions.md#MIN_TOP_WEIGHT%28%29)
* [MONTH()](../Functions/Date_and_time_functions.md#MONTH%28%29)
* [NOW()](../Functions/Date_and_time_functions.md#NOW%28%29)
* [PACKEDFACTORS()](../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29)
* [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29)
* [POW()](../Functions/Mathematical_functions.md#POW%28%29)
* [RAND()](../Functions/Mathematical_functions.md#RAND%28%29)
* [RANGE()](../Functions/Arrays_and_conditions_functions.md#RANGE%28%29)
* [REGEX()](../Functions/String_functions.md#REGEX%28%29)
* [REMAP()](../Functions/Arrays_and_conditions_functions.md#REMAP%28%29)
* [SECOND()](../Functions/Date_and_time_functions.md#SECOND%28%29)
* [SIN()](../Functions/Mathematical_functions.md#SIN%28%29)
* [SINT()](../Functions/Type_casting_functions.md#SINT%28%29)
* [SQRT()](../Functions/Mathematical_functions.md#SQRT%28%29)
* [SUBSTRING_INDEX()](../Functions/String_functions.md#SUBSTRING_INDEX%28%29)
* [TO_STRING()](../Functions/Type_casting_functions.md#TO_STRING%28%29)
* [UINT()](../Functions/Type_casting_functions.md#UINT%28%29)
* [YEAR()](../Functions/Date_and_time_functions.md#YEAR%28%29)
* [YEARMONTH()](../Functions/Date_and_time_functions.md#YEARMONTH%28%29)
* [YEARMONTHDAY()](../Functions/Date_and_time_functions.md#YEARMONTHDAY%28%29)
* [WEIGHT()](../Functions/Searching_and_ranking_functions.md#WEIGHT%28%29)

## Expressions in HTTP JSON

In the HTTP JSON interface, expressions are supported via `script_fields` and `expressions`.

### script_fields

<!-- example script_fields -->
```json
{
	"table": "test",
	"query": {
		"match_all": {}
	}, "script_fields": {
		"add_all": {
			"script": {
				"inline": "( gid * 10 ) | crc32(title)"
			}
		},
		"title_len": {
			"script": {
	   			"inline": "crc32(title)"
			}
		}
	}
}
```

In this example, two expressions are created: `add_all` and `title_len`. The first expression calculates `( gid * 10 ) | crc32(title)` and stores the result in the `add_all` attribute. The second expression calculates `crc32(title)` and stores the result in the `title_len` attribute.

Currently, only `inline` expressions are supported. The value of the `inline` property (the expression to compute) has the same syntax as SQL expressions.

The expression name can be utilized in filtering or sorting.


<!-- intro -->
##### script_fields:

<!-- request script_fields -->
```json
{
	"table":"movies_rt",
	"script_fields":{
		"cond1":{
			"script":{
				"inline":"actor_2_facebook_likes =296 OR movie_facebook_likes =37000"
			}
		},
		"cond2":{
			"script":{
				"inline":"IF (IN (content_rating,'TV-PG','PG'),2, IF(IN(content_rating,'TV-14','PG-13'),1,0))"
			}
		}
	},
	"limit":10,
	"sort":[
		{
			"cond2":"desc"
		},
		{
			"actor_1_name":"asc"
		},
		{
			"actor_2_name":"desc"
		}
	],
	"profile":true,
	"query":{
		"bool":{
			"must":[
				{
					"match":{
						"*":"star"
					}
				},
				{
					"equals":{
						"cond1":1
					}
				}
			],
			"must_not":[
				{
					"equals":{
						"content_rating":"R"
					}
				}
			]
		}
	}
}
```

<!-- end -->

By default, expression values are included in the `_source` array of the result set. If the source is selective (see [Source selection](../Searching/Search_results.md#Source-selection)), the expression name can be added to the `_source` parameter in the request. Note, the names of the expressions must be in lowercase.

### expressions

<!-- example expressions -->

`expressions` is an alternative to `script_fields` with a simpler syntax. The example request adds two expressions and stores the results into `add_all` and `title_len` attributes. Note, the names of the expressions must be in lowercase.

<!-- request expressions -->
```json
{
  "table": "test",
  "query": { "match_all": {} },
  "expressions":
  {
	  "add_all": "( gid * 10 ) | crc32(title)",
      "title_len": "crc32(title)"
  }
}
```

<!-- end -->

<!-- proofread -->
