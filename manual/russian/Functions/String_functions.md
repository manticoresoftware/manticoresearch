# Функции строк

### CONCAT()
Объединяет две или более строк в одну. Аргументы, не являющиеся строками, должны быть явно преобразованы в строку с помощью функции `TO_STRING()`.

```sql
CONCAT(TO_STRING(float_attr), ',', TO_STRING(int_attr), ',', title)
```

### LEVENSHTEIN()
`LEVENSHTEIN ( pattern, source, {normalize=0, length_delta=0})` возвращает число (расстояние Левенштейна) односимвольных изменений (вставок, удалений или замен) между строками `pattern` и `source`, необходимых для преобразования `pattern` в `source`.

   * `pattern`, `source` - константная строка, имя строкового поля, имя JSON-поля или любое выражение, которое возвращает строку (например, [SUBSTRING_INDEX()](../Functions/String_functions.md#SUBSTRING_INDEX%28%29))
   * `normalize` - опция для возврата расстояния в виде числа с плавающей точкой в диапазоне `[0.0 - 1.0]`, где 0.0 — точное совпадение, а 1.0 — максимальное различие. Значение по умолчанию 0, что означает отсутствие нормализации и возврат результата в виде целого числа.
   * `length_delta` - пропускает вычисление расстояния Левенштейна и возвращает `max(strlen(pattern), strlen(source))`, если опция установлена и длины строк отличаются более чем на значение `length_delta`. Значение по умолчанию 0, что означает вычисление расстояния Левенштейна для любых входных строк. Эта опция может быть полезна при проверке преимущественно похожих строк.

``` sql
SELECT LEVENSHTEIN('gily', attr1) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC, dist ASC;
SELECT LEVENSHTEIN('gily', j.name, {length_delta=6}) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC;
SELECT LEVENSHTEIN(title, j.name, {normalize=1}) AS dist, WEIGHT() AS w FROM test WHERE MATCH ('test') ORDER BY w DESC, dist ASC;
```

### REGEX()
Функция `REGEX(attr,expr)` возвращает 1, если регулярное выражение совпадает со строкой атрибута, и 0 в противном случае. Работает как со строковыми, так и с JSON-атрибутами.

```sql
SELECT REGEX(content, 'box?') FROM test;
SELECT REGEX(j.color, 'red | pink') FROM test;
```

Выражения должны соответствовать синтаксису RE2. Для выполнения поиска без учета регистра, например, можно использовать:
```sql
SELECT REGEX(content, '(?i)box') FROM test;
```

### SNIPPET()
Функция `SNIPPET()` может использоваться для выделения результатов поиска в заданном тексте. Первые два аргумента: текст для выделения и запрос. [Опции](../Searching/Highlighting.md#Highlighting-options) могут передаваться функции в качестве третьего, четвертого и последующих аргументов. `SNIPPET()` может получать текст для выделения непосредственно из таблицы. В этом случае первым аргументом должно быть имя поля:

```sql
SELECT SNIPPET(body, QUERY()) FROM myIndex WHERE MATCH('my.query')
```

В этом примере выражение `QUERY()` возвращает текущий полнотекстовый запрос. `SNIPPET()` также может выделять неиндексированный текст:

```sql
mysql  SELECT id, SNIPPET('text to highlight', 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

Кроме того, она может использоваться для выделения текста, полученного из других источников с помощью функции, определенной пользователем (UDF):

```sql
SELECT id, SNIPPET(myUdf(id), 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

В данном контексте `myUdf()` — это функция, определенная пользователем (UDF), которая извлекает документ по его ID из внешнего источника хранения. Функция `SNIPPET()` считается функцией "post limit", что означает, что вычисление сниппетов откладывается до подготовки всего итогового набора результатов, даже после применения оператора `LIMIT`. Например, если используется оператор `LIMIT 20,10`, `SNIPPET()` будет вызвана не более 10 раз.

Важно отметить, что `SNIPPET()` не поддерживает ограничения по полям. Для этой функциональности используйте [HIGHLIGHT()](../Searching/Highlighting.md#Highlighting-via-SQL).

### SUBSTRING_INDEX()

<!-- example substring_index -->
`SUBSTRING_INDEX(string, delimiter, number)` возвращает подстроку исходной строки, основанную на указанном количестве вхождений разделителя:

   *   string - исходная строка, которая может быть константной строкой или строкой из строкового/JSON-атрибута.
   *   delimiter - разделитель для поиска.
   *   number - количество раз для поиска разделителя. Может быть положительным или отрицательным числом. Если число положительное, функция вернет все слева от разделителя. Если отрицательное — все справа от разделителя.

По умолчанию `SUBSTRING_INDEX()` возвращает строку, но при необходимости может быть приведена к другим типам (например, целому или числу с плавающей точкой). Числовые значения можно преобразовать с помощью специальных функций (например, `BIGINT()`, `DOUBLE()` и т.д.).

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

`UPPER(string)` преобразует аргумент в верхний регистр, `LOWER(string)` преобразует аргумент в нижний регистр.

Результат также может быть преобразован в числовой тип, но только если строковый аргумент можно преобразовать в число. Числовые значения могут быть преобразованы с помощью произвольных функций (`BITINT`, `DOUBLE` и т.д.).

```sql
SELECT upper('www.w3schools.com', '.', 2); /* WWW.W3SCHOOLS.COM  */
SELECT double (upper ('1.2e3')); /* 1200.000000 */
SELECT integer (lower ('12345')); /* 12345 */
```
<!-- proofread -->

