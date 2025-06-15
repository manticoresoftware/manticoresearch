# Интеграция с Logstash

> ПРИМЕЧАНИЕ: Интеграция с Logstash требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если интеграция не работает, убедитесь, что Buddy установлен.

[Logstash](https://www.elastic.co/logstash) — это инструмент управления журналами, который собирает данные из различных источников, преобразует их на лету и отправляет в нужное место назначения. Он часто используется как канал передачи данных для Elasticsearch — открытой системы аналитики и поиска.

Теперь Manticore поддерживает использование Logstash как конвейера обработки. Это позволяет отправлять собранные и преобразованные данные в Manticore так же, как и в Elasticsearch. В настоящее время поддерживаются версии 7.6-7.15.

Рассмотрим простой пример конфигурационного файла Logstash, используемого для индексирования `dpkg.log` — стандартного файла журнала менеджера пакетов Debian. Сам журнал имеет простую структуру, как показано ниже:

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Конфигурация Logstash

Пример конфигурации Logstash:

```
input {
  file {
    path => ["/var/log/dpkg.log"]
    start_position => "beginning"
    sincedb_path => "/dev/null"
    mode => "read"
    exit_after_read => "true"
   file_completed_action => "log"
   file_completed_log_path => "/dev/null"
  }
}

output {
  elasticsearch {
   index => " dpkg_log"
   hosts => ["http://localhost:9308"]
   ilm_enabled => false
   manage_template => false
  }
}
```

Обратите внимание, что перед продвижением далее необходимо учитывать важное замечание: Manticore не поддерживает управление шаблонами логов (Log Template Management) и управление жизненным циклом индекса (Index Lifecycle Management) в Elasticsearch. Поскольку эти функции включены по умолчанию в Logstash, их необходимо явно отключить в конфигурации. Кроме того, параметр hosts в секции вывода (output) должен соответствовать HTTP-порту прослушивания Manticore (по умолчанию localhost:9308).

## Результаты Logstash

После корректировки конфигурации, как описано выше, можно запустить Logstash, и данные из журнала dpkg будут переданы в Manticore и корректно проиндексированы.

Ниже приведена итоговая схема созданной таблицы и пример вставленного документа:

```
mysql> DESCRIBE dpkg_log;
+------------------+--------+---------------------+
| Field            | Type   | Properties          |
+------------------+--------+---------------------+
| id               | bigint |                     |
| message          | text   | indexed stored      |
| @version         | text   | indexed stored      |
| @timestamp       | text   | indexed stored      |
| path             | text   | indexed stored      |
| host             | text   | indexed stored      |
+------------------+--------+---------------------+
```

```
mysql> SELECT * FROM dpkg_log LIMIT 1\G

*************************** 1. row ***************************
id: 7280000849080746110
host: logstash-db848f65f-lnlf9
message: 2023-04-12 02:03:21 status unpacked libc-bin:amd64 2.31-0ubuntu9
path: /var/log/dpkg.log
@timestamp: 2023-06-16T09:23:57.405Z
@version: 1
```

