# Включение и отключение плагинов Buddy

Чтобы упростить управление плагинами Buddy, особенно при разработке нового или изменении существующего, предоставляются команды для включения и отключения плагина Buddy. Эти команды действуют временно во время выполнения и будут сброшены к своим значениям по умолчанию после перезапуска демона или выполнения сброса Buddy. Чтобы永久но отключить плагин, его необходимо удалить.

Вам нужно полное имя пакета плагина, чтобы включить или отключить его. Чтобы найти его, вы можете выполнить запрос `SHOW BUDDY PLUGINS` и найти полное имя в поле `package`. Например, плагин `SHOW` имеет полное имя `manticoresoftware/buddy-plugin-show`.

<!-- example enable_buddy_plugin -->
## ВКЛЮЧИТЬ ПЛАГИН BUDDY

```sql
ENABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> ЗАМЕТКА: `ENABLE BUDDY PLUGIN` требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не сработает, убедитесь, что Buddy установлен.

Эта команда повторно активирует ранее отключенный плагин Buddy, позволяя ему снова обрабатывать ваши запросы.

<!-- intro -->
### Пример

<!-- request SQL -->
```sql
ENABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```
<!-- end -->

<!-- example disable_buddy_plugin -->
## ОТКЛЮЧИТЬ ПЛАГИН BUDDY

```sql
DISABLE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

Эта команда деактивирует активный плагин Buddy, предотвращая его дальнейшую обработку запросов.

<!-- intro -->
### Пример

<!-- request SQL -->
```sql
DISABLE BUDDY PLUGIN manticoresoftware/buddy-plugin-show
```

После отключения, если вы попробуете выполнить команду `SHOW QUERIES`, вы получите ошибку, потому что плагин отключен.
<!-- end -->
