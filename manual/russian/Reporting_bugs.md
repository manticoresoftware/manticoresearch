# Сообщение об ошибках

К сожалению, Manticore пока не является на 100% свободным от ошибок, хотя команда разработчиков усердно работает над достижением этой цели. Время от времени вы можете столкнуться с некоторыми проблемами.
Очень важно сообщать как можно больше информации о каждой ошибке для её эффективного исправления.
Чтобы исправить ошибку, её либо нужно воспроизвести и исправить, либо определить причину на основе предоставленной вами информации. Чтобы помочь в этом, пожалуйста, следуйте приведённым ниже инструкциям.

### Трекер ошибок
Ошибки и запросы на новые функции отслеживаются на [Github](https://github.com/manticoresoftware/manticore/issues). Вы можете создать новый тикет и подробно описать вашу ошибку, чтобы сэкономить время как для себя, так и для разработчиков.

### Обновления документации
Обновления документации (то, что вы сейчас читаете) также выполняются на [Github](https://github.com/manticoresoftware/manticoresearch/tree/master/manual).

### Сбои

Manticore Search написан на C++, что является языком программирования низкого уровня, позволяющим напрямую взаимодействовать с компьютером для более высокой производительности. Однако у такого подхода есть недостаток: в редких случаях может не получиться элегантно обработать ошибку, записав её в лог и пропустив обработку команды, вызвавшей проблему. Вместо этого программа может аварийно завершиться, полностью остановившись и требуя перезапуска.

Когда Manticore Search аварийно завершается, важно сообщить об этом команде Manticore, [отправив отчет об ошибке](https://github.com/manticoresoftware/manticoresearch/issues/new?assignees=&labels=&template=bug_report.md&title=) на GitHub или через профессиональные услуги Manticore в вашем приватном хелпдеске. Команда Manticore требует следующую информацию:

1. Лог searchd
2. Coredump
3. Лог запросов

Кроме того, будет полезно, если вы сможете сделать следующее:
1. Запустите gdb для анализа coredump:
```
gdb /usr/bin/searchd </path/to/coredump>
```
2. Найдите ID аварийного потока в имени файла coredump (убедитесь, что у вас есть `%p` в /proc/sys/kernel/core_pattern), например, `core.work_6.29050.server_name.1637586599` означает thread_id=29050
3. В gdb выполните:
```
set pagination off
info threads
# find thread number by it's id (e.g. for `LWP 29050` it will be thread number 8
thread apply all bt
thread <thread number>
bt full
info locals
quit
```
4. Предоставьте выводы

### Что делать, если Manticore Search завис?

Если Manticore Search завис, вам нужно собрать некоторую информацию, которая может помочь понять причину. Вот как это сделать:

1. Выполните `show threads option format=all` через [VIP порт](https://manual.manticoresearch.com/Connecting_to_the_server/HTTP#VIP-connection)
2. Соберите вывод lsof, так как зависание может быть вызвано слишком большим количеством соединений или открытых дескрипторов файлов.
```
lsof -p `cat /run/manticore/searchd.pid`
```
3. Создайте дамп памяти:
```
gcore `cat /run/manticore/searchd.pid`
```
(Дамп будет сохранён в текущую директорию.)

4. Установите и запустите gdb:
```
gdb /usr/bin/searchd `cat /run/manticore/searchd.pid`
```
Обратите внимание, что это остановит работающий searchd, но если он уже завис, это не должно быть проблемой.
5. В gdb выполните:
```
set pagination off
info threads
thread apply all bt
quit
```
6. Соберите все выводы и файлы и предоставьте их в отчёте об ошибке.

Для экспертов: макросы, добавленные в [этот коммит](https://github.com/manticoresoftware/manticoresearch/commit/e317f7aa30aad51cb19d34595458bb7c8811be21), могут быть полезны при отладке.

### Как включить сохранение coredump при сбое?

* Убедитесь, что вы запускаете searchd с `--coredump`. Чтобы не изменять скрипты, вы можете использовать метод https://manual.manticoresearch.com/Starting_the_server/Linux#Custom-startup-flags-using-systemd . Например::

```
[root@srv lib]# systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--coredump'
[root@srv lib]# systemctl restart manticore
[root@srv lib]# ps aux|grep searchd
mantico+  1955  0.0  0.0  61964  1580 ?        S    11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
mantico+  1956  0.6  0.0 392744  2664 ?        Sl   11:02   0:00 /usr/bin/searchd --config /etc/manticoresearch/manticore.conf --coredump
```

* Убедитесь, что ваша операционная система позволяет сохранять дампы памяти, проверив, что: `/proc/sys/kernel/core_pattern` не пуст. Это место, куда будут сохраняться дампы памяти. Чтобы сохранять дампы в файл, например `core.searchd.1773.centos-4gb-hel1-1.1636454937`, выполните следующую команду:
```
echo "/cores/core.%e.%p.%h.%t" > /proc/sys/kernel/core_pattern
```

* searchd должен запускаться с `ulimit -c unlimited`. Если вы запускаете Manticore через systemctl, он автоматически установит лимит в бесконечность, как указано в следующей строке файла manticore.service:
```
[root@srv lib]# grep CORE /lib/systemd/system/manticore.service
LimitCORE=infinity
```

### Как установить отладочные символы?

Manticore Search и Manticore Columnar Library написаны на C++, что приводит к созданию скомпилированных бинарных файлов, которые оптимально выполняются на вашей операционной системе. Однако при запуске бинарника ваша система не имеет полного доступа к именам переменных, функций, методов и классов. Эта информация предоставляется в отдельных пакетах "debuginfo" или "symbol packages".

Отладочные символы необходимы для устранения неполадок и отладки, так как они позволяют визуализировать состояние системы в момент сбоя, включая имена функций. Manticore Search предоставляет backtrace в логе searchd и генерирует coredump, если запущен с флагом --coredump. Без символов вы увидите только внутренние смещения, что затрудняет или делает невозможным расшифровку причины сбоя. Если вам нужно сделать отчет об ошибке по сбою, команда Manticore часто потребует отладочные символы для помощи.

Чтобы установить отладочные символы Manticore Search/Manticore Columnar Library, вам нужно установить пакет `*debuginfo*` для CentOS, пакет `*dbgsym*` для Ubuntu и Debian или пакет `*dbgsymbols*` для Windows и macOS. Эти пакеты должны соответствовать версии установленного Manticore. Например, если вы установили Manticore Search в Centos 8 из пакета https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-4.0.2_210921.af497f245-1.el8.x86_64.rpm , соответствующий пакет с символами будет https://repo.manticoresearch.com/repository/manticoresearch/release/centos/8/x86_64/manticore-debuginfo-4.0.2_210921.af497f245-1.el8.x86_64.rpm

Обратите внимание, что оба пакета имеют одинаковый идентификатор коммита `af497f245`, который соответствует коммиту, из которого была собрана эта версия.

Если вы установили Manticore из репозитория Manticore APT/YUM, вы можете использовать один из следующих инструментов:
* `debuginfo-install` в CentOS 7
* `dnf debuginfo-install` в CentOS 8
* `find-dbgsym-packages` в Debian и Ubuntu

чтобы найти пакет отладочных символов для вас.

### Как проверить наличие отладочных символов?

1. Найдите build ID в выводе `file /usr/bin/searchd`:

```
[root@srv lib]# file /usr/bin/searchd
/usr/bin/searchd: ELF 64-bit LSB executable, x86-64, version 1 (GNU/Linux), dynamically linked (uses shared libs), for GNU/Linux 2.6.32, BuildID[sha1]=2c582e9f564ea1fbeb0c68406c271ba27034a6d3, stripped
```

В данном случае build ID — 2c582e9f564ea1fbeb0c68406c271ba27034a6d3.

2. Найдите символы в `/usr/lib/debug/.build-id` вот так:

```
[root@srv ~]# ls -la /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3*
lrwxrwxrwx. 1 root root 23 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3 -> ../../../../bin/searchd
lrwxrwxrwx. 1 root root 27 Nov  9 10:42 /usr/lib/debug/.build-id/2c/582e9f564ea1fbeb0c68406c271ba27034a6d3.debug -> ../../usr/bin/searchd.debug
```

### Загрузка ваших данных

<!-- example s3 -->
Чтобы исправить вашу ошибку, разработчикам часто нужно воспроизвести её локально. Для этого им нужны ваш конфигурационный файл, файлы таблиц, binlog (если есть), а иногда исходные данные (например, данные из внешних хранилищ или XML/CSV файлы) и запросы.

Прикрепляйте ваши данные при [создании тикета на Github](https://github.com/manticoresoftware/manticoresearch/issues/new). Если данные слишком большие или конфиденциальные, вы можете загрузить их в наше хранилище S3 с правом только на запись по адресу `s3://s3.manticoresearch.com/write-only/`.

Для удобства мы предоставляем механизм загрузки с использованием Docker-образа. Этот образ построен из нашего открытого репозитория на [github.com/manticoresoftware/s3-upload](https://github.com/manticoresoftware/s3-upload) и помогает легко загружать данные в S3-хранилище Manticore с правом только на запись. Вот как это сделать:
1. Перейдите в каталог с файлами, которые хотите загрузить, и выполните:
   ```bash
   docker run -it --rm -v $(pwd):/upload manticoresearch/upload
   ```
2. Это:
   - Попросит вас ввести URL/номер связанной задачи
   - Загрузит **все файлы в текущем каталоге** в наше S3-хранилище с правом только на запись
   - В конце вы увидите путь загрузки. Пожалуйста, поделитесь этим путем с разработчиками.

<!-- intro -->
Пример:

<!-- request Example -->
```bash
docker run -it --rm -v $(pwd):/upload manticoresearch/upload
```

<!-- response Example -->
```bash
🚀 Welcome to Manticore Search Upload Tool! 🚀

📂 Files to be uploaded:
  tt (800)

🔗 Please enter the related issue URL/number
(e.g., https://github.com/manticoresoftware/manticoresearch/issues/123 or just 123):
123

📤 Starting upload process...
INFO: Cache file not found or empty, creating/populating it.
INFO: Compiling list of local files...
INFO: Running stat() and reading/calculating MD5 values on 23 files, this may take some time...
INFO: Summary: 23 local files to upload
upload: './tt/tt.0.spa' -> 's3://write-only/issue-20250219-123/tt/tt.0.spa'  [1 of 23]
 40 of 40   100% in    2s    15.03 B/s  done
upload: './tt/tt.0.spd' -> 's3://write-only/issue-20250219-123/tt/tt.0.spd'  [2 of 23]
 1 of 1   100% in    0s     1.99 B/s  done
upload: './tt/tt.0.spe' -> 's3://write-only/issue-20250219-123/tt/tt.0.spe'  [3 of 23]
 1 of 1   100% in    0s     2.02 B/s  done
upload: './tt/tt.0.sph' -> 's3://write-only/issue-20250219-123/tt/tt.0.sph'  [4 of 23]
 420 of 420   100% in    0s   895.32 B/s  done
upload: './tt/tt.0.sphi' -> 's3://write-only/issue-20250219-123/tt/tt.0.sphi'  [5 of 23]
 66 of 66   100% in    0s   142.67 B/s  done
upload: './tt/tt.0.spi' -> 's3://write-only/issue-20250219-123/tt/tt.0.spi'  [6 of 23]
 18 of 18   100% in    0s    39.13 B/s  done
upload: './tt/tt.0.spidx' -> 's3://write-only/issue-20250219-123/tt/tt.0.spidx'  [7 of 23]
 145 of 145   100% in    0s   313.38 B/s  done
upload: './tt/tt.0.spm' -> 's3://write-only/issue-20250219-123/tt/tt.0.spm'  [8 of 23]
 4 of 4   100% in    0s     8.36 B/s  done
upload: './tt/tt.0.spp' -> 's3://write-only/issue-20250219-123/tt/tt.0.spp'  [9 of 23]
 1 of 1   100% in    0s     2.15 B/s  done
upload: './tt/tt.0.spt' -> 's3://write-only/issue-20250219-123/tt/tt.0.spt'  [10 of 23]
 36 of 36   100% in    0s    78.35 B/s  done
upload: './tt/tt.1.spa' -> 's3://write-only/issue-20250219-123/tt/tt.1.spa'  [11 of 23]
 48 of 48   100% in    0s    81.35 B/s  done
upload: './tt/tt.1.spd' -> 's3://write-only/issue-20250219-123/tt/tt.1.spd'  [12 of 23]
 1 of 1   100% in    0s     1.65 B/s  done
upload: './tt/tt.1.spe' -> 's3://write-only/issue-20250219-123/tt/tt.1.spe'  [13 of 23]
 1 of 1   100% in    0s     1.95 B/s  done
upload: './tt/tt.1.sph' -> 's3://write-only/issue-20250219-123/tt/tt.1.sph'  [14 of 23]
 420 of 420   100% in    0s   891.58 B/s  done
upload: './tt/tt.1.sphi' -> 's3://write-only/issue-20250219-123/tt/tt.1.sphi'  [15 of 23]
 82 of 82   100% in    0s   166.42 B/s  done
upload: './tt/tt.1.spi' -> 's3://write-only/issue-20250219-123/tt/tt.1.spi'  [16 of 23]
 18 of 18   100% in    0s    39.46 B/s  done
upload: './tt/tt.1.spidx' -> 's3://write-only/issue-20250219-123/tt/tt.1.spidx'  [17 of 23]
 183 of 183   100% in    0s   374.04 B/s  done
upload: './tt/tt.1.spm' -> 's3://write-only/issue-20250219-123/tt/tt.1.spm'  [18 of 23]
 4 of 4   100% in    0s     8.42 B/s  done
upload: './tt/tt.1.spp' -> 's3://write-only/issue-20250219-123/tt/tt.1.spp'  [19 of 23]
 1 of 1   100% in    0s     1.28 B/s  done
upload: './tt/tt.1.spt' -> 's3://write-only/issue-20250219-123/tt/tt.1.spt'  [20 of 23]
 50 of 50   100% in    1s    34.60 B/s  done
upload: './tt/tt.lock' -> 's3://write-only/issue-20250219-123/tt/tt.lock'  [21 of 23]
 0 of 0     0% in    0s     0.00 B/s  done
upload: './tt/tt.meta' -> 's3://write-only/issue-20250219-123/tt/tt.meta'  [22 of 23]
 456 of 456   100% in    0s   923.34 B/s  done
upload: './tt/tt.settings' -> 's3://write-only/issue-20250219-123/tt/tt.settings'  [23 of 23]
 3 of 3   100% in    0s     6.41 B/s  done

✅ Upload complete!
📋 Please share this path with the developers:
issue-20250219-123

💡 Tip: Make sure to include this path when communicating with the Manticore team
```
<!-- end -->

В качестве альтернативы вы можете использовать S3 [Minio client](https://min.io/docs/minio/linux/reference/minio-mc.html) или инструмент Amazon [s3cmd](https://s3tools.org/s3cmd) для того же, например:

1. Установите клиент https://min.io/docs/minio/linux/reference/minio-mc.html#install-mc
Например, для 64-битного Linux:
   ```
   curl https://dl.min.io/client/mc/release/linux-amd64/mc \
   --create-dirs \
   -o $HOME/minio-binaries/mc
   chmod +x $HOME/minio-binaries/mc
   export PATH=$PATH:$HOME/minio-binaries/
   ```
2. Добавьте наш s3-хост (используйте полный путь к исполняемому файлу или перейдите в его каталог): `cd $HOME/minio-binaries` и затем `./mc config host add manticore http://s3.manticoresearch.com:9000 manticore manticore`
3. Скопируйте ваши файлы (используйте полный путь к исполняемому файлу или перейдите в его каталог): `cd $HOME/minio-binaries` и затем `./mc cp -r issue-1234/ manticore/write-only/issue-1234`. Убедитесь, что имя папки уникально и желательно соответствует задаче на GitHub, где вы описали ошибку.

### DEBUG

```sql
DEBUG [ subcommand ]
```

Оператор `DEBUG` предназначен для разработчиков и тестировщиков для вызова различных внутренних или VIP-команд. Однако он не предназначен для использования в продакшене, так как синтаксис компонента `subcommand` может свободно изменяться в любой сборке.

Чтобы просмотреть список полезных команд и подкоманд оператора `DEBUG`, доступных в текущем контексте, просто вызовите `DEBUG` без параметров.

```sql
mysql> debug;
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| command                                                                 | meaning                                                                                |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| flush logs                                                              | emulate USR1 signal                                                                    |
| reload indexes                                                          | emulate HUP signal                                                                     |
| debug token <password>                                                  | calculate token for password                                                           |
| debug malloc_stats                                                      | perform 'malloc_stats', result in searchd.log                                          |
| debug malloc_trim                                                       | pefrorm 'malloc_trim' call                                                             |
| debug sleep <N>                                                         | sleep for <N> seconds                                                                  |
| debug tasks                                                             | display global tasks stat (use select from @@system.tasks instead)                     |
| debug sched                                                             | display task manager schedule (use select from @@system.sched instead)                 |
| debug merge <TBL> [chunk] <X> [into] [chunk] <Y> [option sync=1,byid=0] | For RT table <TBL> merge disk chunk X into disk chunk Y                                |
| debug drop [chunk] <X> [from] <TBL> [option sync=1]                     | For RT table <TBL> drop disk chunk X                                                   |
| debug files <TBL> [option format=all|external]                          | list files belonging to <TBL>. 'all' - including external (wordforms, stopwords, etc.) |
| debug close                                                             | ask server to close connection from it's side                                          |
| debug compress <TBL> [chunk] <X> [option sync=1]                        | Compress disk chunk X of RT table <TBL> (wipe out deleted documents)                   |
| debug split <TBL> [chunk] <X> on @<uservar> [option sync=1]             | Split disk chunk X of RT table <TBL> using set of DocIDs from @uservar                 |
| debug wait <cluster> [like 'xx'] [option timeout=3]                     | wait <cluster> ready, but no more than 3 secs.                                         |
| debug wait <cluster> status <N> [like 'xx'] [option timeout=13]         | wait <cluster> commit achieve <N>, but no more than 13 secs                            |
| debug meta                                                              | Show max_matches/pseudo_shards. Needs set profiling=1                                  |
| debug trace OFF|'path/to/file' [<N>]                                    | trace flow to file until N bytes written, or 'trace OFF'                               |
| debug curl <URL>                                                        | request given url via libcurl                                                          |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
19 rows in set (0.00 sec)
```

То же из VIP-подключения:
```sql
mysql> debug;
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| command                                                                 | meaning                                                                                |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
| flush logs                                                              | emulate USR1 signal                                                                    |
| reload indexes                                                          | emulate HUP signal                                                                     |
| debug shutdown <password>                                               | emulate TERM signal                                                                    |
| debug crash <password>                                                  | crash daemon (make SIGSEGV action)                                                     |
| debug token <password>                                                  | calculate token for password                                                           |
| debug malloc_stats                                                      | perform 'malloc_stats', result in searchd.log                                          |
| debug malloc_trim                                                       | pefrorm 'malloc_trim' call                                                             |
| debug procdump                                                          | ask watchdog to dump us                                                                |
| debug setgdb on|off                                                     | enable or disable potentially dangerous crash dumping with gdb                         |
| debug setgdb status                                                     | show current mode of gdb dumping                                                       |
| debug sleep <N>                                                         | sleep for <N> seconds                                                                  |
| debug tasks                                                             | display global tasks stat (use select from @@system.tasks instead)                     |
| debug sched                                                             | display task manager schedule (use select from @@system.sched instead)                 |
| debug merge <TBL> [chunk] <X> [into] [chunk] <Y> [option sync=1,byid=0] | For RT table <TBL> merge disk chunk X into disk chunk Y                                |
| debug drop [chunk] <X> [from] <TBL> [option sync=1]                     | For RT table <TBL> drop disk chunk X                                                   |
| debug files <TBL> [option format=all|external]                          | list files belonging to <TBL>. 'all' - including external (wordforms, stopwords, etc.) |
| debug close                                                             | ask server to close connection from it's side                                          |
| debug compress <TBL> [chunk] <X> [option sync=1]                        | Compress disk chunk X of RT table <TBL> (wipe out deleted documents)                   |
| debug split <TBL> [chunk] <X> on @<uservar> [option sync=1]             | Split disk chunk X of RT table <TBL> using set of DocIDs from @uservar                 |
| debug wait <cluster> [like 'xx'] [option timeout=3]                     | wait <cluster> ready, but no more than 3 secs.                                         |
| debug wait <cluster> status <N> [like 'xx'] [option timeout=13]         | wait <cluster> commit achieve <N>, but no more than 13 secs                            |
| debug meta                                                              | Show max_matches/pseudo_shards. Needs set profiling=1                                  |
| debug trace OFF|'path/to/file' [<N>]                                    | trace flow to file until N bytes written, or 'trace OFF'                               |
| debug curl <URL>                                                        | request given url via libcurl                                                          |
+-------------------------------------------------------------------------+----------------------------------------------------------------------------------------+
24 rows in set (0.00 sec)
```

Все команды `debug XXX` следует считать нестабильными и подверженными изменениям в любое время, поэтому не удивляйтесь, если они изменятся. Этот пример вывода может не отражать фактически доступные команды, поэтому попробуйте на вашей системе, чтобы увидеть, что доступно на вашем экземпляре. Кроме того, подробной документации, кроме этой короткой колонки «значение», не предоставлено.

В качестве быстрой иллюстрации ниже описаны две команды, доступные только VIP-клиентам — shutdown и crash. Обе требуют токен, который можно сгенерировать с помощью подкоманды debug token, и добавить в параметр [shutdown_token](Server_settings/Searchd.md#shutdown_token) в секции searchd конфигурационного файла. Если такой секции нет или если предоставленный хэш пароля не совпадает с токеном, хранящимся в конфиге, подкоманды ничего не сделают.

```sql
mysql> debug token hello;
+-------------+------------------------------------------+
| command     | result                                   |
+-------------+------------------------------------------+
| debug token | aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d |
+-------------+------------------------------------------+
1 row in set (0,00 sec)
```

Подкоманда `shutdown` отправит серверу сигнал TERM, вызывая его завершение работы. Это может быть опасно, так как никто не хочет случайно остановить продакшен-сервис. Поэтому она требует VIP-подключения и пароля для использования.

Подкоманда `crash` буквально вызывает сбой. Она может использоваться для тестирования, например, чтобы проверить, как системный менеджер поддерживает живучесть сервиса или проверить возможность отслеживания coredump.

Если некоторые команды окажутся полезными в более общем контексте, их могут перенести из подкоманд debug в более стабильное и общее место (как показано на примере `debug tasks` и `debug sched` в таблице).

<!-- proofread -->

