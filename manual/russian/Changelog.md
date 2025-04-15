# Изменения

# Версия 9.2.14
Выпущена: 28 марта 2025 года

### Небольшие изменения
* [Коммит ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) Реализован флаг `--mockstack` для расчета требований к стеку для рекурсивных операций. Режим `--mockstack` анализирует и сообщает необходимые размеры стека для оценки рекурсивных выражений, операций сопоставления шаблонов, обработки фильтров. Расчетные требования к стеку выводятся в консоль для отладки и оптимизации.
* [Проблема #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) Включен [boolean_simplify](../Searching/Options.md#boolean_simplify) по умолчанию.
* [Проблема #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) Добавлена новая настройка конфигурации: `searchd.kibana_version_string`, которая может быть полезна при использовании Manticore с конкретными версиями Kibana или OpenSearch Dashboards, которые ожидают определенную версию Elasticsearch.
* [Проблема #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) Исправлена [CALL SUGGEST](../Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) для работы с двухсимвольными словами.
* [Проблема #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) Улучшен [нечеткий поиск](../Searching/Spell_correction.md#Fuzzy-Search): ранее он иногда не находил "defghi" при поиске "def ghi", если существовал другой соответствующий документ.
* ⚠️ ЛОМКА [Проблема #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) Изменено `_id` на `id` в некоторых ответах HTTP JSON для согласованности. Убедитесь, что вы обновили свое приложение соответствующим образом.
* ⚠️ ЛОМКА [Проблема #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) Добавлена проверка для `server_id` во время присоединения к кластерам, чтобы убедиться, что каждый узел имеет уникальный идентификатор. Операция `JOIN CLUSTER` теперь может завершиться с сообщением об ошибке, указывающим на дублирующий [server_id](../Server_settings/Searchd.md#server_id), когда присоединяющийся узел имеет тот же `server_id`, что и существующий узел в кластере. Чтобы решить эту проблему, убедитесь, что каждый узел в кластере репликации имеет уникальный [server_id](../Server_settings/Searchd.md#server_id). Вы можете изменить значение по умолчанию [server_id](../Server_settings/Searchd.md#server_id) в разделе "searchd" вашего конфигурационного файла на уникальное значение перед тем, как пытаться присоединиться к кластеру. Это изменение обновляет протокол репликации. Если вы запускаете кластер репликации, вам нужно:
  - Сначала аккуратно остановить все ваши узлы
  - Затем запустить узел, который был остановлен последним, с помощью `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Прочитайте о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для получения дополнительной информации.

### Исправления ошибок
* [Коммит 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) Исправлен сбой, вызванный потерей планировщика после ожидания; теперь определенные планировщики, такие как `serializer`, правильно восстанавливаются.
* [Коммит c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) Исправлена ошибка, из-за которой веса из правой присоединенной таблицы не могли использоваться в предложении `ORDER BY`.
* [Проблема #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0: исправлена ошибка вызова `lower_bound` с `const knn::DocDist_t*&`. ❤️ Спасибо [@Azq2](https://github.com/Azq2) за PR.
* [Проблема #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) Исправлена проблема с обработкой имен таблиц с заглавными буквами во время автоматических вставок схемы.
* [Проблема #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) Исправлен сбой при декодировании недопустимого входа base64.
* [Проблема #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) Исправлены две связанные проблемы индексации KNN на `ALTER`: векторы с плавающей точкой теперь сохраняют свои оригинальные размеры, и индексы KNN теперь правильно создаются.
* [Проблема #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) Исправлен сбой при создании вторичного индекса на пустом JSON-столбце.
* [Проблема #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) Исправлен сбой, вызванный дублирующими записями.
* [Проблема #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) Исправлено: опция `fuzzy=1` не могла использоваться с `ranker` или `field_weights`.
* [Проблема #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) Исправлена ошибка, из-за которой `SET GLOBAL timezone` не имел эффекта.
* [Проблема #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) Исправлена проблема, из-за которой значения текстовых полей могли быть потеряны при использовании идентификаторов, превышающих 2^63.
* [Проблема #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) Исправлено: операторы `UPDATE` теперь корректно учитывают настройку `query_log_min_msec`.
* [Проблема #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) Исправлено состояние гонки при сохранении реальных дисковых сегментов, которое могло привести к сбоям `JOIN CLUSTER`.

# Версия 7.4.6
Выпущена: 28 февраля 2025 года

### Ключевые изменения
* [Проблема #832](https://github.com/manticoresoftware/manticoresearch/issues/832) Интеграция с [Kibana](../Integration/Kibana.md) для более простого и эффективного визуализирования данных.

### Небольшие изменения
* [Проблема #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) Исправлены различия в точности плавающей точки между arm64 и x86_64.
* [Проблема #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) Реализованы оптимизации производительности для пакетирования соединений.
* [Проблема #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) Реализованы оптимизации производительности для EstimateValues в гистограммах.
* [Проблема #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) Добавлена поддержка Boost 1.87.0. ❤️ Спасибо, [@cho-m](https://github.com/cho-m) за PR.
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) Оптимизирован повторное использование блочных данных при создании фильтров с несколькими значениями; добавлены min/max в метаданные атрибутов; реализовано предварительное фильтрование значений фильтров на основе min/max.

### Исправления ошибок
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) Исправлено обработка выражений в объединенных запросах, когда используются атрибуты из обеих таблиц; исправлена опция index_weights для правой таблицы.
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) Использование `avg()` в запросе `SELECT ... JOIN` могло привести к неправильным результатам; теперь это исправлено.
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) Исправлен неверный набор результатов, вызванный неявным ограничением, когда группировка объединений была включена.
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) Исправлен сбой в демоне во время завершения работы, когда выполнялось активное объединение чанков.
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) Исправлена проблема, когда `IN(...)` мог давать неправильные результаты.
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) Установка `max_iops` / `max_iosize` в версии 7.0.0 могла ухудшить производительность индексации; теперь это исправлено.
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) Исправлен утечка памяти в кэше запросов объединения.
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) Исправлено обработка параметров запроса в объединенных запросах JSON.
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) Исправлена проблема с командой ATTACH TABLE.
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) Исправлены несоответствия в сообщениях об ошибках.
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) Установка `diskchunk_flush_write_timeout=-1` для каждой таблицы не отключала сброс индекса; теперь это исправлено.
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) Разрешены дублирующиеся записи после массовой замены больших ID.
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) Исправлен сбой демона, вызванный полнотекстовым запросом с единственным оператором `NOT` и оценщиком выражений.
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) Исправлена потенциальная уязвимость в библиотеке CJSON. ❤️ Спасибо, [@tabudz](https://github.com/tabudz) за PR.

# Версия 7.0.0
Выпущено: 30 января 2025 года

### Основные изменения
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) Добавлены новые функции [Нечеткого поиска](../Searching/Spell_correction.md#Fuzzy-Search) и [Авозаполнения](../Searching/Autocomplete.md#CALL-AUTOCOMPLETE) для упрощения поиска.
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Интеграция с Kafka](../Integration/Kafka.md#Syncing-from-Kafka).
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) Введены [вторичные индексы для JSON](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes).
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) Обновления и запросы во время обновлений больше не блокируются объединением чанков.
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) Автоматический [сброс чанка диска](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) для RT таблицы для улучшения производительности; теперь мы автоматически сбрасываем RAM чанк в чанк диска, предотвращая проблемы с производительностью, вызванные отсутствием оптимизаций в RAM чанках, что иногда может привести к нестабильности в зависимости от размера чанка.
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) Опция [Прокрутки](../Searching/Pagination.md#Scroll-Search-Option) для удобной пагинации.
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) Интеграция с [Jieba](https://github.com/fxsjy/jieba) для лучшей [токенизации на китайском](../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

### Небольшие изменения
* ⚠️ ЛОМКА [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) Исправлена поддержка `global_idf` в RT таблицах. Требуется пересоздание таблицы.
* ⚠️ ЛОМКА [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Удалены тайские символы из внутреннего набора символов `cjk`. Обновите ваши определения набора символов соответственно: если у вас есть `cjk,non_cjk` и тайские символы важны для вас, измените на `cjk,thai,non_cjk`, или `cont,non_cjk`, где `cont` - новое обозначение для всех языков с непрерывным письмом (т.е. `cjk` + `thai`). Измените существующие таблицы, используя [ALTER TABLE](../Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode).
* ⚠️ ЛОМКА [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](../Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) теперь совместимы с распределенными таблицами. Это увеличивает версию протокола мастер/агент. Если вы запускаете Manticore Search в распределенной среде с несколькими экземплярами, убедитесь, что сначала обновите агентов, а затем мастеров.
* ⚠️ ЛОМКА [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) Изменено имя столбца с `Name` на `Variable name` для PQ [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables).
* ⚠️ НОВОЕ [Проблема #879](https://github.com/manticoresoftware/manticoresearch/issues/879) Введен [двойной журнал для каждой таблицы](../Logging/Binary_logging.md#Per-table-binary-logging-configuration) с новыми опциями: [binlog_common](../Logging/Binary_logging.md#Binary-logging-strategies), [binlog](../Logging/Binary_logging.md#Per-table-binary-logging-configuration) для `create table` / `alter table`. Вам необходимо выполнить чистое завершение работы экземпляра Manticore перед обновлением до новой версии.
* ⚠️ НОВОЕ [Проблема #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) Исправлено неправильное сообщение об ошибке, когда узел присоединяется к кластеру с неправильной версией протокола репликации. Это изменение обновляет протокол репликации. Если вы используете кластер репликации, вам необходимо:
  - Прежде всего, корректно остановить все ваши узлы
  - Затем запустите узел, который был остановлен последним, с `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Ознакомьтесь с [перезапуском кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для получения более подробной информации.
* ⚠️ НОВОЕ [Проблема #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) Добавлена поддержка нескольких таблиц в [`ALTER CLUSTER ADD` и `DROP`](../Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster). Это изменение также влияет на протокол репликации. Обратитесь к предыдущему разделу за рекомендациями по обработке этого обновления.
* [Проблема #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) Исправлена проблема с dlopen на Macos.
* [Коммит 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) Изменен стандартный предел для OPTIMIZE TABLE для таблиц с индексами KNN для улучшения производительности поиска.
* [Коммит cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) Добавлена поддержка `COUNT(DISTINCT)` для `ORDER BY` в `FACET` и `GROUP BY`.
* [Проблема #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) Улучшена ясность в [логировании](../Logging/Server_logging.md#Server-logging) слияния чанков.
* [Проблема #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка для [DBeaver](../Integration/DBeaver.md).
* [Проблема #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) Реализованы вторичные индексы для функций [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29).
* [Проблема #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP-запросы теперь поддерживают `Content-Encoding: gzip`.
* [Проблема #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) Добавлена команда `SHOW LOCKS`.
* [Проблема #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) Разрешен запрос Buddy к демону для обхода ограничения [searchd.max_connections](../Server_settings/Searchd.md#max_connections).
* [Проблема #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) Добавлена поддержка соединений таблиц через JSON HTTP интерфейс.
* [Проблема #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) Логирование успешно обработанных запросов через Buddy в их оригинальной форме.
* [Проблема #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) Добавлен специальный режим для выполнения `mysqldump` для реплицируемых таблиц.
* [Проблема #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) Улучшено переименование внешних файлов при копировании для операторов `CREATE TABLE` и `ALTER TABLE`.
* [Проблема #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) Обновлен стандартный [searchd.max_packet_size](../Server_settings/Searchd.md#max_packet_size) до 128 МБ.
* [Проблема #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) Добавлена поддержка [модификатора увеличения IDF](../Searching/Full_text_matching/Operators.md#IDF-boost-modifier) в JSON ["match"](../Searching/Full_text_matching/Basic_usage.md#match).
* [Проблема #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) Улучшена синхронизация записи [binlog](../Logging/Binary_logging.md#Binary-logging) для предотвращения ошибок.
* [Проблема #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) Включена поддержка zlib в пакетах для Windows.
* [Проблема #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) Добавлена поддержка команды SHOW TABLE INDEXES.
* [Проблема #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) Установлены метаданные сессии для ответов Buddy.
* [Проблема #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) Миллисекундное разрешение для агрегатов на конечных точках совместимости.
* [Проблема #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) Изменены сообщения об ошибках для операций с кластером, когда не удается запустить репликацию.
* [Проблема #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) Новые [метрики производительности в SHOW STATUS](../Node_info_and_management/Node_status.md#Query-Time-Statistics): мин/макс/среднее/95-й/99-й процентиль для каждого типа запроса за последние 1, 5 и 15 минут.
* [Проблема #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) Все вхождения `index` заменены на `table` в запросах и ответах.
* [Проблема #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) Добавлен столбец `distinct` в результаты агрегации HTTP `/sql`.
* [Проблема #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) Реализована автодетекция типов данных, импортируемых из Elasticsearch.
* [Проблема #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) Добавлена поддержка сопоставления строк в выражениях JSON полей.
* [Проблема #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) Добавлена поддержка выражения `uuid_short` в списке выборки.
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search теперь запускает Buddy непосредственно без обертки `manticore-buddy`.
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) Различные сообщения об ошибках для отсутствующих таблиц и таблиц, которые не поддерживают операции вставки.
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 теперь статически встроен в `searchd`.
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) Добавлен оператор `CALL uuid_short` для генерации последовательностей с несколькими значениями `uuid_short`.
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) Добавлены отдельные параметры для правой таблицы в операции JOIN.
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) Улучшена производительность агрегации HTTP JSON, чтобы соответствовать `GROUP BY` в SphinxQL.
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) Добавлена поддержка `fixed_interval` в запросах Kibana, связанных с датами.
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) Реализовано пакетирование для запросов JOIN, что улучшает производительность определенных запросов JOIN в сотни или даже тысячи раз.
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) Включена возможность использования веса объединенной таблицы в запросах fullscan.
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) Исправлено логирование для запросов join.
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) Скрыты исключения Buddy из логов `searchd` в режиме без отладки.
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) Демон завершает работу с сообщением об ошибке, если пользователь задает неправильные порты для прослушивателя репликации.

### Исправления ошибок
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) Исправлено: Неверные результаты в запросах JOIN с более чем 32 столбцами.
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) Решена проблема, из-за которой объединение таблиц не выполнялось, когда в условии использовались два json-атрибута.
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) Исправлено неверное значение total_relation в мультизапросах с [cutoff](../Searching/Options.md#cutoff).
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) Исправлено фильтрация по `json.string` в правой таблице при [объединении таблиц](../Searching/Joining.md).
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) Включено использование `null` для всех значений в любых POST HTTP JSON конечных точках (insert/replace/bulk). В этом случае используется значение по умолчанию.
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) Оптимизировано потребление памяти путем настройки выделения сетевого буфера [max_packet_size](../Server_settings/Searchd.md#max_packet_size) для начального зондирования сокета.
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) Исправлено вставка беззнакового целого в атрибут bigint через интерфейс JSON.
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) Исправлено поведение вторичных индексов при работе с фильтрами исключения и включенной псевдо-шардинговой.
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) Устранена ошибка в [manticore_new_cluster](../Starting_the_server/Manually.md#searchd-command-line-options).
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) Исправлен сбой демона на неправильно сформированном запросе `_update`.
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) Исправлена неспособность гистограмм обрабатывать фильтры значений с исключениями.
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) Исправлены запросы [knn](../Searching/KNN.md#KNN-vector-search) к распределенным таблицам.
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) Улучшена обработка фильтров исключения при кодировании таблицы в колонном доступе.
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) Исправлен парсер выражений, не подчиняющийся переопределенному `thread_stack`.
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) Исправлен сбой при клонировании выражения IN в колонном доступе.
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) Исправлена проблема инверсии в итераторе битмапа, что приводило к сбою.
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) Исправлена проблема, из-за которой некоторые пакеты Manticore автоматически удалялись при `unattended-upgrades`.
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) Улучшена обработка запросов от инструмента DbForge MySQL.
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) Исправлено экранирование специальных символов в `CREATE TABLE` и `ALTER TABLE`.  ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) Исправлена взаимная блокировка при обновлении атрибута blob в зафиксированном индексе. Взаимная блокировка произошла из-за конфликтующих блокировок при попытке разморозить индекс. Это могло привести к сбою и в manticore-backup.
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) `OPTIMIZE` теперь выдает ошибку, когда таблица зафиксирована.
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) Разрешены имена функций для использования в качестве имен столбцов.
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) Исправлен сбой демона при запросе настроек таблицы с неизвестным дисковым чанком.
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) Исправлена проблема, из-за которой `searchd` зависал при остановке после `FREEZE` и `FLUSH RAMCHUNK`.
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) Удалены токены, связанные с датой/временем (и регулярные выражения), из зарезервированных слов.
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) Исправлен сбой при использовании `FACET` с более чем 5 полями сортировки.
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) Исправлена ошибка восстановления `mysqldump` с включенными `index_field_lengths`.
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) Исправлен сбой при выполнении команды `ALTER TABLE`.
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) Исправлен MySQL DLL в пакетe для Windows, чтобы индексатор работал правильно.
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) Исправлена ошибка компиляции GCC. ❤️ Спасибо, [@animetosho](https://github.com/animetosho) за PR.
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) Исправлена проблема с экранированием в [_update](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial).
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) Исправлен сбой индексатора при объявлении нескольких атрибутов или полей с одинаковым именем.
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) Исправлен сбой демона при неправильной трансформации для вложенных булевых запросов для конечных точек, связанных с "совместимостью".
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) Исправлено расширение в фразах с модификаторами.
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) Исправлен сбой демона при использовании полнотекстового оператора [ZONE](../Searching/Full_text_matching/Operators.md#ZONE-limit-operator) или [ZONESPAN](../Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator).
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) Исправлено создание инфиксов для обычных и RT таблиц с словарем ключевых слов.
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) Исправлен ошибочный ответ в запросе `FACET`; установлен порядок сортировки по умолчанию на `DESC` для `FACET` с `COUNT(*)`.
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) Исправлен сбой демона на Windows при запуске.
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) Исправлено усечение запроса для HTTP конечных точек `/sql` и `/sql?mode=raw`; запросы с этих конечных точек сделаны согласованными без требования заголовка `query=`.
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) Исправлена проблема, при которой авто-схема создает таблицу, но одновременно не удается это сделать.
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) Исправлена библиотека HNSW для поддержки загрузки нескольких KNN индексов.
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) Исправлена проблема с зависанием, когда несколько условий возникают одновременно.
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) Исправлен сбой с фатальной ошибкой при использовании `OR` с KNN поиском.
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) Исправлена проблема, при которой `indextool --mergeidf *.idf --out global.idf` удаляет выходной файл после создания.
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) Исправлен сбой демона при подвыборке с `ORDER BY` строкой во внешнем выборе.
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) Исправлен сбой при обновлении атрибута float вместе с атрибутом строки.
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) Исправлена проблема, когда несколько стоп-слов из `lemmatize_xxx_all` токенизаторов увеличивают `hitpos` последующих токенов.
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) Исправлен сбой при `ALTER ... ADD COLUMN ... TEXT`.
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) Исправлена проблема, при которой обновление атрибута blob в замороженной таблице с хотя бы одним RAM chunk вызывает ожидание последующих `SELECT` запросов, пока таблица не будет разморожена.
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) Исправлен пропуск кэша запросов для запросов с упакованными факторами.
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore теперь сообщает об ошибке при неизвестном действии вместо сбоя на запросах `_bulk`.
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) Исправлено возвращение ID вставленного документа для HTTP конечной точки `_bulk`.
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) Исправлен сбой в группировщике при обработке нескольких таблиц, одна из которых пуста, а другая имеет другое количество совпадающих записей.
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) Исправлен сбой в сложных запросах `SELECT`.
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) Добавлено сообщение об ошибке, если аргумент `ALL` или `ANY` в выражении `IN` не является JSON атрибутом.
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) Исправлен сбой демона при обновлении MVA в больших таблицах.
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) Исправлен сбой при неудаче токенизации с `libstemmer`. ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, при которой объединенный вес из правой таблицы не работал корректно в выражениях.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, из-за которой вес таблицы с правым соединением не работал в выражениях.
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) Исправлена ошибка `CREATE TABLE IF NOT EXISTS ... WITH DATA`, когда таблица уже существует.
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) Исправлена ошибка неопределенного ключа массива "id" при подсчете по KNN с идентификатором документа.
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) Исправлена функциональность `REPLACE INTO cluster_name:table_name`.
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) Исправлена фатальная ошибка при запуске контейнера Manticore Docker с `--network=host`.

# Версия 6.3.8
Выпущена: 22 ноября 2024 года

Версия 6.3.8 продолжает серию 6.3 и включает только исправления ошибок.

### Исправления ошибок

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) Исправлен расчет доступных потоков, когда параллельность запросов ограничена настройками `threads` или `max_threads_per_query`.

# manticore-extra v1.1.20

Выпущен: 7 октября 2024 года

### Исправления ошибок

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) Решена проблема, при которой утилита `unattended-upgrades`, автоматически устанавливающая обновления пакетов в системах на основе Debian, ошибочно помечала несколько пакетов Manticore, включая `manticore-galera`, `manticore-executor` и `manticore-columnar-lib`, для удаления. Это происходило из-за того, что `dpkg` ошибочно считал виртуальный пакет `manticore-extra` избыточным. Были внесены изменения, чтобы гарантировать, что `unattended-upgrades` больше не пытается удалить важные компоненты Manticore.

# Версия 6.3.6
Выпущена: 2 августа 2024 года

Версия 6.3.6 продолжает серию 6.3 и включает только исправления ошибок.

### Исправления ошибок

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) Исправлен сбой, вызванный в версии 6.3.4, который мог происходить при работе с выражениями и распределенными или несколькими таблицами.
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) Исправлен сбой демона или внутренняя ошибка при раннем выходе, вызванном `max_query_time`, при запросах к нескольким индексам.

# Версия 6.3.4
Выпущена: 31 июля 2024 года

Версия 6.3.4 продолжает серию 6.3 и включает только незначительные улучшения и исправления ошибок.

### Незначительные изменения
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка [DBeaver](../Integration/DBeaver.md#Integration-with-DBeaver).
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) Улучшено экранирование разделителей в формах слов и исключениях.
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) Добавлены операторы сравнения строк в выражения списка SELECT.
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) Добавлена поддержка значений null в запросах Elastic-like bulk.
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) Добавлена поддержка mysqldump версии 9.
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) Улучшена обработка ошибок в HTTP JSON запросах с JSON путем к узлу, где происходит ошибка.

### Исправления ошибок
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) Исправлено снижение производительности в запросах с подстановочными символами с множеством совпадений, когда disk_chunks > 1.
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) Исправлен сбой в выражениях MIN или MAX списка SELECT для пустых MVA массивов.
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) Исправлена некорректная обработка запроса бесконечного диапазона Kibana.
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) Исправлен фильтр соединения по колоннарным атрибутам из правой таблицы, когда атрибут не входит в список SELECT.
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) Исправлен дублирующийся спецификатор 'static' в Manticore 6.3.2.
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) Исправлен LEFT JOIN, возвращающий не соответствующие записи, когда MATCH() используется по правой таблице.
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) Исправлено сохранение дискового куска на RT индексе с `hitless_words`.
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) Свойство 'aggs_node_sort' теперь может добавляться в любом порядке среди других свойств.
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) Исправлена ошибка на порядок full-text и filter в JSON запросе.
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) Исправлена ошибка, связанная с некорректным JSON ответом для длинных запросов UTF-8.
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) Исправлен расчет выражений presort/prefilter, которые зависят от объединенных атрибутов.
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) Изменен метод расчета размера данных для метрик на чтение из файла `manticore.json` вместо проверки общего размера каталога данных.
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) Добавлена обработка запросов проверки подлинности от Vector.dev.

# Версия 6.3.2
Выпущена: 26 июня 2024 года

Версия 6.3.2 продолжает серию 6.3 и включает несколько исправлений ошибок, некоторые из которых были обнаружены после выпуска 6.3.0.

### Ломаемые изменения
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) Обновлены значения aggs.range до числовых.

### Исправления ошибок
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) Исправлено группирование по сохраненной проверке против объединения rset.
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) Исправлен сбой в демоне при запросе с использованием символов подстановки в RT индексе с включенным словарем CRC и `local_df`.
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) Исправлен сбой в JOIN при использовании `count(*)` без GROUP BY.
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) Исправлено, что JOIN не возвращал предупреждение при попытке группировки по полю полного текста.
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) Устранена проблема, при которой добавление атрибута через `ALTER TABLE` не учитывало параметры KNN.
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) Исправлено неудачное удаление пакета `manticore-tools` в Redhat версии 6.3.0.
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) Исправлены проблемы с JOIN и несколькими операторами FACET, возвращающими некорректные результаты.
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) Исправлено, что ALTER TABLE выдавал ошибку, если таблица находится в кластере.
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) Исправлен исходный запрос, передаваемый в buddy из интерфейса SphinxQL.
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) Улучшено развертывание символов подстановки в `CALL KEYWORDS` для RT индекса с дисковыми чанками.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) Исправлено зависание некорректных запросов `/cli`.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Решены проблемы, при которых параллельные запросы к Manticore могли зависать.
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) Исправлено зависание `drop table if exists t; create table t` через `/cli`.

### Связано с репликацией
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) Добавлена поддержка формата `cluster:name` в HTTP-endpoint `/_bulk`.

# Версия 6.3.0
Выпущено: 23 мая 2024 года

### Основные изменения
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) Реализован тип данных [float_vector](Creating_a_table/Data_types.md#Float-vector); реализован [векторный поиск](Searching/KNN.md#KNN-vector-search).
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) [INNER/LEFT JOIN](Searching/Joining.md) (**бета стадия**).
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) Реализовано автоматическое определение форматов даты для полей [timestamp](../Creating_a_table/Data_types.md#Timestamps).
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) Изменена лицензия Manticore Search с GPLv2-or-later на GPLv3-or-later.
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Теперь для работы Manticore в Windows требуется Docker для работы Buddy.
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) Добавлен оператор [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) для полнотекстового поиска.
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) Поддержка Ubuntu Noble 24.04.
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) Обновлены временные операции для улучшения производительности и новые функции даты/времени:
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - Возвращает текущую дату в локальном часовом поясе
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - Возвращает целый квартал года из аргумента timestamp
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - Возвращает название дня недели для данного аргумента timestamp
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - Возвращает название месяца для данного аргумента timestamp
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - Возвращает целый индекс дня недели для данного аргумента timestamp
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - Возвращает целый день года для данного аргумента timestamp
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - Возвращает целый год и код дня первой недели для данного аргумента timestamp
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - Возвращает количество дней между двумя заданными временными метками
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - Форматирует дату из аргумента timestamp
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - Форматирует время из аргумента timestamp
  - [timezone](Server_settings/Searchd.md#timezone) - Часовой пояс, используемый функциями, связанными с датой/временем.
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) Добавлены агрегаты [range](Searching/Faceted_search.md#Facet-over-set-of-ranges), [histogram](Searching/Faceted_search.md#Facet-over-histogram-values), [date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) и [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) в HTTP-интерфейс и подобные выражения в SQL.

### Малые изменения
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) Поддержка версий Filebeat 8.10 - 8.11.
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table).
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) Добавлена возможность копировать таблицы с помощью SQL оператора [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:).
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) Оптимизирован алгоритм [compact таблиц](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table): Ранее как ручные [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE), так и автоматические [auto_optimize](Server_settings/Searchd.md#auto_optimize) процессы сначала объединяли части для обеспечения того, чтобы их количество не превышало лимит, а затем удаляли документы из всех других частей, содержащих удаленные документы. Этот подход иногда был слишком ресурсоемким и был отключен. Теперь объединение частей происходит исключительно в соответствии с настройкой [progressive_merge](Server_settings/Common.md#progressive_merge). Однако части с большим количеством удаленных документов более вероятно будут объединены.
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) Добавлена защита от загрузки вторичного индекса более новой версии.
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) Частичная замена через [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE).
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) Обновлены размеры буферов слияния по умолчанию: `.spa` (скалярные атрибуты): 256KB -> 8MB; `.spb` (blob атрибуты): 256KB -> 8MB; `.spc` (колонковые атрибуты): 1MB, без изменений; `.spds` (docstore): 256KB -> 8MB; `.spidx` (вторичные индексы): 256KB буфер -> 128MB лимит памяти; `.spi` (словарь): 256KB -> 16MB; `.spd` (doclists): 8MB, без изменений; `.spp` (hitlists): 8MB, без изменений; `.spe` (skiplists): 256KB -> 8MB.
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) Добавлено [композитное агрегирование](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once) через JSON.
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) Отключен PCRE.JIT из-за проблем с некоторыми регулярными выражениями и отсутствия значительной выгоды по времени.
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) Добавлена поддержка ванильной Galera v.3 (api v25) (`libgalera_smm.so` из MySQL 5.x).
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) Изменен суффикс метрики с `_rate` на `_rps`.
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) Улучшены документы о поддержке HA балансировщика.
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) Изменено `index` на `table` в сообщениях об ошибках; исправлено сообщение об ошибке парсера bison.
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) Поддержка `manticore.tbl` в качестве имени таблицы.
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) Поддержка запуска индексатора через systemd ([docs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)). ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) Поддержка вторичных индексов в GEODIST().
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) Упрощен [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS).
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) Добавлена поддержка значений по умолчанию (`agent_connect_timeout` и `agent_query_timeout`) для оператора `create distributed table`.
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) Добавлена поисковая опция [expansion_limit](Searching/Options.md#expansion_limit), которая переопределяет `searchd.expansion_limit`.
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) Реализован [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) для преобразования int->bigint.
* [Issue #146](https://github.com/manticoresoftware/manticoresearch/issues/1456) Метаинформация в ответе MySQL.
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION).
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) Поддержка [удаления документов по массиву id](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents) через JSON.
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) Улучшено сообщение об ошибке "unsupported value type".
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) Добавлена версия Buddy в `SHOW STATUS`.
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) Оптимизация запросов совпадений в случае нуля документов для ключевого слова.
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) Добавлено преобразование в булевый атрибут из строкового значения "true" и "false" при отправке данных.
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) Добавлена таблица [access_dict](Server_settings/Searchd.md#access_dict) и опция searchd.
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) Добавлены новые опции: [expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) и [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits) в секцию searchd конфигурации; сделан порог для объединения крошечных терминов расширенных терминов настраиваемым.
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) Добавлено отображение времени последней команды в `@@system.sessions`.
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) Обновлен протокол Buddy до версии 2.
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) Реализованы дополнительные форматы запросов для эндпоинта `/sql` для упрощения интеграции с библиотеками.
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) Добавлен раздел Info в SHOW BUDDY PLUGINS.
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) Улучшено потребление памяти в `CALL PQ` с большими пакетами.
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) Переключен компилятор с Clang 15 на Clang 16.
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) Добавлено сравнение строк. ❤️ Спасибо, [@etcd](https://github.com/etcd) за PR.
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) Добавлена поддержка объединенных хранимых полей.
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) Логирование хоста:порта клиента в query-log.
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) Исправлена неверная ошибка.
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) Внедрена поддержка уровней подробности для [плана запроса через JSON](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan).
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) Отключено логирование запросов от Buddy, если не установлено `log_level=debug`.
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) Поддержка Linux Mint 21.3.
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore не удалось собрать с Mysql 8.3+.
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) Команда `DEBUG DEDUP` для фрагментов таблицы в реальном времени, которые могут содержать дублирующиеся записи после прикрепления обычной таблицы с дубликатами.
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) Добавлено время в [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES).
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) Обработка столбца `@timestamp` как временной метки.
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) Реализована логика для включения/выключения плагинов Buddy.
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) Обновлен composer до более свежей версии, в которой исправлены недавние уязвимости.
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Небольшая оптимизация в системной единице Manticore systemd, связанная с `RuntimeDirectory`.
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) Добавлена поддержка rdkafka и обновлено до PHP 8.3.3.
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) Поддержка [прикрепления](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) RT таблицы. Новая команда [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table).

### Обратные изменения и устаревания
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) Исправлен вопрос вычисления IDF. `local_df` теперь является значением по умолчанию. Улучшен протокол поиска мастер-агент (версия обновлена). Если вы запускаете Manticore Search в распределенной среде с несколькими экземплярами, сначала обновите агентов, затем мастеров.
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) Добавлена репликация распределенных таблиц и обновлен протокол репликации. Если вы запускаете кластер репликации, вам нужно:
  - Сначала корректно остановить все ваши узлы
  - Затем запустить узел, который был остановлен последним, с `--new-cluster`, используя утилиту `manticore_new_cluster` в Linux.
  - Ознакомьтесь с [перезапуском кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для получения дополнительной информации.
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) Псевдонимы конечных точек HTTP API `/json/*` были объявлены устаревшими.
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) Изменен [профиль](Node_info_and_management/Profiling/Query_profile.md#Query-profile) на [план](Node_info_and_management/Profiling/Query_plan.md#Query-plan) в JSON, добавлен профилирование запросов для JSON.
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup больше не создает резервные копии `plugin_dir`.
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) Мигрирован Buddy на Swoole для повышения производительности и стабильности. При переходе на новую версию убедитесь, что все пакеты Manticore обновлены.
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) Все основные плагины объединены в Buddy и изменена основная логика.
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) Рассмотрение идентификаторов документов как чисел в ответах `/search`.
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) Добавлен Swoole, отключен ZTS и удалено расширение для параллельной обработки.
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) Переопределение в `charset_table` не работало в некоторых случаях.

### Изменения, связанные с репликацией
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) Исправлена ошибка репликации при SST больших файлов.
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) Добавлен механизм повторной попытки для команд репликации; исправлена ошибка соединения репликации в загруженной сети с потерей пакетов.
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) Изменено сообщение FATAL в репликации на сообщение WARNING.
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) Исправлен расчет `gcache.page_size` для кластеров репликации без таблиц или с пустыми таблицами; также исправлено сохранение и загрузка параметров Galera.
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) Добавлена функциональность для пропуска команды репликации обновления узлов на узле, который присоединяется к кластеру.
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) Исправлен взаимоблокировка во время репликации при обновлении атрибутов blob по сравнению с заменой документов.
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) Добавлены настройки конфигурации searchd [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout), [replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout), [replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay), [replication_retry_count](Server_settings/Searchd.md#replication_retry_count) для управления сетью во время репликации, аналогично `searchd.agent_*`, но с другими значениями по умолчанию.
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) Исправлена повторная попытка узлов репликации после того, как некоторые узлы были пропущены и не удалось разрешить имена этих узлов.
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) Исправлен уровень подробности журнала репликации в `show variables`.
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) Исправлена проблема репликации для узла-участника, подключающегося к кластеру на поде, перезапущенном в Kubernetes.
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) Исправлено длительное ожидание изменения репликации на пустом кластере с недопустимым именем узла.

### Исправления ошибок
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) Исправлена очистка неиспользуемых совпадений в `count distinct`, что могло привести к сбою.
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) Двоичный журнал теперь записывается с гранулярностью транзакций.
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) Исправлена ошибка, связанная с 64-битными идентификаторами, которая могла привести к ошибке "Неверный пакет" при вставке через MySQL, что привело к [поврежденным таблицам и дублированным идентификаторам](https://github.com/manticoresoftware/manticoresearch/issues/2090).
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) Исправлены даты, вставляемые так, как если бы они были в UTC, вместо местного часового пояса.
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) Исправлен сбой, который произошел при выполнении поиска в таблице реального времени с не пустым `index_token_filter`.
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) Изменена фильтрация дубликатов в RT столбчатом хранилище для устранения сбоев и неправильных результатов запросов.
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) Исправлен html стриппер, который портили память после обработки объединенного поля.
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) Избежано перематывание потока после сброса, чтобы предотвратить проблемы с неверной коммуникацией с mysqldump.
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) Не ждать завершения предварительного чтения, если оно не началось.
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) Исправлена большая строка вывода Buddy для разбивки на несколько строк в журнале searchd.
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) Перемещено предупреждение интерфейса MySQL о сбое заголовка `debugv` уровня подробности.
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) Исправлено состояние гонки при выполнении нескольких операций управления кластерами; запрещено создание нескольких кластеров с одинаковым именем или путем.
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) Исправлено неявное ограничение в полнотекстовых запросах; разделен MatchExtended на.template partD.
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) Исправлено несоответствие `index_exact_words` между индексированием и загрузкой таблицы в демон.
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) Исправлено пропущенное сообщение об ошибке для недопустимого удаления кластера.
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) Исправлен CBO против объединения очереди; исправлен CBO против RT псевдо шардирования.
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) При запуске без библиотеки вторичного индекса (SI) и параметров в конфигурации было выдано вводящее в заблуждение предупреждающее сообщение 'WARNING: secondary_indexes set but failed to initialize secondary library'.
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) Исправлена сортировка hit в кворуме.
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) Исправлена проблема с заглавными опциями в плагине ModifyTable.
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) Исправлено восстановление из дампа с пустыми json значениями (представленными как NULL).
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) Исправлено время ожидания SST на узле соединителя при получении SST с использованием pcon.
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) Исправлен сбой при выборе алиасированного строкового атрибута.
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) Добавлено преобразование запроса термина в `=term` полнотекстового запроса с полем `morphology_skip_fields`.
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) Добавлен отсутствующий ключ конфигурации (skiplist_cache_size).
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) Исправлен сбой в ранжировании выражений с большим сложным запросом.
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) Исправлено поведение CBO для полнотекстовых запросов по сравнению с недействительными подсказками индекса.
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) Прерывание предварительного чтения при завершении для более быстрого завершения.
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) Изменен расчет стека для полнотекстовых запросов, чтобы избежать сбоя в случае сложного запроса.
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) Исправлен сбой индексации при индексации SQL источника с несколькими колонками, имеющими одно и то же имя.
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) Возвращать 0 вместо <empty> для несуществующих sysvars.
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) Исправлена ошибка indextool при проверке внешних файлов RT таблицы.
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) Исправлена ошибка анализа запроса из-за многословной формы внутри фразы.
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) Добавлена возможность воспроизведения пустых файлов binlog с старыми версиями binlog.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) Исправлено удаление последнего пустого файла binlog.
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) Исправлены некорректные относительные пути (конвертированы в абсолютные из рабочей директории демона) после изменения `data_dir`, влияющие на текущую рабочую директорию при запуске демона.
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) Замедление времени в hn_small: получение/кеширование информации о процессоре при запуске демона.
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) Исправлено предупреждение о недостающем внешнем файле во время загрузки индекса.
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) Исправлен сбой в глобальных группировщиках при освобождении указателей атрибутов данных.
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS не работает.
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) Исправлено, что per table `agent_query_timeout` заменяется на опцию по умолчанию `agent_query_timeout`.
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) Исправлен сбой в группировщике и ранжировщике при использовании `packedfactors()` с несколькими значениями на совпадение.
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore выдает сбой при частых обновлениях индексов.
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) Исправлен сбой при очистке распарсенного запроса после ошибки анализа.
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) Исправлены HTTP JSON запросы, которые не направляются к buddy.
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) Корневая величина JSON атрибута не может быть массивом. Исправлено.
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) Исправлен сбой при воссоздании таблицы в рамках транзакции.
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) Исправлено расширение сокращенных форм русских лемм.
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) Исправлено использование JSON и STRING атрибутов в выражении [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()).
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) Исправлен группировщик для нескольких алиасов к JSON полям.
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) Неправильный total_related в dev: исправлен имплицитный обрез в сравнении с лимитом; добавлено лучшее обнаружение полного сканирования в json запросах.
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) Исправлено использование JSON и STRING атрибутов во всех выражениях даты.
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) сбой при использовании LEVENSHTEIN().
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) Исправлена порча памяти после ошибки анализа поискового запроса с выделением.
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) Отключено расширение подстановочных символов для терминов короче, чем `min_prefix_len` / `min_infix_len`.
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) Изменено поведение, чтобы не записывать ошибку, если Buddy успешно обрабатывает запрос.
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) Исправлено общее значение в метаданных поискового запроса для запросов с установленным лимитом.
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) Невозможно использовать таблицу с заглавными буквами через JSON в обычном режиме.
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) Установлено значение по умолчанию для `SPH_EXTNODE_STACK_SIZE`.
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) Исправлен SphinxQL журнал отрицательного фильтра с ALL/ANY на атрибуте MVA.
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) Исправлено применение списков блокировки docid из других индексов. ❤️ Спасибо, [@raxoft](https://github.com/raxoft) за PR.
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) Исправлены пропущенные совпадения из-за преждевременного выхода при полном сканировании сырого индекса (без каких-либо итераторов индексов); удален обрез от обычного итератора строк.
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) Исправлена ошибка `FACET` при запросе распределённой таблицы с агентом и локальными таблицами.
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) Исправлен сбой при оценке гистограммы для больших значений.
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) сбой при добавлении столбца col uint в таблицу tbl.
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) Пустой результат для условия `WHERE json.array IN (<value>)`.
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) Исправлена проблема с TableFormatter при отправке запроса на `/cli`.
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) `CREATE TABLE` не завершалось с ошибкой в случае отсутствия файла словоформ.
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) Порядок атрибутов в RT таблицах теперь следует порядку конфигурации.
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) HTTP булев запрос с условием 'should' возвращает неправильные результаты.
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) Сортировка по строковым атрибутам не работает с `SPH_SORT_ATTR_DESC` и `SPH_SORT_ATTR_ASC`.
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) Отключен HTTP заголовок `Expect: 100-continue` для запросов curl к Buddy.
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) сбой, вызванный GROUP BY псевдонимом.
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) SQL мета сводка показывает неправильное время в Windows.
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) Исправлено падение производительности при запросах JSON с одним термином.
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) Несоответствующие фильтры не вызывали ошибки на `/search`.
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) Исправлены операции `ALTER CLUSTER ADD` и `JOIN CLUSTER` для ожидания друг друга, предотвращая гонку, когда `ALTER` добавляет таблицу в кластер, пока донор отправляет таблицы в узел-участник.
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) Неправильная обработка запросов `/pq/{table}/*`.
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` не работал в некоторых случаях.
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) Исправлена проблема с восстановлением MVA в некоторых случаях.
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) Исправлен сбой indextool при завершении работы, если использовался с MCL.
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) Исправлено ненужное декодирование URL для запросов `/cli_json`.
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) изменить логику установки plugin_dir при запуске демона.
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) исключения alter table ... не работают.
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) Manticore выдает сбой с `signal 11` при вставке данных.
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) Уменьшено ограничение для [low_priority](Searching/Options.md#low_priority).
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Ошибка mysqldump + восстановление mysql.
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) Исправлено неправильное создание распределённой таблицы в случае отсутствия локальной таблицы или неправильного описания агента; теперь возвращает сообщение об ошибке.
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) Реализован счетчик `FREEZE`, чтобы избежать проблем с замораживанием/размораживанием.
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) Соблюдать тайм-аут запроса в узлах OR. Ранее `max_query_time` мог не работать в некоторых случаях.
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) Не удалось переименовать new в текущий [manticore.json].
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) Полнотекстовый запрос мог игнорировать подсказку CBO `SecondaryIndex`.
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) Исправлено `expansion_limit` для нарезки конечного набора результатов для ключевых слов вызова из нескольких дисковых чанков или RAM чанков.
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) неправильные внешние файлы.
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) Несколько процессов manticore-executor могли остаться активными после остановки Manticore.
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) Сбой при использовании расстояния Левенштейна.
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) Получена ошибка после запуска нескольких операторов max на пустом индексе.
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) сбой в мульти-группе с JSON.field.
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) Manticore аварийно завершался при некорректном запросе к _update.
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) Исправлена ошибка с компараторами строковых фильтров для закрытых диапазонов в интерфейсе JSON.
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) `alter` завершался неудачно, когда путь data_dir находился на символьной ссылке.
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) Улучшено специальное обработка запросов SELECT в mysqldump, чтобы гарантировать, что результирующие операторы INSERT совместимы с Manticore.
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Тайские символы находились в неправильных кодировках.
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) Сбой, если я использую SQL с зарезервированным словом.
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) Таблицы с формами слов не могли быть импортированы.
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) Исправлен сбой, который происходил, когда параметр engine был установлен на 'columnar', и дубликаты ID добавлялись через JSON.
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) Правильная ошибка при попытке вставить документ без схемы и без имен столбцов.
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) Авто-схема многопоточной вставки могла завершиться неудачей.
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) Добавлено сообщение об ошибке при индексации, если атрибут id объявлен в источнике данных.
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Поломка кластера Manticore.
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) optimize.php завершался бы аварийно, если таблица перколяторов была присутствовала.
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) Исправлены ошибки при развертывании в Kubernetes.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Исправлена некорректная обработка конкурентных запросов к Buddy

### Связано с manticore-backup
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) Установить VIP HTTP порт по умолчанию, когда он доступен.
Различные улучшения: улучшенная проверка версий и потоковая декомпрессия ZSTD; добавлены подсказки для пользователей о несовпадениях версий во время восстановления; исправлено некорректное поведение при подсказках для разных версий при восстановлении; улучшена логика декомпрессии для чтения непосредственно из потока, а не в рабочую память; добавлен флаг `--force`
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) Добавлено отображение версии резервной копии после запуска Manticore search для выявления проблем на этом этапе.
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) Обновлено сообщение об ошибке для неудачных подключений к демону.
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) Исправлена проблема с преобразованием абсолютных директорий резервного копирования в относительные и убран переносимый контроль при восстановлении для включения восстановления из различных путей.
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) Добавлена сортировка в итераторе файлов для обеспечения согласованности в различных ситуациях.
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) Резервное копирование и восстановление нескольких конфигураций.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлено defattr для предотвращения необычных пользовательских прав в файлах после установки на RHEL.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлен дополнительный chown для обеспечения того, чтобы файлы по умолчанию принадлежали пользователю root в Ubuntu.

### Связано с MCL (колоночные, вторичные, knn библиотеки)
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) Поддержка векторного поиска.
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) Исправлена очистка временных файлов во время прерванной настройки сборки вторичного индекса. Это решает проблему, когда демон превышал лимит открытых файлов при создании `tmp.spidx` файлов.
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) Используйте отдельную библиотеку streamvbyte для колоночного и SI.
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) Добавлено предупреждение о том, что колоночное хранилище не поддерживает JSON атрибуты.
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) Исправлена распаковка данных в SI.
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) Исправлен сбой при сохранении дискового чанка с смешанным построчным и колоночным хранением.
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) Исправлен итератор SI, который указывался на уже обработанный блок.
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) Обновление сломано для построчной MVA колонки с колоночным движком.
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) Исправлен сбой при агрегации к колоночному атрибуту, использованному в `HAVING`.
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) Исправлен сбой в `expr` ранжировщике при использовании колонного атрибута.

### Связано с Docker
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) Поддержка [обычной индексации](https://github.com/manticoresoftware/docker#building-plain-tables) через переменные окружения.
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) Улучшена гибкость конфигурации через переменные окружения.
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) Улучшены процессы [резервного копирования и восстановления](https://github.com/manticoresoftware/docker#backup-and-restore) для Docker.
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) Улучшен скрипт запуска для обработки восстановления резервной копии только при первом запуске.
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) Исправлено логирование запросов в stdout.
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) Заглушить предупреждения BUDDY, если EXTRA не установлен.
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) Исправлено имя хоста в `manticore.conf.sh`.

# Версия 6.2.12
Выпущена: 23 августа 2023 года

Версия 6.2.12 продолжает серию 6.2 и решает проблемы, выявленные после выпуска 6.2.0.

### Исправления ошибок
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) "Manticore 6.2.0 не запускается через systemctl на Centos 7": Изменено `TimeoutStartSec` с `infinity` на `0` для лучшей совместимости с Centos 7.
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) "Сбой после обновления с 6.0.4 до 6.2.0": Добавлена функция воспроизведения для пустых файлов binlog из более ранних версий binlog.
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) "исправить опечатку в searchdreplication.cpp": Исправлена опечатка в `searchdreplication.cpp`: beggining -> beginning.
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) "Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: выбрасывание на неудачном заголовке MySQL, AsyncNetInputBuffer_c::AppendData: ошибка 11 (Ресурс временно недоступен) возврат -1": Уменьшен уровень детализации предупреждения интерфейса MySQL о заголовке до logdebugv.
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) "присоединение к кластеру зависает, когда node_address не может быть разрешен": Улучшен повтор попытки репликации, когда определенные узлы недоступны, и их разрешение имен не удается. Это должно решить проблемы в Kubernetes и узлах Docker, связанные с репликацией. Улучшено сообщение об ошибке для сбоев начала репликации и внесены обновления в тестовую модель 376. Дополнительно обеспечено четкое сообщение об ошибке для сбоев разрешения имен.
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "Нет сопоставления в нижнем регистре для "Ø" в наборе символов non_cjk": Изменено сопоставление для символа 'Ø'.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd оставляет binlog.meta и binlog.001 после чистой остановки": Обеспечено правильное удаление последнего пустого файла binlog.
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): Исправлена проблема сборки `Thd_t` на Windows, связанная с ограничениями атомарного копирования.
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): Решена проблема с FT CBO против `ColumnarScan`.
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): Внесены коррективы в тест 376 и добавлена замена для ошибки `AF_INET` в тесте.
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): Решена проблема взаимной блокировки во время репликации при обновлении атрибутов блобов по сравнению с заменой документов. Также убрано rlock индекса во время коммита, так как он уже заблокирован на более базовом уровне.

### Небольшие изменения
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) Обновлена информация о конечных точках `/bulk` в руководстве.

### MCL
* Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

# Версия 6.2.0
Выпущена: 4 августа 2023 года

### Основные изменения
* Оптимизатор запросов был улучшен для поддержки полнотекстовых запросов, что значительно повышает эффективность поиска и производительность.
* Интеграции с:
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - для [создания логических резервных копий](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) с использованием `mysqldump`
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) и [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) для визуализации данных, хранящихся в Manticore
  - [HeidiSQL](https://www.heidisql.com/) и [DBForge](https://www.devart.com/dbforge/) для более простого разработки с Manticore
* Мы начали использовать [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions), что упрощает процесс использования теми, кто вносит вклад, того же процесса непрерывной интеграции (CI), который применяет основная команда при подготовке пакетов. Все задания могут выполняться наRunner-е, размещенном на GitHub, что облегчает тестирование изменений в вашей ветке Manticore Search.
* Мы начали использовать [CLT](https://github.com/manticoresoftware/clt) для тестирования сложных сценариев. Например, теперь мы можем гарантировать, что пакет, собранный после коммита, может быть правильно установлен на всех поддерживаемых операционных системах Linux. Командный тестер (CLT) предоставляет удобный способ записи тестов в интерактивном режиме и простого их воспроизведения.
* Значительное улучшение производительности операции подсчета уникальных значений благодаря использованию комбинации хеш-таблиц и HyperLogLog.
* Включена многопоточность при выполнении запросов с вторичными индексами, при этом количество потоков ограничено числом физических ядер ЦП. Это должно значительно ускорить выполнение запросов.
* `pseudo_sharding` был настроен так, чтобы ограничиваться числом свободных потоков. Это обновление значительно повышает производительность пропускной способности.
* Теперь пользователи могут указать [движок хранения атрибутов по умолчанию](../Server_settings/Searchd.md#engine) через настройки конфигурации, обеспечивая лучшую настройку под конкретные требования рабочей нагрузки.
* Поддержка [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/) с многочисленными исправлениями ошибок и улучшениями в [вторичных индексах](../Server_settings/Searchd.md#secondary_indexes).

### Незначительные изменения
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): HTTP-эндпоинт [/pq](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) теперь служит псевдонимом для HTTP-эндпоинта `/json/pq`.
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): Мы обеспечили совместимость с многобайтовыми данными для `upper()` и `lower()`.
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): Вместо сканирования индекса для запросов `count(*)`, теперь возвращается предрассчитанное значение.
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): Теперь можно использовать `SELECT` для выполнения произвольных расчетов и отображения `@@sysvars`. В отличие от предыдущего, вы больше не ограничены только одним расчетом. Поэтому запросы, такие как `select user(), database(), @@version_comment, version(), 1+1 as a limit 10`, вернут все столбцы. Обратите внимание, что параметр 'limit' всегда будет игнорироваться.
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): Реализован заглушечный запрос `CREATE DATABASE`.
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): При выполнении `ALTER TABLE table REBUILD SECONDARY` вторичные индексы теперь всегда перестраиваются, даже если атрибуты не были обновлены.
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): Сортировщики, использующие предрассчитанные данные, теперь определяются перед использованием CBO, чтобы избежать ненужных вычислений CBO.
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): Реализация макета и использование стека полнотекстового выражения для предотвращения сбоев демона.
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): Добавлен быстрый путь кода для клонирования совпадений для совпадений, которые не используют строковые/mvas/json атрибуты.
* [Commit a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): Добавлена поддержка команды `SELECT DATABASE()`. Однако она всегда будет возвращать `Manticore`. Это дополнение имеет решающее значение для интеграции с различными инструментами MySQL.
* [Commit bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): Изменен формат ответа эндпоинта [/cli](../Connecting_to_the_server/HTTP.md#/cli), и добавлен эндпоинт `/cli_json`, чтобы функционировать как предыдущий `/cli`.
* [Commit d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): Параметр `thread_stack` теперь можно изменять во время выполнения, используя оператор `SET`. Доступны как локальные для сеанса, так и глобальные для демона варианты. Текущие значения можно посмотреть в выводе `show variables`.
* [Commit d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): Код был интегрирован в CBO для более точной оценки сложности выполнения фильтров по строковым атрибутам.
* [Commit e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): Расчет стоимости DocidIndex был улучшен, что повысило общую производительность.
* [Commit f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): Метрики загрузки, аналогичные 'uptime' в Linux, теперь видны в команде `SHOW STATUS`.
* [Commit f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): Порядок полей и атрибутов для `DESC` и `SHOW CREATE TABLE` теперь соответствует тому, что у `SELECT * FROM`.
* [Commit f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): Разные внутренние парсеры теперь предоставляют свой внутренний мнемонический код (например, `P01`) во время различных ошибок. Это улучшение помогает определить, какой парсер вызвал ошибку, а также скрывает несущественные внутренние детали.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) "Иногда CALL SUGGEST не предлагает исправление опечатки на одну букву": Улучшено поведение [SUGGEST/QSUGGEST](../Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) для коротких слов: добавлена опция `sentence`, чтобы показать целое предложение.
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) "Индекс перколяции неправильно ищет по запросу точной фразы, когда включено стемминг": Запрос на перколяцию был изменен для обработки модификатора точного термина, улучшая функциональность поиска.
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) "Методы форматирования даты": добавлено выражение списка выбора [date_format()](../Functions/Date_and_time_functions.md#DATE_FORMAT()), которое открывает функцию `strftime()`.
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) "Сортировка бакетов через HTTP JSON API": введено необязательное [свойство сортировки](../Searching/Faceted_search.md#HTTP-JSON) для каждого бакета агрегатов в HTTP-интерфейсе.
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) "Улучшить логирование ошибок при использовании JSON insert api - "unsupported value type"": Точка доступа `/bulk` сообщает информацию о количестве обработанных и необработанных строк (документов) в случае ошибки.
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) "Подсказки CBO не поддерживают несколько атрибутов": Включены подсказки индекса для обработки нескольких атрибутов.
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) "Добавить теги к http поисковому запросу": Теги были добавлены к [HTTP PQ ответам](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table).
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) "buddy не должен создавать таблицы параллельно": Решена проблема, которая вызывала сбои при параллельных операциях CREATE TABLE. Теперь только одна операция `CREATE TABLE` может выполняться одновременно.
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) "добавить поддержку @ в названия колонок".
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) "Запросы по набору данных такси медленные при ps=1": Логика CBO была уточнена, а разрешение по умолчанию для гистограмм установлено на 8k для лучшей точности по атрибутам с случайно распределенными значениями.
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) "Исправить CBO против полнотекстового поиска на наборе данных hn": Улучшенная логика была внедрена для определения, когда использовать пересечение итераторов битовых карт и когда использовать очередь приоритетов.
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) "columnar: изменить интерфейс итератора на однократный вызов" : Колонные итераторы теперь используют один вызов `Get`, заменяя предыдущие двухшаговые вызовы `AdvanceTo` + `Get` для получения значения.
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) "Ускорение вычислений агрегатов (убрать CheckReplaceEntry?)": Вызов `CheckReplaceEntry` был удален из сортировщика групп, чтобы ускорить вычисление агрегатных функций.
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) "возможности создания таблицы read_buffer_docs/hits не понимают синтаксис k/m/g": Опции `CREATE TABLE` `read_buffer_docs` и `read_buffer_hits` теперь поддерживают синтаксис k/m/g.
* Языковые пакеты [для английского, немецкого и русского](../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) теперь можно легко установить на Linux, выполнив команду `apt/yum install manticore-language-packs`. На macOS используйте команду `brew install manticoresoftware/tap/manticore-language-packs`.
* Порядок полей и атрибутов теперь согласован между операциями `SHOW CREATE TABLE` и `DESC`.
* Если в процессе выполнения запросов `INSERT` недостаточно дискового пространства, новые запросы `INSERT` будут завершаться неудачей, пока не станет доступно достаточно дискового пространства.
* Функция преобразования типа [UINT64()](../Functions/Type_casting_functions.md#UINT64%28%29) была добавлена.
* Точка доступа `/bulk` теперь обрабатывает пустые строки как команду [commit](../Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK). Дополнительная информация [здесь](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md?client=JSON#Bulk-adding-documents).
* Внедрены предупреждения для [недействительных подсказок индекса](../Searching/Options.md#Query-optimizer-hints), обеспечивая большую прозрачность и позволяя снижать количество ошибок.
* Когда используется `count(*)` с одним фильтром, запросы теперь используют предвычисленные данные из вторичных индексов, если они доступны, что существенно ускоряет время выполнения запросов.

### ⚠️ Серьезные изменения
* ⚠️ Таблицы, созданные или измененные в версии 6.2.0, не могут быть прочитаны более старыми версиями.
* ⚠️ Идентификаторы документов теперь обрабатываются как беззнаковые 64-битные целые числа во время индексации и операций INSERT.
* ⚠️ Синтаксис подсказок оптимизатора запросов был обновлен. Новый формат - `/*+ SecondaryIndex(uid) */`. Обратите внимание, что старый синтаксис больше не поддерживается.
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): Использование `@` в названиях таблиц запрещено, чтобы предотвратить конфликты синтаксиса.
* ⚠️ Строковые поля/атрибуты, помеченные как `indexed` и `attribute`, теперь рассматриваются как одно поле во время операций `INSERT`, `DESC` и `ALTER`.
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): Библиотеки MCL больше не загружаются на системах, которые не поддерживают SSE 4.2.
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) был сломан. Исправлено и теперь эффективно.

### Исправления ошибок
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) "Сбой при DROP TABLE": решена проблема, вызывающая длительное ожидание завершения операций записи (оптимизация, сохранение дискового блока) на таблице RT при выполнении оператора DROP TABLE. Добавлено предупреждение для уведомления о том, что директория таблицы не пуста после выполнения команды DROP TABLE.
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): Поддержка колонных атрибутов, которой не хватало в коде, используемом для группировки по нескольким атрибутам, была добавлена.
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) Исправлена проблема с падением, потенциально вызванная нехваткой дискового пространства, путем правильной обработки ошибок записи в binlog.
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): Исправлена ошибка, которая иногда возникала при использовании нескольких колонных сканирующих итераторов (или итераторов вторичного индекса) в запросе.
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): Фильтры не удалялись при использовании сортировщиков, использующих предварительно рассчитанные данные. Эта проблема была исправлена.
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): Код CBO был обновлен для предоставления лучших оценок для запросов, использующих фильтры по способам-атрибутам, выполняемым в нескольких потоках.
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) "фатальный сбой в кластере Kubernetes": Исправлен дефектный фильтр bloom для корневого объекта JSON; исправлен сбой демона из-за фильтрации по полю JSON.
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) Исправлен сбой демона, вызванный недействительным конфигом `manticore.json`.
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) Исправлен фильтр диапазона json для поддержки значений int64.
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) `.sph` файлы могли быть повреждены `ALTER`. Исправлено.
* [Commit 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): Для репликации оператора замены добавлен общий ключ для устранения ошибки `pre_commit`, возникающей при репликации замены от нескольких мастер-узлов.
* [Commit 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) решены проблемы с проверками bigint по функциям, подобным 'date_format()'.
* [Commit 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): Итераторы больше не отображаются в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), когда сортировщики используют предварительно рассчитанные данные.
* [Commit a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): Размер стека узла полного текста был обновлен для предотвращения сбоев при сложных запросах полного текста.
* [Commit a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): Исправлена ошибка, вызывающая сбой во время репликации обновлений с JSON и строковыми атрибутами.
* [Commit b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): Строковый строитель был обновлен для использования 64-битных целых чисел, чтобы избежать сбоев при работе с большими наборами данных.
* [Commit c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): Устранен сбой, возникающий при подсчете уникальных значений по нескольким индексам.
* [Commit d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): Исправлена проблема, когда запросы по дисковым кускам RT индексов могли выполняться в нескольких потоках, даже если `pseudo_sharding` был отключен.
* [Commit d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) Набор значений, возвращаемых командой `show index status`, был изменен и теперь варьируется в зависимости от типа используемого индекса.
* [Commit e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) Исправлена ошибка HTTP при обработке массовых запросов и проблема, при которой ошибка не возвращалась клиенту из сетевого цикла.
* [Commit f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) использование расширенного стека для PQ.
* [Commit fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) Обновлен вывод ранжировщика экспорта для согласования с [packedfactors()](../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29).
* [Commit ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): Исправлена проблема со строковым списком в фильтре логов запросов SphinxQL.
* [Issue #589](https://github.com/manticoresoftware/manticoresearch/issues/589) "Определение кодировки, похоже, зависит от порядка кодов": Исправлено некорректное сопоставление кодировки для дубликатов.
* [Issue #811](https://github.com/manticoresoftware/manticoresearch/issues/811) "Сопоставление нескольких слов в формах слов мешает поиску по фразам с CJK знаками препинания между ключевыми словами": Исправлена позиция токена ngram внутри запроса фразы с формами слов.
* [Issue #834](https://github.com/manticoresoftware/manticoresearch/issues/834) "Знак равенства в поисковом запросе нарушает запрос": Обеспечено, что точный символ может быть экранирован, и исправлено двойное точное расширение с помощью параметра `expand_keywords`.
* [Issue #864](https://github.com/manticoresoftware/manticoresearch/issues/864) "исключения/конфликт стоп-слов"
* [Issue #910](https://github.com/manticoresoftware/manticoresearch/issues/910) "Сбой Manticore при вызове call snippets() с libstemmer_fr и index_exact_words": Устранены внутренние конфликты, вызывающие сбои, когда вызывается `SNIPPETS()`.
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) "Дублирующие записи во время SELECT": Исправлена проблема с дубликатами документов в наборе результатов для запроса с параметром `not_terms_only_allowed` к RT индексу с убитыми документами.
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) "Использование аргументов JSON в UDF-функциях приводит к сбою": Исправлен сбой демона при обработке поиска с включенным псевдо-шардированием и UDF с аргументом JSON.
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) "count(*) в FEDERATED": Исправлен сбой демона, возникающий с запросом через движок `FEDERATED` с агрегатом.
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) Исправлена проблема, когда столбец `rt_attr_json` оказался несовместимым с колонным хранением.
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) "* удаляется из поискового запроса с помощью ignore_chars": Исправлена эта проблема, чтобы подстановочные знаки в запросе не затрагивались `ignore_chars`.
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) "indextool --check fails if there's a distributed table": indextool теперь совместим с инстанциями, имеющими 'distributed' и 'template' индексы в json конфигурации.
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) "particular select on particular RT dataset leads to crash of searchd": Устранен сбой демона при запросе с packedfactors и большим внутренним буфером.
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) "With not_terms_only_allowed deleted documents are ignored"
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) "indextool --dumpdocids is not working": Восстановлена функциональность команды `--dumpdocids`.
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) "indextool --buildidf is not working": indextool теперь закрывает файл после завершения globalidf.
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) "Count(*) is trying to be treated as schema set in remote tables": Устранена проблема, когда сообщение об ошибке отправлялось демоном для запросов к распределенному индексу, когда агент возвращал пустой набор результатов.
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) "FLUSH ATTRIBUTES hangs with threads=1".
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "Lost connection to MySQL server during query - manticore 6.0.5": Устранены сбои, которые происходили при использовании нескольких фильтров по колоннарным атрибутам.
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "JSON string filtering case sensitivity": Исправлена сортировка для правильной работы фильтров, используемых в HTTP запросах.
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "Match in a wrong field": Исправлены повреждения, связанные с `morphology_skip_fields`.
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "system remote commands via API should pass g_iMaxPacketSize": Обновлены настройки для обхода проверки `max_packet_size` для команд репликации между узлами. Кроме того, последние ошибки кластера добавлены в отображение статуса.
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "tmp files left on failed optimize": Исправлена проблема, когда временные файлы оставались после возникновения ошибки во время слияния или оптимизации.
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "add env var for buddy start timeout": Добавлена переменная окружения `MANTICORE_BUDDY_TIMEOUT` (по умолчанию 3 секунды) для контроля времени ожидания демона для сообщения о "друге" при старте.
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "Int overflow when saving PQ meta": Уменьшено чрезмерное потребление памяти демоном при сохранении большого PQ индекса.
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "Can't recreate RT table after altering its external file": Исправлена ошибка изменения с пустой строкой для внешних файлов; исправлены внешние файлы RT индекса, оставшиеся после изменения внешних файлов.
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "SELECT statement sum(value) as value doesn't work properly": Исправлена проблема, когда выражение списка выбора с псевдонимом могло скрыть свойство индекса; также исправлена сумма для подсчета в int64 для целого числа.
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "Avoid binding to localhost in replication": Обеспечено, чтобы репликация не связывалась с localhost для имен хостов с несколькими IP.
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "reply to mysql client failed for data larger 16Mb": Исправлена проблема возврата пакета SphinxQL размером более 16 Мб клиенту.
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) "wrong reference in "paths to external files should be absolute": Исправлено отображение полного пути к внешним файлам в `SHOW CREATE TABLE`.
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "debug build crashes on long strings in snippets": Теперь длинные строки (>255 символов) разрешены в тексте, на который нацелена функция `SNIPPET()`.
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "spurious crash on use-after-delete in kqueue polling (master-agent)": Исправлены сбои, когда мастер не может подключиться к агенту на системах, управляемых kqueue (FreeBSD, MacOS и т. д.).
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "too long connect to itself": При подключении от мастера к агентам на MacOS/BSD теперь используется единый тайм-аут соединения+запроса вместо простого соединения.
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq (json meta) with unreached embedded synonyms fails to load": Исправлен флаг встроенных синонимов в pq.
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "Allow some functions (sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday) to use implicitly promoted argument values".
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "Enable multithreaded SI in fullscan, but limit threads": Код был внедрен в CBO, чтобы лучше предсказать многопоточную производительность вторичных индексов, когда они используются в полнотекстовом запросе.
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "count(*) queries still slow after using precalc sorters": Итераторы больше не инициализируются при использовании сортировщиков, которые используют предварительно рассчитанные данные, что позволяет избежать пагубных эффектов на производительность.
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "журнал запросов в sphinxql не сохраняет исходные запросы для MVA": Теперь `all()/any()` регистрируется.

# Версия 6.0.4
Выпущено: 15 марта 2023 года

### Новые функции
* Улучшенная интеграция с Logstash, Beats и т.д., включая:
  - Поддержка версий Logstash 7.6 - 7.15, версий Filebeat 7.7 - 7.12  
  - Поддержка автосхемы.
  - Добавлена обработка массовых запросов в формате, похожем на Elasticsearch.
* [Коммит Buddy ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Лог версии Buddy при запуске Manticore.

### Исправления ошибок
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) Исправлен некорректный символ в поисковом мета и вызывающих ключевых словах для биграммного индекса.
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) Строки HTTP в нижнем регистре отклоняются.
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) Исправлен утечка памяти в демоне при чтении вывода консоли Buddy.
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) Исправлено неожиданное поведение знака вопроса.
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - Исправлено состояние гонки в таблицах нижнего регистра токенизатора, вызывающее сбой.
* [Коммит 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) Исправлена обработка массовых записей в интерфейсе JSON для документов с явно установленным id в null.
* [Коммит 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) Исправлена статистика терминов в CALL KEYWORDS для нескольких одинаковых терминов.
* [Коммит f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Конфигурация по умолчанию теперь создается установщиком Windows; пути больше не заменяются во время выполнения.
* [Коммит 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Коммит cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) Исправлены проблемы с репликацией для кластера с узлами в нескольких сетях.
* [Коммит 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) Исправлен HTTP-эндпоинт `/pq`, чтобы быть алиасом HTTP-эндпоинта `/json/pq`.
* [Коммит 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) Исправлен сбой демона при перезапуске Buddy.
* [Коммит Buddy fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) Отображать оригинальную ошибку при получении недействительного запроса.
* [Коммит Buddy db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) Разрешить пробелы в пути резервного копирования и добавить немного магии в regexp, чтобы поддерживать одинарные кавычки также.

# Версия 6.0.2
Выпущено: 10 февраля 2023 года

### Исправления ошибок
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / Ошибка сегментации при поиске по аспекту с большим количеством результатов
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - ПРЕДУПРЕЖДЕНИЕ: Скомпилированное значение KNOWN_CREATE_SIZE (16) меньше измеренного (208). Рассмотрите возможность исправления значения!
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Чистый индекс Manticore 6.0.0 вызывает сбой
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - множественные распределенные теряются при перезагрузке демона
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - состояние гонки в таблицах нижнего регистра токенизатора

# Версия 6.0.0
Выпущено: 7 февраля 2023 года

Начиная с этого выпуска, Manticore Search поставляется с Manticore Buddy, демон-партнер, написанный на PHP, который обрабатывает функции высокого уровня, которые не требуют супер низкой задержки или высокой пропускной способности. Manticore Buddy работает за кулисами, и вы можете даже не осознавать, что он работает. Хотя он невидим для конечного пользователя, это была значительная задача сделать Manticore Buddy легко устанавливаемым и совместимым с основным демоном на основе C++. Это главное изменение позволит команде разрабатывать широкий спектр новых функций высокого уровня, таких как оркестрация шардов, контроль доступа и аутентификация, а также различные интеграции, такие как mysqldump, DBeaver, графический соединитель mysql. Пока что он уже обрабатывает [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES), [BACKUP](../Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) и [Автосхему](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

Этот выпуск также включает более 130 исправлений ошибок и множество функций, многие из которых можно считать важными.

### Основные изменения
* 🔬 Экспериментально: теперь вы можете выполнять совместимые с Elasticsearch [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) и [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON-запросы, которые позволяют использовать Manticore с инструментами, такими как Logstash (версия < 7.13), Filebeat и другими инструментами из семьи Beats. Включено по умолчанию. Вы можете отключить это с помощью `SET GLOBAL ES_COMPAT=off`.
* Поддержка [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) с многочисленными исправлениями и улучшениями в [Вторичных индексах](../Server_settings/Searchd.md#secondary_indexes). **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ**: Вторичные индексы включены по умолчанию с этого выпуска. Убедитесь, что вы выполняете [ALTER TABLE table_name REBUILD SECONDARY](../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index), если вы обновляете с Manticore 5. См. ниже для получения дополнительной информации.
* [Commit c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) Авто-схема: теперь вы можете пропустить создание таблицы, просто вставьте первый документ, и Manticore создаст таблицу автоматически на основе ее полей. Узнайте больше об этом в деталях [здесь](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema). Вы можете включить/выключить эту функцию с помощью [searchd.auto_schema](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).
* Значительное обновление [оптимизатора на основе затрат](../Searching/Cost_based_optimizer.md), который снижает время отклика на запросы во многих случаях.
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) Оценка производительности параллелизации в CBO.
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO теперь осведомлен о [вторичных индексах](../Server_settings/Searchd.md#secondary_indexes) и может действовать более разумно.
  - [Commit cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) Статистика кодирования колоннарных таблиц/полей теперь хранится в метаданных, чтобы помочь CBO принимать более разумные решения.
  - [Commit 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) Добавлены подсказки CBO для настройки его поведения.
* [Телеметрия](../Telemetry.md#Telemetry): мы рады объявить о добавлении телеметрии в этом релизе. Эта функция позволяет нам собирать анонимные и деперсонализированные метрики, которые помогут нам улучшить производительность и пользовательский опыт нашего продукта. Будьте уверены, все собранные данные **абсолютно анонимны и не будут связаны с какой-либо личной информацией**. Эту функцию можно [легко отключить](../Telemetry.md#Telemetry) в настройках, если это необходимо.
* [Commit 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) для восстановления вторичных индексов в любое время, например:
  - когда вы мигрируете с Manticore 5 на новую версию,
  - когда вы сделали [UPDATE](../Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE) (т.е. [обновление на месте, а не замена](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)) атрибута в индексе
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) Новый инструмент `manticore-backup` для [резервного копирования и восстановления экземпляра Manticore](../Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL команда [BACKUP](../Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) для выполнения резервных копий изнутри Manticore.
* SQL команда [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) как простой способ увидеть выполняющиеся запросы, а не потоки.
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL команда `KILL` для завершения длительно выполняющегося `SELECT`.
* Динамический `max_matches` для агрегатных запросов для увеличения точности и снижения времени отклика.

### Небольшие изменения
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL команды [FREEZE/UNFREEZE](../Securing_and_compacting_a_table/Freezing_a_table.md) для подготовки реального/плоского таблицы к резервному копированию.
* [Commit c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) Новые настройки `accurate_aggregation` и `max_matches_increase_threshold` для контролируемой точности агрегации.
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) Поддержка подписанных отрицательных 64-битных идентификаторов. Обратите внимание, что вы все еще не можете использовать идентификаторы > 2^63, но теперь вы можете использовать идентификаторы в диапазоне от -2^63 до 0.
* Поскольку мы недавно добавили поддержку вторичных индексов, ситуация стала запутанной, поскольку "индекс" может относиться к вторичному индексу, полнотекстовому индексу или обычному/реальному `индексу`. Чтобы уменьшить путаницу, мы переименовываем последний в "таблицу". Следующие SQL/командные строки подвержены этому изменению. Их старые версии устарели, но все еще функционируют:
  - `index <table name>` => `table <table name>`,
  - `searchd -i / --index` => `searchd -t / --table`,
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`,
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`,
  - `FLUSH RTINDEX` => `FLUSH TABLE`,
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`,
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`,
  - `RELOAD INDEX` => `RELOAD TABLE`,
  - `RELOAD INDEXES` => `RELOAD TABLES`.

  Мы не планируем делать старые формы устаревшими, но чтобы обеспечить совместимость с документацией, мы рекомендуем изменить названия в вашем приложении. Что будет изменено в будущем релизе, так это переименование "индекса" в "таблицу" в выводе различных SQL и JSON команд.
* Запросы с состоянием UDF теперь вынуждены выполняться в одном потоке.
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) Рефакторинг всего, что касается планирования времени, как предварительное условие для объединения параллельных частей.
* **⚠️ ЛОМКА СОВМЕСТИМОСТИ**: Формат хранения колонок был изменен. Вам необходимо восстановить те таблицы, которые имеют колоннарные атрибуты.
* **⚠️ ЛОМКА СОВМЕСТИМОСТИ**: Формат файла вторичных индексов был изменен, поэтому если вы используете вторичные индексы для поиска и у вас в конфигурационном файле установлено `searchd.secondary_indexes = 1`, имейте в виду, что новая версия Manticore **пропустит загрузку таблиц, которые имеют вторичные индексы**. Рекомендуется:
  - Перед обновлением измените `searchd.secondary_indexes` на 0 в конфигурационном файле.
  - Запустите экземпляр. Manticore загрузит таблицы с предупреждением.
  - Выполните `ALTER TABLE <table name> REBUILD SECONDARY` для каждого индекса, чтобы восстановить вторичные индексы.
  Если вы запускаете кластер репликации, вам нужно выполнить `ALTER TABLE <имя таблицы> REBUILD SECONDARY` на всех узлах или следовать [этой инструкции](../Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables) с единственным изменением: выполнить `ALTER .. REBUILD SECONDARY` вместо `OPTIMIZE`.
* **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ**: Версия binlog была обновлена, поэтому любые binlog из предыдущих версий не будут воспроизведены. Важно убедиться, что Manticore Search остановлен корректно во время процесса обновления. Это означает, что в `/var/lib/manticore/binlog/` не должно быть файлов binlog, кроме `binlog.meta`, после остановки предыдущего экземпляра.
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`: теперь вы можете видеть настройки из конфигурационного файла внутри Manticore.
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](../Server_settings/Setting_variables_online.md#SET) включает/выключает отслеживание времени процессора; [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md) теперь не показывает статистику CPU, когда отслеживание времени процессора выключено.
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) Сегменты RAM-чанка RT таблицы теперь могут быть объединены во время сброса RAM-чанка.
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) Добавлен прогресс вторичного индекса в вывод [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) Ранее запись таблицы могла быть удалена Manticore из списка индексов, если она не могла начать обслуживать её при запуске. Новое поведение заключается в том, чтобы сохранить её в списке и попытаться загрузить при следующем запуске.
* [indextool --docextract](../Miscellaneous_tools.md#indextool) возвращает все слова и попадания, принадлежащие запрашиваемому документу.
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) Переменная окружения `dump_corrupt_meta` включает дамп поврежденных метаданных таблицы в лог, если searchd не может загрузить индекс.
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` может показать `max_matches` и статистику псевдоразделения.
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) Более полезная ошибка вместо запутанного "Формат заголовка индекса не json, попробуем его как бинарный...".
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) Путь укровайского лемматизатора был изменён.
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) Статистика вторичных индексов была добавлена в [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META).
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON интерфейс теперь можно легко визуализировать с использованием Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification.
* **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ**: Протокол репликации был изменён. Если вы запускаете кластер репликации, то при обновлении до Manticore 5 вам нужно:
  - сначала корректно остановить все ваши узлы
  - а затем запустить узел, который был остановлен последним с помощью `--new-cluster` (запустите инструмент `manticore_new_cluster` в Linux).
  - прочитайте о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для получения дополнительных деталей.

### Изменения, связанные с Manticore Columnar Library
* Рефакторинг интеграции вторичных индексов с колонковым хранилищем.
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Оптимизация Manticore Columnar Library, которая может снизить время ответа за счёт частичной предварительной оценки min/max.
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) Если слияние дискового чанка прерывается, демон теперь очищает временные файлы, связанные с MCL.
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) Версии колонковых и вторичных библиотек записываются в лог при сбое.
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) Добавлена поддержка быстрого перематывания списка документов для вторичных индексов.
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) Запросы вроде `select attr, count(*) from plain_index` (без фильтрации) теперь быстрее, если вы используете MCL.
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) @@autocommit в HandleMysqlSelectSysvar для совместимости с .net коннектором для mysql больше 8.25
* **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ**: [MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL: добавить код SSE для сканирования колонок. MCL теперь требует как минимум SSE4.2.

### Изменения, связанные с упаковкой
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ**: Поддержка Debian Stretch и Ubuntu Xenial была прекращена.
* Поддержка RHEL 9, включая Centos 9, Alma Linux 9 и Oracle Linux 9.
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) Поддержка Debian Bookworm.
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) Упаковка: сборки arm64 для Linux и MacOS.
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) Многоархитектурный (x86_64 / arm64) образ docker.
* [Упрощенная сборка пакетов для участников](../Installation/Compiling_from_sources.md#Building-using-CI-Docker).
* Теперь можно установить конкретную версию с использованием APT.
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Установщик для Windows (ранее мы предоставляли только архив).
* Переключился на компиляцию с использованием CLang 15.
* **⚠️ СЛОМАННОЕ ИЗМЕНЕНИЕ**: пользовательские формулы Homebrew, включая формулу для Manticore Columnar Library. Чтобы установить Manticore, MCL и все другие необходимые компоненты, используйте следующую команду `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`.

### Исправления ошибок
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) Поле с именем `text`
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id не может не быть bigint
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER против поля с именем "text"
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) Возможная ОШИБКА: HTTP (JSON) смещение и лимит влияют на результаты фасета
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd зависает/сбрасывается при интенсивной загрузке
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ индекс не хватает памяти
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` был сломан все время с Sphinx. Исправлено.
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL: авария при выборе, когда слишком много ft полей
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field не может быть сохранен
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) Авария при использовании LEVENSHTEIN()
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore неожиданно зависает и не может нормально перезапуститься
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) CALL KEYWORDS через /sql возвращает управляющий символ, который приводит к сбою json
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb не может создать таблицу FEDERATED
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) ПРЕДУПРЕЖДЕНИЕ: dlopen() не удалось: /usr/bin/lib_manticore_columnar.so: не удалось открыть общий объектный файл: Нет такого файла или директории
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) Manticore зависает, когда поиск с ZONESPAN выполняется через api
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) неправильный вес при использовании нескольких индексов и отличий фасетов
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) Запрос группы SphinxQL зависает после повторной обработки SQL индекса
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL: Индексатор зависает в 5.0.2 и manticore-columnar-lib 1.15.4
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED возвращает пустое множество (MySQL 8.0.28)
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) выбор COUNT DISTINCT по 2 индексам, когда результат равен нулю, вызывает внутреннюю ошибку
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) АВАРИЯ на запросе удаления
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL: Ошибка с длинным текстовым полем
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex: Поведение лимита агрегации поиска не соответствует ожиданиям
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) Возвращенные_hits_ - это объект Nonetype даже для запросов, которые должны вернуть несколько результатов
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) Авария при использовании атрибута и сохраненного поля в выражении SELECT
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) таблица становится невидимой после аварии
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) Два отрицательных термина в запросе поиска дают ошибку: запрос не вычисляемый
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c не работает через json query_string
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) pseudo_sharding с запросом совпадения
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 min/max функция не работает так, как ожидалось ...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) Поле "weight" не парсится корректно
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Сервис Manticore аварийно завершает работу при запуске и продолжает перезапускаться
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) группировать по j.a, что-то работает неправильно
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) Авария Searchd при использовании expr в ранжировании, но только для запросов с двумя близостями
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action сломан
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL: Manticore зависает при выполнении запроса и другие аварии при восстановлении кластера.
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE выводит без обратных кавычек
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) Теперь возможно выполнять запросы к Manticore из Java через JDBC коннектор
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) проблемы ранжирования bm25f
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) индексы без конфигурации заморожены в watchdog в состоянии первой загрузки
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) Segfault при сортировке данных фасетов
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) авария при CONCAT(TO_STRING)
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) В некоторых случаях простой одиночный запрос мог привести к зависанию всего экземпляра, так что вы не могли войти в него или выполнить какой-либо другой запрос до завершения выполняющегося запроса.
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Авария индексатора
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) неправильное количество из уникальных значений фасета
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) LCS рассчитывается неправильно в встроенном ранжировании sph04
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 dev авария
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) FACET с json в столбце engine приводит к аварии
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL: 5.0.3 авария из-за вторичного индекса
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit в HandleMysqlSelectSysvar
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) Исправление распределения потоков в RT индексах
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) Исправление распределения потоков в RT индексах
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) неправильное значение по умолчанию max_query_time
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) Авария при использовании регулярного выражения в многопоточном выполнении
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) Плохая обратная совместимость индекса
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool сообщает об ошибке при проверке атрибутов столбца
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) утечка памяти клонов json grouper
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) Утечка памяти клонирования функции levenshtein
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) Сообщение об ошибке потеряно при загрузке метаданных
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) Распространение ошибок из динамических индексов/подключей и системных переменных
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) Авария при подсчете уникальных значений по столбцу в колонно-ориентированном хранилище
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: min(pickup_datetime) из taxi1 вызывает авария
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) пустой исключения JSON запроса удаляет столбцы из списка выбора
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) Вторичные задачи, выполняющиеся на текущем планировщике, иногда вызывают ненормальные побочные эффекты
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) авария с уникальными значениями фасета и различными схемами
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL: Колонно-ориентированный rt индекс стал поврежден после выполнения без колонно-ориентированной библиотеки
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) неявный обрезка не работает в json
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) Проблема с колонным группером
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) Невозможно удалить последнее поле из индекса
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) неправильное поведение после --new-cluster
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) "колонная библиотека не загружена", но она не требуется
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) отсутствие ошибки для запроса на удаление
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) Исправлено расположение файла данных ICU в сборках Windows
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) Проблема отправки рукопожатия
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) Отображение id в show create table
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Авария / сегментация при поиске по фасету с большим количеством результатов.
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT индекс: searchd "застревает" навсегда, когда в базе данных вставляется много документов и RAMchunk заполняется
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) Поток зависает при завершении работы, пока репликация занята между узлами
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) Смешивание чисел с плавающей запятой и целых в фильтре диапазона JSON может заставить Manticore игнорировать фильтр
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) Фильтры с плавающей запятой в JSON были неточными
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) Отменить неподтвержденные транзакции, если индекс изменен (или это может привести к аварии)
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) Ошибка синтаксиса запроса при использовании обратной косой черты
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) workers_clients могут быть неправильными в SHOW STATUS
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) исправлена авария при слиянии ram сегментов без хранилищ документов
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) Исправлено пропущенное условие ALL/ANY для фильтра равенства JSON
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) Репликация могла провалиться с `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`, если searchd был запущен из директории, в которую он не может записывать.
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) Начиная с версии 4.0.2, дампы аварийного завершения включали только смещения. Этот коммит исправляет это.

# Версия 5.0.2
Выпущено: 30 мая 2022 года

### Исправления ошибок
* ❗[Вопрос #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - неправильный размер стека может вызывать сбой.

# Версия 5.0.0
Выпущено: 18 мая 2022 года


### Основные новые функции
* 🔬 Поддержка [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/), которая включает бета-версию [вторичных индексов](../Server_settings/Searchd.md#secondary_indexes). Построение вторичных индексов включено по умолчанию для обычных и реального времени колоннарных и построчных индексов (если используется [библиотека Manticore Columnar](https://github.com/manticoresoftware/columnar)), но чтобы включить это для поиска, вам необходимо установить `secondary_indexes = 1` либо в вашем конфигурационном файле, либо с использованием [SET GLOBAL](../Server_settings/Setting_variables_online.md). Новая функциональность поддерживается на всех операционных системах, кроме старых Debian Stretch и Ubuntu Xenial.
* [Режим только для чтения](Security/Read_only.md): теперь вы можете указывать слушателей, которые обрабатывают только запросы на чтение, игнорируя любые записи.
* Новый [/cli](../Connecting_to_the_server/HTTP.md#/cli) конечный пункт для выполнения SQL-запросов по HTTP еще проще.
* Быстрее массовый INSERT/REPLACE/DELETE через JSON по HTTP: ранее вы могли предоставлять несколько команд записи через HTTP JSON протокол, но они обрабатывались по одной, теперь они обрабатываются как одна транзакция.
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) Поддержка [вложенных фильтров](../Searching/Filters.md#Nested-bool-query) в JSON-протоколе. Ранее вы не могли закодировать вещи вроде `a=1 and (b=2 or c=3)` в JSON: `must` (AND), `should` (OR) и `must_not` (NOT) работали только на самом высоком уровне. Теперь они могут быть вложенными.
* Поддержка [кодирования передачи с чанками](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) в HTTP-протоколе. Теперь вы можете использовать кодирование с чанками в вашем приложении для передачи больших партий с уменьшенным потреблением ресурсов (так как расчет `Content-Length` ненужен). На стороне сервера Manticore теперь всегда обрабатывает входящие HTTP-данные в потоковом режиме, не дожидаясь, пока вся партия будет передана, как это было раньше, что:
  - снижает пиковое использование RAM, уменьшая риск OOM
  - сокращает время ответа (наши тесты показали 11% сокращения для обработки 100MB партии)
  - позволяет вам обойти [max_packet_size](../Server_settings/Searchd.md#max_packet_size) и передавать партии, значительно превышающие максимальное разрешенное значение `max_packet_size` (128MB), например, 1GB за раз.
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) Поддержка HTTP-интерфейса `100 Continue`: теперь вы можете передавать большие партии из `curl` (включая библиотеки curl, используемые различными языками программирования), которые по умолчанию используют `Expect: 100-continue` и ждут некоторое время перед фактической отправкой партии. Ранее вам нужно было добавлять заголовок `Expect:`, теперь это не нужно.

  <details>

  Ранее (обратите внимание на время ответа):

  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Mr. Johann Smith","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Hector Pouros","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   Trying 127.0.0.1...
  * Connected to localhost (127.0.0.1) port 9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  * Done waiting for 100-continue
  * We are completely uploaded and fine
  < HTTP/1.1 200 OK
  < Server: 4.2.0 15e927b@211223 release (columnar 1.11.4 327b3d4@211223)
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 434
  <
  * Connection #0 to host localhost left intact
  {"items":[{"insert":{"table":"user","_id":2811798918248005633,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005634,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005635,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005636,"created":true,"result":"created","status":201}}],"errors":false}
  real	0m1.022s
  user	0m0.001s
  sys	0m0.010s
  ```

  Теперь:
  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Mr. Johann Smith","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Hector Pouros","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   Trying 127.0.0.1...
  * Connected to localhost (127.0.0.1) port 9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  < HTTP/1.1 100 Continue
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 0
  * We are completely uploaded and fine
  < HTTP/1.1 200 OK
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 147
  <
  * Connection #0 to host localhost left intact
  {"items":[{"bulk":{"table":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real	0m0.015s
  user	0m0.005s
  sys	0m0.004s
  ```

  </details>

* **⚠️ РАЗРУШАЮЩЕЕ ИЗМЕНЕНИЕ**: [Псевдо шардирование](../Server_settings/Searchd.md#pseudo_sharding) включено по умолчанию. Если вы хотите отключить его, убедитесь, что добавили `pseudo_sharding = 0` в секцию `searchd` вашего конфигурационного файла Manticore.
* Наличие хотя бы одного текстового поля в индексе реального времени/простом индексе больше не является обязательным. Теперь вы можете использовать Manticore даже в случаях, не связанных с полнотекстовым поиском.
* [Быстрый выбор](../Creating_a_table/Data_types.md#fast_fetch) для атрибутов, поддерживаемых [Библиотекой колонок Manticore](https://github.com/manticoresoftware/columnar): запросы типа `select * from <columnar table>` теперь намного быстрее, чем раньше, особенно если в схеме много полей.
* **⚠️ РАЗРУШАЮЩЕЕ ИЗМЕНЕНИЕ**: Неявный [порог](../Searching/Options.md#cutoff). Manticore теперь не тратит время и ресурсы на обработку данных, которые вам не нужны в результирующем наборе, который будет возвращён. Недостаток заключается в том, что это влияет на `total_found` в [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META) и [hits.total](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) в JSON-выводе. Теперь это точно только в том случае, если вы видите `total_relation: eq`, в то время как `total_relation: gte` означает, что фактическое количество совпадающих документов больше значения `total_found`, который вы получили. Для сохранения предыдущего поведения вы можете использовать опцию поиска `cutoff=0`, которая делает `total_relation` всегда `eq`.
* **⚠️ РАЗРУШАЮЩЕЕ ИЗМЕНЕНИЕ**: Все полнотекстовые поля теперь [хранятся](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) по умолчанию. Вам нужно использовать `stored_fields = ` (пустое значение), чтобы сделать все поля не сохранёнными (т.е. вернуться к предыдущему поведению).
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON поддерживает [опции поиска](../Searching/Options.md#General-syntax).

### Незначительные изменения
* **⚠️ РАЗРУШАЮЩЕЕ ИЗМЕНЕНИЕ**: Изменение формата метафайла индекса. Ранее метафайлы (`.meta`, `.sph`) были в бинарном формате, теперь это просто json. Новая версия Manticore автоматически конвертирует старые индексы, но:
  - вы можете получить предупреждение, такое как `WARNING: ... syntax error, unexpected TOK_IDENT`
  - вы не сможете запустить индекс с предыдущими версиями Manticore, убедитесь, что у вас есть резервная копия
* **⚠️ РАЗРУШАЮЩЕЕ ИЗМЕНЕНИЕ**: Поддержка состояния сеанса с помощью [HTTP keep-alive](../Connecting_to_the_server/HTTP.md#Keep-alive). Это делает HTTP состоянием, если клиент тоже это поддерживает. Например, используя новую [/cli](../Connecting_to_the_server/HTTP.md#/cli) конечную точку и HTTP keep-alive (который включён по умолчанию во всех браузерах) вы можете вызвать `SHOW META` после `SELECT`, и он будет работать так же, как он работает через mysql. Обратите внимание, что ранее заголовок HTTP `Connection: keep-alive` также поддерживался, но это только способствовало повторному использованию того же соединения. С этой версии это также делает сеанс состоянием.
* Теперь вы можете указать `columnar_attrs = *`, чтобы определить все ваши атрибуты как колоннарные в [простом режиме](Read_this_first.md#Real-time-mode-vs-plain-mode), что полезно в случае, если список длинный.
* Быстрая репликация SST
* **⚠️ РАЗРУШАЮЩЕЕ ИЗМЕНЕНИЕ**: Протокол репликации был изменён. Если вы запускаете кластер репликации, то при обновлении до Manticore 5 вам нужно:
  - сначала корректно остановить все ваши узлы
  - а затем запустить узел, который был остановлен последним, с помощью `--new-cluster` (запустите инструмент `manticore_new_cluster` в Linux).
  - прочитать о [перезапуске кластера](Создание_кластера/Настройка_репликации/Перезапуск_кластера.md#Перезапуск_кластера) для получения дополнительных сведений.
* Усовершенствования репликации:
  - Быстрая SST
  - Устойчивость к шуму, что может помочь в случае нестабильной сети между узлами репликации
  - Улучшенное логирование
* Улучшение безопасности: Manticore теперь слушает на `127.0.0.1` вместо `0.0.0.0`, если в конфигурации не указано `listen`. Хотя в стандартной конфигурации, поставляемой с Manticore Search, параметр `listen` указан и нетипично иметь конфигурацию без `listen`, это все еще возможно. Ранее Manticore слушал на `0.0.0.0`, что небезопасно, теперь он слушает на `127.0.0.1`, который обычно не подвержен интернету.
* Более быстрая агрегация по колонным атрибутам.
* Увеличена точность `AVG()`: ранее Manticore использовал `float` для агрегаций, теперь используется `double`, что значительно увеличивает точность.
* Улучшена поддержка JDBC MySQL драйвера.
* Поддержка `DEBUG malloc_stats` для [jemalloc](https://github.com/jemalloc/jemalloc).
* [optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) теперь доступен как параметр для каждой таблицы, который можно задать при создании или изменении таблицы.
* **⚠️ СЛОМАННОЕ ИЗМЕНЕНИЕ**: [query_log_format](../Server_settings/Searchd.md#query_log_format) теперь **`sphinxql` по умолчанию**. Если вы привыкли к формату `plain`, вам необходимо добавить `query_log_format = plain` в ваш файл конфигурации.
* Значительные улучшения потребления памяти: Manticore теперь потребляет значительно меньше ОЗУ в случае долгой и интенсивной нагрузки на вставку/замену/оптимизацию, если используются хранимые поля.
* [shutdown_timeout](../Server_settings/Searchd.md#shutdown_timeout) значение по умолчанию было увеличено с 3 секунд до 60 секунд.
* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) Поддержка Java mysql connector >= 6.0.3: в [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) они изменили способ подключения к mysql, что нарушило совместимость с Manticore. Теперь поддерживается новое поведение.
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) отключено сохранение нового дискового чанка при загрузке индекса (например, при запуске searchd).
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) Поддержка glibc >= 2.34.
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) счет 'VIP' подключений отдельно от обычных (не-VIP). Ранее VIP подключения учитывались в лимите `max_connections`, что могло привести к ошибке "достигнут максимум" для не-VIP подключений. Теперь VIP подключения не учитываются в лимите. Текущее количество VIP подключений также можно увидеть в `SHOW STATUS` и `status`.
* [ID](../Creating_a_table/Data_types.md#Document-ID) теперь можно явно указывать.
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) поддержка zstd сжатия для mysql proto

### ⚠️ Другие незначительные сломанные изменения
* ⚠️ Формула BM25F была немного обновлена для улучшения релевантности поиска. Это влияет только на результаты поиска в случае, если вы используете функцию [BM25F()](../Functions/Searching_and_ranking_functions.md#BM25F%28%29), она не изменяет поведение формулы ранжирования по умолчанию.
* ⚠️ Изменилось поведение REST [/sql](../Connecting_to_the_server/HTTP.md#mode=raw) конечной точки: `/sql?mode=raw` теперь требует экранирования и возвращает массив.
* ⚠️ Изменение формата ответа `/bulk` на запросы INSERT/REPLACE/DELETE:
  - ранее каждый подзапрос составлял отдельную транзакцию и приводил к отдельному ответу
  - теперь весь пакет рассматривается как одна транзакция, которая возвращает один ответ
* ⚠️ Параметры поиска `low_priority` и `boolean_simplify` теперь требуют значение (`0/1`): ранее можно было использовать `SELECT ... OPTION low_priority, boolean_simplify`, теперь нужно делать `SELECT ... OPTION low_priority=1, boolean_simplify=1`.
* ⚠️ Если вы используете старые клиентские библиотеки [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php), [python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) или [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java), пожалуйста, перейдите по соответствующей ссылке и найдите обновленную версию. **Старые версии не полностью совместимы с Manticore 5.**
* ⚠️ HTTP JSON запросы теперь регистрируются в другом формате в режиме `query_log_format=sphinxql`. Ранее регистрировалась только часть полнотекстового запроса, теперь это регистрируется как есть.

### Новые пакеты
* **⚠️ СЛОМАННОЕ ИЗМЕНЕНИЕ**: из-за новой структуры при обновлении до Manticore 5 рекомендуется удалить старые пакеты перед установкой новых:
  - RPM-основанные: `yum remove manticore*`
  - Debian и Ubuntu: `apt remove manticore*`
* Новая структура deb/rpm пакетов. Предыдущие версии предоставляли:
  - `manticore-server` с `searchd` (основной поисковый демон) и всем необходимым для него
  - `manticore-tools` с `indexer` и `indextool`
  - `manticore` со всем
  - `manticore-all` RPM как мета-пакет, ссылающийся на `manticore-server` и `manticore-tools`

  Новая структура:
  - `manticore` - мета-пакет deb/rpm, который устанавливает все вышеупомянутое как зависимости
  - `manticore-server-core` - `searchd` и всё необходимое для его работы отдельно
  - `manticore-server` - файлы systemd и другие вспомогательные скрипты
  - `manticore-tools` - `indexer`, `indextool` и другие инструменты
  - `manticore-common` - файл конфигурации по умолчанию, каталог данных по умолчанию, стоп-слова по умолчанию
  - `manticore-icudata`, `manticore-dev`, `manticore-converter` не изменились значительно
  - `.tgz` пакет, который включает все пакеты
* Поддержка Ubuntu Jammy
* Поддержка Amazon Linux 2 через [YUM репозиторий](../Installation/RHEL_and_Centos.md#YUM-repository)

### Исправления ошибок
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) Случайный сбой при использовании UDF функции
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) нехватка памяти при индексации RT индекса
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) Ломающее изменение 3.6.0, 4.2.0 sphinxql-parser
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) FATAL: недостаточно памяти (невозможно выделить 9007199254740992 байт)
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) Строки неправильно передаются в UDF
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) Searchd аварийно завершается после попытки добавить текстовый столбец в индекс rt
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Индексатор не смог найти все столбцы
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) Группировка по json.boolean работает неправильно
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) Команды indextool, связанные с индексом (например, --dumpdict) останавливаются с ошибкой
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) Поля исчезают из выбора
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) Несовместимость Content-Type в .NET HttpClient при использовании `application/x-ndjson`
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) Расчет длины поля
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) create/insert into/drop columnar table вызывает утечку памяти
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) Пустой столбец в результатах при определенных условиях
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) Ошибка демона при запуске
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) Демон зависает при запуске
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) Аварийное завершение на SST
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) Json-атрибут помечен как колонный, когда engine='columnar'
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) Репликация слушает на 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * не работает с csvpipe
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) Авария при выборе float в колонном формате в rt
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool изменяет rt индекс во время проверки
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) Необходима проверка на пересечение диапазонов портов слушателей
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) Зарегистрировать оригинальную ошибку в случае, если индекс RT не смог сохранить чанк диска
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) Отчет об ошибках только один для конфигурации RE2
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) Изменения потребления оперативной памяти в коммите 5463778558586d2508697fa82e71d657ac36510f
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 3-й узел не образует непервичный кластер после грязного перезапуска
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) Счетчик обновлений увеличивается на 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) Новая версия 4.2.1 портит индекс, созданный с 4.2.0 с использованием морфологии
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) Нет экранирования в json-ключах /sql?mode=raw
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) Использование функции скрывает другие значения
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) Утечка памяти, вызванная строкой в FixupAttrForNetwork
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) Утечка памяти в 4.2.0 и 4.2.1, связанная с кэшем docstore
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) Странное поведение ping-pong с хранимыми полями через сеть
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) lemmatizer_base сбрасывается в пустое состояние, если не упомянуто в разделе 'common'
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding делает SELECT по id медленнее
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) DEBUG malloc_stats выводит нули при использовании jemalloc
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) Удаление/добавление столбца делает значение невидимым
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) Невозможно добавить столбец bit(N) в колонный таблицу
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) "cluster" становится пустым при запуске в manticore.json
* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP действия не отслеживаются в SHOW STATUS
* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) отключить pseudo_sharding для запросов с низкой частотой по отдельным ключевым словам
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) исправлено хранимые атрибуты против слияния индекса
* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) обобщенные извлекатели уникальных значений; добавлены специализированные извлекатели уникальных значений для колонных строк
* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) исправлено извлечение нулевых целочисленных атрибутов из docstore
* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` может быть указан дважды в логах запросов

## Версия 4.2.0, 23 декабря 2021 года

### Основные новшества
* **Поддержка псевдо-шардинга для индексов реального времени и полнотекстовых запросов**. В предыдущем релизе мы добавили ограниченную поддержку псевдо-шардинга. Начиная с этой версии, вы можете получить все преимущества псевдо-шардинга и вашего многопроцессорного процессора, просто включив [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding). Самое крутое в том, что вам не нужно делать ничего с вашими индексами или запросами, просто включите это, и если у вас есть свободный ЦП, он будет использован для снижения времени ответа. Поддерживаются обычные и индексы реального времени для полнотекстовых, фильтрации и аналитических запросов. Например, вот как включение псевдо-шардинга может сделать время **ответа большинства запросов в среднем примерно в 10 раз ниже** на [искусно подобранном наборе комментариев Hacker news](https://zenodo.org/record/45901/), умноженном на 100 (116 миллионов документов в обычном индексе).

![Псевдо-шардинг включен и выключен в 4.2.0](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) теперь поддерживается.

<!-- example pq_transactions_4.2.0 -->
* Транзакции PQ теперь атомарны и изолированы. Ранее поддержка транзакций PQ была ограничена. Это позволяет значительно **ускорить REPLACE в PQ**, особенно когда вам нужно заменить много правил одновременно. Подробности о производительности:

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

Вставка **1M правил PQ занимает 48 секунд** и **REPLACE всего 40K** за 406 секунд в партиях по 10K.

```sql
root@perf3 ~ # mysql -P9306 -h0 -e "drop table if exists pq; create table pq (f text, f2 text, j json, s string) type='percolate';"; date; for m in `seq 1 1000`; do (echo -n "insert into pq (id,query,filters,tags) values "; for n in `seq 1 1000`; do echo -n "(0,'@f (cat | ( angry dog ) | (cute mouse)) @f2 def', 'j.json.language=\"en\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; [ $n != 1000 ] && echo -n ","; done; echo ";")|mysql -P9306 -h0; done; date; mysql -P9306 -h0 -e "select count(*) from pq"

Wed Dec 22 10:24:30 AM CET 2021
Wed Dec 22 10:25:18 AM CET 2021
+----------+
| count(*) |
+----------+
|  1000000 |
+----------+

root@perf3 ~ # date; (echo "begin;"; for offset in `seq 0 10000 30000`; do n=0; echo "replace into pq (id,query,filters,tags) values "; for id in `mysql -P9306 -h0 -NB -e "select id from pq limit $offset, 10000 option max_matches=1000000"`; do echo "($id,'@f (tiger | ( angry bear ) | (cute panda)) @f2 def', 'j.json.language=\"de\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; n=$((n+1)); [ $n != 10000 ] && echo -n ","; done; echo ";"; done; echo "commit;") > /tmp/replace.sql; date
Wed Dec 22 10:26:23 AM CET 2021
Wed Dec 22 10:26:27 AM CET 2021
root@perf3 ~ # time mysql -P9306 -h0 < /tmp/replace.sql

real	6m46.195s
user	0m0.035s
sys	0m0.008s
```

<!-- intro -->

#### 4.2.0

<!-- request 4.2.0 -->

Вставка **1M правил PQ занимает 34 секунды** и **REPLACE их** за 23 секунды в партиях по 10K.

```sql
root@perf3 ~ # mysql -P9306 -h0 -e "drop table if exists pq; create table pq (f text, f2 text, j json, s string) type='percolate';"; date; for m in `seq 1 1000`; do (echo -n "insert into pq (id,query,filters,tags) values "; for n in `seq 1 1000`; do echo -n "(0,'@f (cat | ( angry dog ) | (cute mouse)) @f2 def', 'j.json.language=\"en\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; [ $n != 1000 ] && echo -n ","; done; echo ";")|mysql -P9306 -h0; done; date; mysql -P9306 -h0 -e "select count(*) from pq"

Wed Dec 22 10:06:38 AM CET 2021
Wed Dec 22 10:07:12 AM CET 2021
+----------+
| count(*) |
+----------+
|  1000000 |
+----------+

root@perf3 ~ # date; (echo "begin;"; for offset in `seq 0 10000 990000`; do n=0; echo "replace into pq (id,query,filters,tags) values "; for id in `mysql -P9306 -h0 -NB -e "select id from pq limit $offset, 10000 option max_matches=1000000"`; do echo "($id,'@f (tiger | ( angry bear ) | (cute panda)) @f2 def', 'j.json.language=\"de\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; n=$((n+1)); [ $n != 10000 ] && echo -n ","; done; echo ";"; done; echo "commit;") > /tmp/replace.sql; date
Wed Dec 22 10:12:31 AM CET 2021
Wed Dec 22 10:14:00 AM CET 2021
root@perf3 ~ # time mysql -P9306 -h0 < /tmp/replace.sql

real	0m23.248s
user	0m0.891s
sys	0m0.047s
```

<!-- end -->

### Небольшие изменения
*  [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) теперь доступна как параметр конфигурации в разделе `searchd`. Это полезно, когда вы хотите ограничить количество RT-кусков во всех ваших индексах до определенного числа глобально.
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) точный [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) и [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) для нескольких локальных физических индексов (реального времени/обычные) с одинаковыми установленными/упорядоченными полями.
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) поддержка bigint для `YEAR()` и других функций временных меток.
* [Коммит 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) Адаптивный [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Ранее Manticore Search собирал ровно до `rt_mem_limit` данных перед сохранением нового дискового чанка на диск, и во время сохранения он продолжал собирать до 10% больше (так называемый двойной буфер), чтобы минимизировать возможные прерывания вставки. Если этот лимит также был исчерпан, добавление новых документов блокировалось до полного сохранения дискового чанка на диск. Новый адаптивный лимит основан на том факте, что у нас теперь есть [автооптимизация](Server_settings/Searchd.md#auto_optimize), так что не так уж важно, если дисковые чанки не полностью соблюдают `rt_mem_limit` и начинают сбрасывать дисковый чанк раньше. Теперь мы собираем до 50% `rt_mem_limit` и сохраняем это как дисковый чанк. При сохранении мы смотрим на статистику (сколько мы сохранили, сколько новых документов поступило во время сохранения) и перерасчитываем начальную ставку, которая будет использована в следующий раз. Например, если мы сохранили 90 миллионов документов, а еще 10 миллионов документов поступило во время сохранения, ставка составляет 90%, поэтому мы знаем, что в следующий раз мы можем собрать до 90% `rt_mem_limit` перед началом сброса другого дискового чанка. Значение ставки рассчитывается автоматически от 33,3% до 95%.
* [Проблема #628](https://github.com/manticoresoftware/manticoresearch/issues/628) [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib) для источника PostgreSQL. Спасибо, [Дмитрию Воронину](https://github.com/dimv36) за [вклад](https://github.com/manticoresoftware/manticoresearch/pull/630).
* [Коммит 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` и `--version`. Ранее вы могли увидеть версию индексатора, но `-v`/`--version` не поддерживались.
* [Проблема #662](https://github.com/manticoresoftware/manticoresearch/issues/662) бесконечный mlock лимит по умолчанию, когда Manticore запускается через systemd.
* [Коммит 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) spinlock -> op queue для coro rwlock.
* [Коммит 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) переменная окружения `MANTICORE_TRACK_RT_ERRORS`, полезная для отладки повреждений сегментов RT.

### Ломающие изменения
* Версия binlog была увеличена, binlog из предыдущей версии не будет воспроизведен, поэтому убедитесь, что вы останавливаете Manticore Search корректно во время обновления: никаких файлов binlog не должно быть в `/var/lib/manticore/binlog/`, кроме `binlog.meta` после остановки предыдущего экземпляра.
* [Коммит 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) новый столбец "chain" в `show threads option format=all`. Он показывает стек некоторых служебных информационных билетов, наиболее полезный для профилирования, так что если вы разбираете вывод `show threads`, будьте внимательны к новому столбцу.
* `searchd.workers` был устаревшим с версии 3.5.0, теперь он объявлен устаревшим, если у вас все еще есть его в вашем конфигурационном файле, это вызовет предупреждение при запуске. Manticore Search запустится, но с предупреждением.
* Если вы используете PHP и PDO для доступа к Manticore, вам нужно установить `PDO::ATTR_EMULATE_PREPARES`

### Исправления ошибок
* ❗[Проблема #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 медленнее, чем Manticore 3.6.3. 4.0.2 был быстрее, чем предыдущие версии по объему вставок, но значительно медленнее для вставок одного документа. Это было исправлено в 4.2.0.
* ❗[Коммит 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) Индекс RT мог быть поврежден под интенсивной нагрузкой REPLACE, или он мог аварийно завершаться.
* [Коммит 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) исправлено среднее значение на слиянии группировщиков и сортировщиков группов N; исправлено слияние агрегатов.
* [Коммит 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` мог аварийно завершиться.
* [Коммит 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) Проблема исчерпания RAM, вызванная UPDATE.
* [Коммит 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) демон мог зависнуть во время INSERT.
* [Коммит 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) демон мог зависнуть при завершении.
* [Коммит f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) демон мог аварийно завершиться при завершении.
* [Коммит 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) демон мог зависнуть при аварийной ситуации.
* [Коммит f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) демон мог аварийно завершиться при запуске, пытаясь повторно присоединиться к кластеру с неверным списком узлов.
* [Коммит 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) распределенный индекс мог быть полностью забыт в режиме RT в случае, если он не мог разрешить одного из своих агентов при старте.
* [Проблема #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' не работает.
* [Проблема #682](https://github.com/manticoresoftware/manticoresearch/issues/682) создание таблицы не удалось, но оставило директорию.
* [Проблема #663](https://github.com/manticoresoftware/manticoresearch/issues/663) Конфигурация не проходит: неизвестное имя ключа 'attr_update_reserve'.
* [Проблема #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore аварийно завершается на пакетных запросах.
* [Проблема #679](https://github.com/manticoresoftware/manticoresearch/issues/679) Пакетные запросы снова вызывают аварии в версии 4.0.3
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) исправил крах демона при запуске, пытающегося повторно подключиться к кластеру с недействительным списком узлов
* [Issue #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 не принимает подключения после пакета вставок
* [Issue #635](https://github.com/manticoresoftware/manticoresearch/issues/635) Запрос FACET с ORDER BY JSON.field или строковым атрибутом может вызвать сбой
* [Issue #634](https://github.com/manticoresoftware/manticoresearch/issues/634) Сбой SIGSEGV при запросе с packedfactors
* [Commit 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields не поддерживался при создании таблицы

## Версия 4.0.2, 21 сен 2021

### Основные новые функции
- **Полная поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**. Ранее Manticore Columnar Library поддерживалась только для обычных индексов. Теперь она поддерживается:
  - в индексах в реальном времени для `INSERT`, `REPLACE`, `DELETE`, `OPTIMIZE`
  - в репликации
  - в `ALTER`
  - в `indextool --check`
- **Автоматическая компактация индексов** ([Issue #478](https://github.com/manticoresoftware/manticoresearch/issues/478)). В конечном итоге, вам больше не нужно вручную вызывать OPTIMIZE или через крон-задачу или другой вид автоматизации. Manticore теперь делает это автоматически и по умолчанию. Вы можете установить порог компактации по умолчанию через глобальную переменную [optimize_cutoff](../Server_settings/Setting_variables_online.md).
- **Революция системы снимков и блокировок чанков**. Эти изменения могут быть с первого взгляда невидимыми снаружи, но они значительно улучшают поведение многих операций, происходящих в индексах в реальном времени. В двух словах, ранее большинство операций манипуляции данными Manticore сильно зависели от блокировок, теперь мы используем снимки дисковых чанков вместо этого.
- **Значительно более высокая производительность пакетного INSERT в индекс в реальном времени**. Например, на [сервере AX101 от Hetzner](https://www.hetzner.com/dedicated-rootserver/ax101) с SSD, 128 ГБ ОЗУ и AMD Ryzen™ 9 5950X (16*2 ядра) **с 3.6.0 вы могли получить 236K документов в секунду** вставлено в таблицу со схемой `name text, email string, description text, age int, active bit(1)` (по умолчанию `rt_mem_limit`, размер пакета 25000, 16 параллельных рабочих процессов вставки, всего вставлено 16 миллионов документов). В 4.0.2 тот же уровень параллелизма/размер пакета/количество дает **357K документов в секунду**.

  <details>

  - операции чтения (например, SELECT, репликация) выполняются с использованием снимков
  - операции, которые просто изменяют внутреннюю структуру индекса, не модифицируя схему/документы (например, объединение сегментов ОЗУ, сохранение дисковых чанков, объединение дисковых чанков) выполняются с использованием только для чтения снимков и в конце заменяют существующие чанки
  - UPDATE и DELETE выполняются по существующим чанкам, но в случае объединения, которое может происходить, записи собираются и затем применяются к новым чанкам
  - UPDATE последовательно получает эксклюзивную блокировку для каждого чанка. Объединения получают совместимую блокировку, когда входят в стадию сбора атрибутов из чанка. Таким образом, в одно и то же время только одна (объединение или обновление) операция имеет доступ к атрибутам чанка.
  - когда объединение доходит до фазы, когда ему нужны атрибуты, оно устанавливает специальный флаг. Когда UPDATE завершает работу, он проверяет флаг, и если он установлен, все обновление сохраняется в специальной коллекции. Наконец, когда объединение заканчивается, оно применяет обновления к новому дисковому чанку.
  - ALTER выполняется через эксклюзивную блокировку
  - репликация выполняется как обычная операция чтения, но дополнительно сохраняет атрибуты перед SST и запрещает обновления во время SST

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) может добавлять/удалять поле полнотекстового поиска** (в режиме RT). Ранее он мог только добавлять/удалять атрибут.
- 🔬 **Экспериментально: псевдораспределение для запросов полного сканирования** - позволяет параллелизировать любой запрос, который не является полнотекстовым. Вместо подготовки шардов вручную теперь вы можете просто включить новую опцию [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) и ожидать до `CPU cores` меньшего времени отклика для запросов, которые не являются полнотекстовыми. Обратите внимание, что это может легко занять все существующие ядра CPU, так что если вы заботитесь не только о задержках, но и о пропускной способности - используйте это с осторожностью.

### Небольшие изменения
<!-- example -->
- Linux Mint и Ubuntu Hirsute Hippo поддерживаются через [APT репозиторий](Installation/Debian_and_Ubuntu.md#APT-repository)
- более быстрое обновление по id через HTTP в больших индексах в некоторых случаях (в зависимости от распределения id)
- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - добавлено кэширование в lemmatizer-uk


<!-- intro -->
#### 3.6.0

<!-- request 3.6.0 -->
```
time curl -X POST -d '{"update":{"index":"idx","id":4611686018427387905,"doc":{"mode":0}}}' -H "Content-Type: application/x-ndjson" http://127.0.0.1:6358/json/bulk

real    0m43.783s
user    0m0.008s
sys     0m0.007s
```

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->
```
time curl -X POST -d '{"update":{"index":"idx","id":4611686018427387905,"doc":{"mode":0}}}' -H "Content-Type: application/x-ndjson" http://127.0.0.1:6358/json/bulk

real    0m0.006s
user    0m0.004s
sys     0m0.001s
```
<!-- end -->
- [кастомные флаги запуска для systemd](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd). Теперь вам не нужно вручную запускать searchd в случае, если вам нужно запустить Manticore с каким-либо конкретным флагом запуска
- новая функция [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29), которая вычисляет расстояние по Левенштейну
- добавлены новые [флаги запуска searchd](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` и `--replay-flags=ignore-all-errors`, так что можно все еще запустить searchd, если двоичный лог поврежден
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - раскрытие ошибок из RE2
- более точный [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) для распределенных индексов, состоящих из локальных простых индексов
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) для удаления дубликатов при выполнении фасетного поиска
- [exact form modifier](Searching/Full_text_matching/Operators.md#Exact-form-modifier) теперь не требует [морфологии](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) и работает для индексов с включенным [infix/prefix](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) поиском

### Ломающее изменения
- новая версия может читать более старые индексы, но более старые версии не могут читать индексы Manticore 4
- убрано неявное сортирование по id. Сортируйте явно, если это необходимо
- значение по умолчанию для `charset_table` изменяется с `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` на `non_cjk`
- `OPTIMIZE` выполняется автоматически. Если вам это не нужно, убедитесь, что установили `auto_optimize=0` в секции `searchd` в конфигурационном файле
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` были устаревшими, теперь они удалены
- для участников: теперь мы используем компилятор Clang для сборок под Linux, так как по нашим тестам он может собрать более быструю Manticore Search и Manticore Columnar Library
- если в запросе на поиск не указано [max_matches](Searching/Options.md#max_matches), его значение обновляется неявно на минимально необходимое для повышения производительности нового колонного хранилища. Это может повлиять на метрику `total` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), но не на `total_found`, которое является фактическим количеством найденных документов.

### Миграция с Manticore 3
- убедитесь, что вы остановили Manticore 3 корректно:
  - в `/var/lib/manticore/binlog/` не должно быть файлов binlog (только `binlog.meta` должен находиться в директории)
  - в противном случае индексы, для которых Manticore 4 не может воспроизвести binlogs, не будут запущены
- новая версия может читать более старые индексы, но более старые версии не могут читать индексы Manticore 4, поэтому убедитесь, что сделали резервную копию, если хотите легко откатить новую версию
- если вы запускаете кластер репликации, убедитесь, что вы:
  - сначала корректно остановили все ваши узлы
  - а затем запустили узел, который был остановлен последним, с `--new-cluster` (запустите инструмент `manticore_new_cluster` в Linux).
  - прочитайте о [перезагрузке кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) для получения более подробной информации

### Исправления ошибок
- Исправлено множество проблем с репликацией:
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - исправлен сбой во время SST на присоединяющем узле с активным индексом; добавлена проверка sha1 на присоединяющем узле при записи файловых сегментов для ускорения загрузки индекса; добавлен вращение измененных файлов индекса на присоединяющем узле при загрузке индекса; удаление файлов индекса на присоединяющем узле, когда активный индекс заменяется новым индексом от узла-доноров; добавлены точки журнала репликации на узле донора для отправки файлов и сегментов
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - сбой при JOIN CLUSTER в случае некорректного адреса
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - во время начальной репликации большого индекса присоединяющий узел мог завершиться с `ERROR 1064 (42000): invalid GTID, (null)`, донор мог стать неотзывчивым, пока другой узел присоединялся
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - хеш мог быть рассчитан неправильно для большого индекса, что могло привести к сбою репликации
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - сбой репликации при перезапуске кластера
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` не отображает параметр `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - высокая загрузка ЦПУ в searchd, когда он в режиме ожидания спустя примерно день
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - немедленное сбрасывание .meta
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json становится пустым
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait завершение под root не удается. Это также исправляет поведение systemctl (ранее он показывал сбой для ExecStop и не ждал достаточно долго, чтобы searchd остановился корректно)
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE против SHOW STATUS. `command_insert`, `command_replace` и другие показывали неправильные метрики
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - `charset_table` для простого индекса имел неправильное значение по умолчанию
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - новые дисковые сегменты не блокируются в памяти
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - узел кластера Manticore завершается при невозможности разрешить узел по имени
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - репликация обновленного индекса может привести к неопределенному состоянию
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - индексатор мог зависнуть при индексации источника простого индекса с json-атрибутом
- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - исправлено неравное выражение фильтра в PQ индексе
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - исправлены выборки окон для запросов списка, превышающих 1000 совпадений. `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` ранее не работал
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - HTTPS-запрос к Manticore может вызывать предупреждение "превышен максимальный размер пакета(8388608)"
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 может зависать после нескольких обновлений строковых атрибутов


## Версия 3.6.0, 3 мая 2021 года
**Релиз для обслуживания перед Manticore 4**

### Основные новые функции
- Поддержка [Библиотека Manticore Columnar](https://github.com/manticoresoftware/columnar/) для обычных индексов. Новый параметр [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) для обычных индексов
- Поддержка [Украинского лемматизатора](https://github.com/manticoresoftware/lemmatizer-uk)
- Полностью переработанные гистограммы. При построении индекса Manticore также строит гистограммы для каждого поля в нем, которые затем используются для более быстрой фильтрации. В версии 3.6.0 алгоритм был полностью пересмотрен, и вы можете получить более высокую производительность, если у вас много данных и вы делаете много фильтрации.

### Незначительные изменения
- инструмент `manticore_new_cluster [--force]`, полезный для перезапуска кластера репликации через systemd
- [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) для `indexer --merge`
- [новый режим](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- добавлена [поддержка экранирования JSON пути](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL) с помощью обратных кавычек
- [indextool --check](Miscellaneous_tools.md#indextool) может работать в режиме RT
- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) для SELECT/UPDATE
- id чанка для объединенного дискового чанка теперь уникален
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### Оптимизации
- [быстрее парсинг JSON](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606), наши тесты показывают 3-4% нижнюю латентность на запросах таких как `WHERE json.a = 1`
- недокументированная команда `DEBUG SPLIT` как предшествующее условие для автоматического шардирования/ребалансировки

### Исправления ошибок
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - неточные и нестабильные результаты FACET
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - Странное поведение при использовании MATCH: те, кто сталкивается с этой проблемой, должны пересобрать индекс, так как проблема была на этапе построения индекса
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - интермитирующий дамп ядра при выполнении запроса с функцией SNIPPET()
- Оптимизации стека, полезные для обработки сложных запросов:
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT приводит к CRASH DUMP
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - определение размера стека для фильтрационных деревьев
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - Обновление с использованием условия IN не срабатывает корректно
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - SHOW STATUS сразу после CALL PQ возвращает - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - Исправлен статический бинарный сборка
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - ошибка в многозапросах
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - Невозможно использовать нетрадиционные имена для столбцов при использовании 'create table'
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - аварийное завершение демона при воспроизведении binlog с обновлением строкового атрибута; установите версию binlog на 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - исправлено определение фрейма стека выражения во время выполнения (тест 207)
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - фильтр индекса перколята и теги были пустыми для пустого сохраненного запроса (тест 369)
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - сбои потока репликации SST в сети с высокой задержкой и высоким уровнем ошибок (репликация между разными дата-центрами); обновленная версия команды репликации до 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - блокировка кластера джойнера при операциях записи после присоединения к кластеру (тест 385)
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - совпадение подстановок с точным модификатором (тест 321)
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - контрольные точки docid против docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - Неконсистентное поведение индексации при парсинге недопустимого xml
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - Сохраненный перколятный запрос с NOTNEAR выполняется бесконечно (тест 349)
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - неверный вес для фразы, начинающейся с подстановки
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - перколяторный запрос с подстановочными знаками генерирует термины без нагрузки на совпадениях, что приводит к переплетенным попаданиям и нарушает совпадение (тест 417)
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - исправлено вычисление 'total' в случае параллелизированного запроса
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - сбой в Windows с несколькими параллельными сеансами в демоне
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - некоторые настройки индекса не могут быть реплицированы
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - при высоком уровне добавления новых событий netloop иногда зависает из-за атомарного события 'kick', которое обрабатывается один раз для нескольких событий и теряет фактические действия из них
статус запроса, а не статус сервера
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - новый сброшенный диск может быть утерян при коммите
- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - неточное 'net_read' в профайлере
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - проблема перколяции с арабским языком (тексты справа налево)
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - id не выбирается правильно при дублирующемся названии столбца
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) сетевых событий для исправления сбоя в редких случаях
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) исправление в `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE работал неправильно с сохраненными полями

### Ломающее изменения:
- Новый формат binlog: вам нужно сделать чистую остановку Manticore перед обновлением
- Формат индекса слегка изменяется: новая версия может корректно читать ваши существующие индексы, но если вы решите откатиться с 3.6.0 на более старую версию, новые индексы будут нечитаемыми
- Изменение формата репликации: не реплицируйте от более старой версии до 3.6.0 и наоборот, переключитесь на новую версию на всех ваших узлах сразу
- `reverse_scan` устарел. Убедитесь, что вы не используете эту опцию в своих запросах с 3.6.0, иначе они не сработают
- С этого релиза мы больше не предоставляем сборки для RHEL6, Debian Jessie и Ubuntu Trusty. Если это критически важно для вас, чтобы они поддерживались, [свяжитесь с нами](https://manticoresearch.com/contact-us/)

### Устаревшие функции
- Больше неявной сортировки по id. Если вы на это полагаетесь, убедитесь, что обновили свои запросы соответствующим образом
- Опция поиска `reverse_scan` устарела

## Версия 3.5.4, 10 декабря 2020 года

### Новые функции
- Новые клиенты Python, Javascript и Java теперь общедоступны и хорошо задокументированы в этом руководстве.
- автоматическое удаление дискового чанка реального времени. Эта оптимизация позволяет автоматически удалять дисковый чанк, когда [OPTIMIZируете](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) индекс реального времени, когда чанк, очевидно, больше не нужен (все документы подавлены). Ранее это все еще требовало слияния, теперь чанк может быть просто немедленно удален. Опция [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) игнорируется, т.е. даже если ничего фактически не сливается, устаревший дисковый чанк удаляется. Это полезно, если вы поддерживаете удержание в своем индексе и удаляете более старые документы. Теперь компактация таких индексов будет быстрее.
- [standalone NOT](Searching/Options.md#not_terms_only_allowed) как опция для SELECT

### Небольшие изменения
- [Проблема #453](https://github.com/manticoresoftware/manticoresearch/issues/453) Новая опция [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) полезна, если вы запускаете `indexer --all` и у вас не только обычные индексы в конфигурационном файле. Без `ignore_non_plain=1` вы получите предупреждение и соответствующий код выхода.
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) и [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) позволяют визуализировать выполнение плана запроса полного текста. Полезно для понимания сложных запросов.

### Устаревшие функции
- `indexer --verbose` устарел, так как никогда ничего не добавлял в вывод индексатора
- Для сброса обратного трассировки сигнал `USR2` теперь используется вместо `USR1`

### Исправления ошибок
- [Проблема #423](https://github.com/manticoresoftware/manticoresearch/issues/423) символы кирица период вызова фрагментов в режиме удержания не выделяются
- [Проблема #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - выражение GROUP N BY select = фатальный сбой
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) статус searchd показывает ошибку сегментации при нахождении в кластере
- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' не учитывает чанки >9
- [Проблема #389](https://github.com/manticoresoftware/manticoresearch/issues/389) Ошибка, которая приводит к сбою Manticore
- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) Конвертер создает поврежденные индексы
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 против CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) count distinct возвращает 0 при низком max_matches на локальном индексе
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) При использовании агрегации хранимые тексты не возвращаются в хите


## Версия 3.5.2, 1 октября 2020 года

### Новые функции

* OPTIMIZE уменьшает дисковые чанки до определенного количества чанков (по умолчанию `2* количество ядер`) вместо одного. Оптимальное количество чанков может быть задействовано с помощью опции [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).
* Оператор NOT теперь может использоваться самостоятельно. По умолчанию он отключен, так как случайные одиночные запросы NOT могут быть медленными. Его можно включить, установив новую директиву searchd [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) в `0`.
* Новая настройка [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) устанавливает, сколько потоков может использовать запрос. Если директива не задана, запрос может использовать потоки в количестве до значения [threads](Server_settings/Searchd.md#threads).
Для запроса `SELECT` количество потоков можно ограничить с помощью [OPTION threads=N](Searching/Options.md#threads), переопределяющего глобальный `max_threads_per_query`.
* Индексы перколяторов теперь могут быть импортированы с помощью [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md).
* HTTP API `/search` получает базовую поддержку для [фасетирования](Searching/Faceted_search.md#HTTP-JSON)/[группировки](Searching/Grouping.md) с помощью нового узла запроса `aggs`.

### Незначительные изменения

* Если директива репликации listen не объявлена, движок попытается использовать порты после определенного порта 'sphinx', до 200.
* `listen=...:sphinx` должен быть явно установлен для соединений SphinxSE или клиентов SphinxAPI.
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) выводит новые метрики: `killed_documents`, `killed_rate`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists` и `disk_mapped_cached_hitlists`.
* SQL команда `status` теперь выводит `Queue\Threads` и `Tasks\Threads`.

### Устаревания:

* `dist_threads` теперь полностью устарел, searchd будет регистрировать предупреждение, если директива все еще используется.

### Docker

Официальный образ Docker теперь основан на Ubuntu 20.04 LTS

### Упаковка

Кроме обычного пакета `manticore`, вы также можете установить Manticore Search по компонентам:

- `manticore-server-core` - предоставляет `searchd`, страницу man, каталог логов, API и модуль galera. Он также установит `manticore-common` как зависимость.
- `manticore-server` - предоставляет автоматизационные скрипты для ядра (init.d, systemd) и обертку `manticore_new_cluster`. Он также установит `manticore-server-core` как зависимость.
- `manticore-common` - предоставляет конфигурацию, стоп-слова, общие документы и скелетные папки (datadir, модули и т.д.)
- `manticore-tools` - предоставляет вспомогательные инструменты ( `indexer`, `indextool` и т.д.), их страницы man и примеры. Он также установит `manticore-common` как зависимость.
- `manticore-icudata` (RPM) или `manticore-icudata-65l` (DEB) - предоставляет файл данных ICU для использования морфологии icu.
- `manticore-devel` (RPM) или `manticore-dev` (DEB) - предоставляет заголовки dev для UDF.

### Исправления ошибок

1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) Краш демона в группировщике на RT индексе с различными чанками
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) Быстрый путь для пустых удаленных документов
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) Обнаружение фреймов стека выражений во время выполнения
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Совпадение более 32 полей на индексах перколяторов
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) Диапазон портов прослушивания репликации
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) Показать создание таблицы на pq
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) Поведение порта HTTPS
8. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) Смешивание строк хранилища документов при замене
9. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) Смена уровня сообщения о недоступности TFO на 'info'
10. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) Краш из-за некорректного использования strcmp
11. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) Добавление индекса в кластер с системными (стоп-слова) файлами
12. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) Слияние индексов с большими словарями; оптимизация RT больших дисковых чанков
13. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool может экспортировать метаданные из текущей версии
14. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) Проблема в порядке группировки в GROUP N
15. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) Явная очистка для SphinxSE после рукопожатия
16. [Commit 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) Избежать копирования огромных описаний, когда это не требуется
17. [Commit 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) Отрицательное время в показе потоков
18. [Commit f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) Плагин фильтра токенов против нулевых дельт позиций
19. [Commit a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) Изменить 'FAIL' на 'WARNING' при множественных попаданиях

## Версия 3.5.0, 22 июля 2020 года

### Основные новые функции:
* Этот релиз занял так много времени, потому что мы усердно работали над изменением многозадачного режима с потоков на **корутины**. Это упрощает настройку и делает параллелизацию запросов гораздо более понятной: Manticore просто использует заданное количество потоков (см. новую настройку [threads](Server_settings/Searchd.md#threads)), а новый режим обеспечивает выполнение в наиболее оптимальном режиме.
* Изменения в [выделении текста](Searching/Highlighting.md#Highlighting-options):
  - любое выделение, которое работает с несколькими полями (`highlight({},'field1, field2'`) или `highlight` в json-запросах) теперь применяет ограничения по полям по умолчанию.
  - любое выделение, которое работает с обычным текстом (`highlight({}, string_attr)` или `snippet()` теперь применяет ограничения ко всему документу.
  - [ограничения по полям](Searching/Highlighting.md#limits_per_field) могут быть переключены на глобальные ограничения с помощью опции `limits_per_field=0` (`1` по умолчанию).
  - [allow_empty](Searching/Highlighting.md#allow_empty) теперь `0` по умолчанию для выделения через HTTP JSON.

* Один и тот же порт [можно теперь использовать](Server_settings/Searchd.md#listen) для http, https и бинарного API (для принятия подключений от удаленного экземпляра Manticore). `listen = *:mysql` по-прежнему требуется для соединений через протокол mysql. Manticore теперь автоматически определяет тип клиента, пытающегося подключиться к нему, за исключением MySQL (из-за ограничений протокола).

* В режиме RT поле теперь может быть [текстовым и строковым атрибутом](Creating_a_table/Data_types.md#String) одновременно - [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331).

  В [обычном режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) это называется `sql_field_string`. Теперь он доступен в [режиме RT](Read_this_first.md#Real-time-mode-vs-plain-mode) и для реальных индексов. Вы можете использовать его, как показано в примере:

  <!-- more -->
  ```sql
  create table t(f string attribute indexed);
  insert into t values(0,'abc','abc');
  select * from t where match('abc');
  +---------------------+------+
  | id                  | f    |
  +---------------------+------+
  | 2810845392541843463 | abc  |
  +---------------------+------+
  1 row in set (0.01 sec)

  mysql> select * from t where f='abc';
  +---------------------+------+
  | id                  | f    |
  +---------------------+------+
  | 2810845392541843463 | abc  |
  +---------------------+------+
  1 row in set (0.00 sec)
  ```
  <!-- \more -->

### Незначительные изменения
* Теперь вы можете [выделять строковые атрибуты](Searching/Highlighting.md#Highlighting-via-SQL).
* Поддержка SSL и компрессии для SQL интерфейса
* Поддержка команды mysql [`status`](Node_info_and_management/Node_status.md#STATUS).
* [Репликация](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) теперь может реплицировать внешние файлы (стоп-слова, исключения и т.д.).
* Оператор фильтра [`in`](Searching/Filters.md#Set-filters) теперь доступен через HTTP JSON интерфейс.
* [`expressions`](Searching/Expressions.md#expressions) в HTTP JSON.
* [Теперь вы можете изменять `rt_mem_limit` на лету](https://github.com/manticoresoftware/manticoresearch/issues/344) в режиме RT, т.е. можно выполнить `ALTER ... rt_mem_limit=<новое значение>`.
* Теперь вы можете использовать [отдельные таблицы кодировок CJK](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology): `chinese`, `japanese` и `korean`.
* [thread_stack](Server_settings/Searchd.md#thread_stack) теперь ограничивает максимальный стек потока, а не начальный.
* Улучшен вывод `SHOW THREADS`.
* Отображение прогресса длительного `CALL PQ` в `SHOW THREADS`.
* cpustat, iostat, coredump могут быть изменены во время выполнения с помощью [SET](Server_settings/Setting_variables_online.md#SET).
* Реализовано `SET [GLOBAL] wait_timeout=NUM` ,

### Ломающее изменения:
* **Формат индекса изменен.** Индексы, построенные в 3.5.0, не могут быть загружены версией Manticore < 3.5.0, но Manticore 3.5.0 понимает более старые форматы.
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) (т.е. без предоставления списка столбцов) ранее ожидал точно `(query, tags)` в качестве значений. Это было изменено на `(id,query,tags,filters)`. Идентификатор может быть установлен в 0, если вы хотите, чтобы он генерировался автоматически.
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) является новым значением по умолчанию для выделения через HTTP JSON интерфейс.
* Для внешних файлов (стоп-слова, исключения и т.д.) в `CREATE TABLE`/`ALTER TABLE` теперь разрешены только абсолютные пути.

### Устаревание:
* `ram_chunks_count` был переименован в `ram_chunk_segments_count` в `SHOW INDEX STATUS`.
* `workers` устарел. Теперь есть только один режим рабочих потоков.
* `dist_threads` устарел. Все запросы теперь выполняются параллельно, насколько это возможно (ограничено `threads` и `jobs_queue_size`).
* `max_children` устарел. Используйте [threads](Server_settings/Searchd.md#threads), чтобы установить количество потоков, которые будет использовать Manticore (по умолчанию установлено на количество ядер CPU).
* `queue_max_length` устарел. Вместо этого, если это действительно необходимо, используйте [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) для тонкой настройки размера внутренней очереди задач (по умолчанию без ограничений).
* Все эндпойнты `/json/*` теперь доступны без `/json/`, например `/search`, `/insert`, `/delete`, `/pq` и т.д.
* `field` значение "полнотекстовое поле" было переименовано в "text" в `describe`.
  <!-- больше -->
  **3.4.2:**
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | field  | индексированное хранилище |
  +-------+--------+----------------+
  ```
  **3.5.0**:
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | text   | индексированное хранилище |
  +-------+--------+----------------+
  ```
  <!-- \more -->
* Кириллическая `и` не отображается как `i` в `non_cjk` charset_table (что является значением по умолчанию), так как это сильно повлияло на русские стеммеры и лемматизаторы.
* `read_timeout`. Используйте [network_timeout](Server_settings/Searchd.md#network_timeout) вместо этого, который управляет как чтением, так и записью.


### Пакеты

* Официальный пакет Ubuntu Focal 20.04
* Имя deb пакета изменено с `manticore-bin` на `manticore`

### Исправления ошибок:
1. [Вопрос #351](https://github.com/manticoresoftware/manticoresearch/issues/351) утечка памяти searchd
2. [Коммит ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) Небольшое чтение за пределами в фрагментах
3. [Коммит 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) Опасная запись в локальную переменную для запросов на сбой
4. [Коммит 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) Небольшая утечка памяти сортировщика в тесте 226
5. [Коммит d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) Огромная утечка памяти в тесте 226
6. [Коммит 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) Кластер показывает, что узлы синхронизированы, но `count(*)` показывает разные числа
7. [Коммит f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметическое: Дублирующие и иногда потерянные сообщения об ошибках в журнале
8. [Коммит f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметическое: (null) имя индекса в журнале
9. [Коммит 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) Невозможно получить более 70M результатов
10. [Коммит 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) Невозможно вставить правила PQ без колонок
11. [Коммит bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) Вводя сообщение об ошибке, вводя документ в индекс кластера
12. [Коммит 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` и `json/update` возвращают id в экспоненциальной форме
13. [Вопрос #324](https://github.com/manticoresoftware/manticoresearch/issues/324) Обновите скалярные свойства json и mva в одном запросе
14. [Коммит d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` не работает в режиме RT
15. [Коммит 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `ALTER RECONFIGURE` в режиме rt должен быть запрещен
16. [Коммит 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `rt_mem_limit` сбрасывается на 128M после перезапуска searchd
17. highlight() иногда зависает
18. [Коммит 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) Не удалось использовать U+код в режиме RT
19. [Коммит 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) Не удалось использовать подстановочный знак в формах слов в режиме RT
20. [Коммит e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) Исправлен `SHOW CREATE TABLE` против нескольких файлов форм слов
21. [Коммит fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON-запрос без "query" вызывает сбой searchd
22. Manticore [официальный docker](https://hub.docker.com/r/manticoresearch/manticore) не мог индексировать из mysql 8
23. [Коммит 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert требует id
24. [Коммит bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` не работает для PQ
25. [Коммит bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` не работает должным образом для PQ
26. [Коммит 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) Конец строки в параметрах в показе статуса индекса
27. [Коммит cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) Пустой заголовок в "highlight" в ответе JSON HTTP
28. [Вопрос #318](https://github.com/manticoresoftware/manticoresearch/issues/318) Ошибка инфикса `CREATE TABLE LIKE`
29. [Коммит 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT аварийно завершает работу под нагрузкой
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) Потеряно аварийное журналирование на аварийном отключении в RT-дисковой части
31. [Вопрос #323](https://github.com/manticoresoftware/manticoresearch/issues/323) Импорт таблицы завершается неудачей и закрывает соединение
32. [Коммит 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` повреждает индекс PQ
33. [Коммит 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) Проблемы с перезагрузкой searchd после изменения типа индекса
34. [Коммит 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) Демон аварийно завершает работу при импорте таблицы с пропущенными файлами
35. [Вопрос #322](https://github.com/manticoresoftware/manticoresearch/issues/322) Авария при выборе с использованием нескольких индексов, группировки и ранжирования = none
36. [Коммит c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` не выделяет в строковых атрибутах
37. [Вопрос #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` не удается отсортировать по строковому атрибуту
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) Ошибка в случае отсутствия каталога данных
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* не поддерживаются в режиме RT
40. [Commit 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) Плохие JSON-объекты в строках: 1. `CALL PQ` возвращает "Плохие JSON-объекты в строках: 1", когда json превышает некоторое значение.
41. [Commit 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) Несоответствие в режиме RT. В некоторых случаях я не могу удалить индекс, так как он неизвестен, и не могу создать его, так как директория не пуста.
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) Падение при выборе
43. [Commit 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M вернул предупреждение при 2M поле
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) Ошибка выполнения условий запроса
45. [Commit dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) Простой поиск из 2 терминов находит документ, содержащий только один термин
46. [Commit 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) В PQ было невозможно сопоставить json с заглавными буквами в ключах
47. [Commit 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) Индексатор падает при работе с csv+docstore
48. [Issue #363](https://github.com/manticoresoftware/manticoresearch/issues/363) Использование `[null]` в json attr в centos 7 вызывает поврежденные вставленные данные
49. Основная [Issue #345](https://github.com/manticoresoftware/manticoresearch/issues/345) Записи не вставляются, count() случайный, "replace into" возвращает ОК
50. max_query_time слишком сильно замедляет SELECT
51. [Issue #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Ошибка связи мастер-агент на Mac OS
52. [Issue #328](https://github.com/manticoresoftware/manticoresearch/issues/328) Ошибка при подключении к Manticore с Connector.Net/Mysql 8.0.19
53. [Commit daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) Исправлено экранирование \0 и оптимизирована производительность
54. [Commit 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) Исправлено различие count distinct и json
55. [Commit 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) Исправлено завершение таблицы на другом узле
56. [Commit 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) Исправьте сбои на резко выполняемом вызове pq


## Версия 3.4.2, 10 апреля 2020
### Критические исправления ошибок
* [Commit 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) Исправлено, что RT индекс из старой версии не удается индексировать данные

## Версия 3.4.0, 26 марта 2020
### Основные изменения
* сервер работает в 2 режимах: rt-mode и plain-mode
   *   rt-mode требует data_dir и отсутствие определения индекса в конфигурации
   *   в plain-mode индексы определяются в конфигурации; data_dir не допускается
* репликация доступна только в rt-mode

### Небольшие изменения
* charset_table по умолчанию устанавливается в non_cjk alias
* в rt-mode полнотекстовые поля индексируются и хранятся по умолчанию
* полнотекстовые поля в rt-mode переименованы с 'field' на 'text'
* ALTER RTINDEX переименован в ALTER TABLE
* TRUNCATE RTINDEX переименован в TRUNCATE TABLE

### Функции
* только хранимые поля
* SHOW CREATE TABLE, IMPORT TABLE

### Улучшения
* значительно более быстрый PQ без блокировок
* /sql может выполнять любой тип SQL-запроса в режиме=raw
* псевдоним mysql для протокола mysql41
* состояние по умолчанию state.sql в data_dir

### Исправления ошибок
* [Commit a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) Исправлено падение из-за неверного синтаксиса поля в highlight()
* [Commit 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) Исправлено падение сервера при репликации RT индекса с docstore
* [Commit 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) Исправлено падение при выделении для индекса с параметрами infix или prefix и для индекса без включенных хранимых полей
* [Commit 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) Исправлена ложная ошибка о пустом docstore и поиске dock-id для пустого индекса
* [Commit a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) Исправлено #314 Команда вставки SQL с завершающей точкой с запятой
* [Commit 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) Устранено предупреждение о несовпадении слова(слов) в запросе
* [Commit b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) Исправлены запросы в фрагментах, сегментированных через ICU
* [Commit 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) Исправлено состояние гонки find/add в кэше блоков docstore
* [Commit f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) Исправлена утечка памяти в docstore
* [Commit a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) Исправлено #316 LAST_INSERT_ID возвращает пустое значение при INSERT
* [Commit 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) Исправлено #317 json/update HTTP конечная точка для поддержки массива для MVA и объекта для JSON атрибута
* [Commit e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) Исправлен сбой индексатора при сбросе rt без явного id

## Версия 3.3.0, 4 февраля 2020
### Функции
* Параллельный поиск индекса в реальном времени
* КОМАНДА EXPLAIN QUERY
* конфигурационный файл без определений индексов (альфа-версия)
* КОМАНДЫ CREATE/DROP TABLE (альфа-версия)
* indexer --print-rt - может читать из источника и выводить INSERT-ы для индекса в реальном времени

### Улучшения
* Обновлено до стеммеров Snowball 2.0
* Фильтр LIKE для SHOW INDEX STATUS
* улучшено использование памяти при высоких max_matches
* SHOW INDEX STATUS добавляет ram_chunks_count для RT индексов
* PQ без блокировок
* изменено LimitNOFILE до 65536


### Исправления ошибок
* [Коммит 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) добавил проверку схемы индекса на дублирующие атрибуты #293
* [Коммит a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) исправление сбоя при отсутствии термина
* [Коммит 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) исправлено повреждение docstore после ATTACH
* [Коммит d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) исправлена проблема с docstore в распределенной настройке
* [Коммит bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) заменен FixedHash на OpenHash в сортировщике
* [Коммит e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) исправлено определение индекса с дублирующимися именами атрибутов
* [Коммит ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) исправлено html_strip в HIGHLIGHT()
* [Коммит 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) исправлен макрос passage в HIGHLIGHT()
* [Коммит a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) исправлены проблемы с двойным буфером при создании RT индекса с маленьким или большим дисковым чунком
* [Коммит a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) исправлено удаление событий для kqueue
* [Коммит 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) исправлено сохранение дискового чунка для большого значения rt_mem_limit RT индекса
* [Коммит 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) исправлена ошибка переполнения при индексировании
* [Коммит a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) исправлено добавление документа с отрицательным ID в RT индекс, теперь завершается ошибкой
* [Коммит bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) исправлен сбой сервера на поле ranker fieldmask
* [Коммит 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) исправлен сбой при использовании кэша запросов
* [Коммит dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) исправлен сбой при использовании сегментов памяти RT индекса с параллельными вставками

## Версия 3.2.2, 19 декабря 2019 г.
### Функции
* Автоматическое увеличение ID для RT индексов
* Поддержка подсветки для docstore через новую функцию HIGHLIGHT(), доступную также в HTTP API
* SNIPPET() может использовать специальную функцию QUERY(), которая возвращает текущий MATCH-запрос
* новый параметр field_separator для функций подсветки.

### Улучшения и изменения
* ленивое извлечение хранимых полей для удаленных узлов (может значительно увеличить производительность)
* строки и выражения больше не нарушают многозапросные оптимизации и FACET
* сборка RHEL/CentOS 8 теперь использует mysql libclient из mariadb-connector-c-devel
* файл данных ICU теперь поставляется с пакетами, icu_data_dir удален
* файлы службы systemd включают политику 'Restart=on-failure'
* indextool теперь может проверять индексы в реальном времени онлайн
* конфигурационный файл по умолчанию теперь /etc/manticoresearch/manticore.conf
* служба на RHEL/CentOS переименована в 'manticore' из 'searchd'
* удалены параметры query_mode и exact_phrase в сноппетах

### Исправления ошибок
* [Коммит 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) исправлен сбой на SELECT запросе через HTTP интерфейс
* [Коммит 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) исправлено то, что RT индекс сохраняет дисковые чунки, но не помечает некоторые документы как удаленные
* [Коммит e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) исправлен сбой при поиске по многим индексам или многим запросам с dist_threads
* [Коммит 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) исправлен сбой при генерации инфиксов для длинных терминов с широкими кодами utf8
* [Коммит 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) исправлена гонка при добавлении сокета к IOCP
* [Коммит cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) исправлена проблема булевых запросов против списка JSON для выбора
* [Коммит 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) исправлено, что indextool проверяет и сообщает неверное смещение пропуска списка, проверка поиска doc2row
* [Коммит 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) исправлено, что индексатор создает плохой индекс с отрицательным смещением списка при большом объеме данных
* [Коммит faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) исправлено, что JSON конвертирует только числа в строки и преобразование строк JSON в числовые значения в выражениях
* [Коммит 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) исправлено, что indextool завершает работу с кодом ошибки в случае установки нескольких команд в командной строке
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) исправить #275 состояние binlog недействительно при ошибке недостаточно места на диске
* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) исправить #279 сбой при использовании фильтра IN для JSON атрибута
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) исправить #281 неверный вызов закрытия канала
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) исправить зависание сервера при вызове PQ с рекурсивным JSON атрибутом, закодированным как строка
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) исправить переход за пределы doclist в узле multiand
* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) исправить получение публичной информации потока
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) исправить блокировки кеша docstore

## Версия 3.2.0, 17 Октября 2019
### Особенности
* Хранение документов
* новые директивы stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level

### Улучшения и изменения
* улучшенная поддержка SSL
* обновлен встроенный charset для non_cjk
* отключено ведение журнала команд UPDATE/DELETE, логируя SELECT в журнале запросов
* пакеты RHEL/CentOS 8

### Исправления
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) исправить сбой при замене документа в дисковом куске RT индекса
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) исправить \#269 LIMIT N OFFSET M
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) исправить команды DELETE с явно установленным id или списком id для пропуска поиска
* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) исправить неверный индекс после удаления события в netloop в poller windowspoll
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) исправить округление float в JSON через HTTP
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) исправить удаленные фрагменты, чтобы сначала проверять пустой путь; исправление тестов на windows
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) исправить перезагрузку конфигурации для работы в windows таким же образом, как и в linux
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) исправить \#194 PQ для работы с морфологией и стеммерами
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) исправить управление снятыми сегментами RT

## Версия 3.1.2, 22 Августа 2019
### Особенности и улучшения
* Экспериментальная поддержка SSL для HTTP API
* фильтр полей для CALL KEYWORDS
* max_matches для /json/search
* автоматическая настройка размера по умолчанию для Galera gcache.size
* улучшенная поддержка FreeBSD

### Исправления
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) исправлена репликация RT индекса в узел, где существует тот же RT индекс и имеет другой путь
* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) исправить повторное планирование слива для индексов без активности
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) улучшить повторное планирование слива индексов RT/PQ
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) исправить \#250 option index_field_lengths для TSV и CSV источников с конвейера
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) исправить неверный отчет индексового инструмента для проверки блоков индекса на пустом индексе
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) исправить пустой список выборки в журнале запросов Manticore SQL
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) исправить ответ indexer -h/--help

## Версия 3.1.0, 16 Июля 2019
### Особенности и улучшения
* репликация для индексов RealTime
* токенизатор ICU для китайского языка
* новая опция морфологии icu_chinese
* новая директива icu_data_dir
* множественные операторы транзакций для репликации
* LAST_INSERT_ID() и @session.last_insert_id
* LIKE 'pattern' для SHOW VARIABLES
* Множественные документы INSERT для индексов перколяторов
* Добавлены парсеры времени для конфигурации
* внутренний менеджер задач
* mlock для компонентов списков документов и hit
* путь для фрагментов тюрем

### Удаления
* Поддержка библиотеки RLP убрана в пользу ICU; все директивы rlp* удалены
* обновление ID документа с помощью UPDATE отключено

### Исправления
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) исправить дефекты в concat и group_concat
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) исправить uid запроса в индексе перколяции для типа атрибута BIGINT
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) не вызывать сбой, если не удалось предвыделить новый дисковый кусок
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) добавить отсутствующий тип данных timestamp для ALTER
* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) исправлена ошибка, приводящая к сбою при некорректном чтении mmap
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) исправлено хэширование блокировки кластеров при репликации
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) исправлена утечка провайдеров в репликации
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) исправлена ошибка \#246: неопределённый sigmask в индексаторе
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) исправлена гонка при формировании отчётов в netloop
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) нулевой зазор для балансировщика стратегий HA
## Версия 3.0.2, 31 мая 2019
### Улучшения
* добавлены считыватели mmap для документов и списков попаданий
* теперь ответ HTTP конечной точки `/sql` такой же, как ответ `/json/search`
* новые директивы `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* новая директива `server_id` для настроек репликации
### Удаления
* удалена HTTP конечная точка `/search`
### Устаревшие
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` заменены директивами `access_*`
### Исправления ошибок
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) разрешены имена атрибутов, начинающиеся с цифр, в списке select
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) исправлены ошибки с MVA в UDF, исправлено использование псевдонимов MVA
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) исправлен сбой \#187 при использовании запроса с SENTENCE
* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) исправлена ошибка \#143: поддержка скобок вокруг MATCH()
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) исправлено сохранение состояния кластера при выполнении оператора ALTER cluster
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) исправлен сбой сервера при выполнении ALTER index с blob-атрибутами
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) исправлена ошибка \#196: фильтрация по id
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) исключён поиск по шаблонным индексам
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) исправлен столбец id для использования обычного типа bigint в ответе SQL
## Версия 3.0.0, 6 мая 2019
### Новые возможности и улучшения
* Новое хранилище индексов. Нескалярные атрибуты больше не ограничены 4 ГБ на индекс
* директива attr_update_reserve
* Строковые, JSON и MVA можно обновлять с помощью UPDATE
* kill-листы применяются при загрузке индекса
* директива killlist_target
* ускорение запросов с множественными операторами AND
* лучшая средняя производительность и использование ОЗУ
* утилита для конвертации при обновлении индексов, созданных с использованием 2.x
* функция CONCAT()
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* директива node_address
* список узлов выводится в SHOW STATUS
### Изменения в поведении
* для индексов с kill-листами сервер не изменяет порядок индексов, заданный в conf, а следует цепочке killlist targets
* порядок индексов в запросе больше не определяет порядок применения kill-листов
* Идентификаторы документов теперь являются знаковыми большими целыми числами
### Удалённые директивы
* docinfo (теперь всегда extern), inplace_docinfo_gap, mva_updates_pool
## Версия 2.8.2 GA, 2 апреля 2019
### Функциональные возможности и улучшения
* Репликация Galera для percolate-индексов
* OPTION morphology
### Замечания по компиляции
Минимальная версия Cmake теперь 3.13. Для компиляции требуются библиотеки boost и libssl
библиотеки для разработки.
### Исправления ошибок
* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) исправлен сбой при использовании большого количества звезд в списке select для запроса по многим распределенным индексам
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) исправлена ошибка [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) с большим пакетом через интерфейс Manticore SQL
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) устранён сбой [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) сервера при оптимизации RT с обновлением MVA
* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) исправлен сбой сервера, когда binlog удалялся из-за удаления RT индекса после перезагрузки конфигурации при SIGHUP
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) исправлены проблемы с полезными данными плагина аутентификации mysql handshake
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) исправлены настройки phrase_boundary в RT индексе [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172)
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) устранена взаимоблокировка [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) при ATTACH индекса к самому себе
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) исправлено сохранение binlog с пустыми метаданными после сбоя сервера
* [Коммит 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) исправил краш сервера из-за строкового сортировщика из RT индекса с дисковыми чанками

## Версия 2.8.1 GA, 6 марта 2019
### Новшества и улучшения
* SUBSTRING_INDEX()
* Поддержка SENTENCE и PARAGRAPH для запросов перколяторов
* Генератор systemd для Debian/Ubuntu; также добавлен LimitCORE для разрешения дампинга ядра

### Исправления ошибок
* [Коммит 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) исправил краш сервера в режиме совпадения all и при пустом полном текстовом запросе
* [Коммит daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) исправил краш при удалении статической строки
* [Коммит 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) исправил код выхода при неудаче indextool с FATAL
* [Коммит 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) исправил [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) отсутствие совпадений для префиксов из-за неверной проверки точной формы
* [Коммит 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) исправил [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) перезагрузку настроек конфигурации для RT индексов
* [Коммит e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) исправил краш сервера при доступе к большой строке JSON
* [Коммит 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) исправил поле PQ в JSON документе, измененное индексным стриппером, что вызывает неверное совпадение из родственного поля
* [Коммит e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) исправил краш сервера при парсинге JSON на сборках RHEL7
* [Коммит 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) исправил краш разборки JSON, когда слэш находится на краю
* [Коммит be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) исправил опцию 'skip_empty' для пропуска пустых документов и отсутствия предупреждений о том, что они не являются допустимым JSON
* [Коммит 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) исправил [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) вывод 8 знаков после запятой для чисел с плавающей запятой, когда 6 не достаточно для точности
* [Коммит 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) исправил создание пустого jsonobj
* [Коммит f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) исправил [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) пустой mva выводит NULL вместо пустой строки
* [Коммит 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) исправил сбой сборки без pthread_getname_np
* [Коммит 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) исправил краш при завершении работы сервера с потоками worker в пуле

## Версия 2.8.0 GA, 28 января 2019
### Улучшения
* Распределенные индексы для индексов перколяторов
* CALL PQ новые опции и изменения:
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   JSON документы можно передавать как массив JSON
    *   shift
    *   Имена столбцов 'UID', 'Documents', 'Query', 'Tags', 'Filters' были переименованы в 'id', 'documents', 'query', 'tags', 'filters'
* DESCRIBE pq TABLE
* SELECT FROM pq WHERE UID больше не возможен, используйте вместо этого 'id'
* SELECT по индексам pq наравне с обычными индексами (например, вы можете фильтровать правила через REGEX())
* ANY/ALL могут быть использованы для тегов PQ
* выражения имеют автоматическое преобразование для JSON полей, не требующее явного приведения типов
* встроенные 'non_cjk' charset_table и 'cjk' ngram_chars
* встроенные коллекции стоп-слов для 50 языков
* несколько файлов в декларации стоп-слов также могут быть разделены запятой
* CALL PQ может принимать массив JSON документов

### Исправления ошибок
* [Коммит a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) исправил утечку, связанную с csjon
* [Коммит 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) исправил краш из-за отсутствующего значения в JSON
* [Коммит bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) исправил сохранение пустых метаданных для RT индекса
* [Коммит 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) исправил потерю флага формы (точной) для последовательности лемматизатора
* [Коммит 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) исправил строковые атрибуты > 4M, использующие насыщение вместо переполнения
* [Коммит 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) исправил краш сервера на SIGHUP с отключенным индексом
* [Коммит 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) исправил краш сервера при одновременных статусных командах API сеанса
* [Коммит cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) исправил краш сервера при запросе удаления для RT индекса с фильтрами по полям
* [Коммит 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) исправил краш сервера при CALL PQ для распределенного индекса с пустым документом
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) исправлено сообщение об ошибке Manticore SQL превышающее 512 символов
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) исправлен крах при сохранении индекса перколяторов без binlog
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) исправлено неработающее http интерфейс на OSX
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) исправлено ложное сообщение об ошибке indextool при проверке MVA
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) исправлена блокировка записи при FLUSH RTINDEX, чтобы не блокировать весь индекс во время сохранения и при регулярном сбросе из rt_flush_period
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) исправлено застревание ALTER индекса перколяторов в ожидании загрузки поисковых данных
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) исправлено max_children для использования значения по умолчанию для количества потоков thread_pool при значении 0
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) исправлена ошибка при индексировании данных в индекс с плагином index_token_filter вместе с стоп-словами и stopword_step=0
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) исправлен крах при отсутствии lemmatizer_base при использовании aot лемматизаторов в определениях индексов

## Версия 2.7.5 GA, 4 декабря 2018
### Улучшения
* Функция REGEX
* limit/offset для поиска json API
* точки профилировщика для qcache

### Исправления ошибок
* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) исправлен крах сервера на FACET с несколькими атрибутами широких типов
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) исправлено неявное группирование в основном списке выбора запроса FACET
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) исправлен крах при запросе с GROUP N BY
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) исправлено взаимное блокирование при обработке краха в процессе работы с памятью
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) исправлена память, потребляемая indextool во время проверки
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) исправлено не требуемое включение gmock, так как upstream решает это самостоятельно

## Версия 2.7.4 GA, 1 ноября 2018
### Улучшения
* SHOW THREADS в случае удаленных распределенных индексов выводит первоначальный запрос вместо вызова API
* SHOW THREADS новая опция `format=sphinxql` выводит все запросы в формате SQL
* SHOW PROFILE выводит дополнительный этап `clone_attrs`

### Исправления ошибок
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) исправлено неудачное построение с libc без malloc_stats, malloc_trim
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) исправлены специальные символы внутри слов для результата CALL KEYWORDS
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) исправлен сломанный CALL KEYWORDS к распределенному индексу через API или к удаленному агенту
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) исправлено распространение agent_query_timeout распределенного индекса к агентам как max_query_time
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) исправлен подсчет общего количества документов на диске, который был затронут командой OPTIMIZE и нарушает расчет веса
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) исправлено несколько хвостовых попаданий в RT индекс из смешанных
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) исправлено взаимное блокирование при ротации

## Версия 2.7.3 GA, 26 сентября 2018
### Улучшения
* опция sort_mode для CALL KEYWORDS
* DEBUG на VIP соединении может выполнять 'crash <password>' для намеренного действия SIGEGV на сервере
* DEBUG может выполнять 'malloc_stats' для вывода статистики malloc в searchd.log 'malloc_trim' для выполнения malloc_trim()
* улучшенный backtrace, если gdb присутствует в системе

### Исправления ошибок
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) исправлен крах или ошибка переименования на Windows
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) исправлены крахи сервера на 32-битных системах
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) исправлен крах или зависание сервера на пустом выражении SNIPPET
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) исправлено сломанное не прогрессивное оптимизированное и исправлено прогрессивное оптимизированное так, чтобы не создавать список уничтожения для старейшего дискового куска
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) исправлен неверный ответ queue_max_length для SQL и API в режиме рабочей нити
* [Коммит ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) исправил сбой при добавлении полного запроса сканирования к индексу PQ с установленными параметрами regexp или rlp
* [Коммит f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) исправил сбой при вызове одного PQ после другого
* [Коммит 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) рефакторинг AcquireAccum
* [Коммит 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) исправил утечку памяти после вызова pq
* [Коммит 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) косметический рефакторинг (стиль c++11 c-trs, значения по умолчанию, nullptr)
* [Коммит 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) исправил утечку памяти при попытке вставить дубликат в индекс PQ
* [Коммит 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) исправил сбой на JSON поле IN с большими значениями
* [Коммит 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) исправил сбой сервера при выполнении оператора CALL KEYWORDS к индексу RT с установленным лимитом расширения
* [Коммит 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) исправил недействительный фильтр в запросе PQ;
* [Коммит 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) вводит маленький аллокатор объектов для атрибутов ptr
* [Коммит 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) рефакторинг ISphFieldFilter в толстый вариант с отслеживанием ссылок
* [Коммит 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) исправил ub/sigsegv при использовании strtod на не завершающих строках
* [Коммит 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) исправил утечку памяти в обработке набора результатов json
* [Коммит e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) исправил чтение за пределами блока памяти при добавлении атрибута
* [Коммит fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) исправил рефакторинг CSphDict для варианта с отслеживанием ссылок
* [Коммит fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) исправил утечку внутреннего типа AOT
* [Коммит 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) исправил утечку памяти в управлении токенизатором
* [Коммит 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) исправил утечку памяти в группировщике
* [Коммит 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) специальное освобождение/копирование для динамических указателей в совпадениях (утечка памяти группировщика)
* [Коммит b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) исправил утечку памяти динамических строк для RT
* [Коммит 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) рефакторинг группировщика
* [Коммит b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) незначительный рефакторинг (c++11 c-trs, некоторые переформатирования)
* [Коммит 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) рефакторинг ISphMatchComparator в толстый вариант с отслеживанием ссылок
* [Коммит b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) приватизация клонирования
* [Коммит efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) упрощение нативного формата little-endian для MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID
* [Коммит 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) добавление поддержки valgrind в ubertests
* [Коммит 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) исправил сбой из-за гонки флага 'успеха' при подключении
* [Коммит 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) переключил epoll на вариант с триггером по границе
* [Коммит 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) исправил оператор IN в выражении с форматом как в фильтре
* [Коммит bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) исправил сбой в индексе RT при коммите документа с большим docid
* [Коммит ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) исправил опции без аргументов в индексовом инструменте
* [Коммит 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) исправил утечку памяти расширенного ключевого слова
* [Коммит 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) исправил утечку памяти в группировщике json
* [Коммит 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) исправил утечку глобальных переменных пользователя
* [Коммит 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) исправил утечку динамических строк на ранних отклоненных совпадениях
* [Коммит 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) исправил утечку на length(<expression>)
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) исправлен утечка памяти из-за strdup() в парсере
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) исправлен рефакторинг парсера выражений для точного соблюдения счетчиков ссылок

## Версия 2.7.2 GA, 27 августа 2018
### Улучшения
* совместимость с клиентами MySQL 8
* [TRUNCATE](Emptying_a_table.md) С ПЕРЕУСТРОЙКОЙ
* убран счетчик памяти на SHOW STATUS для RT индексов
* глобальный кеш для мульти-агентов
* улучшен IOCP на Windows
* VIP-соединения для HTTP-протокола
* Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG) команда, которая может выполнять различные подкоманды
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1 хеш пароля, необходимый для вызова `shutdown` с помощью команды DEBUG
* новые статистические данные для SHOW AGENT STATUS (_ping, _has_perspool, _need_resolve)
* опция --verbose индексатора теперь принимает \[debugvv\] для вывода отладочных сообщений

### Исправления ошибок
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) убран wlock при оптимизации
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) исправлен wlock при перезагрузке настроек индекса
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) исправлена утечка памяти при запросе с фильтром JSON
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) исправлены пустые документы в результате PQ
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) исправлена путаница задач из-за удаленной
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) исправлен неправильный подсчет удаленного хоста
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) исправлена утечка памяти разобранных агентских дескрипторов
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) исправлена утечка в поиске
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) косметические изменения в явных/встроенных c-trs, использование override/final
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) исправлена утечка json в локальной/удаленной схеме
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) исправлена утечка выражения сортировки json в локальной/удаленной схеме
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) исправлена утечка константного псевдонима
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) исправлена утечка потока предварительного чтения
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) исправлено зависание при выходе из-за зависшего ожидания в netloop
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) исправлено зависание поведения 'ping' при смене HA-агента на обычный хост
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) отдельный сборщик для хранения панели управления
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) исправлен фикс указателя с подсчетом ссылок
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) исправлен крах indextool на несуществующем индексе
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) исправлено имя вывода превышающего атрибута/поля в xmlpipe индексации
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) исправлено значение индексатора по умолчанию, если в конфиге нет секции индексатора
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) исправлено неправильное встраивание стоп-слов в дисковом фрагменте по RT индексу после перезапуска сервера
* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) исправлено пропускание фантомных (уже закрытых, но не окончательно удаленных из опроса) соединений
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) исправлены смешанные (осиротевшие) сетевые задачи
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) исправлен крах при чтении после записи
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) исправлены крах searchd при выполнении тестов на windows
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) исправлен код EINPROGRESS при обычном connect()
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) исправлены тайм-ауты соединения при работе с TFO

## Версия 2.7.1 GA, 4 июля 2018
### Улучшения
* улучшена производительность подстановочных знаков при сопоставлении нескольких документов в PQ
* поддержка запросов полного сканирования в PQ
* поддержка MVA атрибутов в PQ
* поддержка regexp и RLP для индексов перколяторов

### Исправления ошибок
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) исправлено потеря строки запроса
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) исправил пустую информацию в операторе SHOW THREADS
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) исправил крах при совпадении с оператором NOTNEAR
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) исправил сообщение об ошибке при неправильном фильтре для удаления PQ


## Версия 2.7.0 GA, 11 июня 2018
### Усовершенствования
* уменьшено количество системных вызовов для минимизации влияния патчей Meltdown и Spectre
* внутреннее переписывание управления локальными индексами
* рефакторинг удаленных фрагментов
* полная перезагрузка конфигурации
* все соединения узлов теперь независимы
* улучшения протокола
* связь в Windows переключена с wsapoll на порты завершения ввода/вывода
* TFO можно использовать для связи между мастером и узлами
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) теперь отображает версию сервера и mysql_version_string
* добавлена опция `docs_id` для документов, упоминаемых в CALL PQ.
* фильтры запросов перколяторов теперь могут содержать выражения
* распределенные индексы могут работать с FEDERATED
* вспомогательное SHOW NAMES COLLATE и `SET wait_timeout` (для лучшей совместимости с ProxySQL)

### Исправления ошибок
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) исправил добавление не равных меток для PQ
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) исправил добавление поля идентификатора документа в операторе CALL PQ для JSON-документа
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) исправил обработчики операторов flush для индекса PQ
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) исправил фильтрацию PQ по JSON и строковым атрибутам
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) исправил разбор пустой строки JSON
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) исправил крах при многозапросе с фильтрами OR
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) исправил indextool для использования общей секции конфигурации (параметр lemmatizer_base) для команд (dumpheader)
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) исправил пустую строку в наборе результатов и фильтре
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) исправил отрицательные значения идентификатора документа
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) исправил длину обрезки слов для очень длинных слов в индексе
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) исправил совпадение нескольких документов запросов с подстановочными знаками в PQ


## Версия 2.6.4 GA, 3 мая 2018
### Функции и улучшения
* Поддержка MySQL FEDERATED engine [support](Extensions/FEDERATED.md)
* Пакеты MySQL теперь возвращают флаг SERVER_STATUS_AUTOCOMMIT, что добавляет совместимость с ProxySQL
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - включить TCP Fast Open соединения для всех слушателей
* indexer --dumpheader может также сбрасывать заголовок RT из .meta файла
* скрипт сборки cmake для Ubuntu Bionic

### Исправления ошибок
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) исправил недопустимые записи кэша запросов для RT индекса;
* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) исправил настройки индекса, которые потерялись после бесшовной ротации
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) исправил длину инфиксного и префиксного установления; добавил предупреждение о неподдерживаемой длине инфикса
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) исправил порядок авто-очистки RT индексов
* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) исправил проблемы со схемой набора результатов для индекса с несколькими атрибутами и запросами к нескольким индексам
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) исправил потерю некоторых результатов при пакетной вставке с дубликатами документов
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) исправил неудачную оптимизацию слияния дискольз новых индексов RT с большим количеством документов

## Версия 2.6.3 GA, 28 марта 2018
### Усовершенствования
* jemalloc при компиляции. Если jemalloc присутствует в системе, его можно включить с помощью флага cmake `-DUSE_JEMALLOC=1`

### Исправления ошибок
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) исправил лог expand_keywords в журнале запросов Manticore SQL
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) исправил HTTP интерфейс для корректной обработки запроса большого размера
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) исправил крах сервера при DELETE для RT индекса с включенными индексными полями длины
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) исправил работу опции cpustats searchd cli для неподдерживаемых систем
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) исправлен utf8 подстрочный поиск с определённой минимальной длиной


## Версия 2.6.2 GA, 23 февраля 2018
### Улучшения
* улучшена производительность [Percolate Queries](Searching/Percolate_query.md) в случае использования оператора NOT и для пакетных документов.
* [percolate_query_call](Searching/Percolate_query.md) может использовать несколько потоков в зависимости от [dist_threads](Server_settings/Searchd.md#threads)
* новый оператор полнотекстового соответствия NOTNEAR/N
* LIMIT для SELECT на индексах перколяторов
* [expand_keywords](Searching/Options.md#expand_keywords) может принимать 'start','exact' (где 'star,exact' имеет тот же эффект, что и '1')
* ranged-main-query для  [соединённых полей](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field), который использует диапазонный запрос, определённый sql_query_range

### Исправления ошибок
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) исправлен сбой при поиске сегментов RAM; взаимная блокировка при сохранении дискового чанка с двойным буфером; взаимная блокировка при сохранении дискового чанка во время оптимизации
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) исправлен сбой индексатора при xml встроенной схеме с пустым именем атрибута
* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) исправлено ошибочное отключение не принадлежащего pid-файла
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) исправлено иногда оставляющиеся сиротливые fifos в временной папке
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) исправлено пустое множество результатов FACET с неверной строкой NULL
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) исправлена поломка блокировки индекса при запуске сервера как службы Windows
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) исправлены неверные библиотеки iconv на mac os
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) исправлено неправильное count(*)


## Версия 2.6.1 GA, 26 января 2018
### Улучшения
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) в случае агентов с зеркалами даёт значение попыток на одно зеркало вместо одной попытки на агента, общие попытки на агента составляют agent_retry_count*mirrors.
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) теперь может быть задан для каждого индекса, переопределяя глобальное значение. Также добавлен псевдоним [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count).
* количество попыток может быть указано в определении агента и значение представляет количество попыток на агента
* Percolate Queries теперь доступны в HTTP JSON API по адресу [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
* Добавлены опции -h и -v (справка и версия) к исполняемым файлам
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) поддержка для индексов в реальном времени

### Исправления ошибок
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) исправлен ranged-main-query для корректной работы с sql_range_step при использовании в MVA поле
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) исправлена проблема с зависанием черной дыры и агенты черной дыры, кажется, отключены
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) исправлен идентификатор запроса для последовательности, исправлен дублирующийся идентификатор для сохранённых запросов
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) исправлен сбой сервера при завершении работы из различных состояний
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) тайм-ауты при длительных запросах
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) рефакторинг сети опроса master-agent на системах на основе kqueue (Mac OS X, BSD).


## Версия 2.6.0, 29 декабря 2017
### Функции и улучшения
* [HTTP JSON](Connecting_to_the_server/HTTP.md): JSON-запросы теперь могут осуществлять равенство по атрибутам, MVA и JSON-атрибуты могут использоваться для вставок и обновлений, обновления и удаления через JSON API могут выполняться на распределённых индексах
* [Percolate Queries](Searching/Percolate_query.md)
* Удалена поддержка 32-битных docid из кода. Также удалён весь код, который преобразует/загружает устаревшие индексы с 32-битными docid.
* [Морфология только для определённых полей](https://github.com/manticoresoftware/manticore/issues/7) . Новая директива индекса morphology_skip_fields позволяет определить список полей, для которых морфология не применяется.
* [expand_keywords теперь может быть директивой времени выполнения запроса, установленной с помощью оператора OPTION](https://github.com/manticoresoftware/manticore/issues/8)

### Исправления ошибок
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) исправлен сбой при отладочной сборке сервера (и возможно ошибки в релизе) при сборке с rlp
* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) исправлена оптимизация RT индекса с включенной прогрессивной опцией, которая объединяет kill-list с неправильным порядком
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee)  мелкий сбой на маке
* множество мелких исправлений после тщательного статического анализа кода
* другие мелкие исправления ошибок

### Обновление
В этом релизе мы изменили внутренний протокол, используемый мастерами и агентами для общения друг с другом. Если вы запускаете Manticoresearch в распределенной среде с множеством экземпляров, убедитесь, что сначала обновите агентов, а затем мастеров.

## Версия 2.5.1, 23 ноября 2017

### Возможности и улучшения

* JSON-запросы по [HTTP API протоколу](Connecting_to_the_server/HTTP.md). Поддерживаются операции поиска, вставки, обновления, удаления, замены. Команды манипуляции данными также могут быть объединены, также в настоящее время существуют ограничения, поскольку атрибуты MVA и JSON не могут быть использованы для вставок, замен или обновлений.

* Команда [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES)

* Команда [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md)

* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) может показывать прогресс оптимизации, ротации или сброса.

* GROUP N BY работает корректно с атрибутами MVA

* агенты blackhole запускаются в отдельном потоке, чтобы больше не влиять на запросы мастера

* реализован подсчет ссылок на индексы, чтобы избежать зависаний, вызванных ротациями и высокой нагрузкой

* Реализовано хеширование SHA1, пока еще не открыто внешне

* исправления для компиляции на FreeBSD, macOS и Alpine

### Исправления ошибок

* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) регрессия фильтра с блочным индексом

* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) переименование PAGE_SIZE -> ARENA_PAGE_SIZE для совместимости с musl

* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) отключение googletests для cmake < 3.1.0

* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) не удалось привязать сокет при перезапуске сервера

* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) устранена ошибка завершения работы сервера

* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) исправлено отображение потоков для системного потока blackhole

* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) переработан контроль конфигурации iconv, исправляет сборку на FreeBSD и Darwin

## Версия 2.4.1 GA, 16 октября 2017

### Возможности и улучшения

* Оператор OR в предложении WHERE между фильтрами атрибутов

* Режим технического обслуживания ( SET MAINTENANCE=1)

* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) доступны на распределенных индексах

* [Группировка по UTC](Server_settings/Searchd.md#grouping_in_utc)

* [query_log_mode](Server_settings/Searchd.md#query_log_mode) для разрешений пользовательских лог файлов

* Веса полей могут быть нулевыми или отрицательными

* [max_query_time](Searching/Options.md#max_query_time) теперь может влиять на полные сканы

* добавлены [net_wait_tm](Server_settings/Searchd.md#net_wait_tm), [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) и [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) для тонкой настройки сетевого потока (в случае workers=thread_pool)

* COUNT DISTINCT работает с фасетными поисками

* IN может использоваться с массивами JSON float

* многозапросная оптимизация больше не нарушается целочисленными/числовыми выражениями

* [SHOW META](Node_info_and_management/SHOW_META.md) показывает строку `multiplier`, когда используется многозапросная оптимизация

### Компиляция

Manticore Search собирается с использованием cmake, минимальная версия gcc, необходимая для компиляции, составляет 4.7.2.

### Папки и служба

* Manticore Search работает под пользователем `manticore`.

* Папка данных по умолчанию теперь `/var/lib/manticore/`.

* Папка логов по умолчанию теперь `/var/log/manticore/`.

* Папка pid по умолчанию теперь `/var/run/manticore/`.
