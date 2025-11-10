# Логирование Docker

Когда вы используете [официальный образ Manticore для Docker](https://hub.docker.com/r/manticoresearch/manticore/), журнал сервера отправляется в `/dev/stdout`, который можно просмотреть с хоста с помощью:

```bash
docker logs manticore
```
Журнал запросов можно перенаправить в лог Docker, передав переменную `QUERY_LOG_TO_STDOUT=true`.

Папка для логов такая же, как и в случае с пакетом для Linux, установлена в `/var/log/manticore`. При желании её можно смонтировать в локальный путь для просмотра или обработки логов.
<!-- proofread -->

