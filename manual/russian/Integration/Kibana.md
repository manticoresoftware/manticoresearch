# Интеграция Manticore с Kibana

[Kibana](https://www.elastic.co/kibana) — это визуальный интерфейс, который позволяет исследовать, визуализировать и создавать дашборды для ваших лог-данных. Интеграция Kibana с Manticore Search может ускорить загрузку визуализаций Kibana до 3 раз по сравнению с Elasticsearch, как показано в этом [демо](https://github.com/manticoresoftware/kibana-demo#manticore-search-kibana-demo). Эта интеграция позволяет пользователям без лишних усилий анализировать данные с помощью интерактивных дашбордов, пользовательских визуализаций и возможностей поиска в реальном времени. Она также упрощает работу с разнородными источниками данных благодаря поддержке таких инструментов, как Logstash и Filebeat, для упрощенного импорта данных, что делает ее отличным выбором для рабочих процессов анализа логов.

## Необходимые условия
1. **Скачайте Kibana**: Убедитесь, что вы загружаете версию Kibana, совместимую с Manticore. На данный момент протестирована и рекомендуется версия 7.6.0. Другие версии 7.x могут работать, но способны вызвать проблемы. Версия 8.x не поддерживается.
2. **Проверьте Manticore**: Убедитесь, что ваш экземпляр Manticore запущен и его HTTP API доступен (по умолчанию: `http://localhost:9308`).

## Настройка
1. Откройте [файл конфигурации Kibana](https://www.elastic.co/guide/en/kibana/current/settings.html) (`kibana.yml`).
2. Укажите URL вашего экземпляра Manticore:
   ```yaml
   elasticsearch.hosts: ["http://localhost:9308"]
   ```
3. Запустите Kibana и откройте ее в браузере по адресу `http://localhost:5601`. При необходимости замените `localhost` на IP-адрес или имя хоста вашего сервера.

> Примечание: Manticore не требует настройки аутентификации при работе с Kibana. Также учтите, что для интеграции с Kibana Manticore должен работать в [режиме реального времени](../Read_this_first.md#Real-time-mode-vs-plain-mode).

### Пример конфигурации Manticore

```searchd {
    listen = 127.0.0.1:9308:http
    pid_file = /var/run/manticore/searchd.pid
    data_dir = /var/lib/manticore
 }
```

## Поддерживаемые возможности
### Discover
- Используйте вкладку **Discover** в Kibana, чтобы интерактивно искать и фильтровать данные.
- Уточняйте поиск с помощью строки запросов и простых запросов на [языке запросов Kibana](https://www.elastic.co/guide/en/kibana/current/kuery-query.html).

### Визуализации
- Перейдите в раздел **Visualizations**, чтобы создавать собственные визуализации:
  - Создайте шаблон таблицы (в Kibana он называется 'index pattern'), если его еще нет, чтобы определить источник данных.
  - Выберите тип визуализации (например, столбчатую диаграмму, линейный график или круговую диаграмму).
  - Настройте визуализацию, запустите ее и изучайте данные.
  - Сохраняйте визуализации для будущего использования.

### Дашборды
- Откройте **Dashboards**, чтобы создавать или просматривать интерактивные дашборды:
  - Добавляйте визуализации, фильтры или элементы управления для персонализированного опыта.
  - Работайте с данными прямо с дашборда.
  - Сохраняйте дашборды для будущего использования.

### Управление
- Перейдите в **Management > Kibana**, чтобы настроить такие параметры, как часовые пояса по умолчанию и предпочтения визуализации.

## Ограничения
- На данный момент протестирована и рекомендуется версия Kibana 7.6.0. Другие версии 7.x могут работать, но способны вызвать проблемы. Версии 8.x не поддерживаются.
- Следующие типы полей, специфичные для Elasticsearch, не поддерживаются:
  - [Пространственные типы данных](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#spatial_datatypes)
  - [Структурированные типы данных](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#structured-data-types)
  - [Типы ранжирования документов](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#document-ranking-types)
  - [Типы полнотекстового поиска](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#text-search-types) (кроме обычного 'text')
  - [Реляционные типы данных](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#object-types)
- Функции агрегации метрик ограничены [поддерживаемыми Manticore](../Searching/Grouping.md#Aggregation-functions).
- Следующие инструменты Kibana не поддерживаются:
  - [Canvas](https://www.elastic.co/guide/en/kibana/7.6/canvas.html) – инструмент для визуализации и презентаций, позволяющий объединять данные с цветами и изображениями.
  - [Elastic Maps](https://www.elastic.co/guide/en/kibana/7.6/maps.html) – инструмент для анализа географических данных.
  - [Metrics](https://www.elastic.co/guide/en/kibana/7.6/xpack-infra.html) – приложение для мониторинга метрик инфраструктуры.
  - [Logs](https://www.elastic.co/guide/en/kibana/7.6/xpack-logs.html) – представление в стиле консоли для изучения логов из распространенных сервисов.
  - Мониторинг:
    - [Uptime](https://www.elastic.co/guide/en/kibana/7.6/xpack-uptime.html) – отслеживает состояние сетевых конечных точек по HTTP/S, TCP и ICMP.
    - [APM (Application Performance Monitoring)](https://www.elastic.co/guide/en/kibana/7.6/xpack-apm.html) – собирает подробные метрики производительности приложений.
    - [SIEM (Security Information and Event Management)](https://www.elastic.co/guide/en/kibana/7.6/xpack-siem.html) – интерактивное рабочее пространство для команд безопасности, чтобы разбирать события и проводить первичные расследования.
    - [ILM (Index lifecycle management)](https://www.elastic.co/guide/en/elasticsearch/reference/7.6/index-lifecycle-management.html) - Автоматически управляет индексами в соответствии с требованиями к производительности, отказоустойчивости и сроку хранения.
    - [Stack Monitoring](https://www.elastic.co/guide/en/kibana/7.6/xpack-monitoring.html) – предоставляет визуализации данных мониторинга по всему Elastic Stack.
  - [Elasticsearch Management](https://www.elastic.co/guide/en/kibana/7.6/management.html) – интерфейс для управления объектами Elastic Stack, включая ILM (Index Lifecycle Management) и другие.

## Загрузка и исследование данных
Интегрируйте Manticore с такими инструментами, как [Logstash](../Integration/Logstash.md), [Filebeat](../Integration/Filebeat.md), [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/) или [Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/), чтобы загружать данные из таких источников, как веб-логи. После того как данные загружены в Manticore, вы можете изучать и визуализировать их в Kibana.

