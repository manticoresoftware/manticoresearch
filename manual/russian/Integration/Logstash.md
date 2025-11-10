# Интеграция с Logstash

> ПРИМЕЧАНИЕ: Интеграция с Logstash требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если интеграция не работает, убедитесь, что Buddy установлен.

[Logstash](https://www.elastic.co/logstash) — это инструмент управления логами, который собирает данные из различных источников, преобразует их на лету и отправляет в нужное место назначения. Часто используется как конвейер данных для Elasticsearch, открытого движка для аналитики и поиска.

Теперь Manticore поддерживает использование Logstash в качестве конвейера обработки. Это позволяет собранным и преобразованным данным передаваться в Manticore так же, как в Elasticsearch. В настоящее время поддерживаются версии 7.6–9.2.

Рассмотрим простой пример конфигурационного файла Logstash, используемого для индексирования `dpkg.log` — стандартного лог-файла менеджера пакетов Debian. Сам лог имеет простую структуру, как показано ниже:

```
2023-05-31 10:42:55 status triggers-awaited ca-certificates-java:all 20190405ubuntu1.1
2023-05-31 10:42:55 trigproc libc-bin:amd64 2.31-0ubuntu9.9 <none>
2023-05-31 10:42:55 status half-configured libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 status installed libc-bin:amd64 2.31-0ubuntu9.9
2023-05-31 10:42:55 trigproc systemd:amd64 245.4-4ubuntu3.21 <none>
```

## Конфигурация Logstash

Вот пример конфигурации Logstash:

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

Обратите внимание, прежде чем продолжить, необходимо учесть один важный момент: Manticore не поддерживает управление шаблонами логов (Log Template Management) и управление жизненным циклом индекса (Index Lifecycle Management), которые имеются в Elasticsearch. Поскольку эти функции по умолчанию включены в Logstash, их нужно явно отключить в конфигурации. Кроме того, опция hosts в разделе output конфигурации должна соответствовать HTTP-порту прослушивания Manticore (по умолчанию localhost:9308).

## Результаты Logstash

После внесения указанных изменений в конфиг вы можете запустить Logstash, и данные из лога dpkg будут переданы в Manticore и корректно индексированы.

Ниже приведена результирующая схема созданной таблицы и пример вставленного документа:

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

