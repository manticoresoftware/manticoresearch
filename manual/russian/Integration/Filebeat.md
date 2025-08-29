# Интеграция с Filebeat

> ПРИМЕЧАНИЕ: интеграция с Filebeat требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

[Filebeat](https://www.elastic.co/beats/filebeat) — это легкий агент для пересылки и централизованного сбора данных журналов. После установки в виде агента он следит за указанными вами файлами журналов или местоположениями, собирает события из журналов и пересылает их для индексирования, обычно в Elasticsearch или Logstash.

Теперь Manticore также поддерживает использование Filebeat в качестве обработчиков данных. Это позволяет отправлять собранные и преобразованные данные в Manticore так же, как в Elasticsearch. В настоящее время полностью поддерживаются все версии до 9.0.

## Конфигурация Filebeat

Конфигурация немного различается в зависимости от версии Filebeat, которую вы используете.

### Конфигурация для Filebeat 7.17 - 8.0

Обратите внимание, что в версиях Filebeat выше 8.10 по умолчанию включена функция сжатия вывода. Вот почему в конфигурационный файл необходимо добавить опцию `compression_level: 0` для обеспечения совместимости с Manticore:

```
filebeat.inputs:
- type: log
  enabled: true
  paths:
    - /var/log/dpkg.log
  close_eof: true
  scan_frequency: 1s

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```

### Конфигурация для Filebeat 8.1 - 8.10

Для версий с 8.1 по 8.10 необходимо добавить опцию allow_older_versions:

```
filebeat.inputs:
- type: log
  enabled: true
  paths:
    - /var/log/dpkg.log
  close_eof: true
  scan_frequency: 1s

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0
  allow_older_versions: true

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```

### Конфигурация для Filebeat 8.11 - 8.18

Начиная с версии 8.11, сжатие вывода включено по умолчанию, поэтому для совместимости с Manticore нужно явно установить `compression_level: 0`:

```
filebeat.inputs:
- type: log
  enabled: true
  paths:
    - /var/log/dpkg.log
  close_eof: true
  scan_frequency: 1s

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0
  allow_older_versions: true

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```

### Конфигурация для Filebeat 9.0

Filebeat 9.0 вводит значительные изменения в архитектуре, заменяя тип ввода log на filestream. Вот необходимая конфигурация:

```
filebeat.inputs:
- type: filestream
  id: dpkg-log-input
  enabled: true
  paths:
    - /var/log/dpkg.log
  prospector.scanner.check_interval: 1s
  close.on_eof: true

output.elasticsearch:
  hosts: ["http://localhost:9308"]
  index: "dpkg_log"
  compression_level: 0
  allow_older_versions: true

setup.ilm.enabled: false
setup.template.enabled: false
setup.template.name: "dpkg_log"
setup.template.pattern: "dpkg_log"
```

## Результаты работы Filebeat

После запуска Filebeat с этой конфигурацией данные журналов будут отправлены в Manticore и корректно индексированы. Ниже приведена итоговая схема таблицы, созданной Manticore, и пример вставленного документа:

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

