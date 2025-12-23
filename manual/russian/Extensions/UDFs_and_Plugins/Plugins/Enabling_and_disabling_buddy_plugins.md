# Включение и отключение плагинов Buddy

Для упрощения управления плагинами Buddy, особенно при разработке нового плагина или изменении существующего, предоставлены команды включения и отключения плагина Buddy. Эти команды действуют временно во время работы и сбрасываются к значениям по умолчанию после перезапуска демона или выполнения сброса Buddy. Чтобы навсегда отключить плагин, его необходимо удалить.

Вам необходимо полное квалифицированное имя пакета плагина, чтобы включить или отключить его. Чтобы найти его, вы можете выполнить запрос `SHOW BUDDY PLUGINS` и посмотреть полное квалифицированное имя в поле `package`. Например, плагин `SHOW` имеет полное квалифицированное имя `manticoresoftware/buddy-plugin-show`.

<!-- example enable_buddy_plugin -->
## ENABLE BUDDY PLUGIN

```sql
ENABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> ПРИМЕЧАНИЕ: `ENABLE BUDDY PLUGIN` требует [Manticore Buddy](../../../Installation/Manticore_Buddy.md). Если команда не работает, убедитесь, что Buddy установлен.

Эта команда повторно активирует ранее отключённый плагин Buddy, позволяя ему снова обрабатывать ваши запросы.

<!-- intro -->
### Пример на SQL

<!-- request SQL -->
```sql
ENABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

<!-- intro -->
### Пример на JSON

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

Эта команда деактивирует активный плагин Buddy, предотвращая обработку им дальнейших запросов.

<!-- intro -->
### Пример

<!-- request SQL -->
```sql
DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

<!-- intro -->
### Пример на JSON

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show"
```

После отключения, если попробовать выполнить команду `SHOW QUERIES`, вы столкнётесь с ошибкой, так как плагин отключён.
<!-- end -->

