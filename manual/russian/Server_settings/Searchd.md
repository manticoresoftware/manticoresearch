# Раздел "Searchd" в конфигурации

Ниже приведены настройки, которые используются в разделе `searchd` файла конфигурации Manticore Search для управления поведением сервера. Ниже представлен краткий обзор каждой настройки:

### access_plain_attrs

Эта настройка задает общие для всего экземпляра значения по умолчанию для [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Опциональна, значение по умолчанию — `mmap_preread`.

Директива `access_plain_attrs` позволяет определить значение по умолчанию для [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы, заданные для каждой таблицы, имеют более высокий приоритет и переопределяют это значение по умолчанию, обеспечивая более тонкую настройку.

### access_blob_attrs

Эта настройка задает общие для всего экземпляра значения по умолчанию для [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Опциональна, значение по умолчанию — `mmap_preread`.

Директива `access_blob_attrs` позволяет определить значение по умолчанию для [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы, заданные для каждой таблицы, имеют более высокий приоритет и переопределяют это значение по умолчанию, обеспечивая более тонкую настройку.

### access_doclists

Эта настройка задает общие для всего экземпляра значения по умолчанию для [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Опциональна, значение по умолчанию — `file`.

Директива `access_doclists` позволяет определить значение по умолчанию для [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы, заданные для каждой таблицы, имеют более высокий приоритет и переопределяют это значение по умолчанию, обеспечивая более тонкую настройку.

### access_hitlists

Эта настройка задает общие для всего экземпляра значения по умолчанию для [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Опциональна, значение по умолчанию — `file`.

Директива `access_hitlists` позволяет определить значение по умолчанию для [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы, заданные для каждой таблицы, имеют более высокий приоритет и переопределяют это значение по умолчанию, обеспечивая более тонкую настройку.

### access_dict

Эта настройка задает общие для всего экземпляра значения по умолчанию для [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Опциональна, значение по умолчанию — `mmap_preread`.

Директива `access_dict` позволяет определить значение по умолчанию для [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы, заданные для каждой таблицы, имеют более высокий приоритет и переопределяют это значение по умолчанию, обеспечивая более тонкую настройку.

### agent_connect_timeout

Эта настройка задает общие для всего экземпляра значения по умолчанию для параметра [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout).


### agent_query_timeout

Эта настройка задает общие для всего экземпляра значения по умолчанию для параметра [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout). Она может быть переопределена для каждого запроса индивидуально с помощью клаузы `OPTION agent_query_timeout=XXX`.


### agent_retry_count

Эта настройка — целое число, которое определяет, сколько раз Manticore попытается подключиться и отправить запрос удалённым агентам через распределённую таблицу, прежде чем сообщить о фатальной ошибке запроса. Значение по умолчанию — 0 (то есть без повторных попыток). Вы также можете задать это значение для каждого запроса индивидуально с помощью клаузы `OPTION retry_count=XXX`. Если задана опция для отдельного запроса, она переопределит значение, указанное в конфигурации.

Обратите внимание, что если вы используете [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors) в определении распределённой таблицы, сервер будет выбирать другой зеркальный агент для каждой попытки подключения согласно выбранной [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy). В этом случае значение `agent_retry_count` будет агрегировано для всех зеркал в наборе.

Например, если у вас есть 10 зеркал и установлен `agent_retry_count=5`, сервер выполнит до 50 попыток, предполагая в среднем по 5 попыток для каждого из 10 зеркал (при опции `ha_strategy = roundrobin` это будет так).

Тем не менее, значение, указанное в опции `retry_count` для [агента](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent), является абсолютным пределом. Другими словами, опция `[retry_count=2]` в определении агента всегда означает максимум 2 попытки, независимо от того, указали ли вы 1 или 10 зеркал для агента.

### agent_retry_delay

Эта настройка — целое число в миллисекундах (или [special_suffixes](../Server_settings/Special_suffixes.md)), которое задает задержку перед повторной попыткой запроса удалённому агенту в случае ошибки. Это значение актуально только тогда, когда задан ненулевой [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) или ненулевое per-query значение `retry_count`. Значение по умолчанию — 500. Вы также можете задать это значение для каждого запроса индивидуально с помощью клаузы `OPTION retry_delay=XXX`. Если задана опция для отдельного запроса, она переопределит значение, указанное в конфигурации.


### attr_flush_period

<!-- example conf attr_flush_period -->
При использовании [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) для модификации атрибутов документа в реальном времени, изменения сначала записываются в копию атрибутов в памяти. Эти обновления происходят в файле с отображением в память, то есть ОС решает, когда записывать изменения на диск. При нормальном завершении работы `searchd` (вызываемом сигналом `SIGTERM`), все изменения принудительно записываются на диск.

Вы также можете указать `searchd` периодически записывать эти изменения на диск, чтобы предотвратить потерю данных. Интервал между такими сбросами определяется параметром `attr_flush_period`, указанным в секундах (или [special_suffixes](../Server_settings/Special_suffixes.md)).

По умолчанию значение равно 0, что отключает периодический сброс. Однако сброс все равно произойдет во время нормального завершения работы.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
Этот параметр управляет автоматическим процессом [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) для уплотнения таблицы.

По умолчанию уплотнение таблицы происходит автоматически. Вы можете изменить это поведение с помощью параметра `auto_optimize`:
* 0 — отключить автоматическое уплотнение таблицы (вы по-прежнему можете вызывать `OPTIMIZE` вручную)
* 1 — явно включить его
* включить с умножением порога оптимизации на 2.

По умолчанию OPTIMIZE выполняется до тех пор, пока количество дисковых кусков не станет меньше или равно количеству логических ядер ЦП, умноженному на 2.

Однако, если таблица содержит атрибуты с индексами KNN, порог отличается. В этом случае он устанавливается равным количеству физических ядер ЦП, делённому на 2 для улучшения производительности поиска KNN.

Обратите внимание, что переключение `auto_optimize` в положение включено или выключено не мешает вам вручную запускать [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE).

<!-- intro -->
##### Пример:

<!-- request Disable -->
```ini
auto_optimize = 0 # disable automatic OPTIMIZE
```

<!-- request Throttle -->
```ini
auto_optimize = 2 # OPTIMIZE starts at 16 chunks (on 4 cpu cores server)
```

<!-- end -->

### auto_schema

<!-- example conf auto_schema -->
Manticore поддерживает автоматическое создание таблиц, которые ещё не существуют, но указаны в операторах INSERT. Эта функция включена по умолчанию. Чтобы отключить её, явно установите в конфигурации `auto_schema = 0`. Чтобы снова включить, установите `auto_schema = 1` или удалите параметр `auto_schema` из конфигурации.

Имейте в виду, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц.

> ПРИМЕЧАНИЕ: [Функциональность авто-схемы](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

<!-- request Disable -->
```ini
auto_schema = 0 # disable automatic table creation
```

<!-- request Enable -->
```ini
auto_schema = 1 # enable automatic table creation
```

<!-- end -->

### binlog_flush

<!-- example conf binlog_flush -->
Этот параметр управляет режимом сброса/синхронизации бинарного журнала транзакций. Он необязательный, по умолчанию равен 2 (сбрасывать после каждой транзакции, синхронизировать каждую секунду).

Директива определяет, как часто бинарный журнал будет сбрасываться в ОС и синхронизироваться с диском. Поддерживаются три режима:

*  0 — сброс и синхронизация каждую секунду. Обеспечивает лучшую производительность, но в случае сбоя сервера или ОС/аппаратного сбоя может быть потеряно до 1 секунды зафиксированных транзакций.
*  1 — сброс и синхронизация после каждой транзакции. Этот режим даёт худшую производительность, но гарантирует сохранение данных каждой зафиксированной транзакции.
*  2 — сброс после каждой транзакции, синхронизация каждую секунду. Этот режим обеспечивает хорошую производительность и гарантирует сохранение каждой зафиксированной транзакции при сбое сервера. Однако при сбое ОС/аппаратуры может быть потеряно до 1 секунды зафиксированных транзакций.

Тем, кто знаком с MySQL и InnoDB, эта директива похожа на `innodb_flush_log_at_trx_commit`. В большинстве случаев режим 2 по умолчанию даёт хороший баланс между скоростью и безопасностью, обеспечивая полную защиту данных таблицы RT от сбоев сервера и некоторую защиту от сбоев аппаратуры.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
Этот параметр регулирует управление файлами бинарного журнала. Параметр необязательный, по умолчанию равен 0 (отдельный файл для каждой таблицы).

Вы можете выбрать один из двух способов управления файлами бинарного журнала:

* Отдельный файл для каждой таблицы (по умолчанию, `0`): каждая таблица сохраняет свои изменения в собственном журнале. Такой способ удобен, если у вас много таблиц, которые обновляются в разное время. Он позволяет обновлять таблицы без ожидания других, а также если с журналом одной таблицы возникнут проблемы, это не повлияет на остальные.
* Один файл для всех таблиц (`1`): все таблицы используют один и тот же файл бинарного журнала. Этот способ упрощает управление файлами, так как их меньше. Однако он может увеличивать время хранения файлов, если одна таблица ещё должна сохранить свои обновления. Этот параметр также может снизить производительность, если много таблиц обновляются одновременно, так как все изменения должны ждать записи в один файл.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
Этот параметр задаёт максимальный размер файла бинарного журнала. Параметр необязательный, по умолчанию 256 МБ.

Новый файл бинарного журнала будет принудительно открыт, когда текущий достигнет этого предела. Это даёт более тонкую гранулярность журналов и может привести к более эффективному использованию диска при некоторых предельных нагрузках. Значение 0 означает, что файл бинарного журнала не будет переоткрываться по размеру.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
Этот параметр определяет путь для файлов бинарного журнала (также известных как транзакционный журнал). Параметр необязательный, по умолчанию равен каталогу данных, сконфигурированному на этапе сборки (например, `/var/lib/manticore/data/binlog.*` в Linux).

Бинарные логи используются для восстановления после сбоев данных таблицы RT и для обновления атрибутов простых дисковых индексов, которые в противном случае хранились бы только в RAM до сброса. Когда логирование включено, каждая транзакция, зафиксированная (COMMIT) в таблице RT, записывается в файл лога. Логи автоматически воспроизводятся при запуске после некорректного завершения работы, восстанавливая зарегистрированные изменения.

Директива `binlog_path` указывает расположение файлов бинарных логов. Она должна содержать только путь; `searchd` будет создавать и удалять несколько файлов `binlog.*` в каталоге по мере необходимости (включая данные binlog, метаданные и файлы блокировки и т.д.).

Пустое значение отключает бинарное логирование, что улучшает производительность, но ставит данные таблицы RT под угрозу.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_path = # disable logging
binlog_path = /var/lib/manticore/data # /var/lib/manticore/data/binlog.001 etc will be created
```
<!-- end -->


### buddy_path

<!-- example conf buddy_path -->
Этот параметр определяет путь к бинарному файлу Manticore Buddy. Он является необязательным, значение по умолчанию — путь, настроенный во время сборки, который варьируется в разных операционных системах. Обычно нет необходимости изменять этот параметр. Однако он может быть полезен, если вы хотите запустить Manticore Buddy в режиме отладки, внести изменения в Manticore Buddy или реализовать новый плагин. В последнем случае вы можете выполнить `git clone` Buddy с https://github.com/manticoresoftware/manticoresearch-buddy, добавить новый плагин в каталог `./plugins/` и после перехода в исходный каталог Buddy выполнить `composer install --prefer-source` для удобства разработки.

Для работы с `composer` на вашей машине должен быть установлен PHP версии 8.2 или выше с следующими расширениями:

```
--enable-dom
--with-libxml
--enable-tokenizer
--enable-xml
--enable-xmlwriter
--enable-xmlreader
--enable-simplexml
--enable-phar
--enable-bcmath
--with-gmp
--enable-debug
--with-mysqli
--enable-mysqlnd
```

Также вы можете выбрать специальную версию `manticore-executor-dev` для Linux amd64, доступную в релизах, например: https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

Если вы выбрали этот вариант, не забудьте связать dev-версию исполнителя manticore с `/usr/bin/php`.

Чтобы отключить Manticore Buddy, установите значение пустым, как показано в примере.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php # use the default Manticore Buddy in Linux
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php --threads=1 # runs Buddy with a single worker
buddy_path = manticore-executor -n /opt/homebrew/share/manticore/modules/manticore-buddy/bin/manticore-buddy/src/main.php # use the default Manticore Buddy in MacOS arm64
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php # use Manticore Buddy from a non-default location
buddy_path = # disables Manticore Buddy
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php --skip=manticoresoftware/buddy-plugin-replace # --skip - skips plugins
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php --enable-plugin=manticoresoftware/buddy-plugin-show # runs Buddy with only the SHOW plugin
```
<!-- end -->

### client_timeout

<!-- example conf client_timeout -->
Этот параметр определяет максимальное время ожидания между запросами (в секундах или [special_suffixes](../Server_settings/Special_suffixes.md)) при использовании постоянных соединений. Он необязательный, значение по умолчанию — пять минут.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
Локаль libc сервера. Необязательный параметр, значение по умолчанию — C.

Определяет локаль libc, влияющую на сортировки на базе libc. Для подробностей смотрите раздел [collations](../Searching/Collations.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
Коллация сервера по умолчанию. Необязательный параметр, значение по умолчанию — libc_ci.

Определяет коллацию по умолчанию для входящих запросов. Коллация может быть переопределена для каждого запроса отдельно. Смотрите раздел [collations](../Searching/Collations.md) для списка доступных коллаций и других подробностей.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
При указании этот параметр включает [режим реального времени](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29), который представляет собой императивный способ управления схемой данных. Значение должно быть путем к каталогу, в котором вы хотите хранить все ваши таблицы, бинарные логи и всё остальное, необходимое для правильного функционирования Manticore Search в этом режиме.
Индексация [простых таблиц](../Creating_a_table/Local_tables/Plain_table.md) не разрешена при указании `data_dir`. Подробнее о разнице между режимом RT и простым режимом читайте в [этом разделе](../Read_this_first.md#Real-time-table-vs-plain-table).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
Таймаут для предотвращения автоматического сброса RAM-чънка, если в таблице отсутствуют поисковые запросы. Необязательный параметр, значение по умолчанию — 30 секунд.

Время проверки наличия поисков перед решением о автоматическом сбросе.
Автоматический сброс происходит только если в таблице был хотя бы один поиск за последние `diskchunk_flush_search_timeout` секунд. Работает совместно с [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout). Соответствующая [настройка для каждой таблицы](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout) имеет более высокий приоритет и переопределит это значение по умолчанию для экземпляра, обеспечивая более тонкий контроль.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
Время в секундах ожидания без записи перед автоматическим сбросом RAM-чънка на диск. Необязательный параметр, значение по умолчанию — 1 секунда.

Если в RAM-чънке не происходит запись в течение `diskchunk_flush_write_timeout` секунд, чънк будет сброшен на диск. Работает совместно с [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout). Чтобы отключить авто-сброс, явно установите `diskchunk_flush_write_timeout = -1` в вашей конфигурации. Соответствующая [настройка для каждой таблицы](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout) имеет более высокий приоритет и переопределит значение по умолчанию для экземпляра, предоставляя более тонкий контроль.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
Этот параметр задает максимальный размер блоков документов из хранилища документов, которые удерживаются в памяти. Он необязательный, значение по умолчанию — 16m (16 мегабайт).

When `stored_fields` is used, document blocks are read from disk and uncompressed. Since every block typically holds several documents, it may be reused when processing the next document. For this purpose, the block is held in a server-wide cache. The cache holds uncompressed blocks.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
Default attribute storage engine used when creating tables in RT mode. Can be `rowwise` (default) or `columnar`.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
This setting determines the maximum number of expanded keywords for a single wildcard. It is optional, with a default value of 0 (no limit).

When performing substring searches against tables built with `dict = keywords` enabled, a single wildcard may potentially result in thousands or even millions of matched keywords (think of matching `a*` against the entire Oxford dictionary). This directive allows you to limit the impact of such expansions. Setting `expansion_limit = N` restricts expansions to no more than N of the most frequent matching keywords (per each wildcard in the query).

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
This setting determines the maximum number of documents in the expanded keyword that allows merging all such keywords together. It is optional, with a default value of 32.

When performing substring searches against tables built with `dict = keywords` enabled, a single wildcard may potentially result in thousands or even millions of matched keywords. This directive allows you to increase the limit of how many keywords will merge together to speed up matching but uses more memory in the search.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
This setting determines the maximum number of hits in the expanded keyword that allows merging all such keywords together. It is optional, with a default value of 256.

When performing substring searches against tables built with `dict = keywords` enabled, a single wildcard may potentially result in thousands or even millions of matched keywords. This directive allows you to increase the limit of how many keywords will merge together to speed up matching but uses more memory in the search.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
This setting controls the maximum number of alternative phrase variants generated due to `OR` operators inside `PHRASE`, `PROXIMITY`, and `QUORUM` operators. It is optional, with a default value of 1024.

When using the `|` (OR) operator inside phrase-like operator, the total number of expanded combinations may grow exponentially depending on the number of alternatives specified. This setting helps prevent excessive query expansion by capping the number of permutations considered during query processing. If the number of generated variants exceeds this limit, the query will fail with an error.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_phrase_limit = 4096
```
<!-- end -->

### grouping_in_utc

This setting specifies whether timed grouping in API and SQL will be calculated in the local timezone or in UTC. It is optional, with a default value of 0 (meaning 'local timezone').

By default, all 'group by time' expressions (like group by day, week, month, and year in API, also group by day, month, year, yearmonth, yearmonthday in SQL) are done using local time. For example, if you have documents with attributes timed `13:00 utc` and `15:00 utc`, in the case of grouping, they both will fall into facility groups according to your local timezone setting. If you live in `utc`, it will be one day, but if you live in `utc+10`, then these documents will be matched into different `group by day` facility groups (since 13:00 utc in UTC+10 timezone is 23:00 local time, but 15:00 is 01:00 of the next day). Sometimes such behavior is unacceptable, and it is desirable to make time grouping not dependent on timezone. You can run the server with a defined global TZ environment variable, but it will affect not only grouping but also timestamping in the logs, which may be undesirable as well. Switching 'on' this option (either in config or using [SET global](../Server_settings/Setting_variables_online.md#SET) statement in SQL) will cause all time grouping expressions to be calculated in UTC, leaving the rest of time-depentend functions (i.e. logging of the server) in local TZ.


### timezone

This setting specifies the timezone to be used by date/time-related functions. By default, the local timezone is used, but you can specify a different timezone in IANA format (e.g., `Europe/Amsterdam`).

Note that this setting has no impact on logging, which always operates in the local timezone.

Also, note that if `grouping_in_utc` is used, the 'group by time' function will still use UTC, while other date/time-related functions will use the specified timezone. Overall, it is not recommended to mix `grouping_in_utc` and `timezone`.

You can configure this option either in the config or by using the [SET global](../Server_settings/Setting_variables_online.md#SET) statement in SQL.


### ha_period_karma

<!-- example conf ha_period_karma -->
This setting specifies the agent mirror statistics window size, in seconds (or [special_suffixes](../Server_settings/Special_suffixes.md)). It is optional, with a default value of 60 seconds.

Для распределённой таблицы с агент-зеркалами (см. подробнее в [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)) мастер отслеживает несколько различных счётчиков для каждого зеркала. Эти счётчики затем используются для аварийного переключения и балансировки (мастер выбирает лучшее зеркало для использования на основе счётчиков). Счётчики накапливаются блоками по `ha_period_karma` секунд.

После начала нового блока мастер может ещё использовать накопленные значения из предыдущего блока, пока новый блок не заполнится наполовину. В результате любой предыдущий исторический период перестаёт влиять на выбор зеркала максимум через 1.5 раза больше ha_period_karma секунд.

Хотя для выбора зеркала используется максимум два блока, для целей инструментирования хранится до 15 последних блоков. Эти блоки можно просмотреть с помощью оператора [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
Этот параметр настраивает интервал между ping-запросами к агент-зеркалам в миллисекундах (или с использованием [special_suffixes](../Server_settings/Special_suffixes.md)). Он необязателен, значение по умолчанию — 1000 миллисекунд.

Для распределённой таблицы с агент-зеркалами (см. подробнее в [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)) мастер отправляет всем зеркалам команду ping в периоды простоя. Это необходимо для отслеживания текущего статуса агента (активен или нет, время сетевого отклика и т.п.). Интервал между такими пингами определяется этим параметром. Чтобы отключить пинги, установите ha_ping_interval в 0.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

Опция `hostname_lookup` определяет стратегию обновления имён хостов. По умолчанию IP-адреса имён агентов кэшируются при запуске сервера, чтобы избежать чрезмерного доступа к DNS. Однако в некоторых случаях IP может менять динамически (например, в облачных хостингах), и может потребоваться не кэшировать IP-адреса. Установка этой опции в значение `request` отключает кэширование и выполняет запрос к DNS при каждом запросе. IP-адреса также можно обновить вручную с помощью команды `FLUSH HOSTNAMES`.

### jobs_queue_size

Параметр jobs_queue_size задаёт, сколько «заданий» может находиться в очереди одновременно. По умолчанию ограничений нет.

В большинстве случаев «задание» означает один запрос к одной локальной таблице (обычной таблице или дисковому чанку real-time таблицы). Например, если у вас есть распределённая таблица, состоящая из 2 локальных таблиц, или real-time таблица с 2 дисковыми чанками, поисковый запрос к любой из них создаст в очереди примерно 2 задания. Пул потоков (размер которого определяется [threads](../Server_settings/Searchd.md#threads)) будет обрабатывать эти задания. Однако в некоторых случаях, если запрос слишком сложный, может создаваться больше заданий. Рекомендуется менять этот параметр, если [max_connections](../Server_settings/Searchd.md#max_connections) и [threads](../Server_settings/Searchd.md#threads) недостаточны для достижения желаемой производительности.

### join_batch_size

Объединения таблиц работают путём накопления пакета совпадений — результатов запроса, выполненного по левой таблице. Этот пакет затем обрабатывается одним запросом по правой таблице.

Эта опция позволяет настроить размер пакета. Значение по умолчанию — `1000`. Установка параметра в `0` отключает пакетную обработку.

Больший размер пакета может улучшить производительность; однако для некоторых запросов это может привести к чрезмерному потреблению памяти.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

Каждый запрос, выполняемый по правой таблице, определяется конкретными условиями JOIN ON, которые задают набор результатов, извлекаемых из правой таблицы.

Если уникальных условий JOIN ON немного, повторное использование результатов может быть эффективнее, чем многократное выполнение запросов по правой таблице. Для этого наборы результатов сохраняются в кеш.

Эта опция позволяет настроить размер этого кеша. Значение по умолчанию — `20 MB`. Установка параметра в 0 отключает кеширование.

Учтите, что каждый поток имеет собственный кеш, поэтому при оценке общего потребления памяти следует учитывать число потоков, выполняющих запросы.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
Параметр listen_backlog определяет длину очереди прослушивания TCP для входящих соединений. Это особенно важно для сборок Windows, которые обрабатывают запросы по одному. Когда очередь соединений достигает предела, новые входящие соединения будут отвергаться.
Для сборок не для Windows значение по умолчанию обычно подходит, и, как правило, менять этот параметр не нужно.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
Строка версии сервера, возвращаемая Kibana или OpenSearch Dashboards. Необязательный параметр — по умолчанию установлено значение `7.6.0`.

Некоторые версии Kibana и OpenSearch Dashboards ожидают, что сервер будет сообщать конкретный номер версии и могут по-разному себя вести в зависимости от него. Чтобы решить такие проблемы, вы можете использовать этот параметр, который заставит Manticore сообщать Kibana или OpenSearch Dashboards произвольную версию.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
Этот параметр позволяет указать IP-адрес и порт или путь к сокету Unix-домена, на которых Manticore будет принимать подключения.

Общий синтаксис для `listen`:

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

Можно указать:
* либо IP-адрес (или имя хоста) и номер порта
* либо только номер порта
* либо путь к Unix-сокету (не поддерживается в Windows)
* либо IP-адрес и диапазон портов

Если указан номер порта без адреса, `searchd` будет слушать все сетевые интерфейсы. Путь Unix-сокета определяется ведущим слэшем. Диапазон портов может быть задан только для протокола репликации.

Вы также можете указать обработчик протокола (listener), который будет использоваться для подключений на этом сокете. Слушатели:

* **Не указано** - Manticore будет принимать подключения на этом порту от:
  - других агентов Manticore (т.е. удалённая распределённая таблица)
  - клиентов через HTTP и HTTPS
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/). **Убедитесь, что у вас есть слушатель такого типа (или слушатель `http`, как указано ниже), чтобы избежать ограничений в функциональности Manticore.**
* `mysql` Протокол MySQL для подключений от клиентов MySQL. Обратите внимание:
  - Поддерживается также сжатый протокол.
  - Если включён [SSL](../Security/SSL.md#SSL), можно установить зашифрованное соединение.
* `replication` - протокол репликации, используемый для связи узлов. Подробнее можно узнать в разделе [репликация](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md). Вы можете указать несколько слушателей репликации, но все они должны слушать на одном и том же IP; только порты могут быть разными. Если вы определяете слушатель репликации с диапазоном портов (например, `listen = 192.168.0.1:9320-9328:replication`), Manticore не начинает сразу слушать эти порты. Вместо этого он будет использовать случайные свободные порты из указанного диапазона только при начале использования репликации. Для корректной работы репликации в диапазоне должно быть как минимум 2 порта.
* `http` - то же, что и **Не указано**. Manticore будет принимать подключения на этом порту от удалённых агентов и клиентов через HTTP и HTTPS.
* `https` - протокол HTTPS. Manticore будет принимать **только** HTTPS-подключения на этом порту. Подробнее смотрите в разделе [SSL](../Security/SSL.md).
* `sphinx` - устаревший бинарный протокол. Используется для обслуживания подключений от удалённых клиентов [SphinxSE](../Extensions/SphinxSE.md). Некоторые реализации Sphinx API клиентов (например, на Java) требуют явного указания слушателя.

Добавление суффикса `_vip` к клиентским протоколам (то есть ко всем, кроме `replication`, например `mysql_vip`, `http_vip` или просто `_vip`) заставляет создать выделенный поток для подключения, чтобы обойти различные ограничения. Это полезно для обслуживания узла в случае сильной перегрузки, когда сервер в противном случае мог бы зависать или не позволять подключаться через обычный порт.

Суффикс `_readonly` устанавливает [режим только для чтения](../Security/Read_only.md) для слушателя и ограничивает его приём только запросами на чтение.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
listen = localhost
listen = localhost:5000 # listen for remote agents (binary API) and http/https requests on port 5000 at localhost
listen = 192.168.0.1:5000 # listen for remote agents (binary API) and http/https requests on port 5000 at 192.168.0.1
listen = /var/run/manticore/manticore.s # listen for binary API requests on unix socket
listen = /var/run/manticore/manticore.s:mysql # listen for mysql requests on unix socket
listen = 9312 # listen for remote agents (binary API) and http/https requests on port 9312 on any interface
listen = localhost:9306:mysql # listen for mysql requests on port 9306 at localhost
listen = localhost:9307:mysql_readonly # listen for mysql requests on port 9307 at localhost and accept only read queries
listen = 127.0.0.1:9308:http # listen for http requests as well as connections from remote agents (and binary API) on port 9308 at localhost
listen = 192.168.0.1:9320-9328:replication # listen for replication connections on ports 9320-9328 at 192.168.0.1
listen = 127.0.0.1:9443:https # listen for https requests (not http) on port 9443 at 127.0.0.1
listen = 127.0.0.1:9312:sphinx # listen for legacy Sphinx requests (e.g. from SphinxSE) on port 9312 at 127.0.0.1
```
<!-- end -->

Может быть несколько директив `listen`. `searchd` будет слушать подключения клиентов на всех указанных портах и сокетах. Конфигурация по умолчанию, поставляемая в пакетах Manticore, определяет прослушивание на портах:
* `9308` и `9312` для подключений от удалённых агентов и клиентов, не использующих MySQL
* и на порту `9306` для подключений MySQL.

Если вы вовсе не укажете директиву `listen` в конфигурации, Manticore будет ожидать подключения на:
* `127.0.0.1:9306` для клиентов MySQL
* `127.0.0.1:9312` для HTTP/HTTPS и подключений от других узлов Manticore и клиентов, использующих бинарный API Manticore.

#### Прослушивание привилегированных портов

По умолчанию Linux не позволит вам заставить Manticore слушать порт ниже 1024 (например, `listen = 127.0.0.1:80:http` или `listen = 127.0.0.1:443:https`), если вы не запускаете searchd под root. Если вы всё-таки хотите запускать Manticore от имени непривилегированного пользователя с прослушиванием портов < 1024, рассмотрите один из следующих вариантов (любой из них должен работать):
* Выполните команду `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* Добавьте `AmbientCapabilities=CAP_NET_BIND_SERVICE` в systemd-юнит Manticore и перезагрузите демон (`systemctl daemon-reload`).

#### Технические детали о протоколе Sphinx API и TFO
<details>
Устаревший протокол Sphinx имеет 2 фазы: обмен рукопожатиями и поток данных. Рукопожатие состоит из пакета в 4 байта от клиента и пакета в 4 байта от демона с единственной целью — клиент убеждается, что удалённый узел — настоящий демон Sphinx, демон убеждается, что удалённый узел — настоящий клиент Sphinx. Основной поток данных довольно прост: обе стороны объявляют свои рукопожатия, и противоположная сторона их проверяет. Такой обмен короткими пакетами подразумевает использование специального флага `TCP_NODELAY`, который отключает алгоритм Нейгла для TCP и определяет, что TCP-соединение будет выполняться как диалог маленьких пакетов.
Однако строго не определено, кто говорит первым в этой переговорной процедуре. Исторически все клиенты, использующие бинарный API, говорят первыми: сначала посылают рукопожатие, затем читают 4 байта от демона, затем посылают запрос и читают ответ от демона.
Когда мы улучшали совместимость протокола Sphinx, мы учли следующие моменты:

1. Обычно связь мастер-агент устанавливается от известного клиента к известному хосту на известном порте. Поэтому маловероятно, что конечная точка предоставит неправильное рукопожатие. Мы можем неявно предположить, что обе стороны валидны и действительно разговаривают по протоколу Sphinx.
2. Исходя из этого предположения, мы можем «склеить» рукопожатие с реальным запросом и послать в одном пакете. Если backend — устаревший демон Sphinx, он просто прочитает этот склеенный пакет как 4 байта рукопожатия, затем тело запроса. Поскольку они пришли в одном пакете, у backend-а в сокете минус один RTT, а буфер frontend-а при этом продолжает работать обычным образом.
3. Продолжая предположение: так как пакет «запроса» достаточно мал, а рукопожатие — ещё меньше, давайте отправим оба в первоначальном TCP-пакете SYN с помощью современной технологии TFO (tcp-fast-open). То есть: мы подключаемся к удалённому узлу с «склеенным» пакетом рукопожатия + тела запроса. Демон принимает подключение и сразу имеет и рукопожатие, и тело в сокетном буфере, так как они пришли в самом первом TCP-пакете SYN. Это устраняет ещё один RTT.
4. Наконец, научите демон принимать это улучшение. Фактически, со стороны приложения это означает НЕ использовать `TCP_NODELAY`. А со стороны системы это означает гарантировать, что на стороне демона включён приём TFO, и на стороне клиента также включена отправка TFO. По умолчанию в современных системах клиентский TFO уже включён, поэтому вам остаётся только настроить серверный TFO, чтобы всё работало.

Все эти улучшения без фактического изменения самого протокола позволили нам устранить 1.5 RTT протокола TCP из соединения. То есть, если запрос и ответ помещаются в один TCP-пакет, сессия бинарного API сокращается с 3.5 RTT до 2 RTT — что делает сетевое согласование примерно в 2 раза быстрее.

Таким образом, все наши улучшения основаны на изначально неопределённом утверждении: «кто говорит первым». Если первым говорит клиент, мы можем применить все эти оптимизации и эффективно обработать соединение + рукопожатие + запрос в одном TFO-пакете. Более того, можно взглянуть на начало полученного пакета и определить реальный протокол. Поэтому вы можете подключаться к одному и тому же порту через API/http/https. Если же первым должен говорить демон, все эти оптимизации невозможны, и мультипротокол тоже невозможен. Поэтому у нас есть выделенный порт для MySQL, и мы не объединили его со всеми остальными протоколами в один порт. Вдруг среди всех клиентов появился один, написанный с предположением, что демон должен первым отправить рукопожатие. Это означает невозможность использования всех описанных улучшений. Это — плагин SphinxSE для mysql/mariadb. Специально для этого единственного клиента мы выделили определение протокола `sphinx` для работы наиболее традиционным способом. А именно: обе стороны включают `TCP_NODELAY` и обмениваются маленькими пакетами. Демон отправляет рукопожатие при подключении, затем клиент отправляет своё, и затем всё работает как обычно. Это не очень оптимально, но просто работает. Если вы используете SphinxSE для подключения к Manticore — вы должны выделить слушатель с явно указанным протоколом `sphinx`. Для других клиентов — избегайте использования этого слушателя, так как он медленнее. Если вы используете другие устаревшие клиенты Sphinx API — сначала проверьте, могут ли они работать с невыделенным мультипротокольным портом. Для связи мастер-агент, использование невыделенного (мультипротокольного) порта и включение на клиенте и сервере TFO работает хорошо и определённо ускорит работу сетевого бэкенда, особенно если у вас очень лёгкие и быстрые запросы.
</details>

### listen_tfo

Этот параметр позволяет включить флаг TCP_FASTOPEN для всех слушателей. По умолчанию он управляется системой, но может быть явно отключён, установив значение '0'.

Для общей информации о расширении TCP Fast Open см. статью на [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open). Кратко, оно позволяет исключить один TCP круг передачи при установлении соединения.

На практике использование TFO во многих случаях оптимизирует сетевую эффективность клиент-агент, как если бы использовались [постоянные агенты](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md), но без удержания активных соединений, и без ограничения максимального количества соединений.

В современных ОС поддержка TFO обычно включена на уровне системы, но это всего лишь «возможность», а не правило. Linux (как самая продвинутая) поддерживает его с 2011 года, начиная с ядра 3.7 (для серверной стороны). Windows поддерживает это с некоторых сборок Windows 10. Другие операционные системы (FreeBSD, MacOS) тоже участвуют.

Для проверки сервером Linux смотрит переменную `/proc/sys/net/ipv4/tcp_fastopen` и ведёт себя в соответствии с ней. Бит 0 управляет клиентской стороной, бит 1 — слушателями. По умолчанию система имеет этот параметр равным 1, то есть клиенты включены, слушатели отключены.

### log

<!-- example conf log -->
Параметр log указывает имя файла журнала, куда будут записываться все события выполнения `searchd`. Если не указано, имя по умолчанию — 'searchd.log'.

В качестве альтернативы можно использовать имя 'syslog'. В этом случае события будут отправляться демону syslog. Чтобы использовать опцию syslog, нужно собрать Manticore с опцией `-–with-syslog`.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
Ограничивает количество запросов в одном батче. Необязательно, по умолчанию 32.

Заставляет searchd проводить проверку корректности количества запросов, отправленных в одном батче при использовании [мультизапросов](../Searching/Multi-queries.md). Установите в 0, чтобы пропустить проверку.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
Максимальное число одновременно подключенных клиентов. По умолчанию неограниченно. Обычно это заметно только при использовании постоянных соединений, например cli mysql-сессий или постоянных удалённых соединений с распределёнными таблицами. При превышении лимита вы всё равно можете подключиться к серверу, используя [VIP-соединение](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection). VIP-соединения не учитываются в лимит.

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
Глобальное ограничение на количество потоков, которое может использовать одна операция. По умолчанию, соответствующие операции могут использовать все ядра CPU, не оставляя ресурсов для других операций. Например, `call pq` по достаточно большой percolate-таблице может использовать все потоки на десятки секунд. Установка `max_threads_per_query` в, скажем, половину от [threads](../Server_settings/Searchd.md#threads) гарантирует, что вы сможете запускать несколько таких `call pq` операций параллельно.

Также можно установить этот параметр как сессионную или глобальную переменную во время выполнения.

Дополнительно поведение можно контролировать по каждому запросу с помощью опции [threads OPTION](../Searching/Options.md#threads).

<!-- intro -->
##### Пример:
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
Максимально допустимое количество фильтров на запрос. Эта настройка используется только для внутренних проверок корректности и не влияет напрямую на использование ОЗУ или производительность. Необязательно, по умолчанию 256.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
Максимально допустимое количество значений на фильтр. Эта настройка используется только для внутренних проверок корректности и не влияет напрямую на использование ОЗУ или производительность. Необязательно, по умолчанию 4096.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
Максимальное количество файлов, которые сервер может открыть, называется «мягким лимитом». Обратите внимание, что для обслуживания крупных фрагментированных таблиц в режиме реального времени этот лимит может потребоваться установить высоким, так как каждый дисковый кусок может занимать дюжину или более файлов. Например, таблица в реальном времени с 1000 кусочков может потребовать одновременного открытия тысяч файлов. Если в логах появляется ошибка «Too many open files», попробуйте изменить этот параметр — это может помочь решить проблему.

Существует также «жесткий лимит», который нельзя превысить с помощью этого параметра. Этот лимит задаётся системой и может быть изменён в файле `/etc/security/limits.conf` на Linux. В других операционных системах могут быть другие подходы, поэтому обратитесь к документации.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
Кроме непосредственных числовых значений, можно использовать магическое слово «max», чтобы установить лимит равным текущему доступному жёсткому лимиту.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
Максимально разрешённый размер сетевого пакета. Эта настройка ограничивает как пакеты запросов от клиентов, так и пакеты ответов от удалённых агентов в распределённой среде. Используется только для внутренних проверок корректности, не влияет напрямую на использование ОЗУ или производительность. Необязательно, по умолчанию 128M.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_packet_size = 32M
```
<!-- end -->


### mysql_version_string

<!-- example conf mysql_version_string -->
Строка версии сервера для возврата через протокол MySQL. Необязательно, по умолчанию пустая (возвращается версия Manticore).

Некоторые придирчивые библиотеки клиентов MySQL зависят от конкретного формата номера версии, используемого MySQL, и более того, иногда выбирают другой путь исполнения в зависимости от возвращаемого номера версии (а не от указанных флагов возможностей). Например, Python MySQLdb 1.2.2 выдаёт исключение, если номер версии не в формате X.Y.ZZ; MySQL .NET connector 6.3.x внутренне падает на версиях 1.x с определённой комбинацией флагов и т.д. Чтобы обойти это, можно использовать директиву `mysql_version_string` и заставить `searchd` возвращать клиентам, подключающимся по протоколу MySQL, другую версию. (По умолчанию возвращается собственная версия.)


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

Количество сетевых потоков, по умолчанию 1.

Эта настройка полезна при экстремально высоких скоростях запросов, когда одного потока недостаточно для обработки всех входящих запросов.


### net_wait_tm

Регулирует интервал опроса в цикле занятости сетевого потока. По умолчанию -1, может быть установлен в -1, 0 или положительное целое число.

В случаях, когда сервер настроен как чистый мастер и просто маршрутизирует запросы агентам, важно обрабатывать запросы без задержек и не допускать спячки сетевого потока. Для этого существует цикл занятости. После входящего запроса сетевой поток использует опрос ЦП в течение `10 * net_wait_tm` миллисекунд, если `net_wait_tm` положительно, или опрашивает только ЦП, если `net_wait_tm` равен `0`. Также цикл занятости может быть отключён через `net_wait_tm = -1` — в этом случае опрос выполняется с таймаутом, установленным на фактические таймауты агента при системном опросе.

> **ВНИМАНИЕ:** Цикл занятости ЦП фактически загружает ядро процессора, поэтому установка этого параметра в любое значение, отличное от значения по умолчанию, приведёт к заметному использованию процессора даже на простое сервера.


### net_throttle_accept

Определяет, сколько клиентов принимается за каждую итерацию сетевого цикла. По умолчанию 0 (неограниченно), что подходит для большинства пользователей. Это параметр тонкой настройки для управления пропускной способностью сетевого цикла в условиях высокой нагрузки.


### net_throttle_action

Определяет, сколько запросов обрабатывается за каждую итерацию сетевого цикла. По умолчанию 0 (неограниченно), что подходит для большинства пользователей. Это параметр тонкой настройки для управления пропускной способностью сетевого цикла в условиях высокой нагрузки.

### network_timeout

<!-- example conf network_timeout -->
Таймаут чтения/записи сетевого клиентского запроса в секундах (или [special_suffixes](../Server_settings/Special_suffixes.md)). Необязательно, по умолчанию 5 секунд. `searchd` принудительно закроет подключение клиента, который не успевает отправить запрос или получить результат в пределах этого таймаута.

Обратите также внимание на параметр [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet). Этот параметр меняет поведение `network_timeout`, применяя его не к всему `query` или `result`, а к отдельным пакетам. Обычно запрос/результат умещается в один или два пакета. Однако в случаях, когда требуется большой объём данных, этот параметр может быть незаменим для поддержания активной работы.

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
Этот параметр позволяет указать сетевой адрес узла. По умолчанию он совпадает с адресом прослушивания репликации [listen](../Server_settings/Searchd.md#listen). Это правильно в большинстве случаев, однако бывают ситуации, когда нужно указать адрес вручную:

* Узел за файерволом
* Включён сетевой транслятор адресов (NAT)
* Развёртывания в контейнерах, например, Docker или облачные развёртывания
* Кластеры с узлами в разных регионах


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
Этот параметр определяет, разрешать ли запросы, содержащие только оператор полного текста [отрицания](../Searching/Full_text_matching/Operators.md#Negation-operator). Опционально, по умолчанию равно 0 (запросы, содержащие только оператор NOT, не проходят).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
Устанавливает порог сжатия таблицы по умолчанию. Подробнее здесь - [Number of optimized disk chunks](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). Этот параметр можно переопределить в опциях конкретного запроса через [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). Также его можно изменить динамически с помощью команды [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
Этот параметр определяет максимальное количество одновременных постоянных соединений с удалёнными [постоянными агентами](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md). Каждый раз, когда подключается агент, определённый под `agent_persistent`, мы пытаемся повторно использовать существующее соединение (если оно есть), либо подключаемся и сохраняем соединение для будущего использования. Однако в некоторых случаях целесообразно ограничить количество таких постоянных соединений. Эта директива задаёт это ограничение. Оно влияет на количество соединений к хосту каждого агента по всем распределённым таблицам.

Рекомендуется устанавливать значение равным или меньше опции [max_connections](../Server_settings/Searchd.md#max_connections) в конфигурации агента.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file — обязательный параметр конфигурации в Manticore search, который указывает путь к файлу, где хранится идентификатор процесса (PID) сервера `searchd`.

Файл с PID процесса searchd пересоздаётся и блокируется при запуске, и содержит PID главного процесса сервера, пока сервер работает. При остановке сервера файл удаляется.
Назначение этого файла — позволить Manticore выполнять разные внутренние задачи, такие как проверка, запущен ли уже экземпляр `searchd`, остановка `searchd` и уведомление его о необходимости ротации таблиц. Файл также может использоваться внешними автоматизированными скриптами.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
pid_file = /var/run/manticore/searchd.pid
```
<!-- end -->


### predicted_time_costs

<!-- example conf predicted_time_costs -->
Затраты для модели прогнозирования времени выполнения запроса, в наносекундах. Опционально, по умолчанию `doc=64, hit=48, skip=2048, match=64`.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
predicted_time_costs = doc=128, hit=96, skip=4096, match=128
```
<!-- end -->

<!-- example conf predicted_time_costs 1 -->
Прекращение запросов до их завершения на основе времени выполнения (с помощью параметра максимального времени запроса) — это хороший страховочный механизм, но с ним связана неустойчивость результатов. То есть, если вы повторите один и тот же (сложный) поисковый запрос с ограничением по времени несколько раз, то лимит времени будет достигнут на различных этапах, и вы получите *разные* наборы результатов.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT … OPTION max_query_time
```
<!-- request API -->

```api
SetMaxQueryTime()
```
<!-- end -->

Есть опция [SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time), которая позволяет ограничить время запроса *и* получить стабильные, повторяемые результаты. Вместо того чтобы регулярно проверять текущее время выполнения запроса, что даёт нестабильность, используется простая линейная модель для прогнозирования текущего времени:

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

Запрос завершается досрочно, когда `predicted_time` достигает заданного лимита.

Конечно, это не жёсткое ограничение по фактическому времени (но является жёстким ограничением количества выполненной *обработки*), и простая линейная модель далеко не идеально точна. Поэтому реальное время выполнения *может* быть как ниже, так и выше целевого лимита. Тем не менее, погрешности достаточно приемлемы: например, в экспериментах с таргетом 100 мс большинство тестовых запросов укладывалось в интервал 95–105 мс, а *все* запросы были в интервале 80–120 мс. Кроме того, приятным побочным эффектом использования прогнозируемого времени запроса вместо измерения реального являются несколько меньше вызовов функции gettimeofday().

Нет двух одинаковых моделей и производителей серверов, поэтому директива `predicted_time_costs` позволяет настроить затраты для указанной модели. Для удобства они заданы целыми числами, измеряемыми в наносекундах. (Лимит в max_predicted_time считается в миллисекундах, и указывать стоимость в формате 0.000128 мс вместо 128 нс более подвержено ошибкам.) Не обязательно указывать все четыре затратных параметра сразу, пропущенные принимают значения по умолчанию. Однако мы настоятельно рекомендуем указывать все четыре для лучшей читаемости.


### preopen_tables

<!-- example conf preopen_tables -->
Директива конфигурации preopen_tables указывает, нужно ли принудительно предварительно открывать все таблицы при запуске. Значение по умолчанию 1 означает, что все таблицы будут предварительно открыты, независимо от настройки [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) для каждой таблицы. При значении 0 будут учитываться настройки на уровне таблиц, а их значение по умолчанию — 0.

Предварительное открытие таблиц помогает избежать конфликтов между поисковыми запросами и ротациями, которые могут привести к случайным сбоям запросов. Однако это требует больше файловых дескрипторов. В большинстве сценариев рекомендуется использовать предварительное открытие таблиц.

Вот пример конфигурации:

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
Опция конфигурации pseudo_sharding позволяет параллелить поисковые запросы к локальным простым и real-time таблицам, независимо от того, запрашиваются ли они напрямую или через распределённую таблицу. Эта функция автоматически распараллеливает запросы до количества потоков, указанного в `searchd.threads` # потоков.

Обратите внимание, что если ваши рабочие потоки уже заняты, потому что у вас:
* высокая конкуренция запросов
* физическое шардинг любого типа:
  - распределённая таблица, состоящая из нескольких простых/real-time таблиц
  - real-time таблица, состоящая из слишком большого количества дисковых чанков

то включение pseudo_sharding может не дать никаких преимуществ и даже привести к небольшому снижению пропускной способности. Если вы отдаёте приоритет высокой пропускной способности над низкой задержкой, рекомендуется отключить эту опцию.

Включено по умолчанию.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

Директива `replication_connect_timeout` определяет тайм-аут для подключения к удалённому узлу. По умолчанию значение считается в миллисекундах, но может иметь [другой суффикс](../Server_settings/Special_suffixes.md). Значение по умолчанию — 1000 (1 секунда).

При подключении к удалённому узлу Manticore будет ожидать успешного завершения подключения не более указанного времени. Если время ожидания истекает, но соединение не установлено, и включены `retries`, будет инициирована новая попытка подключения.


### replication_query_timeout

Параметр `replication_query_timeout` задаёт максимально допустимое время ожидания выполнения запроса на удалённом узле для searchd. Значение по умолчанию — 3000 миллисекунд (3 секунды), но может иметь суффикс, указывающий другую единицу времени.

После установления соединения Manticore будет ждать максимум `replication_query_timeout` до завершения выполнения запроса удалённым узлом. Отметим, что этот тайм-аут отличается от `replication_connect_timeout`, и общая возможная задержка из-за удалённого узла будет суммой обоих значений.


### replication_retry_count

Это целочисленный параметр, который указывает, сколько раз Manticore попытается подключиться и выполнить запрос к удалённому узлу во время репликации, прежде чем сообщить о критической ошибке запроса. Значение по умолчанию — 3.


### replication_retry_delay

Этот параметр — целое число в миллисекундах (или с [специальным суффиксом](../Server_settings/Special_suffixes.md)), которое задаёт задержку перед повторной попыткой запроса к удалённому узлу в случае сбоя во время репликации. Значение имеет смысл только при ненулевом значении. По умолчанию — 500.

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
Эта настройка задаёт максимальный объём оперативной памяти, выделенной для кэшированных наборов результатов, в байтах. Значение по умолчанию — 16777216, что эквивалентно 16 мегабайтам. Если значение установлено в 0, кэш запросов отключается. Для получения дополнительной информации о кэше запросов смотрите [query cache](../Searching/Query_cache.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

Целое число в миллисекундах. Минимальное пороговое значение времени выполнения запроса, необходимое для его кэширования. По умолчанию — 3000, или 3 секунды. Значение 0 означает кэшировать всё. Подробнее см. [query cache](../Searching/Query_cache.md). Это значение также может быть выражено с помощью временных [специальных суффиксов](../Server_settings/Special_suffixes.md), но используйте их с осторожностью, чтобы не запутаться, поскольку имя параметра содержит '_msec'.


### qcache_ttl_sec

Целое число в секундах. Период времени истечения срока действия кэшированного набора результатов. По умолчанию — 60, или 1 минута. Минимально возможное значение — 1 секунда. Подробнее см. [query cache](../Searching/Query_cache.md). Это значение также может быть выражено с помощью временных [специальных суффиксов](../Server_settings/Special_suffixes.md), но используйте их с осторожностью, чтобы не запутаться, поскольку имя параметра содержит '_sec'.


### query_log_format

<!-- example conf query_log_format -->
Формат журнала запросов. Опционально, допустимые значения: `plain` и `sphinxql`, значение по умолчанию — `sphinxql`.

В режиме `sphinxql` логируются корректные SQL-запросы. В режиме `plain` запросы записываются в формате обычного текста (в основном подходит для чисто полнотекстовых случаев использования). Эта директива позволяет переключаться между двумя форматами при запуске поискового сервера. Формат журнала также можно изменить на лету, используя синтаксис `SET GLOBAL query_log_format=sphinxql`. Подробнее см. [Query logging](../Logging/Query_logging.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

Ограничение (в миллисекундах), которое предотвращает запись запроса в журнал запросов. Опционально, по умолчанию 0 (все запросы записываются в журнал). Эта директива задаёт, что в журнал будут записываться только запросы с временем выполнения, превышающим указанное значение (это значение также может быть выражено с помощью временных [специальных суффиксов](../Server_settings/Special_suffixes.md), но используйте с осторожностью, чтобы не запутаться из-за имени параметра, содержащего `_msec`).

### query_log

<!-- example conf query_log -->
Имя файла журнала запросов. Опционально, по умолчанию пусто (запись запросов не ведётся). Все поисковые запросы (например, SELECT ..., но не INSERT/REPLACE/UPDATE запросы) будут записываться в этот файл. Формат описан в [Query logging](../Logging/Query_logging.md). В случае формата 'plain' можно использовать 'syslog' в качестве пути к лог-файлу. В этом случае все поисковые запросы будут отправляться демону syslog с приоритетом `LOG_INFO`, с префиксом '[query]' вместо временной метки. Для использования опции syslog Manticore должен быть собран с опцией `-–with-syslog`.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
Директива query_log_mode позволяет задать разные права доступа для файлов журналов searchd и query log. По умолчанию эти файлы создаются с правами 600, что означает, что читать журналы могут только пользователь, под которым запущен сервер, и root.
Эта директива может быть полезна, если вы хотите разрешить другим пользователям читать файлы журналов, например, решениям для мониторинга, работающим под пользователями без прав root.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
Директива read_buffer_docs управляет размером буфера чтения на ключевое слово для списков документов. Для каждого вхождения ключевого слова в каждом поисковом запросе существуют два связанных буфера чтения: один для списка документов и один для списка попаданий (hit list). Эта настройка позволяет контролировать размер буфера списка документов.

Больший размер буфера может увеличить использование оперативной памяти на запрос, но возможно уменьшит время ввода-вывода. Имеет смысл задавать большие значения для медленных накопителей, а для накопителей с высокой производительностью IOPS стоит экспериментировать с малыми значениями.

Значение по умолчанию — 256K, минимальное — 8K. Вы также можете задать [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) для каждой таблицы отдельно, что переопределит настройку на уровне сервера.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
Директива read_buffer_hits указывает размер буфера чтения на ключевое слово для списков попаданий (hit lists) в поисковых запросах. По умолчанию размер равен 256K, минимальное значение — 8K. Для каждого вхождения ключевого слова в поисковом запросе существует два связанных буфера чтения: один для списка документов и один для списка попаданий. Увеличение размера буфера может увеличить использование оперативной памяти на запрос, но уменьшить время ввода-вывода. Для медленных накопителей целесообразны большие размеры буферов, а для накопителей с высокой производительностью IOPS стоит экспериментировать с малыми значениями.

Эта настройка также может быть задана для каждой таблицы отдельно с помощью опции read_buffer_hits в [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits), что переопределит настройку на уровне сервера.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
Размер чтения без подсказки. Опционально, значение по умолчанию 32K, минимальное 1K.

При выполнении запросов некоторые операции чтения заранее знают, сколько именно данных нужно считать, но некоторые — нет. Например, размер списка попаданий пока неизвестен заранее. Эта настройка позволяет контролировать, сколько данных читать в таких случаях. Она влияет на время ввода-вывода для списков попаданий, уменьшая его для списков, больших, чем размер чтения без подсказки, но увеличивая для меньших списков. Она **не** влияет на использование оперативной памяти, так как буфер уже будет выделен. Поэтому значение не должно превышать read_buffer.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
Уточняет поведение таймаутов сети (таких как `network_timeout`, `read_timeout` и `agent_query_timeout`).

При значении 0 таймауты ограничивают максимальное время на отправку всего запроса/запроса.
При значении 1 (по умолчанию) таймауты ограничивают максимальное время между сетевой активностью.

При репликации узел может понадобиться отправить большой файл (например, 100ГБ) другому узлу. Предположим, сеть может передавать данные со скоростью 1ГБ/сек, с серией пакетов по 4-5МБ каждый. Чтобы передать весь файл, потребуется 100 секунд. Если таймаут по умолчанию 5 секунд, он позволит передать только 5ГБ, после чего соединение будет разорвано. Увеличение таймаута может быть обходным решением, но оно не масштабируется (например, следующий файл может быть 150ГБ, что опять вызовет ошибку). Однако при значении `reset_network_timeout_on_packet` по умолчанию 1 таймаут применяется не к всей передаче, а к отдельным пакетам. Пока передача идет (и данные реально принимаются по сети в течение периода таймаута), соединение поддерживается. Если передача застрянет, то таймаут между пакетами приведет к разрыву соединения.

Обратите внимание, что если вы настроили распределённую таблицу, каждый узел — и мастер, и агенты — должен быть настроен. На стороне мастера влияет параметр `agent_query_timeout`, на агентах важен `network_timeout`.

<!-- intro -->

##### Пример:

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
Период проверки сброса RAM-чанков для RT таблиц, в секундах (или [special_suffixes](../Server_settings/Special_suffixes.md)). Опционально, по умолчанию 10 часов.

Активно обновляемые RT таблицы, полностью помещающиеся в RAM-чанки, могут приводить к постоянно растущим бинарным логам, что влияет на использование диска и время восстановления после сбоя. С помощью этой директивы поисковый сервер периодически проверяет возможность сброса и может сохранить подходящие RAM-чанки, что позволяет последующую очистку бинарных логов. Подробнее смотрите [Binary logging](../Logging/Binary_logging.md).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
Максимальное количество операций ввода-вывода (в секунду), которые поток слияния RT-чанов может начать. Опционально, по умолчанию 0 (без ограничений).

Эта директива позволяет ограничить нагрузку ввода-вывода, возникающую от операторов `OPTIMIZE`. Гарантируется, что все операции оптимизации RT не создадут больше дисковых IOPS (операций ввода-вывода в секунду), чем заданный лимит. Ограничение rt_merge_iops может снизить ухудшение производительности поиска, вызванное слиянием.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
Максимальный размер операции ввода-вывода, которую поток слияния RT-чанов разрешено начать. Опционально, по умолчанию 0 (без ограничений).

Эта директива позволяет ограничить влияние операций ввода-вывода (I/O), возникающее при выполнении операторов `OPTIMIZE`. Операции ввода-вывода, превышающие этот лимит, будут разбиты на две или более операций, которые затем будут учитываться как отдельные операции в отношении лимита [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops). Таким образом гарантируется, что все операции оптимизации не будут генерировать более чем (rt_merge_iops * rt_merge_maxiosize) байт дискового ввода-вывода в секунду.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
Предотвращает зависания `searchd` во время вращения таблиц с огромным количеством данных для предварительного кэширования. Необязательно, по умолчанию 1 (включена бесшовная ротация). В системах Windows бесшовная ротация отключена по умолчанию.

Таблицы могут содержать данные, которые необходимо предварительно кэшировать в оперативной памяти (RAM). В данный момент файлы `.spa`, `.spb`, `.spi` и `.spm` полностью предварительно кэшируются (они содержат данные атрибутов, данные blob-атрибутов, таблицу ключевых слов и карту удалённых строк соответственно). Без бесшовной ротации при вращении таблицы пытаются использовать минимальное количество оперативной памяти и работают следующим образом:

1. Новые запросы временно отклоняются (с кодом ошибки "retry");
2. `searchd` дожидается завершения всех текущих запросов;
3. Старая таблица освобождается, и её файлы переименовываются;
4. Файлы новой таблицы переименовываются, выделяется необходимая оперативная память;
5. Данные атрибутов и словаря новой таблицы предварительно загружаются в оперативную память;
6. `searchd` продолжает обслуживать запросы из новой таблицы.

Однако если объём данных атрибутов или словаря большой, шаг предварительной загрузки может занять заметное время — до нескольких минут в случае предварительной загрузки файлов размером 1-5+ ГБ.

При включённой бесшовной ротации вращение происходит следующим образом:

1. Выделяется оперативная память для новой таблицы;
2. Данные атрибутов и словаря новой таблицы асинхронно предварительно загружаются в оперативную память;
3. В случае успеха старая таблица освобождается, и файлы обеих таблиц переименовываются;
4. В случае неудачи новая таблица освобождается;
5. В любой момент запросы обслуживаются либо из старой, либо из новой копии таблицы.

Бесшовная ротация требует большего пикового использования оперативной памяти во время вращения (поскольку обе копии данных `.spa/.spb/.spi/.spm` — старая и новая — должны находиться в оперативной памяти при предварительной загрузке новой копии). Среднее использование остаётся прежним.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

Эта опция включает или отключает использование вторичных индексов для поисковых запросов. Она необязательна, по умолчанию 1 (включено). Обратите внимание, что для индексации её включать не нужно, так как она всегда включена при установленной [Manticore Columnar Library](https://github.com/manticoresoftware/columnar). Последняя также необходима для использования индексов при поиске. Доступны три режима:

* `0`: Отключить использование вторичных индексов для поиска. Их можно включать для отдельных запросов с помощью [подсказок анализатора](../Searching/Options.md#Query-optimizer-hints)
* `1`: Включить использование вторичных индексов для поиска. Их можно отключать для отдельных запросов с помощью [подсказок анализатора](../Searching/Options.md#Query-optimizer-hints)
* `force`: Аналогично включению, но любые ошибки при загрузке вторичных индексов будут зафиксированы, и весь индекс не будет загружен в демон.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
Целое число, служащее идентификатором сервера, используемое как зерно для генерации уникального короткого UUID для узлов, входящих в кластер репликации. server_id должен быть уникальным среди узлов кластера и находиться в диапазоне от 0 до 127. Если server_id не задан, он вычисляется как хэш MAC-адреса и пути к файлу PID либо используется случайное число в качестве зерна для короткого UUID.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
Время ожидания `searchd --stopwait` в секундах (или с использованием [special_suffixes](../Server_settings/Special_suffixes.md)). Необязательно, по умолчанию 60 секунд.

Когда вы запускаете `searchd --stopwait`, серверу нужно выполнить несколько действий перед остановкой, таких как завершение запросов, сброс RT-чанков из оперативной памяти, сброс атрибутов и обновление binlog. Эти задачи требуют времени. `searchd --stopwait` будет ждать до `shutdown_timeout` секунд, пока сервер не завершит работу. Подходящее время зависит от размера ваших таблиц и нагрузки.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

SHA1-хэш пароля, необходимого для вызова команды 'shutdown' через VIP Manticore SQL подключение. Без него,[debug](../Reporting_bugs.md#DEBUG) подкоманда 'shutdown' никогда не приведёт к остановке сервера. Обратите внимание, что этот простой хэш не следует считать надёжной защитой, так как используется не подсаленный хэш и никакой современный алгоритм хеширования. Это мера предосторожности для кнопочных служб в локальной сети.

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
Префикс, который добавляется к локальным именам файлов при генерации сниппетов. Необязательно, по умолчанию — текущая рабочая папка.

Этот префикс может использоваться при распределённой генерации сниппетов вместе с опциями `load_files` или `load_files_scattered`.

Обратите внимание, что это именно префикс, а **не** путь! Это означает, что если префикс установлен в "server1" и запрос относится к файлу "file23", `searchd` попытается открыть "server1file23" (всё без кавычек). Поэтому, если вы хотите, чтобы это был путь, вам нужно включить завершающий слэш.

После формирования окончательного пути к файлу сервер разворачивает все относительные директории и сравнивает полученный результат с значением `snippet_file_prefix`. Если результат не начинается с этого префикса, такой файл будет отклонён с сообщением об ошибке.

Например, если вы установите его в `/mnt/data` и кто-то вызовет генерацию сниппета с файлом `../../../etc/passwd` в качестве источника, он получит сообщение об ошибке:

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

вместо содержимого файла.

Также, при неустановленном параметре и чтении `/etc/passwd`, фактически будет прочитан файл /daemon/working/folder/etc/passwd, так как по умолчанию параметр указывает рабочую папку сервера.

Учтите также, что это локальная опция; она никак не влияет на агенты. Поэтому вы можете спокойно установить префикс на мастер-сервере. Запросы, перенаправляемые агентам, не будут затронуты настройкой мастера. Однако они будут зависеть от собственных настроек агента.

Это может быть полезно, например, когда расположения хранилищ документов (локальное хранилище или точки монтирования NAS) не совпадают на разных серверах.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **ПРЕДУПРЕЖДЕНИЕ:** Если вы всё же хотите получить доступ к файлам из корня файловой системы, необходимо явно установить `snippets_file_prefix` в пустое значение (через строку `snippets_file_prefix=`) или в корень (через `snippets_file_prefix=/`).


### sphinxql_state

<!-- example conf sphinxql_state -->
Путь к файлу, в котором будет сериализовано текущее состояние SQL.

При запуске сервера этот файл воспроизводится. При подходящих изменениях состояния (например, SET GLOBAL) он автоматически переписывается. Это помогает предотвратить трудно диагностируемую проблему: если загружены функции UDF, но Manticore аварийно завершил работу, при (автоматическом) перезапуске ваши UDF и глобальные переменные станут недоступны. Использование постоянного состояния помогает обеспечить корректное восстановление без таких неожиданностей.

`sphinxql_state` нельзя использовать для выполнения произвольных команд, таких как `CREATE TABLE`.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
sphinxql_state = uservars.sql
```
<!-- end -->


### sphinxql_timeout

<!-- example conf sphinxql_timeout -->
Максимальное время ожидания между запросами (в секундах или с использованием [специальных суффиксов](../Server_settings/Special_suffixes.md)) при работе через SQL-интерфейс. Необязательно, значение по умолчанию — 15 минут.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
Путь к файлу сертификата Удостоверяющего Центра SSL (CA) (также известному как корневой сертификат). Необязательно, по умолчанию пустое значение. Если задано, сертификат в `ssl_cert` должен быть подписан этим корневым сертификатом.

Сервер использует файл CA для проверки подписи сертификата. Файл должен быть в формате PEM.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
Путь к SSL-сертификату сервера. Необязательно, по умолчанию пустое значение.

Сервер использует этот сертификат как самоподписанный открытый ключ для шифрования HTTP-трафика по SSL. Файл должен быть в формате PEM.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
Путь к закрытому ключу SSL-сертификата. Необязательно, по умолчанию пустое значение.

Сервер использует этот закрытый ключ для шифрования HTTP-трафика по SSL. Файл должен быть в формате PEM.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
Максимальный размер кэша документов общего поддерева на запрос. Необязательно, по умолчанию 0 (отключено).

Эта настройка ограничивает использование ОЗУ оптимизатором общего поддерева (см. [мультизапросы](../Searching/Multi-queries.md)). В сумме для каждого запроса будет выделено не больше этой памяти на кэширование записей документов. Установка лимита в 0 отключает оптимизатор.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
Максимальный размер кэша попаданий общего поддерева на запрос. Необязательно, по умолчанию 0 (отключено).

Эта настройка ограничивает использование ОЗУ оптимизатором общего поддерева (см. [мультизапросы](../Searching/Multi-queries.md)). В сумме для каждого запроса будет выделено не больше этой памяти на кэширование вхождений ключевых слов (попаданий). Установка лимита в 0 отключает оптимизатор.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Количество рабочих потоков (размер пула потоков) для демона Manticore. Manticore создаёт данное количество потоков ОС при запуске, и они выполняют всю работу внутри демона: выполнение запросов, создание сниппетов и прочее. Некоторые операции могут разбиваться на подзадачи и выполняться параллельно, например:

* Поиск в таблице реального времени
* Поиск в распределённой таблице, состоящей из локальных таблиц
* Вызов перколяции запроса
* и другие

По умолчанию устанавливается в количество ядер процессора сервера. Manticore создаёт потоки при запуске и удерживает их до остановки. Каждая подзадача может использовать один из потоков по мере необходимости. По завершении подзадачи поток освобождается для другого использования.

В условиях интенсивной нагрузки типа ввода-вывода может иметь смысл установить значение выше количества ядер процессора.

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
Максимальный размер стека для задачи (корутина, один поисковый запрос может вызвать несколько задач/корутин). Необязательно, по умолчанию 128К.

У каждой задачи есть свой стек размером 128К. При запуске запроса определяется, сколько стека он требует. Если стандартных 128К достаточно, запрос просто обрабатывается. Если требуется больше, планируется другая задача с увеличенным стеком, которая продолжает обработку. Максимальный размер такого расширенного стека ограничен этой настройкой.

Установка разумно высокого значения поможет обрабатывать очень глубокие запросы без значительного увеличения общего потребления ОЗУ. Например, установка 1G не значит, что каждая новая задача будет занимать 1G ОЗУ, а только если выясняется, что задаче требуется, скажем, стек 100M, то для неё выделяется 100M. Другие задачи при этом работают с обычным стеком в 128K. Аналогично можно запускать ещё более сложные запросы, требующие 500M. И только если будет обнаружено, что задача требует более 1G стека, будет ошибка с сообщением о том, что `thread_stack` слишком мало.

Однако на практике даже запрос, которому требуется 16M стека, часто оказывается слишком сложным для парсинга и требует слишком много времени и ресурсов для обработки. Поэтому демон будет его обрабатывать, но ограничение таких запросов с помощью настройки `thread_stack` выглядит вполне разумным.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
Определяет, следует ли отлинковывать копии таблиц с расширением `.old` после успешного ротационного обновления. Опционально, значение по умолчанию — 1 (выполнять отлинковку).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
Потоковый сторож сервера. Опционально, значение по умолчанию — 1 (сторож включён).

Когда запрос в Manticore завершается с ошибкой, он может привести к сбою всего сервера. С включённой функцией сторожа `searchd` также поддерживает отдельный легковесный процесс, который контролирует основной процесс сервера и автоматически перезапускает его в случае ненормального завершения работы. Сторож включён по умолчанию.

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

