# Функции строк

### CONCAT()
Конкатенирует две или более строк в одну. Несстроковые аргументы должны быть явно преобразованы в строку с помощью функции `TO_STRING()`.

```sql
CONCAT(TO_STRING(float_attr), ',', TO_STRING(int_attr), ',', title)
```

### LEVENSHTEIN()
`LEVENSHTEIN ( pattern, source, {normalize=0, length_delta=0})` возвращает число (расстояние Левенштейна) однобуквенных правок (вставок, удалений или замен) между строками `pattern` и `source`, необходимых для преобразования `pattern` в `source`.

   * `pattern`, `source` - константная строка, имя строкового поля, имя JSON-поля или любое выражение, возвращающее строку (например, [SUBSTRING_INDEX()](../Functions/String_functions.md#SUBSTRING_INDEX%28%29))
   * `normalize` - опция возврата расстояния в виде числа с плавающей точкой в диапазоне `[0.0 - 1.0]`, где 0.0 - точное совпадение, а 1.0 - максимальное различие. Значение по умолчанию 0, означает, что нормализация не применяется и результат выдается как целое число.
   * `length_delta` - пропускает вычисление расстояния Левенштейна и возвращает `max(strlen(pattern), strlen(source))`, если опция установлена и длины строк отличаются более чем на `length_delta`. Значение по умолчанию 0, значит вычислять расстояние Левенштейна для любых входных строк. Эта опция может быть полезна при проверке преимущественно схожих строк.

``` sql
SELECT LEVENSHTEIN('gily', attr1) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC, dist ASC;
SELECT LEVENSHTEIN('gily', j.name, {length_delta=6}) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC;
SELECT LEVENSHTEIN(title, j.name, {normalize=1}) AS dist, WEIGHT() AS w FROM test WHERE MATCH ('test') ORDER BY w DESC, dist ASC;
```

### REGEX()
Функция `REGEX(attr,expr)` возвращает 1, если регулярное выражение совпадает со строкой атрибута, и 0 в противном случае. Работает как с строковыми, так и с JSON-атрибутами.

```sql
SELECT REGEX(content, 'box?') FROM test;
SELECT REGEX(j.color, 'red | pink') FROM test;
```

Выражения должны соответствовать синтаксису RE2. Для выполнения поиска без учета регистра, например, можно использовать:
```sql
SELECT REGEX(content, '(?i)box') FROM test;
```

### SNIPPET()
Функция `SNIPPET()` может быть использована для выделения результатов поиска в данном тексте. Первые два аргумента — это текст для выделения и запрос. [Опции](../Searching/Highlighting.md#Highlighting-options) могут передаваться третьим, четвертым и последующими аргументами. `SNIPPET()` может брать текст для выделения непосредственно из таблицы. В этом случае первым аргументом должно быть имя поля:

```sql
SELECT SNIPPET(body, QUERY()) FROM myIndex WHERE MATCH('my.query')
```

В этом примере выражение `QUERY()` возвращает текущий полнотекстовый запрос. `SNIPPET()` также может выделять неиндексированный текст:

```sql
mysql  SELECT id, SNIPPET('text to highlight', 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

Кроме того, ее можно использовать для подсветки текста, извлекаемого из других источников с использованием пользовательской функции (UDF):

```sql
SELECT id, SNIPPET(myUdf(id), 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

В данном контексте `myUdf()` — это пользовательская функция (UDF), которая извлекает документ по его ID из внешнего хранилища. Функция `SNIPPET()` считается функцией "post limit", то есть вычисление сниппетов откладывается до подготовки всего конечного набора результатов и даже после применения оператора `LIMIT`. Например, если используется `LIMIT 20,10`, вызов `SNIPPET()` произойдет не более 10 раз.

Важно отметить, что `SNIPPET()` не поддерживает ограничения на уровне полей. Для такой функциональности используйте [HIGHLIGHT()](../Searching/Highlighting.md#Highlighting-via-SQL).

### SUBSTRING_INDEX()

<!-- example substring_index -->
`SUBSTRING_INDEX(string, delimiter, number)` возвращает подстроку исходной строки, основанную на указанном количестве вхождений разделителя:

   *   string - исходная строка, которая может быть константной строкой или строкой из строкового/JSON-атрибута.
   *   delimiter - разделитель, который нужно искать.
   *   number - количество раз, которое нужно искать разделитель. Может быть положительным или отрицательным числом. При положительном значении функция возвращает все левее разделителя. При отрицательном — все правее разделителя.

По умолчанию `SUBSTRING_INDEX()` возвращает строку, но при необходимости она может быть преобразована в другие типы (например, целое или с плавающей точкой). Числовые значения можно преобразовать с помощью специальных функций (например, `BIGINT()`, `DOUBLE()` и др.).

<!-- request SQL -->
```sql
SELECT SUBSTRING_INDEX('www.w3schools.com', '.', 2) FROM test;
SELECT SUBSTRING_INDEX(j.coord, ' ', 1) FROM test;
SELECT          SUBSTRING_INDEX('1.2 3.4', ' ',  1);  /* '1.2' */
SELECT          SUBSTRING_INDEX('1.2 3.4', ' ', -1);  /* '3.4' */
SELECT sint (   SUBSTRING_INDEX('1.2 3.4', ' ',  1)); /* 1 */
SELECT sint (   SUBSTRING_INDEX('1.2 3.4', ' ', -1)); /* 3 */
SELECT double ( SUBSTRING_INDEX('1.2 3.4', ' ',  1)); /* 1.200000 */
SELECT double ( SUBSTRING_INDEX('1.2 3.4', ' ', -1)); /* 3.400000 */
```

<!-- end -->

### UPPER() и LOWER()

`UPPER(string)` преобразует аргумент в верхний регистр, `LOWER(string)` — в нижний регистр.

Результат также может быть приведен к числовому типу, но только если строковый аргумент преобразуем в число. Числовые значения могут быть приведены с помощью произвольных функций (`BITINT`, `DOUBLE` и др.).

```sql
SELECT upper('www.w3schools.com', '.', 2); /* WWW.W3SCHOOLS.COM  */
SELECT double (upper ('1.2e3')); /* 1200.000000 */
SELECT integer (lower ('12345')); /* 12345 */
```
<!-- proofread -->

