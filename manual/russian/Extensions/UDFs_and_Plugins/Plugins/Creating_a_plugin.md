# Системные плагины
## CREATE PLUGIN

```sql
CREATE PLUGIN plugin_name TYPE 'plugin_type' SONAME 'plugin_library'
```

Загружает указанную библиотеку (если она ещё не загружена) и загружает из неё указанный плагин. Доступные типы плагинов включают:

- `ranker`
- `index_token_filter`
- `query_token_filter`

Для получения дополнительной информации о написании плагинов, пожалуйста, обратитесь к документации [plugins](../../../Extensions/UDFs_and_Plugins/UDFs_and_Plugins.md#Plugins).

```sql
mysql> CREATE PLUGIN myranker TYPE 'ranker' SONAME 'myplugins.so';
Query OK, 0 rows affected (0.00 sec)
```

## CREATE BUDDY PLUGIN

<!-- example create_buddy_plugin -->

Buddy плагины могут расширить функциональность Manticore Search и включить определённые запросы, которые не поддерживаются изначально. Чтобы узнать больше о создании Buddy плагинов, рекомендуем прочитать [эту статью](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/).

Чтобы создать Buddy плагин, выполните следующую команду SQL:

```sql
CREATE PLUGIN <username/package name on https://packagist.org/> TYPE 'buddy' VERSION <package version>
```

> ПРИМЕЧАНИЕ: `CREATE BUDDY PLUGIN` требует [Manticore Buddy](../../../Installation/Manticore_Buddy.md). Если команда не срабатывает, убедитесь, что Buddy установлен.

Вы также можете использовать алиас-команду, специально созданную для Buddy плагинов, которую легче запомнить:

```sql
CREATE BUDDY PLUGIN <username/package name on https://packagist.org/> VERSION <package version>
```

Эта команда установит плагин `show-hostname` в [plugin_dir](../../../Server_settings/Common.md#plugin_dir) и включит его без необходимости перезапуска сервера.

<!-- intro -->
### Примеры

<!-- request Example -->

```sql
CREATE PLUGIN manticoresoftware/buddy-plugin-show-hostname TYPE 'buddy' VERSION 'dev-main';

CREATE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname VERSION 'dev-main';
```

<!-- end -->
<!-- proofread -->

