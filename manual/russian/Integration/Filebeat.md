# Интеграция с Filebeat

> ПРИМЕЧАНИЕ: Интеграция с Filebeat требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

[Filebeat](https://www.elastic.co/beats/filebeat) — это легковесный шиппер для передачи и централизованного хранения данных журналов. После установки в качестве агента он отслеживает файлы журналов или расположения, которые вы укажете, собирает события журналов и передает их на индексацию, обычно в Elasticsearch или Logstash.

Теперь Manticore также поддерживает использование Filebeat в качестве конвейеров обработки. Это позволяет отправлять собранные и преобразованные данные в Manticore так же, как и в Elasticsearch. В настоящее время поддерживаются все версии >= 7.10.

## Конфигурация Filebeat

Ниже представлена конфигурация Filebeat для работы с нашим примером журнала dpkg:

```
filebeat.inputs:
- type: filestream
  id: example
  paths:
	- /var/log/dpkg.log

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index:  "dpkg_log"
  allow_older_versions: true

setup.ilm:
  enabled: false

setup.template:
  name: "dpkg_log"
  pattern: "dpkg_log"
```

### Конфигурация для версий Filebeat >= 8.11

Обратите внимание, что в версиях Filebeat выше 8.10 функция сжатия вывода включена по умолчанию. Поэтому опция `compression_level: 0` должна быть добавлена в файл конфигурации для обеспечения совместимости с Manticore:

```
filebeat.inputs:
- type: filestream
  id: example
  paths:
	- /var/log/dpkg.log

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index:  "dpkg_log"
  allow_older_versions: true
  compression_level: 0

setup.ilm:
  enabled: false

setup.template:
  name: "dpkg_log"
  pattern: "dpkg_log"
```

## Результаты Filebeat

После того как вы запустите Filebeat с этой конфигурацией, данные журналов будут отправлены в Manticore и правильно проиндексированы. Вот схема таблицы, созданной Manticore, и пример вставленного документа:

```
mysql> DESCRIBE dpkg_log;
+------------------+--------+--------------------+
| Field            | Type   | Properties         |
+------------------+--------+--------------------+
| id               | bigint |                    |
| @timestamp       | text   | indexed stored     |
| message          | text   | indexed stored     |
| log              | json   |                    |
| input            | json   |                    |
| ecs              | json   |                    |
| host             | json   |                    |
| agent            | json   |                    |
+------------------+--------+--------------------+
```

```
mysql> SELECT * FROM dpkg_log LIMIT 1\G
*************************** 1. row ***************************
id: 7280000849080753116
@timestamp: 2023-06-16T09:27:38.792Z
message: 2023-04-12 02:06:08 status half-installed libhogweed5:amd64 3.5.1+really3.5.1-2
input: {"type":"filestream"}
ecs: {"version":"1.6.0"}
host: {"name":"logstash-db848f65f-lnlf9"}
agent: {"ephemeral_id":"587c2ebc-e7e2-4e27-b772-19c611115996","id":"2e3d985b-3610-4b8b-aa3b-2e45804edd2c","name":"logstash-db848f65f-lnlf9","type":"filebeat","version":"7.10.0","hostname":"logstash-db848f65f-lnlf9"}
log: {"offset":80,"file":{"path":"/var/log/dpkg.log"}}
```
