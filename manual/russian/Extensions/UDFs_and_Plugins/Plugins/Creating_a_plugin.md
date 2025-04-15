# Плагины системы
## СОЗДАТЬ ПЛАГИН

```sql
CREATE PLUGIN plugin_name TYPE 'plugin_type' SONAME 'plugin_library'
```

Загружает данную библиотеку (если она еще не загружена) и загружает указанный плагин из нее. Доступные типы плагинов включают:

- `ranker`
- `index_token_filter`
- `query_token_filter`

Для получения дополнительной информации о написании плагинов, пожалуйста, обратитесь к документации [плагины](../../../Extensions/UDFs_and_Plugins/UDFs_and_Plugins.md#Plugins).

```sql
mysql> CREATE PLUGIN myranker TYPE 'ranker' SONAME 'myplugins.so';
Query OK, 0 rows affected (0.00 sec)
```

## СОЗДАТЬ ПЛАГИН С СОБАКОЙ

<!-- example create_buddy_plugin -->

Плагины с собаками могут расширить функциональные возможности Manticore Search и включить определенные запросы, которые не поддерживаются изначально. Чтобы узнать больше о создании плагинов с собаками, мы рекомендуем прочитать [эту статью](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/).

Чтобы создать плагин с собакой, выполните следующую SQL-команду:

```sql
CREATE PLUGIN <username/package name on https://packagist.org/> TYPE 'buddy' VERSION <package version>
```

> ЗАМЕТКА: `CREATE BUDDY PLUGIN` требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

Вы также можете использовать команду псевдонима, специально созданную для плагинов с собаками, которую легче запомнить:

```sql
CREATE BUDDY PLUGIN <username/package name on https://packagist.org/> VERSION <package version>
```

Эта команда установит плагин `show-hostname` в [plugin_dir](../../Server_settings/Common.md#plugin_dir) и активирует его без необходимости перезапуска сервера.

<!-- intro -->
### Примеры

<!-- request Example -->

```sql
CREATE PLUGIN manticoresoftware/buddy-plugin-show-hostname TYPE 'buddy' VERSION 'dev-main';

CREATE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname VERSION 'dev-main';
```

<!-- end -->
<!-- proofread -->
