# DELETE PLUGIN

```sql
DROP PLUGIN plugin_name TYPE 'plugin_type'
```

Отмечает указанный плагин для выгрузки. Процесс выгрузки **не** является мгновенным, поскольку одновременные запросы могут всё ещё использовать его. Тем не менее, после выполнения `DROP` новые запросы больше не смогут получить доступ к плагину. Впоследствии, когда все текущие запросы, использующие плагин, завершатся, плагин будет выгружен. Если все плагины из указанной библиотеки выгружены, библиотека также будет автоматически выгружена.

```sql
mysql> DROP PLUGIN myranker TYPE 'ranker';
Query OK, 0 rows affected (0.00 sec)
```

## DELETE BUDDY PLUGIN

<!-- example delete_buddy_plugin -->

```sql
DELETE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> ПРИМЕЧАНИЕ: `DELETE BUDDY PLUGIN` требует [Manticore Buddy](../../../Installation/Manticore_Buddy.md). Если команда не работает, убедитесь, что Buddy установлен.

Это действие мгновенно и навсегда удаляет установленный плагин из [plugin_dir](../../../Server_settings/Common.md#plugin_dir). После удаления функции плагина больше не будут доступны.

<!-- intro -->
### Пример

<!-- request Example -->
```sql
DELETE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname
```

<!-- end -->
<!-- proofread -->

