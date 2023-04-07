# Listing plugins

## SHOW PLUGINS
<!-- example Example -->

```sql
SHOW PLUGINS
```

Displays all the loaded plugins (except for Buddy plugins, see below) and UDFs. "Type" column should be one of the `udf`, `ranker`, `index_token_filter`, or `query_token_filter`. "Users" column is the number of thread that are currently using that plugin in a query. "Extra" column is intended for various additional plugin-type specific information; currently, it shows the return type for the UDFs and is empty for all the other plugin types.


<!-- intro -->
##### Example:

<!-- request Example -->

```sql
SHOW PLUGINS;
```

<!-- response -->

```sql
+------+----------+----------------+-------+-------+
| Type | Name     | Library        | Users | Extra |
+------+----------+----------------+-------+-------+
| udf  | sequence | udfexample.dll | 0     | INT   |
+------+----------+----------------+-------+-------+
1 row in set (0.00 sec)
```

<!-- end -->

## SHOW BUDDY PLUGINS

<!-- example Example_buddy -->

```sql
SHOW BUDDY PLUGINS
```

This will display all available plugins, including core and local ones.
To remove a plugin, make sure to use the name listed in the Package column.

<!-- request Example -->

```sql
SHOW BUDDY PLUGINS;
```

<!-- response -->

```
+-------------------------------------------------+------------------+----------+----------+
| Package                                         | Plugin           | Version  | Type     |
+-------------------------------------------------+------------------+----------+----------+
| manticoresoftware/buddy-plugin-empty-string     | empty-string     | dev-main | core     |
| manticoresoftware/buddy-plugin-backup           | backup           | dev-main | core     |
| manticoresoftware/buddy-plugin-emulate-elastic  | emulate-elastic  | dev-main | core     |
| manticoresoftware/buddy-plugin-insert           | insert           | dev-main | core     |
| manticoresoftware/buddy-plugin-lock-tables      | lock-tables      | dev-main | core     |
| manticoresoftware/buddy-plugin-unlock-tables    | unlock-tables    | dev-main | core     |
| manticoresoftware/buddy-plugin-select           | select           | dev-main | core     |
| manticoresoftware/buddy-plugin-show-full-tables | show-full-tables | dev-main | core     |
| manticoresoftware/buddy-plugin-show-fields      | show-fields      | dev-main | core     |
| manticoresoftware/buddy-plugin-show-queries     | show-queries     | dev-main | core     |
| manticoresoftware/buddy-plugin-cli-table        | cli-table        | dev-main | core     |
| manticoresoftware/buddy-plugin-plugin           | plugin           | dev-main | core     |
| manticoresoftware/buddy-plugin-test             | test             | dev-main | core     |
| manticoresoftware/buddy-plugin-show-hostname    | show-hostname    | dev-main | external |
+-------------------------------------------------+------------------+----------+----------+
14 rows in set (0.006 sec)
```

<!-- end -->
