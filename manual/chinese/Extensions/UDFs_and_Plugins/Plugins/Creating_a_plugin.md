# 系统插件
## CREATE PLUGIN

```sql
CREATE PLUGIN plugin_name TYPE 'plugin_type' SONAME 'plugin_library'
```

加载给定的库（如果尚未加载）并从中加载指定的插件。可用的插件类型包括：

- `ranker`
- `index_token_filter`
- `query_token_filter`

有关编写插件的更多信息，请参阅[插件](../../../Extensions/UDFs_and_Plugins/UDFs_and_Plugins.md#Plugins)文档。

```sql
mysql> CREATE PLUGIN myranker TYPE 'ranker' SONAME 'myplugins.so';
Query OK, 0 rows affected (0.00 sec)
```

## CREATE BUDDY PLUGIN

<!-- example create_buddy_plugin -->

Buddy 插件可以扩展 Manticore Search 的功能，并启用某些原生不支持的查询。要了解有关创建 Buddy 插件的更多信息，建议阅读[这篇文章](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/)。

要创建 Buddy 插件，请运行以下 SQL 命令：

```sql
CREATE PLUGIN <username/package name on https://packagist.org/> TYPE 'buddy' VERSION <package version>
```

> 注意：`CREATE BUDDY PLUGIN` 需要 [Manticore Buddy](../../../Installation/Manticore_Buddy.md)。如果不起作用，请确保 Buddy 已安装。

您还可以使用专门为 Buddy 插件创建的别名命令，这个命令更容易记住：

```sql
CREATE BUDDY PLUGIN <username/package name on https://packagist.org/> VERSION <package version>
```

此命令将把 `show-hostname` 插件安装到 [plugin_dir](../../../Server_settings/Common.md#plugin_dir) 并启用它，无需重启服务器。

<!-- intro -->
### 示例

<!-- request Example -->

```sql
CREATE PLUGIN manticoresoftware/buddy-plugin-show-hostname TYPE 'buddy' VERSION 'dev-main';

CREATE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname VERSION 'dev-main';
```

<!-- end -->
<!-- proofread -->

