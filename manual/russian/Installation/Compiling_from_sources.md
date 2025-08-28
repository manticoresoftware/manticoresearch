# Компиляция Manticore из исходников

Компиляция Manticore Search из исходных кодов позволяет задать индивидуальные параметры сборки, такие как отключение определённых функций или добавление новых патчей для тестирования. Например, вы можете захотеть скомпилировать из исходников и отключить встроенный ICU, чтобы использовать другую версию, установленную в вашей системе, которую можно обновлять независимо от Manticore. Это также полезно, если вы хотите внести свой вклад в проект Manticore Search.

## Сборка с использованием CI Docker
Для подготовки [официальных релизов и пакетов разработки](https://repo.manticoresearch.com/) мы используем Docker и специальный образ для сборки. Этот образ включает необходимые инструменты и предназначен для работы с внешними sysroot, поэтому один контейнер может собирать пакеты для всех операционных систем. Вы можете собрать образ, используя [Dockerfile](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/cross/external_toolchain/Dockerfile) и [README](https://github.com/manticoresoftware/manticoresearch/blob/master/dist/build_dockers/README.md) или использовать образ с [Docker Hub](https://hub.docker.com/r/manticoresearch/external_toolchain/tags). Это самый простой способ создать бинарные файлы для любой поддерживаемой операционной системы и архитектуры. При запуске контейнера вам также нужно указать следующие переменные окружения:

* `DISTR`: целевая платформа: `bionic`, `focal`, `jammy`, `buster`, `bullseye`, `bookworm`, `rhel7`, `rhel8`, `rhel9`, `rhel10`, `macos`, `windows`, `freebsd13`
* `arch`: архитектура: `x86_64`, `x64` (для Windows), `aarch64`, `arm64` (для Macos)
* `SYSROOT_URL`: URL-адрес архивов системных root. Вы можете использовать https://repo.manticoresearch.com/repository/sysroots, если не собираете sysroot самостоятельно (инструкции находятся [здесь](https://github.com/manticoresoftware/manticoresearch/tree/master/dist/build_dockers/cross/sysroots)).
* Используйте файлы workflow CI как пример, чтобы узнать другие необходимые переменные окружения:
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/pack_publish.yml
  - https://github.com/manticoresoftware/manticoresearch/blob/master/.github/workflows/build_template.yml

Для поиска возможных значений `DISTR` и `arch` вы можете использовать каталог https://repo.manticoresearch.com/repository/sysroots/roots_with_zstd/ в качестве справочника, так как он содержит sysroot для всех поддерживаемых комбинаций.

После этого сборка пакетов внутри Docker-контейнера сводится к вызову:

```bash
cmake -DPACK=1 /path/to/sources
cmake --build .
```

Например, чтобы создать пакет для Ubuntu Jammy, аналогичный официальной версии, предоставляемой командой Manticore Core, необходимо выполнить следующие команды в каталоге с исходниками Manticore Search. Этот каталог — корень клонированного репозитория с https://github.com/manticoresoftware/manticoresearch:

```bash
docker run -it --rm \
-e CACHEB="../cache" \
-e DIAGNOSTIC=1 \
-e PACK_ICUDATA=0 \
-e NO_TESTS=1 \
-e DISTR=jammy \
-e boost=boost_nov22 \
-e sysroot=roots_nov22 \
-e arch=x86_64 \
-e CTEST_CMAKE_GENERATOR=Ninja \
-e CTEST_CONFIGURATION_TYPE=RelWithDebInfo \
-e WITH_COVERAGE=0 \
-e SYSROOT_URL="https://repo.manticoresearch.com/repository/sysroots" \
-e HOMEBREW_PREFIX="" \
-e PACK_GALERA=0 \
-e UNITY_BUILD=1 \
-v $(pwd):/manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa \
manticoresearch/external_toolchain:vcpkg331_20250114 bash

# далее выполнить внутри docker shell
cd /manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/
mkdir build && cd build
cmake -DPACK=1 ..
export CMAKE_TOOLCHAIN_FILE=$(pwd)/dist/build_dockers/cross/linux.cmake
cmake --build .
# или если хотите собрать пакеты:
# cmake --build . --target package
```
Для некоторых случаев (например, Centos) требуется длинный путь к каталогу исходников, иначе сборка может завершиться неудачей.

Таким же образом можно собрать бинарные файлы или пакеты не только для популярных дистрибутивов Linux, но также для FreeBSD, Windows и macOS.

#### Сборка SRPM с помощью Docker

Также можно использовать тот же специальный образ docker для сборки SRPM:

```bash
docker run -it --rm \
-e CACHEB="../cache" \
-e DIAGNOSTIC=1 \
-e PACK_ICUDATA=0 \
-e NO_TESTS=1 \
-e DISTR=rhel8 \
-e boost=boost_rhel_feb17 \
-e sysroot=roots_nov22 \
-e arch=x86_64 \
-e CTEST_CMAKE_GENERATOR=Ninja \
-e CTEST_CONFIGURATION_TYPE=RelWithDebInfo \
-e WITH_COVERAGE=0 \
-e SYSROOT_URL="https://repo.manticoresearch.com/repository/sysroots" \
-e HOMEBREW_PREFIX="" \
-e PACK_GALERA=0 \
-e UNITY_BUILD=1 \
-v $(pwd):/manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa \
manticoresearch/external_toolchain:vcpkg331_20250114 bash

# далее выполнить внутри docker shell
cd /manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/
mkdir build && cd build
cmake -DPACK=1 ..
export CMAKE_TOOLCHAIN_FILE=$(pwd)/../dist/build_dockers/cross/linux.cmake
# Файл CPackSourceConfig.cmake теперь создается в каталоге сборки
cpack -G RPM --config ./CPackSourceConfig.cmake
```

Это создаст Source RPM (`.src.rpm` файл), содержащий весь исходный код.

#### Сборка бинарных RPM из SRPM

После создания SRPM вы можете использовать его для сборки полного набора бинарных RPM пакетов:

```bash
# Установите инструменты сборки и зависимости
dnf install -y rpm-build cmake gcc-c++ boost-devel epel-release

# Автоматически установите зависимости для сборки SRPM
dnf builddep -y manticore-*.src.rpm

# Соберите все бинарные RPM из SRPM
rpmbuild --rebuild manticore-*.src.rpm

# Найдите созданные пакеты
ls ~/rpmbuild/RPMS/*/manticore*
```

> ПРИМЕЧАНИЕ: **Чтобы создать RPM из SRPM, необходимо убедиться, что все зависимости, перечисленные в SRPM, полностью установлены, что может быть непросто.** SRPM всё ещё может быть полезен для:
> - Аудита процесса сборки или проверки исходных и spec-файлов
> - Внесения собственных модификаций или патчей в сборку
> - Понимания, как были созданы бинарные файлы
> - Соблюдения требований по лицензиям с открытым исходным кодом
## Сборка вручную
Компиляция Manticore без использования Docker для сборки **не рекомендуется**, но если вам нужно это сделать, вот что вам следует знать:
### Необходимые инструменты
* Компилятор C++
  * В Linux — можно использовать GNU (4.7.2 и выше) или Clang
  * В Windows — Microsoft Visual Studio 2019 и выше (достаточно Community Edition)
  * На macOS — Clang (из командных инструментов XCode, используйте `xcode-select --install` для установки).
* Bison, Flex — в большинстве систем они доступны как пакеты, а в Windows входят в состав фреймворка Cygwin.
* Cmake — используется на всех платформах (требуется версия 3.19 или выше)
### Получение исходников
#### Из git
Исходный код Manticore [размещён на GitHub](https://github.com/manticoresoftware/manticoresearch).
Чтобы получить исходный код, клонируйте репозиторий и затем переключитесь на нужную ветку или тег. Ветка `master` представляет основную ветку разработки. При выпуске создаётся тег с версией, например `3.6.0`, и запускается новая ветка для этого релиза, в данном случае `manticore-3.6.0`. Голову этой версионированной ветки после всех изменений используют в качестве исходника для сборки всех бинарных выпусков. Например, чтобы получить исходники версии 3.6.0, можно выполнить:
```bash
git clone https://github.com/manticoresoftware/manticoresearch.git
cd manticoresearch
git checkout manticore-3.6.0
```
#### Из архива
Вы можете скачать нужный код с GitHub, используя кнопку "Download ZIP". Подходят оба формата: .zip и .tar.gz.
```bash
wget -c https://github.com/manticoresoftware/manticoresearch/archive/refs/tags/3.6.0.tar.gz
tar -zxf 3.6.0.tar.gz
cd manticoresearch-3.6.0
```
### Настройка
Manticore использует CMake. Предположим, вы находитесь в корневом каталоге клонированного репозитория:
```bash
mkdir build && cd build
cmake ..
```
CMake обнаруживает доступные возможности и настраивает сборку в соответствии с ними. По умолчанию все возможности считаются включёнными, если они доступны. Скрипт также загружает и собирает некоторые внешние библиотеки, предполагая, что вы хотите их использовать. Неявно вы получаете поддержку максимального числа возможностей.
Вы также можете явно настроить сборку с помощью флагов и опций. Чтобы включить функцию `FOO`, добавьте `-DFOO=1` к вызову CMake.
Чтобы отключить её, используйте `-DFOO=0`. Если прямо не указано иначе, включение функции, которая недоступна((например, `WITH_GALERA` при сборке MS Windows)) приведёт к ошибке конфигурации. Отключение функции, помимо исключения её из сборки, также предотвращает её обнаружение в системе и отключает загрузку/сборку любых связанных внешних библиотек.
#### Флаги и опции конфигурации
- **USE_SYSLOG** — позволяет использовать `syslog` в [логировании запросов](../Logging/Query_logging.md).
- **WITH_GALERA** — включает поддержку репликации в демоне поиска. Поддержка будет настроена при сборке, а исходники библиотеки Galera будут загружены, собраны и включены в дистрибутив/установку. Обычно безопасно собирать с Galera, но не распространять саму библиотеку (то есть без модуля Galera и без репликации). Однако иногда может понадобиться явно отключить её, например, если вы хотите собрать статический бинарный файл, который по замыслу не может загружать библиотеки, так что даже наличие вызова функции 'dlopen' внутри демона приведёт к ошибке линковки.
- **WITH_RE2** — сборка с использованием библиотеки регулярных выражений RE2. Это необходимо для функций, таких как [REGEX()](../Functions/String_functions.md#REGEX%28%29) и [regexp_filter](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)
  функция.
- **WITH_RE2_FORCE_STATIC** — скачивает исходники RE2, компилирует их и линкует статически, чтобы итоговые бинарные файлы не зависели от наличия разделяемой библиотеки `RE2` в вашей системе.
- **WITH_STEMMER** — сборка с использованием библиотеки стемминга Snowball.
- **WITH_STEMMER_FORCE_STATIC** — скачивает исходники Snowball, компилирует их и линкует статически, чтобы итоговые бинарные файлы не зависели от наличия разделяемой библиотеки `libstemmer` в вашей системе.
- **WITH_ICU** — сборка с библиотекой ICU (International Components for Unicode). Она используется для сегментации китайского текста и применяется, когда задано morphology=`icu_chinese`.
- **WITH_JIEBA** — сборка с инструментом сегментации китайского текста Jieba. Он используется для сегментации китайского текста и применяется, когда задано morphology=`jieba_chinese`.
- **WITH_ICU_FORCE_STATIC** — скачивает исходники ICU, компилирует их и линкует статически, чтобы итоговые бинарные файлы не зависели от наличия разделяемой библиотеки `icu` в вашей системе. Кроме того, в установку/дистрибутив включается файл данных ICU. Цель статической линковки ICU — получить библиотеку известной версии, чтобы поведение было детерминированным и не зависело от системных библиотек. Скорее всего вы предпочтёте использовать системную ICU, так как она может обновляться со временем без необходимости пересобирать демон Manticore. В этом случае нужно явно отключить эту опцию. Это также сэкономит место, занимаемое файлом данных ICU (примерно 30M), поскольку он не будет включён в дистрибутив.
- **WITH_SSL** - Используется для поддержки HTTPS, а также зашифрованных соединений MySQL с демон. Системная библиотека OpenSSL будет связана с демоном. Это означает, что для запуска демона потребуется OpenSSL. Это обязательно для поддержки HTTPS, но не строго обязательно для сервера (т.е. отсутствие SSL означает невозможность подключения через HTTPS, но другие протоколы будут работать). Для Manticore могут использоваться версии SSL-библиотеки с 1.0.2 до 1.1.1, однако обратите внимание, что **в целях безопасности настоятельно рекомендуется использовать как можно более свежую версию SSL
  библиотеки**. На данный момент поддерживается только версия v1.1.1, остальные устарели (
  см. [стратегию релизов openssl](https://www.openssl.org/policies/releasestrat.html)
- **WITH_ZLIB** - используется индексатором для работы с сжатыми колонками из MySQL. Используется демоном для обеспечения поддержки сжатого протокола MySQL.
- **WITH_ODBC** - используется индексатором для поддержки индексирования источников из провайдеров ODBC (обычно это UnixODBC и iODBC). В MS Windows ODBC — это правильный способ работы с источниками MS SQL, поэтому индексирование `MSSQL` подразумевает этот флаг.
- **DL_ODBC** - не связываться с библиотекой ODBC. Если ODBC связана, но недоступна, нельзя запустить инструмент indexer, даже если хотите обрабатывать что-то не связанное с ODBC. Эта опция заставляет индексатор загружать библиотеку в рантайме только при работе с источником ODBC.
- **ODBC_LIB** - имя файла библиотеки ODBC. Индексатор попытается загрузить этот файл при работе с источником ODBC. Эта опция записывается автоматически на основании исследования доступных динамических библиотек ODBC. Также вы можете переопределить это имя в рантайме, задав переменную окружения `ODBC_LIB` с корректным путем к альтернативной библиотеке перед запуском индексатора.
- **WITH_EXPAT** - используется индексатором для поддержки индексирования источников xmlpipe.
- **DL_EXPAT** - не связываться с библиотекой EXPAT. Если EXPAT связана, но недоступна, нельзя запустить инструмент `indexer`, даже если хотите обрабатывать что-то не связанное с xmlpipe. Эта опция заставляет индексатор загружать библиотеку в рантайме только при работе с источником xmlpipe.
- **EXPAT_LIB** - имя файла библиотеки EXPAT. Индексатор попытается загрузить этот файл при работе с источником xmlpipe. Эта опция записывается автоматически на основании исследования доступных динамических библиотек EXPAT. Также вы можете переопределить это имя в рантайме, задав переменную окружения EXPAT_LIB с корректным путем к альтернативной библиотеке перед запуском индексатора.
- **WITH_ICONV** - для поддержки различных кодировок при индексировании источников xmlpipe с помощью индексатора.
- **DL_ICONV** - не связываться с библиотекой iconv. Если iconv связана, но недоступна, нельзя запустить инструмент `indexer`, даже если хотите обрабатывать что-то не связанное с xmlpipe. Эта опция заставляет индексатор загружать библиотеку в рантайме только при работе с источником xmlpipe.
- **ICONV_LIB** - имя файла библиотеки iconv. Индексатор попытается загрузить этот файл при работе с источником xmlpipe. Эта опция записывается автоматически на основании исследования доступных динамических библиотек iconv. Также вы можете переопределить это имя в рантайме, задав переменную окружения `ICONV_LIB` с корректным путем к альтернативной библиотеке перед запуском индексатора.
- **WITH_MYSQL** - используется индексатором для поддержки индексирования источников MySQL.
- **DL_MYSQL** - не связываться с библиотекой MySQL. Если MySQL связана, но недоступна, нельзя запустить инструмент `indexer`, даже если хотите обрабатывать что-то не связанное с MySQL. Эта опция заставляет индексатор загружать библиотеку в рантайме только при работе с источником MySQL.
- **MYSQL_LIB** -- имя файла библиотеки MySQL. Индексатор попытается загрузить этот файл при работе с источником MySQL. Эта опция записывается автоматически на основании исследования доступных динамических библиотек MySQL. Также вы можете переопределить это имя в рантайме, задав переменную окружения `MYSQL_LIB` с корректным путем к альтернативной библиотеке перед запуском индексатора.
- **WITH_POSTGRESQL** - используется индексатором для поддержки индексирования источников PostgreSQL.
- **DL_POSTGRESQL** - не связываться с библиотекой PostgreSQL. Если PostgreSQL связана, но недоступна, нельзя запустить инструмент `indexer`, даже если хотите обрабатывать что-то не связанное с PostgreSQL. Эта опция заставляет индексатор загружать библиотеку в рантайме только при работе с источником PostgreSQL.
- **POSTGRESQL_LIB** - имя файла библиотеки postgresql. Индексатор попытается загрузить указанный файл библиотеки postgresql при обработке источника postgresql. Эта опция автоматически определяется на основании исследования доступных динамических библиотек postgresql. Также вы можете переопределить имя в рантайме, задав переменную окружения `POSTGRESQL_LIB` с корректным путем к альтернативной библиотеке перед запуском индексатора.
- **LOCALDATADIR** - путь по умолчанию, где демон хранит binlog-файлы. Если этот путь не задан или явно отключен в конфигурации рантайма демона (т.е. в файле `manticore.conf`, который не является частью этой конфигурации сборки), binlog-файлы будут размещаться в этом пути. Обычно это абсолютный путь, однако разрешены и относительные пути. Вероятно, менять значение по умолчанию, определённое конфигурацией, вам не потребуется. В зависимости от целевой системы это может быть что-то вроде `/var/data`, `/var/lib/manticore/data` или `/usr/local/var/lib/manticore/data`.
- **FULL_SHARE_DIR** - путь по умолчанию, где размещены все ресурсы. Он может быть переопределён переменной окружения `FULL_SHARE_DIR` перед запуском любого инструмента, который использует файлы из этой папки. Это важный путь, так как в нём по умолчанию ожидается найти множество вещей. В том числе предопределённые таблицы символов, стоп-слова, модули manticore и файлы данных icu, все размещённые в этой папке. Скрипт конфигурации обычно определяет этот путь как что-то вроде `/usr/share/manticore` или `/usr/local/share/manticore`.
- **DISTR_BUILD** - сокращение для опций выпуска пакетов. Это строковое значение с названием целевой платформы. Его можно использовать вместо ручной настройки всех опций. В Debian и Redhat Linux значение по умолчанию может определяться лёгкой интуицией и устанавливаться в общее 'Debian' или 'RHEL'. В остальных случаях значение не определено.
- **PACK** - ещё более удобное сокращение. Оно считывает переменную окружения `DISTR`, присваивает её параметру **DISTR_BUILD**, а затем работает как обычно. Это очень полезно при сборке в подготовленных системах сборки, например, в Docker-контейнерах, где переменная `DISTR` установлена на уровне системы и отражает целевую систему, для которой предназначён контейнер.
- **CMAKE_INSTALL_PREFIX** (путь) - где ожидается установка Manticore. Сборка не выполняет установку, но готовит директивы установки, которые выполняются при запуске команды `cmake --install` или при создании пакета с последующей установкой. Префикс можно изменить в любое время, даже во время установки, вызвав
  `cmake --install . --prefix /path/to/installation`. Однако при конфигурировании эта переменная используется для инициализации значений по умолчанию для `LOCALDATADIR` и `FULL_SHARE_DIR`. Например, установка её в `/my/custom` на этапе конфигурации
  зафиксирует `LOCALDATADIR` как `/my/custom/var/lib/manticore/data`, а `FULL_SHARE_DIR` как
  `/my/custom/usr/share/manticore`.
- **BUILD_TESTING** (bool) поддержка тестирования. Если включено, после сборки можно запустить 'ctest' и протестировать сборку. Обратите внимание, что тестирование требует дополнительных зависимостей, таких как наличие PHP cli, Python и доступного сервера MySQL с тестовой базой данных. По умолчанию этот параметр включён. Поэтому для простой сборки возможно стоит явно указать значение 'off' для отключения.
- **BUILD_SRPMS** (bool) показывать инструкции по сборке исходных RPM-пакетов (SRPM). Из-за ограничений CPack с компонентно-ориентированной упаковкой, SRPM не могут быть сгенерированы напрямую вместе с бинарными RPM. При включении система сборки покажет инструкции для корректного создания SRPM с использованием метода исходной конфигурации. По умолчанию параметр выключен.
- **LIBS_BUNDLE** - путь к папке с разными библиотеками. В основном актуально для сборки под Windows, но может быть полезно и при частых сборках, чтобы избегать повторного скачивания сторонних исходников. По умолчанию этот путь скрипт конфигурации не меняет; всё нужно помещать туда вручную. Например, если требуется поддержка стеммера, исходники будут скачаны с главной страницы Snowball, затем распакованы, сконфигурированы, собраны и т.д. Вместо этого можно хранить исходный tarball (который `libstemmer_c.tgz`) в этой папке. При следующей сборке скрипт конфигурации сначала проверит этот каталог и, если найдёт стеммер, не будет скачивать его снова из Интернета.
- **CACHEB** - путь к папке со собранными сборками сторонних библиотек. Обычно это такие компоненты, как galera, re2, icu и т.д., которые сначала скачиваются или берутся из bundle, затем распаковываются, строятся и устанавливаются во временную внутреннюю папку. При сборке manticore эта папка используется как место, где находятся необходимые для поддержки требуемой функции компоненты. В итоге они либо линкуются с manticore, если это библиотека, либо идут напрямую в дистрибутив/установку (например, данные galera или icu). Если **CACHEB** задано либо как параметр конфигурации cmake, либо как системная переменная окружения, оно используется как целевая папка для этих сборок. Эту папку можно хранить между сборками, чтобы библиотеки в ней не собирались заново, значительно сокращая время сборки.

Обратите внимание, что некоторые опции организованы троичками: `WITH_XXX`, `DL_XXX` и `XXX_LIB` — например, поддержка mysql, odbc и т.п. `WITH_XXX` определяет, влияют ли следующие две опции. Т.е. если установить `WITH_ODBC` в `0`, то нет смысла указывать `DL_ODBC` и `ODBC_LIB`, поскольку они не будут учитываться при отключённой функции. Также `XXX_LIB` не имеет смысла без `DL_XXX`, потому что если опция `DL_XXX` выключена, динамическая загрузка не используется, и указанное имя по `XXX_LIB` бесполезно. Это учитывается по умолчанию в интроспекции.

Кроме того, использование библиотеки `iconv` предполагает `expat` и не имеет смысла, если последний отключён.

Также некоторые библиотеки могут быть всегда доступны, поэтому нет смысла избегать их линковки. Например, в Windows это ODBC. В macOS — Expat, iconv и, возможно, другие. Интроспекция по умолчанию определяет такие библиотеки и фактически выводит только `WITH_XXX` для них, без `DL_XXX` и `XXX_LIB`, что упрощает настройку.

Конфигурация с некоторыми опциями может выглядеть так:

```bash
mkdir build && cd build
cmake -DWITH_MYSQL=1 -DWITH_RE2=1 ..
```

Помимо основных значений конфигурации, вы можете изучить файл `CMakeCache.txt`, который остаётся в папке сборки сразу после запуска конфигурации. Любые значения, определённые там, могут быть явно переопределены при вызове cmake. Например, можно запустить `cmake -DHAVE_GETADDRINFO_A=FALSE ...`, и эта конфигурация не будет использовать исследуемое значение переменной, а применит то, что вы указали.

#### Специфичные переменные окружения

Переменные окружения полезны для предоставления глобальных настроек, которые хранятся отдельно от конфигурации сборки и всегда присутствуют. Для постоянства их можно установить глобально в системе разными способами — например, добавить в файл `.bashrc`, или встроить в Dockerfile при создании docker-образа, или прописать в системных переменных окружения Windows. Также можно установить их временно с помощью `export VAR=value` в shell. Или ещё короче — перед вызовом cmake, например, так: `CACHEB=/my/cache cmake ...` — тогда переменная будет видна только в этом вызове и не отразится на следующих.
Некоторые из таких переменных известны как общие для cmake и некоторых других инструментов. Это такие вещи, как `CXX`, которая определяет текущий компилятор C++, или `CXX_FLAGS` для предоставления флагов компилятора и т.д.

Тем не менее, у нас есть некоторые переменные, которые специфичны для конфигурации manticore, которые были придуманы исключительно для наших сборок.

- **CACHEB** - то же самое, что и параметр конфигурации **CACHEB**
- **LIBS_BUNDLE** - то же самое, что и параметр конфигурации **LIBS_BUNDLE**
- **DISTR** - используется для инициализации параметра `DISTR_BUILD`, когда используется `-DPACK=1`.
- **DIAGNOSTIC** - делает вывод конфигурации cmake намного более подробным, объясняя все происходящее
- **WRITEB** - предполагает **LIBS_BUNDLE** и, если установлено, будет загружать архивы исходных файлов для различных инструментов в папку LIBS_BUNDLE. То есть, если выйдет свежая версия стеммера - вы можете вручную удалить libstemmer_c.tgz из пакета, а затем выполнить однократный `WRITEB=1 cmake ...` - он не найдет исходники стеммера в пакете и затем загрузит их с сайта поставщика в пакет (без WRITEB они будут загружены во временную папку внутри сборки и исчезнут, когда вы очистите папку сборки).

В конце конфигурации вы можете увидеть, что доступно и будет использоваться в списке, подобном этому:

```
-- Включенные функции, скомпилированные в:
* Galera, репликация таблиц
* re2, библиотека регулярных выражений
* stemmer, библиотека стемминга (Snowball)
* icu, Международные компоненты для Unicode
* OpenSSL, для зашифрованной сети
* ZLIB, для сжатых данных и сети
* ODBC, для индексации MSSQL (windows) и общих источников ODBC с индексатором
* EXPAT, для индексации источников xmlpipe с индексатором
* Iconv, для поддержки различных кодировок при индексации источников xmlpipe с индексатором
* MySQL, для индексации источников MySQL с индексатором
* PostgreSQL, для индексации источников PostgreSQL с индексатором
```

### Сборка

```bash
cmake --build . --config RelWithDebInfo
```

### Установка

Чтобы установить, выполните:

```bash
cmake --install . --config RelWithDebInfo
```

чтобы установить в пользовательскую (не по умолчанию) папку, выполните

```bash
cmake --install . --prefix path/to/build --config RelWithDebInfo
```

### Сборка пакетов

Для сборки пакета используйте цель `package`. Она создаст пакет в соответствии с выбором, установленным с помощью параметра `-DDISTR_BUILD`. По умолчанию это будет простой архив .zip или .tgz со всеми бинарными и сопутствующими файлами.

```bash
cmake --build . --target package --config RelWithDebInfo
```

## Некоторые продвинутые вещи о сборке

### Пере компиляция (обновление) на одноконфигурируемой

Если вы не изменяли путь к исходникам и сборке, просто перейдите в папку сборки и выполните:

```bash
cmake .
cmake --build . --clean-first --config RelWithDebInfo
```

Если по какой-то причине это не работает, вы можете удалить файл `CMakeCache.txt`, находящийся в папке сборки. После этого шага вы
должны снова запустить cmake, указав папку с исходниками и настроив параметры.

Если это также не помогает, просто очистите свою папку сборки и начните с нуля.

### Типы сборки

Кратко - просто используйте `--config RelWithDebInfo`, как написано выше. Это не приведет к ошибкам.

Мы используем два типа сборки. Для разработки это `Debug` - он задает флаги компилятора для оптимизации и других вещей так, что это очень удобно для разработки, что означает, что отладка выполняется пошагово. Однако полученные бинарные файлы довольно большие и медленные для производства.

Для релиза мы используем другой тип - `RelWithDebInfo` - что означает 'релизная сборка с отладочной информацией'. Она создает продукционные бинарные файлы с встроенной отладочной информацией. Последняя затем разделяется на отдельные пакеты debuginfo, которые хранятся отдельно от пакетов релиза и могут быть использованы в случае некоторых проблем, таких как сбои - для расследования и исправления ошибок. Cmake также предоставляет `Release` и `MinSizeRel`, но мы их не используем. Если тип сборки недоступен, cmake выполнит сборку `noconfig`.

#### Генераторы систем сборки

Существуют два типа генераторов: одноконфигурируемый и многоконфигурируемый.

- Одноконфигурируемый требует указания типа сборки во время конфигурации, через параметр `CMAKE_BUILD_TYPE`. Если он не определен, сборка вернется к типу `RelWithDebInfo`, который подходит, если вы просто хотите собрать Manticore из исходников и не участвовать в разработке. Для явных сборок вы должны указать тип сборки, например `-DCMAKE_BUILD_TYPE=Debug`.
- Многоконфигурируемый выбирает тип сборки во время сборки. Его следует указать с опцией `--config`, иначе он создаст что-то вроде `noconfig`, что нежелательно. Поэтому вы всегда должны указывать тип сборки, например `--config Debug`.

Если вы хотите указать тип сборки, но не хотите беспокоиться о том, является ли он 'одноконфигурируемым' или 'многоконфигурируемым' генератором - просто укажите необходимые ключи в обоих местах. То есть, настроить с `-DCMAKE_BUILD_TYPE=Debug`, а затем собрать с `--config Debug`. Просто убедитесь, что оба значения одинаковы. Если целевой сборщик является одноконфигурируемым, он примет параметр конфигурации. Если это многоконфигурируемый, параметр конфигурации будет проигнорирован, но правильная конфигурация сборки будет выбрана по ключу `--config`.

Если вы хотите `RelWithDebInfo` (т.е. просто собрать для производства) и знаете, что вы находитесь на платформе одноконфигурирования (то есть всё, кроме Windows) - вы можете пропустить флаг `--config` при вызове cmake. По умолчанию будет настроен `CMAKE_BUILD_TYPE=RelWithDebInfo`, и он будет использован. Все команды для 'сборки', 'установки' и 'сборки пакета' тогда станут короче.

#### Явно выбирайте генераторы систем сборки

Cmake - это инструмент, который не выполняет сборку сам по себе, но генерирует правила для локальной системы сборки.
Обычно он хорошо определяет доступную систему сборки, но иногда вам может понадобиться явно указать генератор. Вы
can run `cmake -G` and review the list of available generators.

- On Windows, if you have more than one version of Visual Studio installed, you might need to specify which one to use,
as:
```bash
cmake -G "Visual Studio 16 2019" ....
  ```
- On all other platforms - usually Unix Makefiles are used, but you can specify another one, such as Ninja, or Ninja Multi-Config, as:
  Multi-Config`, as:
```bash
  cmake -GNinja ...
  ```
  or
```bash
  cmake -G"Ninja Multi-Config" ...
```
Ninja Multi-Config is quite useful as it is really 'multi-config' and available on Linux/macOS/BSD. With this generator, you may shift the choosing of configuration type to build time, and also you may build several configurations in one and the same build folder, changing only the  `--config` param.

### Caveats

1. If you want to finally build a full-featured RPM package, the path to the build directory must be long enough in order to correctly build debug symbols.
Like `/manticore012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789`, for example. That is because RPM tools modify the path over compiled binaries when building debug info, and it can just write over existing room and won't allocate more. The aforementioned long path has 100 chars and that is quite enough for such a case.

## External dependencies

Some libraries should be available if you want to use them.
- For indexing (`indexer` tool): `expat`, `iconv`, `mysql`, `odbc`, `postgresql`. Without them, you can only process `tsv` and `csv` sources.
- For serving queries (`searchd` daemon): `openssl` might be necessary.
- For all (required, mandatory!) we need the Boost library. The minimal version is 1.61.0, however, we build the binaries with a fresher version 1.75.0. Even more recent versions (like 1.76) should also be okay. On Windows, you can download pre-built Boost from their site (boost.org) and install it into the default suggested path (i.e. `C:\\boost...`). On MacOs, the one provided in brew is okay. On Linux, you can check the available version in official repositories, and if it doesn't match requirements, you can build from sources. We need the component 'context', you can also build components 'system' and 'program_options', they will be necessary if you also want to build Galera library from the sources. Look into `dist/build_dockers/xxx/boost_175/Dockerfile` for a short self-documented script/instruction on how to do it.

On the build system, you need the 'dev' or 'devel' versions of these packages installed (i.e. - libmysqlclient-devel, unixodbc-devel, etc. Look to our dockerfiles for the names of concrete packages).

On run systems, these packages should be present at least in the final (non-dev) variants. (devel variants usually larger, as they include not only target binaries, but also different development stuff like include headers, etc.).

### Building on Windows

Apart from necessary prerequisites, you might need prebuilt `expat`, `iconv`, `mysql`, and `postgresql` client libraries. You have to either build them yourself or contact us to get our build bundle (a simple zip archive where the folder with these targets is located).

- ODBC is not necessary as it is a system library.
- OpenSSL might be built from sources or downloaded prebuilt from https://slproweb.com/products/Win32OpenSSL.html (as mentioned in the cmake internal script on FindOpenSSL).
- Boost might be downloaded pre-built from https://www.boost.org/ releases.

### See what is compiled

Run `indexer -h`. It will show which features were configured and built (whether they're explicit or investigated, doesn't matter):

```
Built on Linux x86_64 by GNU 8.3.1 compiler.

Configured with these definitions: -DDISTR_BUILD=rhel8 -DUSE_SYSLOG=1 -DWITH_GALERA=1 -DWITH_RE2=1 -DWITH_RE2_FORCE_STATIC=1
-DWITH_STEMMER=1 -DWITH_STEMMER_FORCE_STATIC=1 -DWITH_ICU=1 -DWITH_ICU_FORCE_STATIC=1 -DWITH_SSL=1 -DWITH_ZLIB=1 -DWITH_ODBC=1 -DDL_ODBC=1
-DODBC_LIB=libodbc.so.2 -DWITH_EXPAT=1 -DDL_EXPAT=1 -DEXPAT_LIB=libexpat.so.1 -DWITH_ICONV=1 -DWITH_MYSQL=1 -DDL_MYSQL=1
-DMYSQL_LIB=libmariadb.so.3 -DWITH_POSTGRESQL=1 -DDL_POSTGRESQL=1 -DPOSTGRESQL_LIB=libpq.so.5 -DLOCALDATADIR=/var/lib/manticore/data
-DFULL_SHARE_DIR=/usr/share/manticore
```
<!-- proofread -->





















