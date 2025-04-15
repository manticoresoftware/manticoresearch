# Выражения в поиске

Manticore позволяет использовать произвольные арифметические выражения как через SQL, так и через HTTP, включая значения атрибутов, внутренние атрибуты (идентификатор документа и вес релевантности), арифметические операции, несколько встроенных функций и пользовательские функции. Ниже приведен полный справочный список для быстрого доступа.

## Арифметические операторы
```sql
+, -, *, /, %, DIV, MOD
```

Доступны стандартные арифметические операторы. Арифметические вычисления с использованием этих операторов могут быть выполнены в трех различных режимах:

1. с использованием чисел с плавающей запятой одинарной точности, 32-битных IEEE 754 (по умолчанию),
2. с использованием знаковых 32-битных целых чисел,
3. с использованием 64-битных знаковых целых чисел.

Парсер выражений автоматически переключается в целочисленный режим, если ни одна операция не приводит к значению с плавающей запятой. В противном случае он использует режим с плавающей запятой по умолчанию. Например, a+b будет вычислено с использованием 32-битных целых чисел, если оба аргумента являются 32-битными целыми числами; или используя 64-битные целые числа, если оба аргумента являются целыми числами, но один из них - 64-битный; или в плавающих числах в противном случае. Однако, `a/b` или `sqrt(a)` всегда будет вычисляться в плавающих числах, так как эти операции возвращают нецелочисленный результат. Чтобы избежать этого, вы можете использовать `IDIV(a,b)` или форму `DIV b`. Кроме того, `a*b` не будет автоматически преобразовываться в 64-битный, когда аргументы являются 32-битными. Чтобы получить 64-битные результаты, используйте [BIGINT()](../Functions/Type_casting_functions.md#BIGINT%28%29), но имейте в виду, что если присутствуют нецелочисленные операции, BIGINT() будет просто игнорироваться.

## Операторы сравнения
```sql
<, > <=, >=, =, <>
```

Операторы сравнения возвращают 1.0, когда условие истинно, и 0.0 в противном случае. Например, `(a=b)+3` оценивается в 4, когда атрибут `a` равен атрибуту `b`, и в 3, когда `a` не равен. В отличие от MySQL, операции равенства (то есть, операторы `=` и `<>`) включают небольшой порог равенства (по умолчанию 1e-6). Если разница между сравниваемыми значениями находится в пределах порога, они считаются равными.

Операторы `BETWEEN` и `IN`, в случае многозначных атрибутов, возвращают истинное значение, если хотя бы одно значение соответствует условию (аналогично [ANY()](../Functions/Arrays_and_conditions_functions.md#ANY%28%29)). Оператор `IN` не поддерживает JSON-атрибуты. Оператор `IS (NOT) NULL` поддерживается только для JSON-атрибутов.

## Логические операторы
```sql
AND, OR, NOT
```

Логические операторы (AND, OR, NOT) ведут себя ожидаемым образом. Они имеют левую ассоциативность и имеют наименьший приоритет по сравнению с другими операторами. NOT имеет более высокий приоритет, чем AND и OR, но все же ниже любого другого оператора. AND и OR имеют одинаковый приоритет, поэтому рекомендуется использовать скобки, чтобы избежать путаницы в сложных выражениях.

## Побитовые операторы
```sql
&, |
```

Эти операторы выполняют побитовые AND и OR соответственно. Операнды должны быть целочисленного типа.

## Функции:
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

## Выражения в HTTP JSON

В HTTP JSON интерфейсе поддерживаются выражения через `script_fields` и `expressions`.

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

В этом примере создаются два выражения: `add_all` и `title_len`. Первое выражение вычисляет `( gid * 10 ) | crc32(title)` и сохраняет результат в атрибуте `add_all`. Второе выражение вычисляет `crc32(title)` и сохраняет результат в атрибуте `title_len`.

В настоящее время поддерживаются только `inline` выражения. Значение свойства `inline` (выражение для вычисления) имеет такую же синтаксис, как SQL выражения.

Имя выражения может быть использовано для фильтрации или сортировки.


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

По умолчанию значения выражений включены в массив `_source` результирующего набора. Если источник является селективным (см. [Выбор источника](../Searching/Search_results.md#Source-selection)), имя выражения может быть добавлено к параметру `_source` в запросе. Обратите внимание, что имена выражений должны быть написаны в строчном регистре.

### expressions

<!-- example expressions -->

`expressions` является альтернативой `script_fields` с более простой синтаксисом. Пример запроса добавляет два выражения и сохраняет результаты в атрибуты `add_all` и `title_len`. Обратите внимание, что имена выражений должны быть написаны в строчном регистре.

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
