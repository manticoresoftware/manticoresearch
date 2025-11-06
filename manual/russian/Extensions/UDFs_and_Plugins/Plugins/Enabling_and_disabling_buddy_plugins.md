# Включение и отключение плагинов Buddy

Для упрощения управления плагинами Buddy, особенно при разработке нового или изменении существующего, предоставляются команды включения и отключения плагинов Buddy. Эти команды действуют временно во время выполнения и сбрасываются к значениям по умолчанию после перезапуска демона или выполнения сброса Buddy. Чтобы отключить плагин навсегда, его нужно удалить.

Для включения или отключения плагина требуется полностью квалифицированное имя пакета плагина. Чтобы найти его, можно выполнить запрос `SHOW BUDDY PLUGINS` и посмотреть полное квалифицированное имя в поле `package`. Например, плагин `SHOW` имеет полное квалифицированное имя `manticoresoftware/buddy-plugin-show`.

<!-- example enable_buddy_plugin -->
## ENABLE BUDDY PLUGIN

```sql
ENABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> ПРИМЕЧАНИЕ: `ENABLE BUDDY PLUGIN` требует [Manticore Buddy](../../../Installation/Manticore_Buddy.md). Если команда не работает, убедитесь, что Buddy установлен.

Эта команда повторно активирует ранее отключенный плагин Buddy, позволяя ему снова обрабатывать ваши запросы.

<!-- intro -->
### Пример SQL

<!-- request SQL -->
```sql
ENABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```
<!-- intro -->
### Пример JSON

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "ENABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show"
```

<!-- end -->

<!-- example disable_buddy_plugin -->
## DISABLE BUDDY PLUGIN

```sql
DISABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

Эта команда деактивирует активный плагин Buddy, предотвращая его обработку новых запросов.

<!-- intro -->

### Пример SQL
<!-- request SQL -->
```sql
DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

После отключения, если вы попробуете выполнить команду `SHOW QUERIES`, возникнет ошибка, так как плагин отключен.
<!-- intro -->
### Пример JSON

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show"
```

<!-- end -->

