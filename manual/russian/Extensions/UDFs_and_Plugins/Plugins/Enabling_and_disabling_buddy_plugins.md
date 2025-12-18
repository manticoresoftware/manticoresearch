# Включение и отключение плагинов Buddy

Для упрощения управления плагинами Buddy, особенно при разработке нового или модификации существующего, предоставляются команды включения и отключения плагинов Buddy. Эти команды действуют временно во время выполнения и сбрасываются к значениям по умолчанию после перезапуска демона или выполнения сброса Buddy. Для постоянного отключения плагина его необходимо удалить.

Для включения или отключения плагина необходим полностью квалифицированный пакетный имени плагина. Чтобы его найти, вы можете выполнить запрос `SHOW BUDDY PLUGINS` и найти полное квалифицированное имя в поле `package`. Например, плагин `SHOW` имеет полностью квалифицированное имя `manticoresoftware/buddy-plugin-show`.

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

Эта команда деактивирует активный плагин Buddy, препятствуя его дальнейшей обработке запросов.

<!-- intro -->
### Пример

<!-- request SQL -->
```sql
DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

<!-- intro -->
### Пример JSON

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show"
```

После отключения, если вы попробуете выполнить команду `SHOW QUERIES`, вы получите ошибку, потому что плагин отключен.
<!-- end -->

