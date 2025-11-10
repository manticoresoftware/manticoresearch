# Включение и отключение плагинов Buddy

Для упрощения управления плагинами Buddy, особенно при разработке нового или изменении существующего, предоставлены команды включения и отключения плагинов Buddy. Эти команды действуют временно во время выполнения и сбрасываются к значениям по умолчанию после перезапуска демона или выполнения сброса Buddy. Для постоянного отключения плагина его необходимо удалить.

Для включения или отключения плагина требуется полное квалифицированное имя пакета плагина. Чтобы его найти, можно выполнить запрос `SHOW BUDDY PLUGINS` и посмотреть полное квалифицированное имя в поле `package`. Например, плагин `SHOW` имеет полное квалифицированное имя `manticoresoftware/buddy-plugin-show`.

<!-- example enable_buddy_plugin -->
## ENABLE BUDDY PLUGIN

```sql
ENABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> ПРИМЕЧАНИЕ: `ENABLE BUDDY PLUGIN` требует [Manticore Buddy](../../../Installation/Manticore_Buddy.md). Если команда не работает, убедитесь, что Buddy установлен.

Эта команда повторно активирует ранее отключённый плагин Buddy, позволяя ему снова обрабатывать ваши запросы.

<!-- intro -->
### Пример

<!-- request SQL -->
```sql
ENABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```
<!-- end -->

<!-- example disable_buddy_plugin -->
## DISABLE BUDDY PLUGIN

```sql
DISABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

Эта команда деактивирует активный плагин Buddy, предотвращая его обработку дальнейших запросов.

<!-- intro -->
### Пример

<!-- request SQL -->
```sql
DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

После отключения, если вы попробуете выполнить команду `SHOW QUERIES`, вы получите ошибку, так как плагин отключён.
<!-- end -->

