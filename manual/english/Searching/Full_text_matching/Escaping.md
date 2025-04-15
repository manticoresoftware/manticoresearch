# Escaping characters in query string

Since certain characters function as operators in the query string, they must be escaped to prevent query errors or unintended matching conditions.

The following characters should be escaped using a backslash (`\`):

```
!    "    $    '    (    )    -    /    <    @    \    ^    |    ~
```

## In MySQL command line client

To escape a single quote ('), use one backslash:
```sql
SELECT * FROM your_index WHERE MATCH('l\'italiano');
```


For the other characters in the list mentioned earlier, which are operators or query constructs, they must be treated as simple characters by the engine, with a preceding escape character.
The backslash must also be escaped, resulting in two backslashes:

```sql
SELECT * FROM your_index WHERE MATCH('r\\&b | \\(official video\\)');
```

To use a backslash as a character, you must escape both the backslash as a character and the backslash as the escape operator, which requires four backslashes:

```sql
SELECT * FROM your_index WHERE MATCH('\\\\ABC');
```

When you are working with JSON data in Manticore Search and need to include a double quote (`"`) within a JSON string, it's important to handle it with proper escaping. In JSON, a double quote within a string is escaped using a backslash (`\`). However, when inserting the JSON data through an SQL query, Manticore Search interprets the backslash (`\`) as an escape character within strings.

To ensure the double quote is correctly inserted into the JSON data, you need to escape the backslash itself. This results in using two backslashes (`\\`) before the double quote. For example:

```sql
insert into tbl(j) values('{"a": "\\"abc\\""}');
```

## Using MySQL drivers

MySQL drivers provide escaping functions (e.g., `mysqli_real_escape_string` in PHP or `conn.escape_string` in Python), but they only escape specific characters.
You will still need to add escaping for the characters from the previously mentioned list that are not escaped by their respective functions.
Because these functions will escape the backslash for you, you only need to add one backslash.

This also applies to drivers that support (client-side) prepared statements. For example, with PHP PDO prepared statements, you need to add a backslash for the `$` character:

```php
$statement = $ln_sph->prepare( "SELECT * FROM index WHERE MATCH(:match)");
$match = '\$manticore';
$statement->bindParam(':match',$match,PDO::PARAM_STR);
$results = $statement->execute();
```

This results in the final query `SELECT * FROM index WHERE MATCH('\\$manticore');`

## In HTTP JSON API

The same rules for the SQL protocol apply, with the exception that for JSON, the double quote must be escaped with a single backslash, while the rest of the characters require double escaping.

When using JSON libraries or functions that convert data structures to JSON strings, the double quote and single backslash are automatically escaped by these functions and do not need to be explicitly escaped.



## In clients

The [official clients](https://github.com/manticoresoftware/) utilize common JSON libraries/functions available in their respective programming languages under the hood. The same rules for escaping mentioned earlier apply.


## Escaping asterisk

The asterisk (`*`) is a unique character that serves two purposes:
* as a wildcard prefix/suffix expander
* as an any-term modifier within a phrase search.

Unlike other special characters that function as operators, the asterisk cannot be escaped when it's in a position to provide one of its functionalities.

In non-wildcard queries, the asterisk does not require escaping, whether it's in the `charset_table` or not.

In wildcard queries, an asterisk in the middle of a word does not require escaping. As a wildcard operator (either at the beginning or end of the word), the asterisk will always be interpreted as the wildcard operator, even if escaping is applied.

## Escaping json node names in SQL

To escape special characters in JSON nodes, use a backtick. For example:

```sql
MySQL [(none)]> select * from t where json.`a=b`=234;
+---------------------+-------------+------+
| id                  | json        | text |
+---------------------+-------------+------+
| 8215557549554925578 | {"a=b":234} |      |
+---------------------+-------------+------+

MySQL [(none)]> select * from t where json.`a:b`=123;
+---------------------+-------------+------+
| id                  | json        | text |
+---------------------+-------------+------+
| 8215557549554925577 | {"a:b":123} |      |
+---------------------+-------------+------+
```
<!-- proofread -->
