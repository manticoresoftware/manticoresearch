# Интеграция Manticore с Kibana

[Kibana](https://www.elastic.co/kibana) - это визуальный интерфейс, который позволяет вам исследовать, визуализировать и создавать панели управления для ваших логов. Интеграция Kibana с Manticore Search может ускорить загрузку визуализаций Kibana до 3 раз по сравнению с Elasticsearch, как показано в этой [демонстрации](https://github.com/manticoresoftware/kibana-demo#manticore-search-kibana-demo). Эта интеграция позволяет пользователям бесшовно анализировать свои данные с помощью интерактивных панелей управления, кастомных визуализаций и возможностей поиска в реальном времени. Она также упрощает работу с различными источниками данных, поддерживая такие инструменты, как Logstash и Filebeat для упрощённой загрузки данных, что делает её отличным выбором для рабочих процессов анализа логов.

## Предварительные требования
1. **Скачайте Kibana**: Убедитесь, что вы скачали версию Kibana, совместимую с Manticore. В настоящее время тестируется и рекомендуется версия 7.6.0. Другие версии 7.x могут работать, но могут вызвать проблемы. Версии 8.x не поддерживаются.
2. **Проверьте Manticore**: Убедитесь, что ваш экземпляр Manticore запущен и его HTTP API доступно (по умолчанию: `http://localhost:9308`).

## Конфигурация
1. Откройте [конфигурационный файл Kibana](https://www.elastic.co/guide/en/kibana/current/settings.html) (`kibana.yml`).
2. Установите URL вашего экземпляра Manticore:
   ```yaml
   elasticsearch.hosts: ["http://localhost:9308"]
   ```
3. Запустите Kibana и откройте его в браузере по адресу `http://localhost:5601`. Замените `localhost` на IP или имя хоста вашего сервера, если это необходимо.

> Примечание: Manticore не требует настройки аутентификации при работе с Kibana.

## Поддерживаемые функции
### Обзор
- Используйте вкладку **Обзор** в Kibana, чтобы интерактивно искать и фильтровать ваши данные.
- Уточняйте ваши поиски, используя строку запроса с простыми запросами на [языке запросов Kibana](https://www.elastic.co/guide/en/kibana/current/kuery-query.html).

### Визуализации
- Перейдите к **Визуализациям**, чтобы создать кастомные визуализации:
  - Создайте таблицу шаблона (в Kibana она называется 'шаблоном индекса'), если она ещё не существует, чтобы определить ваш источник данных.
  - Выберите тип визуализации (например, столбчатая диаграмма, линейная диаграмма или круговая диаграмма).
  - Настройте вашу визуализацию, выполните её и исследуйте ваши данные.
  - Сохраните ваши визуализации для будущего использования.

### Панели
- Перейдите к **Панелям**, чтобы создать или просмотреть интерактивные панели:
  - Добавьте визуализации, фильтры или элементы управления для персонализированного опыта.
  - Взаимодействуйте с вашими данными прямо с панели.
  - Сохраняйте панели для будущего использования.

### Управление
- Перейдите в **Управление > Kibana**, чтобы настроить такие параметры, как временные зоны по умолчанию и предпочтения визуализации.

## Ограничения
- В настоящее время тестируется и рекомендуется версия Kibana 7.6.0. Другие версии 7.x могут работать, но могут вызвать проблемы. Версии 8.x не поддерживаются.
- Следующие специфические для Elasticsearch типы полей не поддерживаются:
  - [Пространственные типы данных](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#spatial_datatypes)
  - [Структурированные типы данных](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#structured-data-types)
  - [Типы ранжирования документов](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#document-ranking-types)
  - [Типы текстового поиска](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#text-search-types) (кроме простого 'text')
  - [Реляционные типы данных](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#object-types)
- Функции агрегации метрик ограничены [теми, которые поддерживает Manticore](../Searching/Grouping.md#Aggregation-functions).
- Следующие инструменты Kibana не поддерживаются:
  - [Canvas](https://www.elastic.co/guide/en/kibana/7.6/canvas.html) – Инструмент визуализации и презентации для объединения данных с цветами и изображениями.
  - [Elastic Maps](https://www.elastic.co/guide/en/kibana/7.6/maps.html) – Инструмент для анализа географических данных.
  - [Metrics](https://www.elastic.co/guide/en/kibana/7.6/xpack-infra.html) – Приложение для мониторинга метрик инфраструктуры.
  - [Logs](https://www.elastic.co/guide/en/kibana/7.6/xpack-logs.html) – Дисплей, похожий на консоль, для исследования логов от распространённых сервисов.
  - Мониторинг:
    - [Uptime](https://www.elastic.co/guide/en/kibana/7.6/xpack-uptime.html) – Мониторит состояние сетевых конечных точек через HTTP/S, TCP и ICMP.
    - [APM (Мониторинг производительности приложений)](https://www.elastic.co/guide/en/kibana/7.6/xpack-apm.html) – Собиратель глубинных метрик производительности из приложений.
    - [SIEM (Управление безопасностью информации и событиями)](https://www.elastic.co/guide/en/kibana/7.6/xpack-siem.html) – Интерактивное рабочее пространство для команд безопасности для сортировки событий и проведения первоначальных расследований.
    - [ILM (Управление жизненным циклом индекса)](https://www.elastic.co/guide/en/elasticsearch/reference/7.6/index-lifecycle-management.html) - Автоматически управляет индексами в соответствии с требованиями к производительности, надежности и удержанию.
    - [Мониторинг стека](https://www.elastic.co/guide/en/kibana/7.6/xpack-monitoring.html) – Предоставляет визуализации данных мониторинга по всему Elastic Stack.
  - [Управление Elasticsearch](https://www.elastic.co/guide/en/kibana/7.6/management.html) – UI для управления объектами Elastic Stack, включая ILM (Управление жизненным циклом индекса) и др.

## Загрузка и исследование данных
Интегрируйте Manticore с инструментами, такими как [Logstash](../Integration/Logstash.md), [Filebeat](../Integration/Filebeat.md), [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/), или [Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/), чтобы загружать данные из таких источников, как веб-логи. Как только данные загружены в Manticore, вы можете исследовать и визуализировать их в Kibana.
