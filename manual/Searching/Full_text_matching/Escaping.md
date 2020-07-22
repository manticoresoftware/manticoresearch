# Escaping characters in query string
 
 As some characters are used as operators in the query string, they should be escaped to avoid query errors or  unwanted matching conditions.
 
 The following characters should be escaped using backslash (`\`):
 
 ```
!    "    $    '    (    )    -    /    <    @    \    ^    |    ~
```

```sql
SELECT * FROM your_index WHERE MATCH('l\'italiano');
```

Use double backslash to escape all other characters.

```sql
SELECT * FROM your_index WHERE MATCH('r\\&b | \\(official video\\)');
```


Pay attention that in order to escape backslash character you should use "\\\\\\\\" syntax.

```sql
SELECT * FROM your_index WHERE MATCH('\\\\ABC');
```

Also, if you run your queries using some programming language don't forget about a mysql escaping function (e.g., `mysqli_real_escape_string` in PHP or `conn.escape_string` in Python) to escape these characters the same way as described above.
