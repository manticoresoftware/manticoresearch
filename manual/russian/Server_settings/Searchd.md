# Section "Searchd" in configuration

Ниже приведены настройки, которые используются в разделе `searchd` файла конфигурации Manticore Search для управления поведением сервера. Ниже представлен краткий обзор каждой настройки:

### access_plain_attrs

Эта настройка задает дефолтные значения для экземпляра для [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, со значением по умолчанию `mmap_preread`.

Директива `access_plain_attrs` позволяет определить значение по умолчанию для [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы на уровне таблиц имеют более высокий приоритет и переопределяют это значение по умолчанию для экземпляра, обеспечивая более тонкое управление.

### access_blob_attrs

Эта настройка задает дефолтные значения для экземпляра для [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, со значением по умолчанию `mmap_preread`.

Директива `access_blob_attrs` позволяет определить значение по умолчанию для [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы на уровне таблиц имеют более высокий приоритет и переопределяют это значение по умолчанию для экземпляра, обеспечивая более тонкое управление.

### access_doclists

Эта настройка задает дефолтные значения для экземпляра для [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, со значением по умолчанию `file`.

Директива `access_doclists` позволяет определить значение по умолчанию для [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы на уровне таблиц имеют более высокий приоритет и переопределяют это значение по умолчанию для экземпляра, обеспечивая более тонкое управление.

### access_hitlists

Эта настройка задает дефолтные значения для экземпляра для [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, со значением по умолчанию `file`.

Директива `access_hitlists` позволяет определить значение по умолчанию для [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы на уровне таблиц имеют более высокий приоритет и переопределяют это значение по умолчанию для экземпляра, обеспечивая более тонкое управление.

### access_dict

Эта настройка задает дефолтные значения для экземпляра для [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Она является необязательной, со значением по умолчанию `mmap_preread`.

Директива `access_dict` позволяет определить значение по умолчанию для [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) для всех таблиц, управляемых этим экземпляром searchd. Директивы на уровне таблиц имеют более высокий приоритет и переопределяют это значение по умолчанию для экземпляра, обеспечивая более тонкое управление.

### agent_connect_timeout

Эта настройка задает дефолтные значения для экземпляра для параметра [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout).


### agent_query_timeout

Эта настройка задает дефолтные значения для экземпляра для параметра [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout). Она может быть переопределена для каждого запроса с помощью конструкции `OPTION agent_query_timeout=XXX`.


### agent_retry_count

Эта настройка — целое число, которое указывает, сколько раз Manticore будет пытаться подключиться и выполнить запрос к удаленным агентам через распределённую таблицу до сообщения о фатальной ошибке запроса. Значение по умолчанию — 0 (то есть без повторных попыток). Вы также можете задать это значение для каждого запроса отдельно с помощью конструкции `OPTION retry_count=XXX`. Если задана опция на уровне запроса, она переопределит значение, указанное в конфигурации.

Обратите внимание, что если в определении вашей распределённой таблицы используются [агентские зеркала](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors), сервер будет выбирать другое зеркало для каждой попытки подключения согласно выбранной [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy). В этом случае параметр `agent_retry_count` будет суммироваться для всех зеркал в наборе.

Например, если у вас 10 зеркал и установлено `agent_retry_count=5`, сервер выполнит до 50 попыток, при условии в среднем 5 попыток для каждого из 10 зеркал (с опцией `ha_strategy = roundrobin` это будет так).

Однако значение, указанное в опции `retry_count` для [агента](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent), служит абсолютным лимитом. Другими словами, опция `[retry_count=2]` в определении агента всегда означает максимум 2 попытки, независимо от того, указано ли у вас 1 или 10 зеркал для агента.

### agent_retry_delay

Эта настройка — целое число в миллисекундах (или [специальных суффиксах](../Server_settings/Special_suffixes.md)), которое указывает задержку перед повторной попыткой запроса к удаленному агенту в случае ошибки. Это значение имеет значение только тогда, когда задан ненулевой [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) или ненулевое значение `retry_count` для отдельного запроса. Значение по умолчанию — 500. Вы также можете указать это значение для каждого запроса отдельно с помощью конструкции `OPTION retry_delay=XXX`. Если задана опция на уровне запроса, она переопределит значение, указанное в конфигурации.


### attr_flush_period

<!-- example conf attr_flush_period -->
When using [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) to modify document attributes in real-time, the changes are first written to an in-memory copy of the attributes. These updates occur in a memory-mapped file, meaning the OS decides when to write the changes to disk. Upon normal shutdown of `searchd` (triggered by a `SIGTERM` signal), all changes are forced to be written to disk.

You can also instruct `searchd` to periodically write these changes back to disk to prevent data loss. The interval between these flushes is determined by `attr_flush_period`, specified in seconds (or [special_suffixes](../Server_settings/Special_suffixes.md)).

By default, the value is 0, which disables periodic flushing. However, flushing will still occur during a normal shutdown.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
This setting controls the automatic [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) process for table compaction.

By default table compaction occurs automatically. You can modify this behavior with the `auto_optimize` setting:
* 0 to disable automatic table compaction (you can still call `OPTIMIZE` manually)
* 1 to explicitly enable it
* to enable it while multiplying the optimization threshold by 2.

By default, OPTIMIZE runs until the number of disk chunks is less than or equal to the number of logical CPU cores multiplied by 2.

However, if the table has attributes with KNN indexes, this threshold is different. In this case, it is set to the number of physical CPU cores divided by 2 to improve KNN search performance.

Note that toggling `auto_optimize` on or off doesn't prevent you from running [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) manually.

<!-- intro -->
##### Example:

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
Manticore supports the automatic creation of tables that don't yet exist but are specified in INSERT statements. This feature is enabled by default. To disable it, set `auto_schema = 0` explicitly in your configuration. To re-enable it, set `auto_schema = 1` or remove the `auto_schema` setting from the configuration.

Keep in mind that the `/bulk` HTTP endpoint does not support automatic table creation.

> NOTE: The [auto schema functionality](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

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
This setting controls the binary log transaction flush/sync mode. It is optional, with a default value of 2 (flush every transaction, sync every second).

The directive determines how frequently the binary log will be flushed to the OS and synced to disk. There are three supported modes:

*  0, flush and sync every second. This offers the best performance, but up to 1 second worth of committed transactions can be lost in the event of a server crash or an OS/hardware crash.
*  1, flush and sync every transaction. This mode provides the worst performance but guarantees that every committed transaction's data is saved.
*  2, flush every transaction, sync every second. This mode delivers good performance and ensures that every committed transaction is saved in case of a server crash. However, in the event of an OS/hardware crash, up to 1 second worth of committed transactions can be lost.

For those familiar with MySQL and InnoDB, this directive is similar to `innodb_flush_log_at_trx_commit`. In most cases, the default hybrid mode 2 provides a nice balance of speed and safety, with full RT table data protection against server crashes and some protection against hardware ones.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
This setting controls how binary log files are managed. It is optional, with a default value of 0 (separate file for each table).

You can choose between two ways to manage binary log files:

* Separate file for each table (default, `0`): Each table saves its changes in its own log file. This setup is good if you have many tables that get updated at different times. It allows tables to be updated without waiting for others. Also, if there is a problem with one table's log file, it does not affect the others.
* Single file for all tables (`1`): All tables use the same binary log file. This method makes it easier to handle files because there are fewer of them. However, this could keep files longer than needed if one table still needs to save its updates. This setting might also slow things down if many tables need to update at the same time because all changes have to wait to be written to one file.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
This setting controls the maximum binary log file size. It is optional, with a default value of 256 MB.

A new binlog file will be forcibly opened once the current binlog file reaches this size limit. This results in a finer granularity of logs and can lead to more efficient binlog disk usage under certain borderline workloads. A value of 0 indicates that the binlog file should not be reopened based on size.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
This setting determines the path for binary log (also known as transaction log) files. It is optional, with a default value of the build-time configured data directory (e.g., `/var/lib/manticore/data/binlog.*` in Linux).

Двоичные журналы используются для восстановления данных таблицы RT после сбоев и для обновления атрибутов обычных дисковых индексов, которые в противном случае хранились бы только в ОЗУ до слива. Когда ведение журналов включено, каждая транзакция, выполненная COMMIT в таблицу RT, записывается в файл журнала. После некорректного завершения работы логи автоматически проигрываются при запуске, восстанавливая записанные изменения.

Директива `binlog_path` задаёт расположение файлов двоичных логов. Она должна содержать только путь; `searchd` будет создавать и удалять несколько файлов `binlog.*` в этой директории по мере необходимости (включая данные бинарных логов, метаданные, файлы блокировок и т.д.).

Пустое значение отключает ведение двоичных логов, что улучшает производительность, но подвергает данные RT таблицы риску.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
binlog_path = # disable logging
binlog_path = /var/lib/manticore/data # /var/lib/manticore/data/binlog.001 etc will be created
```
<!-- end -->

### boolean_simplify

<!-- example conf boolean_simplify -->
Этот параметр управляет значением по умолчанию для опции поиска [boolean_simplify](../Searching/Options.md#boolean_simplify). Он необязателен, значение по умолчанию — 1 (включено).

При установке в 1 сервер автоматически применяет [оптимизацию булевых запросов](../Searching/Full_text_matching/Boolean_optimization.md) для улучшения производительности запросов. При значении 0 запросы выполняются без оптимизации по умолчанию. Это значение по умолчанию можно переопределить для каждого запроса отдельно с помощью соответствующей опции поиска `boolean_simplify`.

<!-- request Example -->
```ini
searchd {
    boolean_simplify = 0  # disable boolean optimization by default
}
```
<!-- end -->

### buddy_path

<!-- example conf buddy_path -->
Этот параметр определяет путь к бинарному файлу Manticore Buddy. Он необязателен, по умолчанию устанавливается путь, заданный при сборке, который варьируется для разных операционных систем. Обычно его не нужно изменять. Однако он может быть полезен, если вы хотите запустить Manticore Buddy в режиме отладки, внести изменения в Manticore Buddy или реализовать новый плагин. В последнем случае можно сделать `git clone` Buddy с https://github.com/manticoresoftware/manticoresearch-buddy, добавить новый плагин в каталог `./plugins/` и запустить `composer install --prefer-source` для удобства разработки, изменив каталог на исходники Buddy.

Чтобы иметь возможность запускать `composer`, на вашем компьютере должен быть установлен PHP 8.2 или выше с следующими расширениями:

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

Также можно выбрать специальную версию `manticore-executor-dev` для Linux amd64 из релизов, например: https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

Если выбрать этот путь, не забудьте связать dev-версию исполнителя manticore с `/usr/bin/php`.

Для отключения Manticore Buddy установите значение пустым, как показано в примере.

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
Этот параметр задаёт максимальное время ожидания между запросами (в секундах или [special_suffixes](../Server_settings/Special_suffixes.md)) при использовании постоянных соединений. Необязателен, значение по умолчанию — пять минут.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
Локаль libc сервера. Необязательна, по умолчанию C.

Задаёт локаль libc, влияющую на основанные на libc сортировки (collations). Подробнее см. раздел [collations](../Searching/Collations.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
Коллация сервера по умолчанию. Необязательна, по умолчанию libc_ci.

Определяет коллацию, используемую по умолчанию для входящих запросов. Коллация может быть переопределена для каждого запроса отдельно. Подробнее о доступных коллациях и других деталях смотрите в разделе [collations](../Searching/Collations.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
При указании данного параметра включается [режим реального времени](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29), импперативный способ управления схемой данных. Значением должен быть путь к директории, в которой вы хотите хранить все свои таблицы, двоичные логи и всё прочее, необходимое для корректной работы Manticore Search в этом режиме.
Индексация [обычных таблиц](../Creating_a_table/Local_tables/Plain_table.md) запрещена при указании `data_dir`. Подробнее о различиях между режимом RT и обычным режимом читайте в [этом разделе](../Read_this_first.md#Real-time-table-vs-plain-table).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
Таймаут для предотвращения автофлеша RAM-чанка, если в таблице нет поисковых запросов. Необязателен, значение по умолчанию — 30 секунд.

Время проверки наличия запросов перед решением о необходимости автофлеша.
Автофлеш будет выполнен только если в таблице был хотя бы один поиск в течение последних `diskchunk_flush_search_timeout` секунд. Работает в связке с [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout). Соответствующая [параметр на уровне таблицы](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout) имеет более высокий приоритет и перекроет это значение по умолчанию для всего инстанса, обеспечивая более тонкий контроль.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
Время ожидания без записи в секундах перед автофлешем RAM-чанка на диск. Необязателен, значение по умолчанию — 1 секунда.

Если в RAM-чанке не происходит запись в течение `diskchunk_flush_write_timeout` секунд, чанка будет сброшена на диск. Работает в связке с [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout). Чтобы отключить авто-сброс, установите `diskchunk_flush_write_timeout = -1` явно в вашей конфигурации. Соответствующая [настройка для конкретной таблицы](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout) имеет более высокий приоритет и перекроет этот глобальный по экземпляру параметр, обеспечивая более тонкий контроль.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
Этот параметр задает максимальный размер блоков документов из хранилища документов, которые удерживаются в памяти. Необязателен, значение по умолчанию — 16m (16 мегабайт).

Когда используется `stored_fields`, блоки документов читаются с диска и распаковываются. Поскольку каждый блок обычно содержит несколько документов, он может быть переиспользован при обработке следующего документа. Для этой цели блок хранится в кэше сервера. Кэш содержит распакованные блоки.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
Движок хранения атрибутов по умолчанию, используемый при создании таблиц в режиме RT. Может быть `rowwise` (по умолчанию) или `columnar`.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
Этот параметр определяет максимальное количество расширенных ключевых слов для одного подстановочного знака. Необязателен, значение по умолчанию — 0 (без ограничений).

При выполнении поиска подстроки в таблицах с включенным `dict = keywords` один подстановочный знак может привести к тысячам или даже миллионам совпадающих ключевых слов (подумайте о сопоставлении `a*` с целым Оксфордским словарем). Эта директива позволяет ограничить влияние таких расширений. Установка `expansion_limit = N` ограничит расширения не более чем N наиболее частотными совпадающими ключевыми словами (для каждого подстановочного знака в запросе).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
Этот параметр определяет максимальное число документов в расширенном ключевом слове, которое позволяет объединять все такие ключевые слова вместе. Необязателен, значение по умолчанию — 32.

При выполнении поиска подстроки в таблицах с включенным `dict = keywords` один подстановочный знак может привести к тысячам или миллионам совпадающих ключевых слов. Эта директива позволяет увеличить лимит количества ключевых слов для объединения с целью ускорения сопоставления, но при этом использует больше памяти при поиске.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
Этот параметр определяет максимальное количество попаданий в расширенном ключевом слове, позволяющее объединять все такие ключевые слова вместе. Необязателен, значение по умолчанию — 256.

При выполнении поиска подстроки в таблицах с включенным `dict = keywords` один подстановочный знак может привести к тысячам или миллионам совпадающих ключевых слов. Эта директива позволяет увеличить лимит количества ключевых слов для объединения с целью ускорения сопоставления, но при этом использует больше памяти при поиске.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
Этот параметр контролирует максимальное количество альтернативных вариантов фразы, генерируемых из-за операторов `OR` внутри операторов `PHRASE`, `PROXIMITY` и `QUORUM`. Необязателен, значение по умолчанию — 1024.

При использовании оператора `|` (OR) внутри оператора, похожего на фразу, общее количество расширенных сочетаний может расти экспоненциально в зависимости от количества альтернатив. Этот параметр помогает предотвратить чрезмерное расширение запроса, ограничивая количество перестановок, учитываемых в процессе обработки запроса. Если количество сгенерированных вариантов превысит этот предел, запрос завершится с ошибкой.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
expansion_phrase_limit = 4096
```
<!-- end -->

### grouping_in_utc

Этот параметр указывает, будет ли группировка по времени в API и SQL рассчитываться в местном часовом поясе или в UTC. Необязателен, значение по умолчанию — 0 (означает «местный часовой пояс»).

По умолчанию все выражения 'group by time' (например, group by day, week, month и year в API, также group by day, month, year, yearmonth, yearmonthday в SQL) выполняются с использованием местного времени. Например, если у вас есть документы с атрибутами во времени `13:00 utc` и `15:00 utc`, то в случае группировки они оба попадут в группы в соответствии с вашим локальным часовым поясом. Если вы живете в `utc`, это будет один день, но если вы живете в `utc+10`, эти документы попадут в разные группы `group by day` (поскольку 13:00 utc в часовом поясе UTC+10 — это 23:00 местного времени, а 15:00 — 01:00 следующего дня). Иногда такое поведение неприемлемо, и желательно, чтобы группировка по времени не зависела от часового пояса. Вы можете запустить сервер с определенной глобальной переменной окружения TZ, но это повлияет не только на группировку, но и на штамп времени в логах, что может быть нежелательно. Включение этой опции (либо в конфигурации, либо с помощью оператора [SET global](../Server_settings/Setting_variables_online.md#SET) в SQL) заставит все выражения группировки по времени вычисляться в UTC, оставляя остальные функции, связанные со временем (например, логирование сервера), в местном часовом поясе.


### timezone

Этот параметр задает часовой пояс, используемый функциями, связанными с датой/временем. По умолчанию используется местный часовой пояс, но вы можете указать другой часовой пояс в формате IANA (например, `Europe/Amsterdam`).

Обратите внимание, что этот параметр не влияет на логирование, которое всегда происходит в местном часовом поясе.

Также обратите внимание, что если используется `grouping_in_utc`, функция 'group by time' по-прежнему будет использовать UTC, в то время как другие функции, связанные с датой/временем, будут использовать указанную временную зону. В целом не рекомендуется смешивать `grouping_in_utc` и `timezone`.

Вы можете настроить эту опцию либо в конфиге, либо с помощью оператора [SET global](../Server_settings/Setting_variables_online.md#SET) в SQL.


### ha_period_karma

<!-- example conf ha_period_karma -->
Этот параметр задаёт размер окна статистики зеркал агента в секундах (или [special_suffixes](../Server_settings/Special_suffixes.md)). Он является опциональным, значение по умолчанию — 60 секунд.

Для распределённой таблицы с зеркалами агента (подробнее в [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)) мастер отслеживает несколько разных счётчиков для каждого зеркала. Эти счётчики используются для переключения отказов и балансировки (мастер выбирает лучшее зеркало для использования на основе счётчиков). Счётчики накапливаются блоками по `ha_period_karma` секунд.

После начала нового блока мастер может ещё использовать накопленные значения из предыдущего, пока новый блок не заполнится наполовину. В результате, любые предыдущие данные перестают влиять на выбор зеркала максимум через 1.5 раза от ha_period_karma секунд.

Хотя для выбора зеркала используется максимум два блока, до 15 последних блоков сохраняются для целей инструментирования. Эти блоки можно просмотреть с помощью команды [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
Этот параметр настраивает интервал между пингами зеркал агента в миллисекундах (или [special_suffixes](../Server_settings/Special_suffixes.md)). Опционально, значение по умолчанию — 1000 миллисекунд.

Для распределённой таблицы с зеркалами агента (подробнее в [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)) мастер отправляет всем зеркалам команду пинга в периоды простоя. Это необходимо для отслеживания текущего статуса агента (жив или мёртв, сетевой обмен, и т.д.). Интервал между такими пингами задаётся этой директивой. Чтобы отключить пинги, установите ha_ping_interval в 0.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

Опция `hostname_lookup` определяет стратегию обновления имён хостов. По умолчанию IP-адреса имён агентов кэшируются при запуске сервера, чтобы избежать избыточных запросов к DNS. Однако в некоторых случаях IP может меняться динамически (например, облачный хостинг), и может быть желательно не кэшировать IP. Установка этой опции в `request` отключает кэширование и запрашивает DNS при каждом запросе. IP-адреса также можно обновить вручную с помощью команды `FLUSH HOSTNAMES`.

### jobs_queue_size

Параметр jobs_queue_size определяет, сколько "заданий" может быть в очереди одновременно. По умолчанию он неограничен.

В большинстве случаев "задание" означает один запрос к одной локальной таблице (обычной таблице или дисковому чанку реального времени). Например, если у вас есть распределённая таблица, состоящая из 2 локальных таблиц или реальная таблица с 2 дисковыми чанками, поисковый запрос к любой из них создаст в очереди примерно 2 задания. Затем их обрабатывает пул потоков (размер которого задаётся параметром [threads](../Server_settings/Searchd.md#threads)). Однако в некоторых случаях, если запрос слишком сложный, может создаваться больше заданий. Изменение этого параметра рекомендуется, когда [max_connections](../Server_settings/Searchd.md#max_connections) и [threads](../Server_settings/Searchd.md#threads) недостаточны для достижения баланса между желаемой производительностью.

### join_batch_size

Объединения таблиц работают путём накопления партии совпадений — результатов запроса, выполненного по левой таблице. Эта партия затем обрабатывается как единый запрос по правой таблице.

Этот параметр позволяет регулировать размер партии. Значение по умолчанию — `1000`, установка в `0` отключает группировку.

Больше размер партии может повысить производительность; однако для некоторых запросов это может привести к чрезмерному потреблению памяти.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

Каждый запрос по правой таблице определяется конкретными условиями JOIN ON, которые определяют набор результатов, извлекаемых из правой таблицы.

Если количество уникальных условий JOIN ON невелико, повторное использование результатов может быть эффективнее, чем постоянное выполнение запросов по правой таблице. Для этого наборы результатов сохраняются в кеше.

Этот параметр позволяет задать размер этого кеша. Значение по умолчанию — `20 MB`, значение 0 отключает кеширование.

Обратите внимание, что каждый поток сохраняет собственный кеш, поэтому при оценке общей памяти учитывайте количество потоков, выполняющих запросы.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
Параметр listen_backlog определяет длину очереди TCP для входящих соединений. Это особенно важно для Windows-сборок, которые обрабатывают запросы последовательно. Когда очередь соединений достигает своего лимита, новые входящие соединения будут отклоняться.
Для сборок не Windows значение по умолчанию обычно подходит, и обычно нет необходимости регулировать эту настройку.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
Строка версии сервера, возвращаемая Kibana или OpenSearch Dashboards. Опционально — по умолчанию установлена в `7.6.0`.

Некоторые версии Kibana и OpenSearch Dashboards ожидают, что сервер будет сообщать конкретный номер версии и могут вести себя по-разному в зависимости от него. Чтобы обойти такие проблемы, вы можете использовать эту настройку, благодаря которой Manticore будет сообщать Kibana или OpenSearch Dashboards пользовательскую версию.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
Эта настройка позволяет указать IP-адрес и порт или путь к Unix-доменному сокету, на которых Manticore будет принимать подключения.

Общий синтаксис для `listen`:

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

Вы можете указать:
* либо IP-адрес (или имя хоста) и номер порта
* либо только номер порта
* либо путь к Unix-сокету (не поддерживается в Windows)
* либо IP-адрес и диапазон портов

Если вы указываете номер порта, но не адрес, `searchd` будет слушать все сетевые интерфейсы. Путь Unix-сокета определяется ведущим слэшем. Диапазон портов может быть установлен только для протокола репликации.

Вы также можете указать обработчик протокола (listener), который будет использоваться для подключений на этом сокете. Слушатели:

* **Не указан** - Manticore будет принимать подключения на этом порту от:
  - других агентов Manticore (то есть удалённая распределённая таблица)
  - клиентов через HTTP и HTTPS
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/). **Убедитесь, что у вас есть слушатель этого типа (или слушатель `http`, как описано ниже), чтобы избежать ограничений в функциональности Manticore.**
* `mysql` протокол MySQL для подключений от клиентов MySQL. Обратите внимание:
  - также поддерживается сжатый протокол.
  - Если включён [SSL](../Security/SSL.md#SSL), вы можете установить зашифрованное соединение.
* `replication` - протокол репликации, используемый для общения узлов. Подробнее см. в разделе [репликация](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md). Можно указать несколько слушателей репликации, но они должны слушать на одном IP; различаться могут только порты. Когда вы определяете слушатель репликации с диапазоном портов (например, `listen = 192.168.0.1:9320-9328:replication`), Manticore не начинает сразу слушать эти порты. Вместо этого он возьмёт случайные свободные порты из указанного диапазона только при начале использования репликации. Для корректной работы репликации требуется не менее 2 портов в диапазоне.
* `http` - то же, что и **Не указан**. Manticore будет принимать подключения на этом порту от удалённых агентов и клиентов через HTTP и HTTPS.
* `https` - протокол HTTPS. Manticore будет принимать **только** HTTPS-соединения на этом порту. Подробнее в разделе [SSL](../Security/SSL.md).
* `sphinx` - устаревший двоичный протокол. Используется для обслуживаний подключений от удалённых клиентов [SphinxSE](../Extensions/SphinxSE.md). Некоторые реализации клиентских библиотек Sphinx API (например, Java-клиент) требуют явного указания слушателя.

Добавление суффикса `_vip` к клиентским протоколам (то есть ко всем, кроме `replication`, например `mysql_vip` или `http_vip` или просто `_vip`) заставляет создать выделенный поток для соединения, чтобы обойти различные ограничения. Это полезно для обслуживания узлов при серьёзных перегрузках, когда сервер иначе либо зависнет, либо не позволит подключиться через обычный порт.

Суффикс `_readonly` устанавливает для слушателя [режим только для чтения](../Security/Read_only.md) и ограничивает его приём только запросами на чтение.

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

Можно указывать несколько директив `listen`. `searchd` будет слушать подключения клиентов на всех указанных портах и сокетах. Конфигурация по умолчанию, поставляемая с пакетами Manticore, задаёт прослушивание на портах:
* `9308` и `9312` для подключений от удалённых агентов и клиентов, не основанных на MySQL
* и на порту `9306` для подключений MySQL.

Если в конфигурации не указан ни один `listen`, Manticore будет ждать подключения на:
* `127.0.0.1:9306` для клиентов MySQL
* `127.0.0.1:9312`  для HTTP/HTTPS и подключений от других узлов Manticore и клиентов, использующих двоичный API Manticore.

#### Прослушивание привилегированных портов

По умолчанию Linux не позволит Manticore слушать порт ниже 1024 (например, `listen = 127.0.0.1:80:http` или `listen = 127.0.0.1:443:https`), если вы не запускаете searchd от имени root. Если вы хотите, чтобы Manticore слушал порты < 1024 под не-root пользователем, рассмотрите один из следующих вариантов (любой из них должен сработать):
* Выполните команду `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* Добавьте `AmbientCapabilities=CAP_NET_BIND_SERVICE` в systemd-юнит Manticore и перезагрузите демон (`systemctl daemon-reload`).

#### Технические детали протокола Sphinx API и TFO
<details>
Устаревший протокол Sphinx имеет 2 этапа: обмен рукопожатием (handshake) и основной поток данных. Рукопожатие состоит из 4-байтового пакета от клиента и 4-байтового пакета от демона, с одной целью — клиент удостоверяется, что на другой стороне настоящий демон Sphinx, а демон удостоверяется, что клиент настоящий. Основной поток данных достаточно прост: обе стороны обмениваются рукопожатиями и проверяют их. Такой обмен короткими пакетами требует использования специального флага `TCP_NODELAY`, который отключает алгоритм Нейгла и задаёт TCP-соединение как диалог из небольших пакетов.
Однако в протоколе не строго определено, кто первый начинает этот обмен. Исторически все клиенты, использующие бинарное API, начинают первыми: отправляют рукопожатие, затем читают 4 байта от демона, потом отправляют запрос и читают ответ.
При улучшении совместимости с протоколом Sphinx мы учли следующие моменты:

1. Обычно связь мастер-агент устанавливается от известного клиента к известному хосту на известном порту. Поэтому маловероятно, что на стороне будет неверное рукопожатие. Мы можем неявно предположить, что обе стороны валидны и действительно общаются по протоколу Sphinx.
2. Исходя из этого, мы можем «приклеить» рукопожатие к реальному запросу и отправить одним пакетом. Если на бэкенде — устаревший демон Sphinx, он просто прочитает этот слитый пакет как 4 байта рукопожатия, затем тело запроса. Поскольку оба пришли в одном пакете, на стороне бэкенда сокет экономит 1 RTT, а на стороне клиента буфер всё равно работает в обычном режиме.
3. Продолжая предположение: поскольку пакет 'query' довольно маленький, а handshake еще меньше, давайте отправим оба в начальном TCP пакете 'SYN' с использованием современной техники TFO (tcp-fast-open). То есть: мы подключаемся к удаленному узлу с "склеенным" handshake + телом пакета. Демон принимает соединение и сразу получает и handshake, и тело в буфере сокета, так как они пришли в самом первом TCP 'SYN' пакете. Это устраняет еще один RTT.
4. Наконец, нужно научить демон принимать это улучшение. На самом деле, со стороны приложения это означает НЕ использовать `TCP_NODELAY`. А со стороны системы это означает, что на стороне демона должен быть активирован прием TFO, а на стороне клиента — отправка TFO. По умолчанию в современных системах клиентский TFO уже активирован, поэтому нужно только настроить серверный TFO, чтобы всё заработало.

Все эти улучшения, без фактического изменения самого протокола, позволили нам устранить 1.5 RTT TCP протокола при соединении. Что, если запрос и ответ могут поместиться в один TCP пакет, уменьшает сессию бинарного API с 3.5 RTT до 2 RTT — что делает сетевой обмен примерно в 2 раза быстрее.

Таким образом, все наши улучшения основаны на изначально неопределенном утверждении: "кто говорит первым". Если клиент говорит первым, мы можем применить все эти оптимизации и эффективно обработать подключение + handshake + запрос в одном TFO пакете. Более того, мы можем взглянуть на начало входящего пакета и определить реальный протокол. Именно поэтому можно подключаться к одному и тому же порту через API/http/https. Если же демон должен говорить первым, все эти оптимизации невозможны, как и мультипротокол. Вот почему у нас есть отдельный порт для MySQL и мы не объединили его с другими протоколами в один порт. Вдруг среди всех клиентов оказался один, написанный с идеей, что демон должен сначала отправить handshake. Это — отсутствие возможностей для всех описанных улучшений. Это плагин SphinxSE для mysql/mariadb. Поэтому специально для этого единственного клиента мы выделили определение протокола `sphinx` для работы в самых наследуемых условиях. А именно: обе стороны активируют `TCP_NODELAY` и обмениваются маленькими пакетами. Демон отправляет handshake при подключении, затем клиент отправляет свой, и потом всё работает обычным способом. Это не очень оптимально, но просто работает. Если вы используете SphinxSE для подключения к Manticore — вам нужно выделить слушатель с явно указанным протоколом `sphinx`. Для других клиентов — избегайте использования этого слушателя, так как он медленнее. Если вы используете другие устаревшие клиенты Sphinx API — сначала проверьте, смогут ли они работать с не выделенным мультипротокольным портом. Для связи мастер-агент через не выделенный (мультипротокольный) порт и с включенным клиентским и серверным TFO работает хорошо и определенно сделает работу сетевой подсистемы быстрее, особенно если у вас очень легкие и быстрые запросы.
</details>

### listen_tfo

Этот параметр включает флаг TCP_FASTOPEN для всех слушателей. По умолчанию им управляет система, но его можно явно отключить, установив значение в '0'.

Для общего понимания расширения TCP Fast Open, пожалуйста, обратитесь к [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open). Кратко говоря, это позволяет исключить один TCP раунд-трип при установлении соединения.

На практике использование TFO во многих случаях может оптимизировать сетевую эффективность клиент-агент, как если бы были [постоянные агенты](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md), но без удержания активных соединений, а также без ограничения по максимальному количеству соединений.

В современных ОС поддержка TFO обычно включена на системном уровне, но это всего лишь 'возможность', а не правило. Linux (как самый прогрессивный) поддерживает ее с 2011 года, начиная с ядер 3.7 (со стороны сервера). Windows поддерживает ее начиная с определенных сборок Windows 10. Другие операционные системы (FreeBSD, MacOS) тоже в игре.

Для Linux система проверяет переменную `/proc/sys/net/ipv4/tcp_fastopen` и ведет себя согласно ей. Бит 0 управляет клиентской стороной, бит 1 — слушателями. По умолчанию параметр установлен в 1, то есть клиенты включены, слушатели отключены.

### log

<!-- example conf log -->
Параметр log задает имя файла журнала, в который будут записаны все события выполнения `searchd`. Если не указано, имя по умолчанию 'searchd.log'.

В качестве альтернативы можно использовать имя файла 'syslog'. В этом случае события будут отправляться в демон syslog. Для использования опции syslog необходимо собрать Manticore с опцией `-–with-syslog`.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
Ограничивает количество запросов в пакете. Опционально, по умолчанию 32.

Заставляет searchd выполнять проверку здравомыслия по количеству запросов, отправленных в одном пакете при использовании [мультизапросов](../Searching/Multi-queries.md). Установите в 0, чтобы пропустить проверку.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
Максимальное количество одновременных клиентских соединений. По умолчанию не ограничено. Обычно заметно при использовании каких-либо постоянных соединений, например CLI сессий mysql или постоянных удаленных соединений из удаленных распределенных таблиц. Если лимит достигнут, вы все равно можете подключиться к серверу используя [VIP соединение](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection). VIP соединения не учитываются в лимите.

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
Глобальное ограничение количества потоков, которые может использовать одна операция. По умолчанию соответствующие операции могут занимать все ядра процессора, не оставляя места для других операций. Например, `call pq` к достаточно большой таблице percolate может использовать все потоки в течение нескольких десятков секунд. Установка `max_threads_per_query` в, скажем, половину от значения [threads](../Server_settings/Searchd.md#threads) обеспечит возможность параллельного выполнения нескольких таких операций `call pq`.

Также можно установить этот параметр как переменную сессии или глобальную переменную во время работы.

Кроме того, поведение можно контролировать для каждого запроса отдельно с помощью [опции threads](../Searching/Options.md#threads).

<!-- intro -->
##### Пример:
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
Максимально допустимое количество фильтров на запрос. Этот параметр используется только для внутренних проверок и не влияет напрямую на использование оперативной памяти или производительность. Необязательный, по умолчанию 256.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
Максимально допустимое количество значений для одного фильтра. Этот параметр используется только для внутренних проверок и не влияет напрямую на использование оперативной памяти или производительность. Необязательный, по умолчанию 4096.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
Максимальное количество файлов, которые серверу разрешено открывать, называется «мягким лимитом». Обратите внимание, что обслуживание больших фрагментированных таблиц реального времени может требовать высокого значения этого лимита, так как каждый дисковый чанк может занимать десятки или более файлов. Например, таблица реального времени с 1000 чанками может потребовать одновременного открытия тысяч файлов. Если в логах появляется ошибка 'Too many open files', попробуйте изменить этот параметр — это может помочь устранить проблему.

Существует также «жёсткий лимит», который нельзя превысить с помощью этой настройки. Этот лимит задаётся системой и может быть изменён в файле `/etc/security/limits.conf` на Linux. В других ОС могут применяться другие методы, поэтому ознакомьтесь с руководствами для дополнительной информации.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
Помимо прямых числовых значений, можно использовать магическое слово 'max', чтобы установить лимит равным текущему доступному жёсткому лимиту.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
Максимально допустимый размер сетевого пакета. Этот параметр ограничивает как пакеты запросов от клиентов, так и пакеты ответов от удалённых агентов в распределённой среде. Используется только для внутренних проверок и не влияет напрямую на использование оперативной памяти или производительность. Необязательный, по умолчанию 128 МБ.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
max_packet_size = 32M
```
<!-- end -->


### mysql_version_string

<!-- example conf mysql_version_string -->
Строка версии сервера, возвращаемая по протоколу MySQL. Необязательный параметр, по умолчанию пустая строка (возвращает версию Manticore).

Некоторые требовательные библиотеки клиентов MySQL зависят от определённого формата номера версии, используемого MySQL, а иногда выбирают разные пути выполнения в зависимости от версии, а не от флагов возможностей. Например, Python MySQLdb 1.2.2 вызывает исключение, если версия не находится в формате X.Y.ZZ; MySQL .NET connector 6.3.x внутренне не справляется с номерами версий 1.x при определённой комбинации флагов и т.п. Чтобы обойти это, можно использовать директиву `mysql_version_string` и заставить `searchd` сообщать клиентам, подключающимся по протоколу MySQL, другую версию. (По умолчанию он сообщает свою собственную версию.)


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

Количество сетевых потоков, по умолчанию 1.

Эта настройка полезна при экстремально высокой интенсивности запросов, когда одного потока недостаточно для обработки всех входящих запросов.


### net_wait_tm

Управляет интервалом опроса в busy loop сетевого потока. По умолчанию -1, может быть установлен в -1, 0, или положительное целое число.

Если сервер настроен как чистый мастер и только маршрутизирует запросы к агентам, важно обрабатывать запросы без задержек и не допускать спячку сетевого потока. Для этого существует busy loop. После входящего запроса сетевой поток использует опрос CPU в течение `10 * net_wait_tm` миллисекунд, если `net_wait_tm` положительно, или опрашивает только CPU, если `net_wait_tm` равен `0`. Также busy loop можно отключить установкой `net_wait_tm = -1` — в таком случае poller устанавливает таймауты согласно реальным таймаутам агентов в системном вызове опроса.

> **ПРЕДУПРЕЖДЕНИЕ:** Busy loop сильно загружает ядро процессора, поэтому установка этого значения в любое, кроме стандартного, приведёт к заметному использованию CPU даже при простое сервера.


### net_throttle_accept

Определяет количество клиентов, принимаемых на каждой итерации сетевого цикла. По умолчанию 0 (без ограничений), что подходит большинству пользователей. Это тонкая настройка для управления пропускной способностью сетевого цикла в условиях высокой нагрузки.


### net_throttle_action

Определяет количество запросов, обрабатываемых на каждой итерации сетевого цикла. По умолчанию 0 (без ограничений), что подходит большинству пользователей. Это тонкая настройка для управления пропускной способностью сетевого цикла в условиях высокой нагрузки.

### network_timeout

<!-- example conf network_timeout -->
Таймаут чтения/записи сетевого запроса клиента, в секундах (или с использованием [special_suffixes](../Server_settings/Special_suffixes.md)). Необязательный, по умолчанию 5 секунд. `searchd` принудительно закроет подключение клиента, который не отправит запрос или не прочитает результат в течение этого таймаута.

Обратите также внимание на параметр [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet). Этот параметр изменяет поведение `network_timeout` с применения к всему `query` или `result` на применение к отдельным пакетам. Обычно запрос/результат помещается в один или два пакета. Однако в случаях, когда требуется большой объем данных, этот параметр может быть незаменим для поддержания активной работы.

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
Этот параметр позволяет указать сетевой адрес узла. По умолчанию он установлен на адрес репликации [listen](../Server_settings/Searchd.md#listen). Это правильно в большинстве случаев; однако, бывают ситуации, когда его приходится указывать вручную:

* Узел за файрволом
* Включен сетевой транслятор адресов (NAT)
* Развертывания в контейнерах, таких как Docker или облачные развертывания
* Кластеры с узлами в более чем одном регионе


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
Этот параметр определяет, разрешать ли запросы с использованием только [оператора отрицания](../Searching/Full_text_matching/Operators.md#Negation-operator) полнотекстового поиска. Опционально, по умолчанию 0 (отклонять запросы с использованием только оператора NOT).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
Задает порог по умолчанию для сжатия таблицы. Подробнее здесь — [Number of optimized disk chunks](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). Этот параметр может быть переопределен опцией на уровне запроса [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). Также может динамически изменяться через [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
Этот параметр определяет максимальное количество одновременных постоянных соединений с удаленными [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md). Каждый раз при подключении агента, определенного в `agent_persistent`, мы пытаемся повторно использовать существующее соединение (если оно есть), или подключиться и сохранить соединение для будущего использования. Однако в некоторых случаях имеет смысл ограничить количество таких постоянных соединений. Эта директива задает лимит. Она влияет на количество соединений с хостом каждого агента во всех распределенных таблицах.

Рекомендуется установить значение равным или меньшим, чем опция [max_connections](../Server_settings/Searchd.md#max_connections) в конфиге агента.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file — обязательная опция конфигурации в Manticore search, которая задает путь к файлу, в котором хранится идентификатор процесса (PID) сервера `searchd`.

Файл с PID процесса searchd пересоздается и блокируется при запуске, и содержит PID главного серверного процесса, пока сервер работает. Файл удаляется при остановке сервера.
Назначение этого файла — позволить Manticore выполнять различные внутренние задачи, такие как проверка, запущен ли уже экземпляр `searchd`, остановка `searchd` и уведомление его о необходимости ротации таблиц. Файл также может использоваться внешними скриптами автоматизации.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
pid_file = /run/manticore/searchd.pid
```
<!-- end -->


### predicted_time_costs

<!-- example conf predicted_time_costs -->
Затраты для модели предсказания времени запроса, в наносекундах. Опционально, по умолчанию `doc=64, hit=48, skip=2048, match=64`.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
predicted_time_costs = doc=128, hit=96, skip=4096, match=128
```
<!-- end -->

<!-- example conf predicted_time_costs 1 -->
Преждевременное прерывание запросов по времени выполнения (с помощью установки max query time) — это хорошая страховка, но она имеет свой недостаток: непредсказуемые (нестабильные) результаты. То есть если повторить один и тот же (сложный) поисковый запрос с ограничением по времени несколько раз, ограничение времени будет достигаться на разных этапах, и вы получите *разные* наборы результатов.

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

Есть опция, [SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time), которая позволяет ограничивать время запроса *и* получать стабильные, воспроизводимые результаты. Вместо того чтобы регулярно проверять фактическое текущее время во время оценки запроса, что является непредсказуемым, она предсказывает текущее время исполнения с помощью простой линейной модели:

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

Запрос прерывается досрочно, когда `predicted_time` достигает заданного предела.

Конечно, это не жесткое ограничение на фактическое затраченное время (зато это жесткое ограничение на объем выполненной *обработки*), и простая линейная модель далеко не идеально точна. Поэтому настенные часы *могут* показывать время как ниже, так и выше целевого предела. Однако погрешности вполне приемлемы: например, в наших экспериментах с целевым лимитом в 100 мс большинство тестовых запросов попадало в диапазон 95–105 мс, и *все* запросы находились в диапазоне 80–120 мс. Также приятным побочным эффектом является то, что использование смоделированного времени запроса вместо измерения фактического времени выполнения приводит к некоторому уменьшению количества вызовов gettimeofday().

Ни одна пара моделей и марок серверов не идентична, поэтому директива `predicted_time_costs` позволяет настроить стоимости для приведённой выше модели. Для удобства они представлены целыми числами, которые считаются в наносекундах. (Предел в max_predicted_time считается в миллисекундах, и необходимость указывать значения стоимости как 0.000128 мc вместо 128 нс несколько более подвержена ошибкам.) Указывать все четыре стоимости одновременно не обязательно, так как пропущенные возьмутся по умолчанию. Тем не менее, мы настоятельно рекомендуем указать все стоимости для удобочитаемости.


### preopen_tables

<!-- example conf preopen_tables -->
Директива конфигурации preopen_tables определяет, следует ли принудительно предварительно открыть все таблицы при запуске. Значение по умолчанию — 1, что означает, что все таблицы будут предварительно открыты независимо от индивидуальной настройки [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) для каждой таблицы. Если установить в 0, то будут действовать настройки для отдельных таблиц, которые по умолчанию равны 0.

Предварительное открытие таблиц может предотвратить гонки между поисковыми запросами и ротациями, которые иногда вызывают сбои запросов. Однако это также использует больше дескрипторов файлов. В большинстве случаев рекомендуется предварительно открывать таблицы.

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
Опция конфигурации pseudo_sharding включает параллелизацию поисковых запросов к локальным простым и реальным таблицам, независимо от того, запрашиваются ли они напрямую или через распределённую таблицу. Эта функция автоматически распараллеливает запросы до количества потоков, указанного в `searchd.threads`.

Обратите внимание, что если ваши рабочие потоки уже заняты из-за:
* высокой конкуренции запросов
* физического шардирования любого типа:
  - распределённая таблица из нескольких простых/реальных таблиц
  - реальная таблица, состоящая из слишком большого количества дисковых чанков

то включение pseudo_sharding может не дать никакой выгоды и даже привести к небольшому снижению пропускной способности. Если вы отдаёте приоритет высокой пропускной способности вместо низкой задержки, рекомендуется отключить эту опцию.

Включено по умолчанию.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

Директива `replication_connect_timeout` определяет таймаут подключения к удалённому узлу. По умолчанию значение считается в миллисекундах, но может иметь [другой суффикс](../Server_settings/Special_suffixes.md). Значение по умолчанию — 1000 (1 секунда).

При подключении к удалённому узлу Manticore будет ждать не более этого времени для успешного установления соединения. Если таймаут достигнут, но соединение не установлено, и включены `retries`, будет выполнена повторная попытка.


### replication_query_timeout

Директива `replication_query_timeout` задаёт время ожидания searchd ответа от удалённого узла на выполнение запроса. Значение по умолчанию — 3000 миллисекунд (3 секунды), но может содержать `суффикс` для указания другой единицы времени.

После установления соединения Manticore будет ждать максимум `replication_query_timeout` для завершения удалённым узлом. Заметьте, что этот таймаут отличается от `replication_connect_timeout`, и общая задержка от удалённого узла будет суммой обоих значений.


### replication_retry_count

Эта настройка — целое число, указывающее, сколько раз Manticore попытается подключиться и выполнить запрос на удалённом узле во время репликации перед тем, как сообщить о критической ошибке запроса. Значение по умолчанию — 3.


### replication_retry_delay

Эта настройка — целое число в миллисекундах (или [special_suffixes](../Server_settings/Special_suffixes.md)), задающее задержку перед повторной попыткой запроса к удалённому узлу в случае сбоя во время репликации. Значение актуально только при ненулевом значении. Значение по умолчанию — 500.

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
Эта настройка конфигурации задаёт максимальный объём оперативной памяти, выделяемый для кешированных наборов результатов, в байтах. Значение по умолчанию — 16777216, что эквивалентно 16 мегабайтам. Если значение установлено в 0, кеш запросов отключен. Подробнее о кешировании запросов см. в разделе [query cache](../Searching/Query_cache.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

Целое число, в миллисекундах. Минимальный порог времени работы запроса для кеширования результата. По умолчанию 3000 (3 секунды). 0 означает кешировать всё. Смотрите подробности в [query cache](../Searching/Query_cache.md). Также значение может быть выражено с помощью [special_suffixes](../Server_settings/Special_suffixes.md), но используйте это аккуратно, чтобы не запутаться с именем самой настройки, содержащей '_msec'.


### qcache_ttl_sec

Целое число, в секундах. Период хранения кешированного результата. По умолчанию 60 (1 минута). Минимальное значение — 1 секунда. Подробности в [query cache](../Searching/Query_cache.md). Также может быть выражено с помощью [special_suffixes](../Server_settings/Special_suffixes.md), но используйте аккуратно, чтобы не запутаться с именем самой настройки, содержащей '_sec'.


### query_log_format

<!-- example conf query_log_format -->
Формат журнала запросов. Опционально, допустимые значения `plain` и `sphinxql`, по умолчанию `sphinxql`.

Режим `sphinxql` логирует валидные SQL-запросы. Режим `plain` логирует запросы в простом текстовом формате (в основном подходит для случаев чисто полнотекстового использования). Эта директива позволяет переключаться между двумя форматами при запуске поискового сервера. Формат журнала также можно изменить на лету с помощью синтаксиса `SET GLOBAL query_log_format=sphinxql`. Подробнее см. в разделе [Query logging](../Logging/Query_logging.md).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

Лимит (в миллисекундах), который предотвращает запись запроса в журнал запросов. По умолчанию 0 (все запросы записываются в журнал запросов). Эта директива указывает, что будут записываться в журнал только запросы с временем выполнения, превышающим заданный лимит (это значение также может быть выражено с помощью [специальных_суффиксов](../Server_settings/Special_suffixes.md), но используйте это с осторожностью и не путайте себя с названием самого значения, содержащего `_msec`).

### query_log

<!-- example conf query_log -->
Имя файла журнала запросов. Опционально, по умолчанию пусто (не записывать запросы). Все поисковые запросы (такие как SELECT ... но не INSERT/REPLACE/UPDATE запросы) будут записаны в этот файл. Формат описан в [Логировании запросов](../Logging/Query_logging.md). В случае формата 'plain' можно использовать 'syslog' как путь к файлу журнала. В этом случае все поисковые запросы будут отправляться демону syslog с приоритетом `LOG_INFO`, с префиксом '[query]' вместо временной метки. Для использования опции syslog, Manticore должен быть собран с опцией `-–with-syslog`.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
Директива query_log_mode позволяет установить разные права доступа для файлов журнала searchd и журнала запросов. По умолчанию эти файлы журнала создаются с правами 600, что означает, что читать журнал могут только пользователь, под которым работает сервер, и root.
Эта директива может быть полезна, если вы хотите разрешить другим пользователям читать файлы журнала, например решениям мониторинга, запущенным от не-root пользователей.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
Директива read_buffer_docs контролирует размер буфера чтения на ключевое слово для списков документов. Для каждого вхождения ключевого слова в каждом поисковом запросе существуют два связанных буфера чтения: один для списка документов и один для списка хитов. Эта настройка позволяет контролировать размер буфера списка документов.

Больший размер буфера может увеличить использование ОЗУ на запрос, но, возможно, уменьшит время ввода-вывода. Имеет смысл устанавливать большие значения для медленных хранилищ, но для хранилищ с высокой производительностью операций ввода-вывода эксперименты следует проводить с низкими значениями.

Значение по умолчанию 256K, минимальное значение 8K. Также можно задать [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) на уровне конкретной таблицы, что переопределит настройки конфигурации сервера.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
Директива read_buffer_hits задаёт размер буфера чтения на ключевое слово для списков хитов в поисковых запросах. По умолчанию размер 256K, минимальное значение 8K. Для каждого вхождения ключевого слова в поисковом запросе существует два связанных буфера чтения — для списка документов и списка хитов. Увеличение размера буфера может увеличить использование ОЗУ на запрос, но снизить время ввода-вывода. Для медленных хранилищ большие размеры буфера имеют смысл, а для хранилищ с высокой производительностью ввода-вывода эксперименты должны проводиться с малыми значениями.

Эта настройка также может быть задана на уровне конкретной таблицы через опцию read_buffer_hits в [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits), что переопределит настройки сервера.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
Размер нечёсанного чтения. Опционально, по умолчанию 32K, минимум 1K

При запросах некоторые операции чтения заранее знают, сколько данных нужно прочитать, а некоторые — нет. Наиболее заметно, что размер списка хитов заранее не известен. Эта настройка позволяет контролировать, сколько данных читать в таких случаях. Она влияет на время ввода-вывода для списка хитов, уменьшая его для списков, больших чем размер нечёсанного чтения, но увеличивая для меньших списков. Эта настройка **не** влияет на использование ОЗУ, так как буфер чтения уже выделен. Поэтому она не должна превышать read_buffer.


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

При значении 0 таймауты ограничивают максимальное время на отправку всего запроса.
При значении 1 (по умолчанию) таймауты ограничивают максимальное время между сетевой активностью.

При репликации узлу может понадобиться отправить большой файл (например, 100GB) другому узлу. Допустим, сеть может передавать данные со скоростью 1GB/c, с серией пакетов по 4-5MB каждый. Для передачи всего файла потребуется 100 секунд. Таймаут по умолчанию в 5 секунд позволил бы передать только 5GB, после чего соединение прервётся. Увеличение таймаута было бы решением, но не масштабируемым (следующий файл может быть 150GB и снова будет сбой). Однако с параметром `reset_network_timeout_on_packet`, установленным по умолчанию в 1, таймаут применяется не к всей передаче, а к отдельным пакетам. Пока передача продолжается (и данные действительно поступают в пределах таймаута), соединение поддерживается. Если передача зависает и возникает таймаут между пакетами, соединение разрывается.

Обратите внимание, что при настройке распределённой таблицы каждый узел — как мастер, так и агенты — должен быть настроен. Со стороны мастера влияет `agent_query_timeout`, на агентах — `network_timeout`.

<!-- intro -->

##### Пример:

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
Период проверки сброса чанка RAM для RT таблиц в секундах (или с [специальными_суффиксами](../Server_settings/Special_suffixes.md)). Опционально, значение по умолчанию 10 часов.

Активно обновляемые RT-таблицы, полностью помещающиеся в чанки RAM, все равно могут привести к постоянно растущим бинлогам, что влияет на использование диска и время восстановления после сбоя. С этой директивой сервер поиска периодически выполняет проверку сброса, и подходящие чанки RAM могут быть сохранены, что позволяет произвести последующую очистку бинлогов. Подробнее см. [Binary logging](../Logging/Binary_logging.md).

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
Максимальное количество I/O операций (в секунду), которые поток слияния RT-чанков может начать. Опционально, значение по умолчанию — 0 (без ограничений).

Эта директива позволяет снизить нагрузку на I/O, возникающую из-за операторов `OPTIMIZE`. Гарантируется, что все операции оптимизации RT не будут генерировать больше IOPS (операций ввода-вывода в секунду), чем заданный лимит. Ограничение rt_merge_iops может уменьшить деградацию производительности поиска, вызванную процессом слияния.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
Максимальный размер одной I/O операции, которую поток слияния RT-чанков может начать. Опционально, значение по умолчанию — 0 (без ограничений).

Эта директива позволяет снизить нагрузку на I/O, возникающую из-за операторов `OPTIMIZE`. Операции ввода-вывода, размер которых превышает этот лимит, будут разбиты на две или более операций, которые затем будут учитываться как отдельные I/O операции с точки зрения лимита [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops). Таким образом, гарантируется, что все операции оптимизации не сгенерируют более чем (rt_merge_iops * rt_merge_maxiosize) байт операций с диском в секунду.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
Предотвращает зависание `searchd` при вращении таблиц с огромным объемом данных для предзагрузки в кеш. Опционально, значение по умолчанию — 1 (включена бесшовная ротация). На системах Windows бесшовная ротация по умолчанию отключена.

Таблицы могут содержать данные, которые необходимо предзагрузить в RAM. В настоящее время файлы `.spa`, `.spb`, `.spi` и `.spm` полностью предзагружаются (они содержат данные атрибутов, blob-атрибутов, таблицу ключевых слов и карту удалённых строк соответственно). Без бесшовной ротации вращение таблицы пытается использовать минимально возможный объем RAM и работает следующим образом:

1. Новые запросы временно отклоняются (с кодом ошибки "retry");
2. `searchd` ожидает завершения всех текущих запросов;
3. Старые таблицы освобождаются, их файлы переименовываются;
4. Файлы новой таблицы переименовываются, и выделяется необходимый объем RAM;
5. Новые данные атрибутов и словаря таблицы предзагружаются в RAM;
6. `searchd` возобновляет обслуживание запросов из новой таблицы.

Однако, если данных атрибутов или словаря много, этап предзагрузки может занять заметное время — до нескольких минут в случае предзагрузки файлов объемом 1-5+ ГБ.

При включённой бесшовной ротации процесс вращения работает так:

1. Выделяется RAM для хранения новой таблицы;
2. Данные атрибутов и словаря новой таблицы асинхронно предзагружаются в RAM;
3. При успешном завершении старая таблица освобождается, и файлы обеих таблиц переименовываются;
4. При ошибке новая таблица освобождается;
5. В любой момент запросы обслуживаются либо из старой, либо из новой копии таблицы.

Бесшовная ротация сопровождается увеличением пикового использования памяти во время вращения (так как обе копии данных `.spa/.spb/.spi/.spm` старой и новой таблицы должны находиться в RAM во время предзагрузки новой копии). Среднее использование памяти остается без изменений.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

Эта опция включает/отключает использование вторичных индексов для поисковых запросов. Опционально, по умолчанию включена (1). Обратите внимание, что для индексирования её включать не нужно, так как она всегда активна при установленной [Manticore Columnar Library](https://github.com/manticoresoftware/columnar). Наличие библиотеки также требуется для использования индексов при поиске. Доступны три режима:

* `0`: Отключить использование вторичных индексов при поиске. Их можно включить для отдельных запросов при помощи [подсказок анализатора](../Searching/Options.md#Query-optimizer-hints)
* `1`: Включить использование вторичных индексов при поиске. Их можно отключить для отдельных запросов при помощи [подсказок анализатора](../Searching/Options.md#Query-optimizer-hints)
* `force`: То же, что и включение, но любые ошибки при загрузке вторичных индексов будут зафиксированы, и весь индекс не будет загружен в демон.

<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
Целое число, служащее идентификатором сервера и используемое в качестве исходного значения для генерации уникального короткого UUID для узлов в кластере репликации. Значение server_id должно быть уникально среди узлов кластера и находиться в диапазоне от 0 до 127. Если server_id не установлен, он вычисляется как хэш MAC-адреса и пути к PID-файлу или для короткого UUID будет использовано случайное число.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
Время ожидания при выполнении `searchd --stopwait`, в секундах (или с использованием [special_suffixes](../Server_settings/Special_suffixes.md)). Опционально, по умолчанию — 60 секунд.

При запуске `searchd --stopwait` сервер должен выполнить ряд задач перед остановкой, таких как завершение запросов, сброс RT RAM чанков, сброс атрибутов и обновление бинлога. На выполнение этих задач требуется некоторое время. `searchd --stopwait` будет ожидать до `shutdown_time` секунд, пока сервер не завершит работу. Подходящее время зависит от размера таблиц и нагрузки.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

SHA1 хеш пароля, необходимого для вызова команды 'shutdown' из подключения VIP Manticore SQL. Без него команда подкоманды 'shutdown' в режиме [debug](../Reporting_bugs.md#DEBUG) никогда не приведет к остановке сервера. Обратите внимание, что такое простое хеширование не следует считать надежной защитой, так как мы не используем соль или какой-либо современный хеш-функционал. Это предназначено как надежная мера для сервисных демонов в локальной сети.

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
Префикс, который добавляется к локальным именам файлов при генерации сниппетов. Необязательно, по умолчанию — текущая рабочая папка.

Этот префикс может использоваться при распределенной генерации сниппетов вместе с опциями `load_files` или `load_files_scattered`.

Обратите внимание, что это именно префикс, а **не** путь! Это означает, что если префикс установлен в "server1", а запрос относится к "file23", `searchd` попытается открыть "server1file23" (все это без кавычек). Поэтому, если вам нужен путь, необходимо включить завершающий слэш.

После построения окончательного пути к файлу сервер разворачивает все относительные каталоги и сравнивает результат с значением `snippet_file_prefix`. Если результат не начинается с префикса, такой файл будет отвергнут с сообщением об ошибке.

Например, если вы установите его в `/mnt/data` и кто-то вызовет генерацию сниппетов с файлом `../../../etc/passwd` в качестве источника, он получит сообщение об ошибке:

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

вместо содержимого файла.

Также, при неопределенном параметре и чтении `/etc/passwd`, фактически будет прочитан /daemon/working/folder/etc/passwd, поскольку значение параметра по умолчанию — рабочая папка сервера.

Обратите внимание, что это локальная опция; она никак не влияет на агенты. Поэтому вы можете без опасений установить префикс на мастер-сервере. Запросы, направляемые агентам, не будут затронуты настройками мастера. Однако они будут затронуты собственными настройками агента.

Это может быть полезно, например, когда местоположения хранилищ документов (локальное хранилище или NAS-монты) неоднородны на разных серверах.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **ПРЕДУПРЕЖДЕНИЕ:** Если вы все же хотите получить доступ к файлам из корня ФС, необходимо явно установить `snippets_file_prefix` в пустое значение (строкой `snippets_file_prefix=`) или в корень (строкой `snippets_file_prefix=/`).


### sphinxql_state

<!-- example conf sphinxql_state -->
Путь к файлу, в котором будет сериализовано текущее состояние SQL.

При старте сервера этот файл воспроизводится. При подходящих изменениях состояния (например, SET GLOBAL) файл автоматически перезаписывается. Это позволяет предотвратить трудно диагностируемую проблему: если вы загрузили функции UDF и Manticore аварийно завершился, при (автоматическом) перезапуске ваши UDF и глобальные переменные больше не будут доступны. Использование постоянного состояния помогает обеспечить плавное восстановление без таких сюрпризов.

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
Максимальное время ожидания между запросами (в секундах или с применением [специальных суффиксов](../Server_settings/Special_suffixes.md)) при работе с SQL-интерфейсом. Необязательно, по умолчанию — 15 минут.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
Путь к файлу сертификата центра сертификации SSL (CA-сертификат, также известный как корневой сертификат). Необязательно, по умолчанию пусто. Если не пусто, сертификат в `ssl_cert` должен быть подписан этим корневым сертификатом.

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
Путь к SSL-сертификату сервера. Необязательно, по умолчанию пусто.

Сервер использует этот сертификат как самоподписанный открытый ключ для шифрования HTTP-трафика через SSL. Файл должен быть в формате PEM.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
Путь к приватному ключу SSL-сертификата. Необязательно, по умолчанию пусто.

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
Максимальный размер кеша документов общего поддерева, на запрос. Необязательно, по умолчанию 0 (отключено).

Эта настройка ограничивает использование ОЗУ оптимизатором общего поддерева (см. [multi-queries](../Searching/Multi-queries.md)). Максимум столько памяти будет потрачено на кеширование записей документов для каждого запроса. Установка значения в 0 отключает оптимизатор.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
Максимальный размер кеша попаданий общего поддерева, на запрос. Необязательно, по умолчанию 0 (отключено).

Эта настройка ограничивает использование ОЗУ оптимизатором общего поддерева (см. [multi-queries](../Searching/Multi-queries.md)). Максимум столько памяти будет потрачено на кеширование вхождений ключевых слов (попаданий) для каждого запроса. Установка значения в 0 отключает оптимизатор.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Количество рабочих потоков (размер пула потоков) для демона Manticore. Manticore создает такое количество потоков ОС при запуске, и они выполняют все задачи внутри демона, такие как выполнение запросов, создание сниппетов и пр. Некоторые операции могут быть разбиты на подзадачи и выполняться параллельно, например:

* Поиск в таблице реального времени
* Поиск в распределенной таблице, состоящей из локальных таблиц
* Вызов перколяции запроса
* и другие

По умолчанию установлено значение, равное количеству ядер CPU на сервере. Manticore создает потоки при запуске и удерживает их до остановки. Каждый подзадача может использовать один из потоков, когда это необходимо. Когда подзадача завершается, она освобождает поток, чтобы его мог использовать другой подзадача.

В случае интенсивной нагрузки типа ввода-вывода может иметь смысл установить значение выше, чем количество ядер CPU.

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
Максимальный размер стека для задания (корутина, один поисковый запрос может вызвать несколько заданий/корутин). Опционально, по умолчанию 128K.

У каждого задания есть свой стек размером 128K. При запуске запроса проверяется, сколько стека ему требуется. Если стандартных 128K достаточно, запрос просто обработается. Если нужно больше, планируется другое задание с увеличенным стеком, которое продолжит обработку. Максимальный размер такого расширенного стека ограничен этой настройкой.

Установка значения на разумно высоком уровне поможет при обработке очень глубоких запросов без увеличения общего потребления ОЗУ до слишком высоких значений. Например, установка 1G не означает, что каждое новое задание займет 1G ОЗУ, но если мы видим, что требуется, скажем, стек в 100M, мы просто выделяем 100M для задания. Другие задания в то же время будут работать со стеком по умолчанию 128K. Аналогично можно запускать еще более сложные запросы, которым требуется 500M. И только если мы **увидим** внутренне, что заданию нужен стек больше 1G, обработка завершится с ошибкой и сообщением о слишком низком значении thread_stack.

Однако на практике даже запрос, требующий 16M стека, часто слишком сложен для разбора и потребляет слишком много времени и ресурсов для обработки. Таким образом, демон обработает его, но ограничение таких запросов настройкой `thread_stack` выглядит вполне разумным.


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
Определяет, следует ли удалять `.old` копии таблиц при успешной ротации. Опционально, по умолчанию 1 (удалять).


<!-- intro -->
##### Пример:

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
Потоковый watchdog сервера. Опционально, по умолчанию 1 (watchdog включен).

Если запрос Manticore завершится аварийно, это может привести к сбою всего сервера. При включенной функции watchdog `searchd` также поддерживает отдельный легковесный процесс, который контролирует основной процесс сервера и автоматически перезапускает его в случае аномального завершения. Watchdog включен по умолчанию.

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

