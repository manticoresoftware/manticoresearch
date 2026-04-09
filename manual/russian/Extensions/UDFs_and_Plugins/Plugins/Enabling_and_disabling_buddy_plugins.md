# Включение и отключение плагинов Buddy

Чтобы упростить управление плагинами Buddy, особенно при разработке нового или изменении существующего, предоставляются команды включения и отключения плагинов Buddy. Эти команды действуют временно во время выполнения и сбрасываются к настройкам по умолчанию после перезапуска демона или выполнения сброса Buddy. Чтобы навсегда отключить плагин, его нужно удалить.

Вам необходимо полное квалифицированное имя пакета плагина для его включения или отключения. Чтобы его найти, вы можете выполнить запрос `SHOW BUDDY PLUGINS` и посмотреть полное квалифицированное имя в поле `package`. Например, плагин `SHOW` имеет полное квалифицированное имя `manticoresoftware/buddy-plugin-show`.

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

После отключения, если вы попробуете команду `SHOW QUERIES`, вы получите ошибку, потому что плагин отключён.
<!-- end -->

