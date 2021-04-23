# Escaping characters in query string

 As some characters are used as operators in the query string, they should be escaped to avoid query errors or  unwanted matching conditions.

 The following characters should be escaped using backslash (`\`):

```
!    "    $    '    (    )    -    /    <    @    \    ^    |    ~
```

## In MySQL command line client

Single quote (') can be escaped using one backslash:
```sql
SELECT * FROM your_index WHERE MATCH('l\'italiano');
```


The rest of the characters in the above list are operators or query constructs. In order to treat them as simple characters, the engine has to see them with a preceding escape character.
The backslash must also be escaped itself, resulting in 2 backslahes:

```sql
SELECT * FROM your_index WHERE MATCH('r\\&b | \\(official video\\)');
```

If we want to use backslash as character, we need to escape both the backslash as character and backslash as the escape operator, resulting in 4 backslashes used:

```sql
SELECT * FROM your_index WHERE MATCH('\\\\ABC');
```

## Using MySQL drivers

MySQL drivers offer escaping functions (e.g., `mysqli_real_escape_string` in PHP or `conn.escape_string` in Python), but they only escape certain characters.
You will still need to add escaping for the characters from the above list that are not escaped by the respective functions.
Since these functions will escape backslash for you, you only need to add one backslash.

This also applies for the drivers that support (client-side) prepared statements. For example for PHP PDO prepared statements, you need to add a backslash for `$` character:

```php
$statement = $ln_sph->prepare( "SELECT * FROM index WHERE MATCH(:match)");
$match = '\$manticore';
$statement->bindParam(':match',$match,PDO::PARAM_STR);
$results = $statement->execute();
```

It will make the final query `SELECT * FROM index WHERE MATCH('\\$manticore');`

## In HTTP JSON API

The same rules for SQL protocol apply with the exception that for JSON the double quote must be escaped with a single backslash, while the rest of the characters must be double escaped.


If using JSON libraries/functions that convert data structures to JSON strings the double quote and single backslash are escaped automatically by these functions and must not be explicitly escaped.


## In clients

The [new official clients](https://github.com/manticoresoftware/) (which use HTTP as protocol) are using under the hood common JSON libraries/functions available on the respective programming language. Same rules of escaping as above are applied.


## Escaping asterisk

Asterisk (`*`) is a special character that can have two functionalities:
* as wildcarding prefix/suffix expander
* and as any term modifier inside a phrase search.

Unlike other special characters that are operators, the asterisk cannot be escaped when it's in a position to offer one of it's functionalities.

In non-wildcard queries, the asterisk doesn't require escaping, regardless if it's in the `charset_table` or not.

In wildcard queries, asterisk in the middle of a word doesn't require escaping. As a wildcard operator (either at start or end of the word), the asterisk will always be interpreted as the wildcard operator even if escaping is applied.

## Escaping json node names in SQL

To escape special characters in json nodes use a backtick. For example:

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
