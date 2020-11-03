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
You will still need to add escaping for characters in the above list that are not escaped by the respective functions. 
Since these functions will escape backslash for you, you only need to add one backslash.

This also applies for drivers that support (client-side) prepared statements. For example for PHP PDO prepared statements, you need to add a backslash for `$` character:

```
$statement = $ln_sph->prepare( "SELECT * FROM index WHERE MATCH(:match)");
$match = '\$manticore';
$statement->bindParam(':match',$match,PDO::PARAM_STR);
$results = $statement->execute();
```

The  query received will be `SELECT * FROM index WHERE MATCH('\\$manticore');`