# Раздел "Searchd" в конфигурации

Ниже приведены настройки, которые используются в разделе `searchd` конфигурационного файла Manticore Search для управления поведением сервера. Ниже приведено краткое описание каждой настройки:

### access_plain_attrs

Эта настройка задает глобальные параметры по умолчанию для [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, значение по умолчанию — `mmap_preread`.

Директива `access_plain_attrs` позволяет определить значение по умолчанию для [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы на уровне таблиц имеют более высокий приоритет и переопределяют это значение по умолчанию, обеспечивая более тонкий контроль.

### access_blob_attrs

Эта настройка задает глобальные параметры по умолчанию для [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, значение по умолчанию — `mmap_preread`.

Директива `access_blob_attrs` позволяет определить значение по умолчанию для [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы на уровне таблиц имеют более высокий приоритет и переопределяют это значение по умолчанию, обеспечивая более тонкий контроль.

### access_doclists

Эта настройка задает глобальные параметры по умолчанию для [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, значение по умолчанию — `file`.

Директива `access_doclists` позволяет определить значение по умолчанию для [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы на уровне таблиц имеют более высокий приоритет и переопределяют это значение по умолчанию, обеспечивая более тонкий контроль.

### access_hitlists

Эта настройка задает глобальные параметры по умолчанию для [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, значение по умолчанию — `file`.

Директива `access_hitlists` позволяет определить значение по умолчанию для [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы на уровне таблиц имеют более высокий приоритет и переопределяют это значение по умолчанию, обеспечивая более тонкий контроль.

### access_dict

Эта настройка задает глобальные параметры по умолчанию для [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, значение по умолчанию — `mmap_preread`.

Директива `access_dict` позволяет определить значение по умолчанию для [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы на уровне таблиц имеют более высокий приоритет и переопределяют это значение по умолчанию, обеспечивая более тонкий контроль.

### agent_connect_timeout

Эта настройка задает глобальные параметры по умолчанию для параметра [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout).


### agent_query_timeout

Эта настройка задает глобальные параметры по умолчанию для параметра [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout). Ее можно переопределять для каждого запроса отдельно с помощью клаузы `OPTION agent_query_timeout=XXX`.


### agent_retry_count

Эта настройка — это целое число, указывающее, сколько раз Manticore попытается подключиться и выполнить запросы к удаленным агентам через распределенную таблицу, прежде чем сообщить о фатальной ошибке запроса. Значение по умолчанию — 0 (т.е. без повторных попыток). Вы также можете задать это значение для каждого запроса отдельно, используя опцию `OPTION retry_count=XXX`. Если указана опция на уровне запроса, она переопределит значение, указанное в конфигурации.

Обратите внимание, что если вы используете [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors) в определении распределенной таблицы, сервер будет выбирать другой зеркало для каждой попытки подключения согласно выбранной [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy). В этом случае значение `agent_retry_count` будет суммироваться для всех зеркал в наборе.

Например, если у вас 10 зеркал и установлено `agent_retry_count=5`, сервер выполнит до 50 попыток, предполагая в среднем 5 попыток для каждого из 10 зеркал (с опцией `ha_strategy = roundrobin` это будет так).

Однако значение, заданное как опция `retry_count` для [агента](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent), служит абсолютным лимитом. Иными словами, опция `[retry_count=2]` в определении агента всегда означает максимум 2 попытки, независимо от того, указано ли у вас 1 или 10 зеркал для агента.

### agent_retry_delay

Эта настройка — целое число в миллисекундах (или [special_suffixes](../Server_settings/Special_suffixes.md)), указывающее задержку перед повторной попыткой запроса к удаленному агенту в случае сбоя. Это значение актуально только при ненулевом [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) или ненулевой опции `retry_count` на уровне запроса. Значение по умолчанию — 500. Вы также можете задать это значение для каждого запроса отдельно с помощью клаузы `OPTION retry_delay=XXX`. Если указана опция на уровне запроса, она переопределит значение, указанное в конфигурации.


### attr_flush_period

<!-- example conf attr_flush_period -->
При использовании [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) для изменения атрибутов документа в реальном времени, изменения сначала записываются в копию атрибутов в памяти. Эти обновления происходят в файле с отображением в память, что означает, что ОС решает, когда записывать изменения на диск. При нормальном завершении работы `searchd` (инициируемом сигналом `SIGTERM`) все изменения принудительно записываются на диск.

Вы также можете указать `searchd` периодически записывать эти изменения на диск, чтобы предотвратить потерю данных. Интервал между этими сбросами определяется параметром `attr_flush_period`, указанным в секундах (или [special_suffixes](../Server_settings/Special_suffixes.md)).

По умолчанию значение равно 0, что отключает периодические сбросы. Однако сбросы всё равно будут происходить при нормальном завершении работы.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
Этот параметр управляет автоматическим процессом [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) для сжатия таблицы.

По умолчанию сжатие таблицы происходит автоматически. Вы можете изменить это поведение с помощью настройки `auto_optimize`:
* 0 — отключить автоматическое сжатие таблицы (вы всё равно можете вызвать `OPTIMIZE` вручную)
* 1 — явно включить его
* для включения с умножением порога оптимизации на 2.

По умолчанию OPTIMIZE запускается, пока число дисковых чанков не станет меньше или равно числу логических ядер CPU, умноженному на 2.

Однако, если таблица имеет атрибуты с KNN индексами, этот порог отличается. В этом случае он устанавливается равным числу физических ядер CPU, делённому на 2, для повышения производительности поиска KNN.

Обратите внимание, что включение или отключение `auto_optimize` не препятствует вам запускать [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) вручную.

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
Manticore поддерживает автоматическое создание таблиц, которые ещё не существуют, но указаны в операторах INSERT. Эта функция включена по умолчанию. Чтобы отключить её, явно установите `auto_schema = 0` в вашей конфигурации. Чтобы снова включить, установите `auto_schema = 1` или удалите настройку `auto_schema` из конфигурационного файла.

Имейте в виду, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц.

> ПРИМЕЧАНИЕ: Функционал [auto schema](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если он не работает, убедитесь, что Buddy установлен.

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
Этот параметр управляет режимом сброса/синхронизации транзакций бинарного лога. Является необязательным, значение по умолчанию — 2 (сброс каждый транзакцию, синхронизация каждую секунду).

Директива определяет, как часто бинарный лог будет сбрасываться в ОС и синхронизироваться на диск. Поддерживаются три режима:

*  0 — сброс и синхронизация каждую секунду. Это обеспечивает наилучшую производительность, но в случае сбоя сервера или ОС/аппаратного сбоя может быть потеряна до 1 секунды коммитов.
*  1 — сброс и синхронизация каждой транзакции. Этот режим обеспечивает худшую производительность, но гарантирует сохранение данных каждой выполненной транзакции.
*  2 — сброс каждой транзакции, синхронизация каждую секунду. Этот режим обеспечивает хорошую производительность и гарантирует, что каждая выполненная транзакция сохранена в случае сбоя сервера. Однако при сбое ОС/аппаратном сбое может быть потеряна до 1 секунды коммитов.

Для тех, кто знаком с MySQL и InnoDB, эта директива аналогична `innodb_flush_log_at_trx_commit`. В большинстве случаев режим по умолчанию 2 обеспечивает хороший баланс скорости и безопасности, с полной защитой данных таблиц RT от сбоев сервера и частичной защитой от аппаратных сбоев.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
Данная настройка управляет тем, как управляются файлы бинарного лога. Является необязательной, значение по умолчанию — 0 (отдельный файл для каждой таблицы).

Вы можете выбрать один из двух способов управления файлами бинарного лога:

* Отдельный файл для каждой таблицы (по умолчанию, `0`): каждая таблица сохраняет изменения в своём файле лога. Это удобно, если у вас много таблиц, которые обновляются в разное время. Позволяет обновлять таблицы без ожидания других. Также при проблемах с файлом лога одной таблицы, это не влияет на другие.
* Один файл для всех таблиц (`1`): все таблицы используют один общий файл бинарного лога. Это облегчает управление файлами, так как их меньше. Однако файлы могут храниться дольше, если одна таблица всё ещё нуждается в сохранении своих обновлений. Эта настройка также может замедлять работу при одновременных обновлениях многих таблиц, поскольку все изменения должны ждать записи в один файл.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
Эта настройка контролирует максимальный размер файла бинарного лога. Необязательна, значение по умолчанию — 256 МБ.

Новый файл бинарного лога будет принудительно открыт, как только текущий достигнет этого размера. Это даёт более мелкую гранулярность логов и может привести к более эффективному использованию диска бинарного лога при определённых граничных нагрузках. Значение 0 означает, что файл бинарного лога не будет переоткрываться по размеру.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
Эта настройка определяет путь для файлов бинарного лога (иначе называемых журналом транзакций). Необязательна, значение по умолчанию — директория данных, заданная на этапе сборки (например, `/var/lib/manticore/data/binlog.*` в Linux).

Двоичные журналы используются для восстановления данных таблиц RT после сбоев и для обновлений атрибутов простых дисковых индексов, которые в противном случае хранились бы только в ОЗУ до сброса. Когда журналирование включено, каждая транзакция, зафиксированная (COMMIT) в таблице RT, записывается в файл журнала. Логи затем автоматически воспроизводятся при запуске после некорректного завершения работы, восстанавливая зарегистрированные изменения.

Директива `binlog_path` задает расположение файлов двоичного журнала. Она должна содержать только путь; `searchd` создаст и удалит несколько файлов `binlog.*` в каталоге по мере необходимости (включая данные журнала, метаданные и файлы блокировки и т.д.).

Пустое значение отключает ведение двоичного журнала, что улучшает производительность, но подвергает данные таблицы RT риску.


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
Этот параметр определяет путь к бинарному файлу Manticore Buddy. Он необязательный, по умолчанию содержит путь, настроенный во время сборки, который различается в зависимости от операционной системы. Обычно не нужно менять этот параметр. Однако это может быть полезно, если вы хотите запустить Manticore Buddy в режиме отладки, внести изменения в Manticore Buddy или реализовать новый плагин. В последнем случае вы можете выполнить `git clone` Buddy с https://github.com/manticoresoftware/manticoresearch-buddy, добавить новый плагин в директорию `./plugins/`, и выполнить `composer install --prefer-source` для удобства разработки после перехода в директорию с исходниками Buddy.

Чтобы иметь возможность запускать `composer`, на вашей машине должен быть установлен PHP версии 8.2 или выше с следующими расширениями:

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

Если выберете этот вариант, не забудьте связать dev-версию исполнителя manticore с `/usr/bin/php`.

Чтобы отключить Manticore Buddy, установите значение в пустую строку, как показано в примере.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php # use the default Manticore Buddy in Linux
buddy_path = manticore-executor -n /opt/homebrew/share/manticore/modules/manticore-buddy/bin/manticore-buddy/src/main.php # use the default Manticore Buddy in MacOS arm64
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php # use Manticore Buddy from a non-default location
buddy_path = # disables Manticore Buddy
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php --skip=manticoresoftware/buddy-plugin-replace # --skip - skips plugins
```
<!-- end -->

### client_timeout

<!-- example conf client_timeout -->
Этот параметр определяет максимальное время ожидания между запросами (в секундах или с использованием [специальных суффиксов](../Server_settings/Special_suffixes.md)) при использовании постоянных соединений. Необязательный параметр, значение по умолчанию — пять минут.


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

Задаёт локаль libc, влияющую на сортировки на основе libc. Подробности см. в разделе [collations](../Searching/Collations.md).


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

Задает коллацию по умолчанию для входящих запросов. Коллация может быть переопределена для каждого запроса отдельно. Список доступных коллаций и подробности см. в разделе [collations](../Searching/Collations.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
Если указано, этот параметр включает [режим реального времени](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29), который является императивным способом управления схемой данных. Значение должно быть путем к каталогу, в котором вы хотите хранить все ваши таблицы, двоичные журналы и все остальное, необходимое для корректной работы Manticore Search в этом режиме.
Индексирование [простых таблиц](../Creating_a_table/Local_tables/Plain_table.md) запрещено, если указан `data_dir`. Подробнее о различиях между режимом RT и простым режимом читайте в [этом разделе](../Read_this_first.md#Real-time-table-vs-plain-table).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
Таймаут для предотвращения автоматического сброса RAM-чаши, если в таблице нет поисков. Необязательный параметр, значение по умолчанию — 30 секунд.

Время проверки наличия поисков перед решением о автосбросе.
Автосброс произойдет только если в таблице был хотя бы один поиск за последние `diskchunk_flush_search_timeout` секунд. Работает совместно с [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout). Соответствующая [параметр для таблицы](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout) имеет более высокий приоритет и переопределит значение по умолчанию для инстанса, обеспечивая более тонкий контроль.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
Время ожидания в секундах без записи перед автоматическим сбросом RAM-чаши на диск. Необязательный параметр, значение по умолчанию — 1 секунда.

Если запись в RAM-чашу не происходит в течение `diskchunk_flush_write_timeout` секунд, чаша сбрасывается на диск. Работает совместно с [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout). Чтобы отключить автосброс, явно установите `diskchunk_flush_write_timeout = -1` в конфигурации. Соответствующая [параметр для таблицы](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout) имеет более высокий приоритет и переопределит значение по умолчанию для инстанса, обеспечивая более тонкий контроль.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
Этот параметр задает максимальный размер блоков документов из хранилища документов, которые удерживаются в памяти. Необязательный параметр, значение по умолчанию — 16m (16 мегабайт).

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

For a distributed table with agent mirrors in it (see more in [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md),  the master tracks several different per-mirror counters. These counters are then used for failover and balancing (the master picks the best mirror to use based on the counters). Counters are accumulated in blocks of `ha_period_karma` seconds.

After beginning a new block, the master may still use the accumulated values from the previous one until the new one is half full. As a result, any previous history stops affecting the mirror choice after 1.5 times ha_period_karma seconds at most.

Even though at most two blocks are used for mirror selection, up to 15 last blocks are stored for instrumentation purposes. These blocks can be inspected using the [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) statement.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
Эта настройка задает интервал между ping-запросами зеркал агента в миллисекундах (или [special_suffixes](../Server_settings/Special_suffixes.md)). Она является необязательной, значение по умолчанию — 1000 миллисекунд.

Для распределенной таблицы с зеркалами агента (см. подробнее в [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)) мастер посылает всем зеркалам команду ping в периоды простоя. Это необходимо для отслеживания текущего состояния агента (живой или не отвечает, сетевой отклик и т. д.). Интервал между такими ping-запросами задается этой директивой. Чтобы отключить ping-запросы, установите ha_ping_interval в 0.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

Опция `hostname_lookup` определяет стратегию обновления имен хостов. По умолчанию IP-адреса имен хостов агента кэшируются при запуске сервера, чтобы избежать чрезмерных обращений к DNS. Однако в некоторых случаях IP может динамически меняться (например, в облачных хостингах), и может потребоваться не кэшировать IP. Установка этой опции в `request` отключает кэширование и выполняет запрос к DNS при каждом обращении. IP-адреса также можно вручную обновить с помощью команды `FLUSH HOSTNAMES`.

### jobs_queue_size

Настройка jobs_queue_size определяет, сколько "заданий" может находиться в очереди одновременно. По умолчанию – без ограничений.

В большинстве случаев "задание" означает один запрос к одной локальной таблице (простая таблица или дисковый чанк таблицы в реальном времени). Например, если у вас распределённая таблица, состоящая из 2 локальных таблиц, или таблица в реальном времени с 2 дисковыми чанками, запрос поиска к любой из них обычно создаст в очереди 2 задания. Затем пул потоков (размер которого задается параметром [threads](../Server_settings/Searchd.md#threads)) обрабатывает эти задания. Однако в некоторых случаях, если запрос очень сложный, может создаваться больше заданий. Рекомендуется изменять эту настройку, когда [max_connections](../Server_settings/Searchd.md#max_connections) и [threads](../Server_settings/Searchd.md#threads) недостаточны для достижения баланса между желаемой производительностью.

### join_batch_size

Объединения таблиц работают путем накопления партии совпадений — результатов запроса, выполненного по левой таблице. Эта партия затем обрабатывается как единый запрос по правой таблице.

Эта опция позволяет отрегулировать размер партии. Значение по умолчанию — `1000`, при установке этой опции в `0` пакетирование отключается.

Больший размер партии может улучшить производительность; однако для некоторых запросов это может привести к избыточному расходу памяти.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

Каждый запрос, выполняемый по правой таблице, определяется конкретными условиями JOIN ON, которые определяют набор результатов, получаемых из правой таблицы.

Если уникальных условий JOIN ON немного, повторное использование результатов может быть более эффективным, чем неоднократное выполнение запросов к правой таблице. Для этого наборы результатов сохраняются в кеше.

Эта опция позволяет настроить размер этого кеша. Значение по умолчанию — `20 MB`, при установке этой опции в 0 кеширование отключается.

Обратите внимание, что каждый поток поддерживает собственный кеш, поэтому при оценке общего использования памяти следует учитывать количество потоков, выполняющих запросы.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
Настройка listen_backlog определяет длину TCP listen backlog для входящих соединений. Это особенно важно для сборок Windows, которые обрабатывают запросы по одному. Когда очередь соединений достигает предела, новые входящие соединения будут отклоняться.
Для сборок на не-Windows операционных системах значение по умолчанию обычно работает хорошо, и обычно нет необходимости в настройке этого параметра.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
Строка версии сервера, возвращаемая Kibana или OpenSearch Dashboards. Опционально — по умолчанию установлена на `7.6.0`.

Некоторые версии Kibana и OpenSearch Dashboards ожидают, что сервер сообщит конкретный номер версии, и могут вести себя по-разному в зависимости от этого. Чтобы обойти такие проблемы, вы можете использовать эту настройку, которая заставляет Manticore сообщать Kibana или OpenSearch Dashboards пользовательскую версию.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
Эта настройка позволяет указать IP-адрес и порт либо путь к Unix-доменному сокету, на которых Manticore будет принимать соединения.

Общий синтаксис для `listen`:

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

Вы можете указать:
* либо IP-адрес (или имя хоста) и номер порта
* либо только номер порта
* либо путь к Unix-сокету (не поддерживается в Windows)
* либо IP-адрес и диапазон портов

Если указан номер порта, но не адрес, `searchd` будет слушать все сетевые интерфейсы. Unix-путь определяется начальным слэшем. Диапазон портов может быть установлен только для протокола репликации.

Вы также можете указать обработчик протокола (listener), который будет использоваться для соединений на этом сокете. Возможные listeners:

* **Не указан** — Manticore примет соединения на этом порту от:
  - других агентов Manticore (например, удаленная распределенная таблица)
  - клиентов через HTTP и HTTPS
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/). **Убедитесь, что у вас есть listener такого типа (или `http` listener, как указано ниже), чтобы не ограничивать функциональность Manticore.**
* `mysql` Протокол MySQL для соединений от MySQL клиентов. Обратите внимание:
  - Поддерживается также сжатый протокол.
  - Если включен [SSL](../Security/SSL.md#SSL), можно установить зашифрованное соединение.
* `replication` - протокол репликации, используемый для связи между узлами. Подробнее можно узнать в разделе [replication](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md). Вы можете указать несколько слушателей репликации, но все они должны слушать на одном и том же IP; портами могут отличаться. При определении слушателя репликации с диапазоном портов (например, `listen = 192.168.0.1:9320-9328:replication`), Manticore не начинает сразу слушать эти порты. Вместо этого он возьмет случайные свободные порты из указанного диапазона только при начале использования репликации. Для корректной работы репликации требуется как минимум 2 порта в диапазоне.
* `http` - то же, что и **Не указано**. Manticore будет принимать подключения на этом порту от удалённых агентов и клиентов через HTTP и HTTPS.
* `https` - протокол HTTPS. Manticore будет принимать **только** HTTPS-соединения на этом порту. Подробнее см. раздел [SSL](../Security/SSL.md).
* `sphinx` - устаревший бинарный протокол. Используется для обслуживания соединений от удалённых клиентов [SphinxSE](../Extensions/SphinxSE.md). Некоторые реализации клиентов Sphinx API (примеры — Java) требуют явного объявления слушателя.

Добавление суффикса `_vip` к клиентским протоколам (то есть ко всем, кроме `replication`, например `mysql_vip` или `http_vip` или просто `_vip`) заставляет создать выделённый поток для соединения, чтобы обойти различные ограничения. Это полезно для обслуживания узла при серьёзной перегрузке, когда сервер в противном случае будет либо зависать, либо не позволит вам подключиться через обычный порт.

Суффикс `_readonly` включает для слушателя [режим только для чтения](../Security/Read_only.md) и ограничивает его приём только запросами на чтение.

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

Может быть указано несколько директив `listen`. `searchd` будет слушать клиентские подключения на всех указанных портах и сокетах. В стандартной конфигурации, предоставляемой в пакетах Manticore, настроено прослушивание на портах:
* `9308` и `9312` для подключений от удалённых агентов и клиентов, не использующих MySQL
* а также на порту `9306` для MySQL соединений.

Если вы вообще не указываете `listen` в конфигурации, Manticore будет ждать подключения на:
* `127.0.0.1:9306` для MySQL клиентов
* `127.0.0.1:9312` для HTTP/HTTPS и соединений от других узлов Manticore и клиентов на базе бинарного API Manticore.

#### Прослушивание привилегированных портов

По умолчанию Linux не позволит Manticore слушать порт ниже 1024 (например, `listen = 127.0.0.1:80:http` или `listen = 127.0.0.1:443:https`), если вы не запускаете searchd от имени root. Если вы хотите, чтобы Manticore слушал порты < 1024 под обычным пользователем, рассмотрите один из следующих вариантов (любой из них должен работать):
* Выполните команду `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* Добавьте `AmbientCapabilities=CAP_NET_BIND_SERVICE` в системный unit Manticore для systemd и перезагрузите демон (`systemctl daemon-reload`).

#### Технические детали о протоколе Sphinx API и TFO
<details>
Устаревший протокол Sphinx состоит из двух фаз: обмена рукопожатием и передачи данных. Рукопожатие — это пакет из 4 байт от клиента и пакет из 4 байт от демона, с единственной целью — клиент удостоверяется, что удалённый сервер — реальный демон Sphinx, а демон удостоверяется, что удалённый узел — реальный клиент Sphinx. Основной поток данных достаточно прост: обе стороны объявляют свои рукопожатия и проверяют рукопожатия противоположной стороны. Этот обмен короткими пакетами предполагает использование специального флага `TCP_NODELAY`, который отключает алгоритм Nagle TCP и объявляет, что TCP соединение будет происходить в виде диалога маленьких пакетов.
Однако строго не определено, кто начинает первый в этом переговоре. Исторически все клиенты, использующие бинарный API, начали первыми: они отправляли рукопожатие, затем читали 4 байта от демона, потом отправляли запрос и читали ответ от демона.
Когда мы улучшали совместимость с протоколом Sphinx, мы учли следующее:

1. Обычно связь master-agent устанавливается с известного клиента к известному хосту на известном порту. Поэтому весьма маловероятно, что конечная точка предоставит неправильное рукопожатие. Можно неявно считать, что обе стороны валидны и действительно разговаривают по протоколу Sphinx.
2. Исходя из этого предположения, мы можем "склеить" рукопожатие с реальным запросом и отправить их в одном пакете. Если backend — это устаревший демон Sphinx, он просто прочитает этот склеенный пакет как 4 байта рукопожатия, а затем тело запроса. Поскольку они пришли одним пакетом, TCP-сокет backend имеет -1 RTT, а фронтенд-буфер по-прежнему функционирует привычным образом.
3. Продолжая предположение: поскольку пакет «запроса» довольно маленький, а рукопожатие ещё меньше, давайте отправим оба в начальном TCP-пакете «SYN» с использованием современной технологии TFO (tcp-fast-open). То есть: мы подключаемся к удалённому узлу с пакетами склеенного рукопожатия + тела. Демон принимает подключение и немедленно имеет и рукопожатие, и тело в буфере сокета, так как они пришли в самом первом TCP-пакете «SYN». Это устраняет ещё один RTT.
4. Наконец, обучаем демон принимать это улучшение. Фактически, это подразумевает НЕ использовать `TCP_NODELAY` на уровне приложения. А с системной стороны — обеспечить активацию TFO на стороне демона при приёме и на стороне клиента при отправке. По умолчанию в современных системах TFO у клиентов уже активирован, поэтому нужно только настроить серверный TFO для исправной работы.

Все эти улучшения без фактического изменения протокола позволили устранить 1.5 RTT TCP протокола при соединении. Если запрос и ответ умещаются в один TCP-пакет, это сокращает всю сессию бинарного API с 3.5 RTT до 2 RTT — что делает сетевой обмен примерно в 2 раза быстрее.

Итак, все наши улучшения построены вокруг изначально неопределенного утверждения: «кто говорит первым». Если клиент говорит первым, мы можем применить все эти оптимизации и эффективно обработать соединение + рукопожатие + запрос в одном пакете TFO. Более того, мы можем посмотреть в начало полученного пакета и определить реальный протокол. Вот почему вы можете подключаться к одному и тому же порту через API/http/https. Если демону нужно говорить первым, все эти оптимизации невозможны, и мультипротокол также невозможен. Вот почему у нас есть выделенный порт для MySQL и мы не объединили его со всеми другими протоколами в один порт. Вдруг среди всех клиентов один был написан с предположением, что демон должен сначала отправить рукопожатие. Это означает — отсутствие возможности для всех описанных улучшений. Это плагин SphinxSE для mysql/mariadb. Поэтому специально для этого единственного клиента мы выделили определение протокола `sphinx`, чтобы работать максимально по устаревшему сценарию. А именно: обе стороны активируют `TCP_NODELAY` и обмениваются мелкими пакетами. Демон отправляет свое рукопожатие при подключении, затем клиент отправляет свое, и затем всё работает в обычном режиме. Это не очень оптимально, но просто работает. Если вы используете SphinxSE для подключения к Manticore — вам нужно выделить слушатель с явно указанным протоколом `sphinx`. Для других клиентов — избегайте использовать этот слушатель, так как он медленнее. Если вы используете других клиентов с устаревшим API Sphinx — сначала проверьте, могут ли они работать с невыделенным мультипротокольным портом. Для связи мастер-агента использование невыделенного (мультипротокольного) порта и включение TFO на клиенте и сервере хорошо работает и определенно ускорит работу сетевого бэкенда, особенно если у вас очень легкие и быстрые запросы.
</details>

### listen_tfo

Этот параметр позволяет включить флаг TCP_FASTOPEN для всех слушателей. По умолчанию он управляется системой, но может быть явно отключен установкой значения в '0'.

Для общего понимания расширения TCP Fast Open, пожалуйста, ознакомьтесь с [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open). Вкратце, оно позволяет исключить один круг TCP при установлении соединения.

На практике использование TFO во многих случаях может оптимизировать сетевую эффективность клиент-агент, словно [постоянные агенты](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) на связи, но без удержания активных соединений, а также без ограничения максимального количества соединений.

На современных ОС поддержка TFO обычно включена на уровне системы, но это лишь «возможность», а не правило. Linux (как самая прогрессивная) поддерживает это с 2011 года, начиная с ядра 3.7 (для серверной стороны). Windows поддерживает это начиная с некоторых сборок Windows 10. Другие операционные системы (FreeBSD, MacOS) тоже поддерживают.

Для Linux система проверяет переменную `/proc/sys/net/ipv4/tcp_fastopen` и ведет себя согласно ей. Бит 0 управляет клиентской стороной, бит 1 — слушателями. По умолчанию система настроена на 1, то есть клиент включен, слушатели отключены.

### log

<!-- example conf log -->
Параметр log задает имя файла журнала, в который будут записаны все события времени выполнения `searchd`. Если не задан, по умолчанию используется имя 'searchd.log'.

В качестве альтернативы можно использовать 'syslog' в качестве имени файла. В этом случае события будут отправлены демону syslog. Чтобы использовать опцию syslog, необходимо при сборке Manticore указать опцию `-–with-syslog`.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
Ограничивает количество запросов в одном пакете. Опционально, по умолчанию 32.

Заставляет searchd выполнять проверку количества запросов, отправляемых в одном пакете при использовании [мультизапросов](../Searching/Multi-queries.md). Установите в 0, чтобы отключить проверку.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
Максимальное количество одновременных соединений клиентов. По умолчанию неограничено. Обычно это заметно только при использовании постоянных соединений, например в cli mysql-сессиях или постоянных удалённых соединениях из удалённых распределённых таблиц. При превышении лимита вы всё равно можете подключиться к серверу через [VIP-соединение](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection). VIP-соединения не учитываются в лимит.

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
Глобальное ограничение количества потоков, которое может использовать одна операция. По умолчанию соответствующие операции могут занимать все ядра процессора, не оставляя ресурсов для других операций. Например, `call pq` к довольно большой перколяторной таблице может использовать все потоки на десятки секунд. Установка `max_threads_per_query` в, скажем, половину от значения [threads](../Server_settings/Searchd.md#threads) позволит параллельно запускать несколько таких `call pq`.

Этот параметр также можно установить как переменную сессии или глобальную переменную во время выполнения.

Дополнительно вы можете контролировать поведение на уровне отдельного запроса с помощью опции [threads OPTION](../Searching/Options.md#threads).

<!-- intro -->
##### Пример:
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
Максимальное допустимое количество фильтров на один запрос. Этот параметр используется только для внутренних проверок и напрямую не влияет на использование ОЗУ или производительность. Опционально, по умолчанию 256.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
Максимальное допустимое количество значений в одном фильтре. Этот параметр используется только для внутренних проверок и напрямую не влияет на использование ОЗУ или производительность. Опционально, по умолчанию 4096.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
Максимальное количество файлов, которое серверу разрешено открыть, называется «мягким лимитом». Обратите внимание, что обслуживание больших фрагментированных таблиц в реальном времени может потребовать установки этого лимита на высоком уровне, поскольку каждый дисковый фрагмент может занимать дюжину и более файлов. Например, таблица в реальном времени с 1000 фрагментами может требовать одновременного открытия тысяч файлов. Если в логах появляется ошибка «Too many open files», попробуйте отрегулировать этот параметр, так как это может помочь решить проблему.

Существует также «жесткий лимит», который не может быть превышен данной опцией. Этот лимит определяется системой и может быть изменен в файле `/etc/security/limits.conf` на Linux. Другие операционные системы могут использовать другие подходы, поэтому обратитесь к вашим руководствам для получения дополнительной информации.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
Помимо прямых числовых значений, вы можете использовать магическое слово «max» для установки лимита, равного текущему доступному жесткому лимиту.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
Максимально допустимый размер сетевого пакета. Эта настройка ограничивает как пакеты запросов от клиентов, так и пакеты ответов от удаленных агентов в распределенной среде. Используется только для внутренних проверок целостности, непосредственно не влияет на использование ОЗУ или производительность. Необязательно, по умолчанию 128M.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_packet_size = 32M
```
<!-- end -->


### mysql_version_string

<!-- example conf mysql_version_string -->
Строка версии сервера, возвращаемая через протокол MySQL. Необязательно, по умолчанию пустая (возвращает версию Manticore).

Некоторые придирчивые библиотеки клиентов MySQL зависят от конкретного формата номера версии, используемого MySQL, и более того, иногда выбирают различный путь выполнения на основе сообщаемого номера версии (а не указанных флагов возможностей). Например, Python MySQLdb 1.2.2 генерирует исключение, если номер версии не в формате X.Y.ZZ; MySQL .NET connector 6.3.x внутренне ошибается на версиях 1.x при определенной комбинации флагов и т.д. Чтобы обойти это, вы можете использовать директиву `mysql_version_string` и заставить `searchd` сообщать клиентам, подключающимся через протокол MySQL, другую версию. (По умолчанию он сообщает свою собственную версию.)


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

Количество сетевых потоков, по умолчанию 1.

Эта настройка полезна при чрезвычайно высокой скорости запросов, когда одного потока недостаточно для обработки всех входящих запросов.


### net_wait_tm

Управляет интервалом busy-loop сетевого потока. По умолчанию -1, может быть установлен в -1, 0 или положительное целое число.

В случаях, когда сервер настроен как чистый мастер и только маршрутизирует запросы агентам, важно обрабатывать запросы без задержек и не позволять сетевому потоку засыпать. Для этого существует busy-loop. После входящего запроса сетевой поток использует CPU polling в течение `10 * net_wait_tm` миллисекунд, если `net_wait_tm` положительное число, или просто опрашивает CPU, если `net_wait_tm` равен `0`. Также busy-loop можно отключить с помощью `net_wait_tm = -1` — в этом случае poller устанавливает таймауты, соответствующие фактическим таймаутам агентов на системном вызове опроса.

> **ВНИМАНИЕ:** busy-loop загрузит ядро CPU, поэтому установка этого значения в любое отличное от дефолтного вызовет заметную загрузку CPU даже при бездействующем сервере.


### net_throttle_accept

Определяет, сколько клиентов принимается на каждой итерации сетевого цикла. По умолчанию 0 (без ограничений), что подходит большинству пользователей. Это опция тонкой настройки для контроля пропускной способности сетевого цикла при высокой нагрузке.


### net_throttle_action

Определяет, сколько запросов обрабатывается на каждой итерации сетевого цикла. По умолчанию 0 (без ограничений), что подходит большинству пользователей. Это опция тонкой настройки для контроля пропускной способности сетевого цикла при высокой нагрузке.

### network_timeout

<!-- example conf network_timeout -->
Таймаут чтения/записи сетевого клиента в секундах (или с использованием [special_suffixes](../Server_settings/Special_suffixes.md)). Необязательно, по умолчанию 5 секунд. `searchd` принудительно закроет соединение клиента, который не отправляет запрос или не читает результат в течение этого таймаута.

Обратите также внимание на параметр [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet). Этот параметр изменяет поведение `network_timeout` с применения к полному `query` или `result` на отдельные пакеты. Обычно запрос/результат помещается в один-два пакета. Однако в случаях, когда требуется большой объем данных, этот параметр может быть незаменим в поддержании активной операции.

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
Этот параметр позволяет указать сетевой адрес узла. По умолчанию он установлен в адрес репликации [listen](../Server_settings/Searchd.md#listen). Это корректно в большинстве случаев; однако бывают ситуации, когда необходимо указать его вручную:

* Узел за файрволом
* Включен NAT (преобразование сетевых адресов)
* Развертывание в контейнерах, например Docker или облачные развертывания
* Кластеры с узлами в нескольких регионах


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
Этот параметр определяет, разрешать ли запросы, содержащие только полнотекстовый оператор [NOT](../Searching/Full_text_matching/Operators.md#Negation-operator). Необязательно, по умолчанию 0 (запросы с только оператором NOT будут отклонены).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
Задает порог компактирования таблицы по умолчанию. Подробнее здесь - [Number of optimized disk chunks](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). Этот параметр может быть переопределен опцией на запрос [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). Также он может динамически изменяться через [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
Этот параметр определяет максимальное количество одновременных постоянных подключений к удалённым [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md). Каждый раз при подключении агента, заданного как `agent_persistent`, происходит попытка повторно использовать существующее соединение (если оно есть), или установить новое и сохранить его для последующего использования. Однако иногда имеет смысл ограничить количество таких постоянных соединений. Эта директива задает этот предел. Она влияет на количество подключений к хосту каждого агента во всех распределенных таблицах.

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
pid_file — обязательный параметр конфигурации в Manticore search, который указывает путь к файлу, где хранится идентификатор процесса сервера `searchd`.

Файл с идентификатором процесса searchd создается заново и блокируется при запуске, в нем хранится PID главного процесса сервера во время работы сервера. При завершении работы сервера файл удаляется.
Назначение этого файла — позволить Manticore выполнять различные внутренние задачи, такие как проверка, запущен ли уже экземпляр `searchd`, остановка `searchd` и уведомление о необходимости перезагрузки таблиц. Файл может также использоваться внешними скриптами автоматизации.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
pid_file = /var/run/manticore/searchd.pid
```
<!-- end -->


### predicted_time_costs

<!-- example conf predicted_time_costs -->
Затраты для модели предсказания времени выполнения запроса в наносекундах. Необязательно, по умолчанию `doc=64, hit=48, skip=2048, match=64`.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
predicted_time_costs = doc=128, hit=96, skip=4096, match=128
```
<!-- end -->

<!-- example conf predicted_time_costs 1 -->
Прекращение выполнения запросов до их завершения на основе времени выполнения (с помощью параметра максимального времени запроса) — хороший механизм безопасности, но у него есть присущий недостаток: недетерминированные (нестабильные) результаты. То есть, если повторить один и тот же (сложный) поисковый запрос с ограничением по времени несколько раз, ограничение времени будет достигаться на разных этапах, и вы получите *разные* наборы результатов.

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

Существует новая опция, [SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time), которая позволяет ограничить время запроса *и* получить стабильные, повторяемые результаты. Вместо регулярной проверки текущего времени во время оценки запроса, что является недетерминированным, она предсказывает текущее время выполнения с помощью простой линейной модели:

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

Запрос затем преждевременно завершается, когда `predicted_time` достигает заданного предела.

Разумеется, это не жесткий предел на фактическое затраченное время (но при этом является жестким пределом на количество выполненной *обрабатывающей* работы), и простая линейная модель вряд ли идеальна по точности. Поэтому реальное время выполнения *может* быть как ниже, так и выше целевого значения. Однако погрешности вполне приемлемы: к примеру, в наших экспериментах с целевым пределом 100 мс большинство тестовых запросов находились в диапазоне от 95 до 105 мс, а *все* запросы — в диапазоне 80–120 мс. Также приятным побочным эффектом является меньшее количество вызовов gettimeofday() благодаря использованию замоделированного времени запроса вместо измерения фактического времени выполнения.

Нет двух серверов одной модели и производителя, поэтому директива `predicted_time_costs` позволяет настроить затраты для описанной выше модели. Для удобства они представлены целыми числами и измеряются в наносекундах. (Предел в max_predicted_time считается в миллисекундах, и указывать значения затрат как 0.000128 мс вместо 128 нс было бы более подвержено ошибкам.) Не нужно обязательно задавать все четыре значения сразу, пропущенные примут значения по умолчанию. Тем не менее, мы настоятельно рекомендуем задавать все для улучшения читаемости.


### preopen_tables

<!-- example conf preopen_tables -->
Директива preopen_tables конфигурации указывает, следует ли принудительно предварительно открывать все таблицы при старте. Значение по умолчанию — 1, что означает, что все таблицы будут предварительно открыты независимо от настройки [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) для каждой конкретной таблицы. Если установлено в 0, тогда в силу войдут настройки для отдельных таблиц, которые по умолчанию будут равны 0.

Предварительное открытие таблиц может предотвратить гонки между поисковыми запросами и ротациями, которые могут иногда приводить к сбоям запросов. Однако это также требует больше файловых дескрипторов. В большинстве сценариев рекомендуется предварительно открывать таблицы.

Пример конфигурации:

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
Опция конфигурации pseudo_sharding включает параллелизацию поисковых запросов к локальным plain и real-time таблицам, вне зависимости от того, запрашиваются ли они напрямую или через распределённую таблицу. Эта возможность автоматически распараллеливает запросы до количества потоков, заданных в параметре `searchd.threads`.

Обратите внимание, что если ваши рабочие потоки уже загружены, поскольку у вас:
* высокая конкуренция запросов
* физическое шардирование любого рода:
  - распределённая таблица из нескольких plain/real-time таблиц
  - real-time таблица, состоящая из слишком большого количества дисковых чанков

тогда включение pseudo_sharding может не принести никаких преимуществ и даже привести к незначительному снижению пропускной способности. Если вы отдаете приоритет большей пропускной способности перед меньшей задержкой, рекомендуется отключить эту опцию.

Включено по умолчанию.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

Директива `replication_connect_timeout` определяет тайм-аут подключения к удаленному узлу. По умолчанию значение считается в миллисекундах, но может иметь [другой суффикс](../Server_settings/Special_suffixes.md). Значение по умолчанию — 1000 (1 секунда).

При подключении к удаленному узлу Manticore будет ждать не более этого времени для успешного завершения подключения. Если тайм-аут достигнут, но соединение не установлено, и `retries` включены, будет выполнена повторная попытка.


### replication_query_timeout

`replication_query_timeout` задает время, в течение которого searchd будет ждать выполнения запроса от удаленного узла. Значение по умолчанию — 3000 миллисекунд (3 секунды), но может иметь `суффикс`, указывающий другую единицу времени.

После установления соединения Manticore будет ждать максимум `replication_query_timeout`, пока удаленный узел выполнит запрос. Обратите внимание, что этот тайм-аут отделен от `replication_connect_timeout`, и общая возможная задержка, вызванная удаленным узлом, будет суммой обеих величин.


### replication_retry_count

Этот параметр — целое число, указывающее, сколько раз Manticore попытается подключиться и выполнить запрос к удаленному узлу во время репликации, прежде чем сообщить о критической ошибке запроса. Значение по умолчанию — 3.


### replication_retry_delay

Этот параметр — целое число в миллисекундах (или [special_suffixes](../Server_settings/Special_suffixes.md)), задающее задержку перед повторной попыткой выполнения запроса к удаленному узлу в случае сбоя во время репликации. Значение актуально только при ненулевом значении. Значение по умолчанию — 500.

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
Эта настройка устанавливает максимальное количество оперативной памяти, выделенной под кешированные наборы результатов в байтах. Значение по умолчанию — 16777216, что эквивалентно 16 мегабайтам. Если значение равно 0, кеш запросов отключается. Для получения дополнительной информации о кеше запросов смотрите раздел [query cache](../Searching/Query_cache.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

Целое число в миллисекундах. Минимальный порог времени выполнения запроса для кеширования результата. По умолчанию 3000, или 3 секунды. Значение 0 означает кешировать все запросы. Смотрите подробности в разделе [query cache](../Searching/Query_cache.md). Это значение также может быть выражено с использованием временных [special_suffixes](../Server_settings/Special_suffixes.md), но используйте это с осторожностью, чтобы не путать себя именем самого параметра, содержащим '_msec'.


### qcache_ttl_sec

Целое число в секундах. Период жизни кешированного набора результатов. По умолчанию — 60, или 1 минута. Минимальное возможное значение — 1 секунда. Смотрите подробности в [query cache](../Searching/Query_cache.md). Это значение также может быть выражено с использованием временных [special_suffixes](../Server_settings/Special_suffixes.md), но используйте это с осторожностью и не путайте себя именем параметра, содержащим '_sec'.


### query_log_format

<!-- example conf query_log_format -->
Формат журнала запросов. Необязательно, допустимые значения — `plain` и `sphinxql`, значение по умолчанию — `sphinxql`.

Режим `sphinxql` записывает валидные SQL-запросы. Режим `plain` записывает запросы в формате простого текста (в основном подходит для чисто полнотекстовых случаев). Эта директива позволяет переключать форматы при запуске сервера поиска. Формат журнала также можно изменить "на лету", используя синтаксис `SET GLOBAL query_log_format=sphinxql`. Подробнее см. в разделе [Query logging](../Logging/Query_logging.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

Лимит (в миллисекундах), который предотвращает запись запроса в журнал запросов. Необязательно, по умолчанию 0 (все запросы записываются в журнал запросов). Эта директива указывает, что в журнал будут записываться только запросы с временем выполнения, превышающим указанный лимит (это значение также может иметь временные [special_suffixes](../Server_settings/Special_suffixes.md), но используйте их осторожно и не путайте себя именем параметра, содержащим `_msec`).

### query_log

<!-- example conf query_log -->
Имя файла журнала запросов. Необязательно, по умолчанию пусто (запросы не логируются). Все поисковые запросы (например, SELECT ..., но не INSERT/REPLACE/UPDATE) будут записаны в этот файл. Формат описан в [Query logging](../Logging/Query_logging.md). В случае формата 'plain' можно использовать 'syslog' как путь к файлу журнала. В этом случае все поисковые запросы будут отправлены в демон syslog с приоритетом `LOG_INFO`, с префиксом '[query]' вместо метки времени. Чтобы использовать опцию syslog, Manticore должен быть собран с флагом `-–with-syslog`.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
Директива query_log_mode позволяет задать другие права доступа для файлов searchd и журнала запросов. По умолчанию эти файлы создаются с правами 600, что означает, что только пользователь, под которым работает сервер, и root могут читать эти файлы.
Эта директива удобна, если вы хотите разрешить другим пользователям читать журналы, например, решениям для мониторинга, запущенным под непользовательскими root аккаунтами.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
Директива read_buffer_docs контролирует размер буфера чтения на ключевое слово для списков документов. Для каждого вхождения ключевого слова в каждом поисковом запросе существуют два связанных буфера чтения: один для списка документов, второй — для списка попаданий. Эта настройка позволяет управлять размером буфера списка документов.

Размер буфера побольше может увеличить использование оперативной памяти на запрос, но при этом потенциально уменьшить время ввода-вывода. Имеет смысл устанавливать большие значения для медленных хранилищ, но для хранилищ с высокими IOPS следует экспериментировать в области малых значений.

Значение по умолчанию — 256K, минимальное — 8K. Вы также можете установить [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) на уровне конкретной таблицы, что переопределит настройки уровня сервера.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
Директива read_buffer_hits задаёт размер буфера чтения на ключевое слово для списков попаданий в поисковых запросах. По умолчанию размер — 256K, минимальное значение — 8K. Для каждого вхождения ключевого слова в поисковом запросе имеется два связанных буфера чтения: один для списка документов и один для списка попаданий. Увеличение размера буфера может повысить использование памяти на запрос, но снизить время ввода-вывода. Для медленных хранилищ большие размеры буфера оправданы, тогда как для хранилищ с высокими IOPS стоит экспериментировать с малыми значениями.

Эта настройка также может быть указана для конкретных таблиц с помощью опции read_buffer_hits в [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits), которая переопределит серверный параметр.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
Размер чтения без подсказок. Необязательно, по умолчанию 32K, минимальное 1K

При выполнении запросов некоторые операции чтения заранее знают, сколько данных нужно прочитать, а некоторые — нет. Особенно, размер списка попаданий заранее не известен. Эта настройка позволяет контролировать, сколько данных читать в таких случаях. Она влияет на время ввода-вывода для списка попаданий, уменьшая его для списков, больших заданного размера, и увеличивая для более мелких. Не влияет на использование оперативной памяти, так как буфер чтения уже выделен. Не должна быть больше, чем read_buffer.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
Уточняет поведение сетевых таймаутов (таких как `network_timeout`, `read_timeout` и `agent_query_timeout`).

При значении 0 таймауты ограничивают максимальное время на отправку всего запроса/команды.
При значении 1 (по умолчанию) таймауты ограничивают максимальное время между сетевой активностью.

При репликации узел может потребовать отправить другому узлу большой файл (например, 100 ГБ). Предположим, сеть передаёт данные со скоростью 1 ГБ/с, пакетами по 4–5 МБ. На передачу всего файла потребуется 100 секунд. Таймаут по умолчанию в 5 секунд позволит передать только 5 ГБ, после чего соединение будет прервано. Увеличение таймаута может быть временным решением, но не масштабируемым (например, следующий файл может быть 150 ГБ, что снова приведёт к сбою). Однако при значении `reset_network_timeout_on_packet` по умолчанию 1 таймаут применяется не к всей передаче, а к отдельным пакетам. Пока передача идёт (и данные получаются в течение таймаута), соединение остаётся живым. Если передача застрянет и таймаут сработает между пакетами, соединение будет прервано.

Обратите внимание, что при настройке распределённой таблицы каждый узел — и мастер, и агенты — должен быть настроен. Для мастера важен параметр `agent_query_timeout`, для агентов — `network_timeout`.

<!-- intro -->

##### Пример:

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
Период проверки сброса чанков оперативной памяти RT-таблиц в секундах (или с использованием [специальных суффиксов](../Server_settings/Special_suffixes.md)). Необязательно, значение по умолчанию — 10 часов.

Активно обновляемые RT-таблицы, полностью помещающиеся в чанках оперативной памяти, всё равно могут приводить к постоянно растущим бинарным логам, что отражается на использовании диска и времени восстановления после сбоя. С этой директивой сервер периодически проверяет возможность сброса, и подходящие чанки памяти сохраняются, что даёт возможность поэтапной очистки бинарных логов. Подробнее см. [Binary logging](../Logging/Binary_logging.md).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
Максимальное число операций ввода-вывода (в секунду), которые поток слияния чанков RT может инициировать. Необязательно, значение по умолчанию — 0 (нет ограничений).

Эта директива позволяет ограничить влияние операций ввода-вывода, возникающих при выполнении операторов `OPTIMIZE`. Гарантируется, что все операции оптимизации RT не превысят указанное ограничение по IOPS. Ограничение rt_merge_iops может уменьшить деградацию производительности поиска, вызванную слиянием.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
Максимальный размер операции ввода-вывода, которую поток слияния чанков RT может начать. Необязательно, значение по умолчанию — 0 (нет ограничений).

Эта директива позволяет уменьшить нагрузку на ввод-вывод от операторов `OPTIMIZE`. Операции ввода-вывода, превышающие этот размер, будут разбиты на две и более меньших операции, каждая из которых будет учитываться отдельно в пределах лимита [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops). Таким образом, гарантируется, что все действия оптимизации не превышают (rt_merge_iops * rt_merge_maxiosize) байт дискового ввода-вывода в секунду.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
Предотвращает зависания `searchd` при вращении таблиц с огромным объёмом данных для предварительного кеширования. Необязательно, значение по умолчанию — 1 (включена бесшовная ротация). В системах Windows бесшовная ротация выключена по умолчанию.

Таблицы могут содержать некоторые данные, которые необходимо предварительно загрузить в RAM. В настоящее время файлы `.spa`, `.spb`, `.spi` и `.spm` полностью предварительно загружаются (они содержат данные атрибутов, данные блоб-атрибутов, таблицу ключевых слов и карту удалённых строк соответственно). Без бесшовного поворота вращение таблицы пытается использовать как можно меньше RAM и работает следующим образом:

1. Новые запросы временно отклоняются (с кодом ошибки "retry");
2. `searchd` ждёт завершения всех текущих запросов;
3. Старая таблица освобождается, и её файлы переименовываются;
4. Новые файлы таблицы переименовываются, и требуемая RAM выделяется;
5. Новые атрибутные и словарные данные таблицы загружаются в RAM;
6. `searchd` возобновляет обработку запросов из новой таблицы.

Однако, если данных атрибутов или словаря много, то этап предварительной загрузки может занять заметное количество времени — до нескольких минут, если загружаются файлы размером 1-5+ ГБ.

При включённом бесшовном повороте, вращение работает следующим образом:

1. Выделяется RAM для нового хранения таблицы;
2. Атрибутные и словарные данные новой таблицы асинхронно загружаются в RAM;
3. При успехе старая таблица освобождается, и файлы обеих таблиц переименовываются;
4. При неудаче новая таблица освобождается;
5. В любой момент запросы обслуживаются либо из старой, либо из новой копии таблицы.

Бесшовный поворот обходится повышенным пиковым использованием памяти во время вращения (поскольку обе копии данных `.spa/.spb/.spi/.spm` — старые и новые — должны находиться в RAM при предварительной загрузке новой копии). Среднее использование памяти остаётся прежним.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

Эта опция включает/выключает использование вторичных индексов для поисковых запросов. Она необязательна, по умолчанию — 1 (включена). Обратите внимание, что для индексации включать её не нужно, так как она всегда активна, если установлена [Manticore Columnar Library](https://github.com/manticoresoftware/columnar). Последняя также необходима для использования индексов при поиске. Доступно три режима:

* `0`: Отключить использование вторичных индексов при поиске. Их можно включить для отдельных запросов с помощью [подсказок анализатора](../Searching/Options.md#Query-optimizer-hints)
* `1`: Включить использование вторичных индексов при поиске. Их можно отключить для отдельных запросов с помощью [подсказок анализатора](../Searching/Options.md#Query-optimizer-hints)
* `force`: То же, что и включено, но любые ошибки при загрузке вторичных индексов будут сообщены, и весь индекс не будет загружен в демон.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
Целое число, служащее идентификатором сервера, используемым как seed для генерации уникального короткого UUID для узлов, входящих в кластер репликации. server_id должен быть уникальным среди узлов кластера и находится в диапазоне от 0 до 127. Если server_id не задан, он вычисляется как хэш MAC-адреса и пути к PID-файлу или будет использовано случайное число как seed для короткого UUID.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
Время ожидания в секундах (или [special_suffixes](../Server_settings/Special_suffixes.md)) для `searchd --stopwait`. Опционально, по умолчанию 60 секунд.

Когда вы запускаете `searchd --stopwait`, серверу необходимо выполнить некоторые действия перед остановкой, такие как завершение запросов, выгрузка RT RAM чанков, выгрузка атрибутов и обновление binlog. Эти задачи требуют времени. `searchd --stopwait` будет ждать до `shutdown_time` секунд, чтобы сервер закончил свои задачи. Подходящее время зависит от размера и нагрузки вашей таблицы.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

SHA1-хэш пароля, необходимого для вызова команды 'shutdown' из VIP Manticore SQL соединения. Без этого [debug](../Reporting_bugs.md#DEBUG) подкоманда 'shutdown' никогда не остановит сервер. Обратите внимание, что такое простое хеширование не следует считать надёжной защитой, так как не используется соль или какая-либо современная криптографическая функция. Это предусмотрено как средство защиты от необдуманных действий демонов обслуживания в локальной сети.

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
Префикс, добавляемый к локальным именам файлов при генерации сниппетов. Опционально, по умолчанию — текущая рабочая папка.

Этот префикс можно использовать при распределённой генерации сниппетов вместе с опциями `load_files` или `load_files_scattered`.

Обратите внимание, что это именно префикс, **а не путь**! Это значит, что если префикс установлен как "server1", а запрос относится к "file23", `searchd` попытается открыть "server1file23" (все без кавычек). Если нужен именно путь, необходимо включить завершающий слэш.

После составления полного пути к файлу сервер разворачивает все относительные директории и сравнивает конечный результат со значением `snippet_file_prefix`. Если результат не начинается с этого префикса, такой файл будет отвергнут с сообщением об ошибке.

Например, если вы установите `/mnt/data`, а кто-то вызовет генерацию сниппета с файлом `../../../etc/passwd` как источник, произойдёт сообщение об ошибке:

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

вместо вывода содержимого файла.

Также при unset параметре и чтении `/etc/passwd` будет фактически прочитан файл `/daemon/working/folder/etc/passwd`, так как значение параметра по умолчанию — рабочая папка сервера.

Также учтите, что это локальная опция; она никак не влияет на агенты. Поэтому можно безопасно устанавливать префикс на мастер-сервере. Запросы, перенаправленные агентам, не будут затронуты настройками мастера. Они будут затронуты собственными настройками агента.

Это может быть полезно, например, когда места хранения документов (локальное хранилище или точки монтирования NAS) не совпадают между серверами.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **ВНИМАНИЕ:** Если вы все еще хотите получить доступ к файлам из корня FS, вам нужно явно установить `snippets_file_prefix` в пустое значение (с помощью строки `snippets_file_prefix=`) или в корень (с помощью `snippets_file_prefix=/`).


### sphinxql_state

<!-- example conf sphinxql_state -->
Путь к файлу, в который будет сериализовано текущее состояние SQL.

При запуске сервера этот файл воспроизводится. При соответствующих изменениях состояния (например, SET GLOBAL) этот файл автоматически перезаписывается. Это может предотвратить трудно диагностируемую проблему: если вы загружаете UDF-функции, но Manticore аварийно завершается, при его (автоматическом) перезапуске ваши UDF и глобальные переменные больше не будут доступны. Использование постоянного состояния помогает обеспечить плавное восстановление без подобных сюрпризов.

`sphinxql_state` не может использоваться для выполнения произвольных команд, таких как `CREATE TABLE`.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
sphinxql_state = uservars.sql
```
<!-- end -->


### sphinxql_timeout

<!-- example conf sphinxql_timeout -->
Максимальное время ожидания между запросами (в секундах или с [специальными суффиксами](../Server_settings/Special_suffixes.md)) при использовании SQL-интерфейса. Необязательно, по умолчанию 15 минут.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
Путь к файлу сертификата удостоверяющего центра (CA) SSL (также известного как корневой сертификат). Необязательно, по умолчанию пусто. Если не пусто, сертификат в `ssl_cert` должен быть подписан этим корневым сертификатом.

Сервер использует файл CA для проверки подписи на сертификате. Файл должен быть в формате PEM.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
Путь к SSL-сертификату сервера. Необязательно, по умолчанию пусто.

Сервер использует этот сертификат в качестве самоподписанного публичного ключа для шифрования HTTP-трафика через SSL. Файл должен быть в формате PEM.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
Путь к ключу SSL-сертификата. Необязательно, по умолчанию пусто.

Сервер использует этот приватный ключ для шифрования HTTP-трафика через SSL. Файл должен быть в формате PEM.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
Максимальный размер кэша общих поддеревьев документов на запрос. Необязательно, по умолчанию 0 (отключено).

Этот параметр ограничивает использование оперативной памяти оптимизатором общих поддеревьев (см. [multi-queries](../Searching/Multi-queries.md)). Максимум столько ОЗУ будет потрачено на кэширование записей документов для каждого запроса. Установка значения 0 отключает оптимизатор.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
Максимальный размер кэша попаданий общих поддеревьев на запрос. Необязательно, по умолчанию 0 (отключено).

Этот параметр ограничивает использование оперативной памяти оптимизатором общих поддеревьев (см. [multi-queries](../Searching/Multi-queries.md)). Максимум столько ОЗУ будет потрачено на кэширование вхождений ключевых слов (попаданий) для каждого запроса. Установка значения 0 отключает оптимизатор.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Количество рабочих потоков (или размер пула потоков) для демона Manticore. Manticore создает такое количество потоков ОС при старте и они выполняют все задачи внутри демона, такие как выполнение запросов, создание сниппетов и т.д. Некоторые операции могут быть разбиты на подзадачи и выполняться параллельно, например:

* Поиск в таблице реального времени
* Поиск в распределенной таблице, состоящей из локальных таблиц
* Вызов перколяционного запроса
* и другие

По умолчанию установлено в количество ядер CPU на сервере. Manticore создает потоки при запуске и сохраняет их до остановки. Каждая подзадача может использовать один из потоков по мере необходимости. Когда подзадача заканчивается, она освобождает поток для использования другой подзадачей.

В случае интенсивной нагрузки типа ввода-вывода может иметь смысл установить значение выше, чем количество ядер CPU.

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
Максимальный размер стека для задачи (корутины, один поисковый запрос может вызвать несколько задач/корутин). Необязательно, по умолчанию 128K.

Каждая задача имеет свой собственный стек размером 128K. При запуске запроса проверяется, сколько стека требуется. Если 128K достаточно, он просто обрабатывается. Если нужно больше, планируется другая задача с увеличенным стеком для продолжения обработки. Максимальный размер такого расширенного стека ограничен этой настройкой.

Установка значения в разумно высокий предел поможет обрабатывать очень глубокие запросы без значительного увеличения общего потребления ОЗУ. Например, установка в 1G не означает, что каждая новая задача будет занимать 1G ОЗУ, но если потребуется, допустим, 100M, мы просто выделим 100M для задачи. Другие задачи в это время будут работать с дефолтным стеком в 128K. Точно так же можно обрабатывать еще более сложные запросы, которым требуется 500M. И только если мы **увидим** внутренне, что задача требует более 1G стека, обработка завершится с ошибкой и сообщением о слишком маленьком thread_stack.

Однако на практике даже запрос, требующий 16M стека, часто слишком сложен для разбора и отнимает слишком много времени и ресурсов для обработки. Поэтому демон будет обрабатывать его, но ограничение таких запросов с помощью параметра `thread_stack` выглядит вполне разумным.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
Определяет, следует ли удалять копии таблиц с расширением `.old` при успешном ротировании. Необязательно, по умолчанию 1 (удалять).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
Многопоточный сторож сервера. Необязательно, по умолчанию 1 (сторож включен).

Когда запрос Manticore завершается с сбоем, это может привести к сбою всего сервера. При включенной функции watchdog, `searchd` также поддерживает отдельный легковесный процесс, который мониторит основной процесс сервера и автоматически перезапускает его в случае ненормального завершения. Watchdog включен по умолчанию.

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

