# Listing plugins

## SHOW PLUGINS
<!-- example Example -->

```sql
SHOW PLUGINS
```

Displays all the loaded plugins (except for Buddy plugins, see below) and UDFs. The "Type" column should be one of the `udf`, `ranker`, `index_token_filter`, or `query_token_filter`. The "Users" column is the number of thread that are currently using that plugin in a query. The "Extra" column is intended for various additional plugin-type specific information; currently, it shows the return type for the UDFs and is empty for all the other plugin types.


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
| Package                                         | Plugin           | Version  | Type     | Info                                                                                      |
+-------------------------------------------------+------------------+----------+----------+
| manticoresoftware/buddy-plugin-empty-string     | empty-string     | dev-main | core     | This plugin handles empty queries, which can occur when trimming comments or dealing with specific SQL protocol instructions in comments that are not supported. |
| manticoresoftware/buddy-plugin-backup           | backup           | dev-main | core     | Handles backup logic using the Manticore backup library.                                 |
| manticoresoftware/buddy-plugin-emulate-elastic  | emulate-elastic  | dev-main | core     | This plugin attempts to emulate Elastic queries and generate responses as if they were made by Elasticsearch. |
| manticoresoftware/buddy-plugin-insert           | insert           | dev-main | core     | Introduces auto schema support. When an insert operation is performed and the table does not exist, it creates it. |
| manticoresoftware/buddy-plugin-lock-tables      | lock-tables      | dev-main | core     | Implementation of lock tables that uses an internal freeze table query.                   |
| manticoresoftware/buddy-plugin-unlock-tables    | unlock-tables    | dev-main | core     | Implementation of unlock tables that uses an internal unfreeze command.                   |
| manticoresoftware/buddy-plugin-select           | select           | dev-main | core     | Handles various selects needed for mysqldump and various software support, aiming to work similarly to MySQL. |
| manticoresoftware/buddy-plugin-show             | show             | dev-main | core     | Handles various "show" queries, for example, introducing `show queries`, `show fields`, `show full tables`, etc. |
| manticoresoftware/buddy-plugin-cli-table        | cli-table        | dev-main | core     | Used to support rendering outputs as a table in CLI, similar to MySQL responses.          |
| manticoresoftware/buddy-plugin-plugin           | plugin           | dev-main | core     | Core logic for plugin support and helpers. Also handles `create buddy plugin`, `delete buddy plugin`, and `show buddy plugins`. |
| manticoresoftware/buddy-plugin-test             | test             | dev-main | core     | A test plugin, used exclusively for tests.                                                 |
| manticoresoftware/buddy-plugin-show-hostname    | show-hostname    | dev-main | external|                                                                                            |
+-------------------------------------------------+------------------+----------+----------+
14 rows in set (0.006 sec)
```

<!-- end -->
<!-- proofread -->
