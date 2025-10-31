# Изменения

## Версия 13.13.0
**Выпущено**: 7 октября 2025 года

### Рекомендуемые библиотеки
- Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 8.1.0
- Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.35.1

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), вам не нужно об этом беспокоиться.

### ⚠️ Важно

Поскольку файл конфигурации был обновлен, **вы можете увидеть предупреждение во время обновления на Linux с вопросом, сохранить ли вашу версию или использовать новую из пакета**. Если у вас есть нестандартная (пользовательская) конфигурация, рекомендуется сохранить вашу версию и обновить путь к `pid_file` на `/run/manticore/searchd.pid`. Тем не менее, все должно работать нормально, даже если вы не измените путь.

### Новые функции и улучшения
* 🆕 [v13.13.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.13.0) Добавлена поддержка MCL 8.1.0 с SI block cache.
* 🆕 [v13.12.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.12.0) Реализована опция [secondary_index_block_cache](../Server_settings/Searchd.md#secondary_index_block_cache), обновлено API вторичного индекса, и встроены аксессоры сортировки.

### Исправления ошибок
* 🪲 [v13.11.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.8) [ Issue #3791](https://github.com/manticoresoftware/manticoresearch/issues/3791) Исправлена гонка между проверкой и вызовом сработавшего таймера.
* 🪲 [v13.11.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.7) [ Issue #1045](https://github.com/manticoresoftware/manticoresearch/issues/1045) Исправлено предупреждение systemctl на RHEL 8 при обновлениях systemd путем замены устаревшего пути `/var/run/manticore` на правильный `/run/manticore` в конфигурации. Поскольку файл конфигурации был обновлен, вы можете увидеть предупреждение во время обновления с вопросом, сохранить ли вашу версию или использовать новую из пакета. Если у вас есть нестандартная (пользовательская) конфигурация, рекомендуется сохранить вашу версию и обновить путь к `pid_file` на `/run/manticore/searchd.pid`.
* 🪲 [v13.11.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.6) [ PR #3766](https://github.com/manticoresoftware/manticoresearch/pull/3766) Добавлена поддержка версии MCL 8.0.6.
* 🪲 [v13.11.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.5) [ PR #3767](https://github.com/manticoresoftware/manticoresearch/pull/3767) Улучшены переводы документации на китайский язык и обновлены подмодули.
* 🪲 [v13.11.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.4) [ PR #3765](https://github.com/manticoresoftware/manticoresearch/pull/3765) Исправлена обработка псевдонимов связанных атрибутов.
* 🪲 [v13.11.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.3) [ PR #3763](https://github.com/manticoresoftware/manticoresearch/pull/3763) Исправлен сбой, который мог возникать при пакетных соединениях по строковым атрибутам, и решена проблема, когда фильтры иногда не работали с LEFT JOIN.
* 🪲 [v13.11.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.2) [ Issue #3065](https://github.com/manticoresoftware/manticoresearch/issues/3065) Исправлен сбой при вставке данных в столбцовую таблицу с включенным index_field_lengths.
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [ Issue #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) Исправлен сбой, происходивший при удалении документа с включенными embedding.

## Версия 13.11.1
**Выпущено**: 13 сентября 2025 года

### Исправления ошибок
* 🪲 [v13.11.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.1) [ Issue #3751](https://github.com/manticoresoftware/manticoresearch/issues/3751) Исправлен сбой при удалении документа с включенными embedding.

## Версия 13.11.0
**Выпущено**: 13 сентября 2025 года

Главной особенностью этого релиза является [Auto Embeddings](Searching/KNN.md#Auto-Embeddings-%28Recommended%29) — новая функция, которая делает семантический поиск таким же простым, как SQL.
Нет необходимости во внешних сервисах или сложных конвейерах: просто вставляйте текст и ищите с помощью естественного языка.

### Что предлагает Auto Embeddings

- **Автоматическое создание embedding** непосредственно из вашего текста
- **Запросы на естественном языке**, которые понимают смысл, а не только ключевые слова
- **Поддержка нескольких моделей** (OpenAI, Hugging Face, Voyage, Jina)
- **Бесшовная интеграция** с SQL и JSON API

### Рекомендуемые библиотеки
- Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 8.0.3
- Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.35.1

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), вам не нужно об этом беспокоиться.

### Новые функции и улучшения
* 🆕 [v13.11.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.11.0) [ PR #3746](https://github.com/manticoresoftware/manticoresearch/pull/3746) Добавлена поддержка "query" в JSON-запросах для генерации embedding.
* 🆕 [v13.10.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.0) [ Issue #3709](https://github.com/manticoresoftware/manticoresearch/issues/3709) RPM-пакет manticore-server больше не владеет `/run`.
* 🆕 [v13.9.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.0) [ PR #3716](https://github.com/manticoresoftware/manticoresearch/pull/3716) Добавлена поддержка `boolean_simplify` в конфигурации.
* 🆕 [v13.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.0) [ Issue #3253](https://github.com/manticoresoftware/manticoresearch/issues/3253) Установлена конфигурация sysctl для увеличения vm.max_map_count для больших наборов данных.
* 🆕 [v13.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.0) [ PR #3681](https://github.com/manticoresoftware/manticoresearch/pull/3681) Добавлена поддержка `alter table <table> modify column <column> api_key=<key>`.

### Исправления ошибок
* 🪲 [v13.10.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.5) [ PR #3737](https://github.com/manticoresoftware/manticoresearch/pull/3737) Опция scroll теперь корректно возвращает все документы с большими 64-битными ID.
* 🪲 [v13.10.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.4) [ PR #3736](https://github.com/manticoresoftware/manticoresearch/pull/3736) Исправлен сбой при использовании KNN с фильтровым деревом.
* 🪲 [v13.10.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.3) [ Issue #3520](https://github.com/manticoresoftware/manticoresearch/issues/3520) Эндпоинт `/sql` больше не разрешает команду SHOW VERSION.
* 🪲 [v13.10.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.2) [ PR #3637](https://github.com/manticoresoftware/manticoresearch/pull/3637) Обновлен скрипт установки для Windows.
* 🪲 [v13.10.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.10.1) Исправлено определение локального часового пояса в Linux.
* 🪲 [v13.9.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.2) [ PR #3726](https://github.com/manticoresoftware/manticoresearch/pull/3726) Дубликаты ID в колоночном режиме теперь корректно возвращают ошибку.
* 🪲 [v13.9.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.9.1) [ PR #3333](https://github.com/manticoresoftware/manticoresearch/pull/3333) Руководство теперь автоматически переводится.
* 🪲 [v13.8.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.6) [ PR #3715](https://github.com/manticoresoftware/manticoresearch/pull/3715) Исправлена ошибка генерации эмбеддингов, когда все документы в батче были пропущены.
* 🪲 [v13.8.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.5) [ PR #3711](https://github.com/manticoresoftware/manticoresearch/pull/3711) Добавлены модели эмбеддингов Jina и Voyage, а также другие связанные с автоматическими эмбеддингами изменения.
* 🪲 [v13.8.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.4) [ PR #3710](https://github.com/manticoresoftware/manticoresearch/pull/3710) Исправлен сбой при джойнах с несколькими фасетами.
* 🪲 [v13.8.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.3) Исправлена проблема, когда коммиты delete/update в транзакции с несколькими запросами на _bulk эндпоинте не учитывались как ошибки.
* 🪲 [v13.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.2) [ PR #3705](https://github.com/manticoresoftware/manticoresearch/pull/3705) Исправлен сбой при джойне по неколоночным строковым атрибутам и улучшено сообщение об ошибках.
* 🪲 [v13.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.8.1) [ PR #3704](https://github.com/manticoresoftware/manticoresearch/pull/3704) Исправлен сбой в query embeddings при отсутствии указанной модели; добавлена строка embeddings в коммуникацию master-agent; добавлены тесты.
* 🪲 [v13.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.2) [ PR #Buddy#589](https://github.com/manticoresoftware/manticoresearch-buddy/pull/589) Убран дефолтный хак IDF для нечеткого поиска.
* 🪲 [v13.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.7.1) [ Issue #3454](https://github.com/manticoresoftware/manticoresearch/issues/3454) Исправлено неправильное декодирование scroll с большими 64-битными ID.
* 🪲 [v13.6.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.9) [ Issue #3674](https://github.com/manticoresoftware/manticoresearch/issues/3674) Исправлена проблема с драйвером JDBC+MySQL и пулом соединений при использовании настройки transaction_read_only.
* 🪲 [v13.6.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.8) [ PR #3676](https://github.com/manticoresoftware/manticoresearch/pull/3676) Исправлен сбой при пустом наборе результатов, возвращаемом моделью эмбеддингов.

## Версия 13.6.7
**Дата выпуска**: 8 августа 2025

Рекомендуемая версия [MCL](https://github.com/manticoresoftware/columnar): 8.0.1
Рекомендуемая версия [Buddy](Installation/Manticore_Buddy.md#Manticore-Buddy): 3.34.2

Если вы следуете [официальному руководству по установке](https://manticoresearch.com/install/), вам не нужно об этом беспокоиться.

### Новые возможности и улучшения
* 🆕 [v13.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.0) [ Issue #2226](https://github.com/manticoresoftware/manticoresearch/issues/2226) Поддержка явного '|' (ИЛИ) в операторах PHRASE, PROXIMITY и QUORUM.
* 🆕 [v13.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.0) [ PR #3591](https://github.com/manticoresoftware/manticoresearch/pull/3591) Автоматическая генерация эмбеддингов в запросах (работа в процессе, еще не для промышленного использования).
* 🆕 [v13.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.0) [ PR #3585](https://github.com/manticoresoftware/manticoresearch/pull/3585) Исправлена логика выбора количества потоков buddy из конфигурации buddy_path, если она установлена, вместо использования значения демона.
* 🆕 [v13.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.0) [ PR #3577](https://github.com/manticoresoftware/manticoresearch/pull/3577) Поддержка джойнов с локальными распределёнными таблицами.
* 🆕 [#3647](https://github.com/manticoresoftware/manticoresearch/issues/3647) Добавлена поддержка Debian 13 "Trixie"

### Исправления ошибок
* 🪲 [v13.6.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.7) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) Исправлена проблема с сохранением сгенерированных эмбеддингов в построчном хранении.
* 🪲 [v13.6.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.6) [ Issue #3563](https://github.com/manticoresoftware/manticoresearch/issues/3563) Исправлены проблемы с Sequel Ace и другими интеграциями, приводившие к ошибкам "unknown sysvar".
* 🪲 [v13.6.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.5) [ Issue #3467](https://github.com/manticoresoftware/manticoresearch/issues/3467) Исправлены проблемы с DBeaver и другими интеграциями, приводившие к ошибкам "unknown sysvar".
* 🪲 [v13.6.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.4) [ Issue #3524](https://github.com/manticoresoftware/manticoresearch/issues/3524) Исправлена ошибка с конкатенацией эмбеддингов из нескольких полей; также исправлено создание эмбеддингов из запросов.
* 🪲 [v13.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.3) [ Issue #3641](https://github.com/manticoresoftware/manticoresearch/issues/3641) Исправлена ошибка в версии 13.6.0, когда фраза теряла все ключевые слова в скобках кроме первого.
* 🪲 [v13.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.2) Исправлена утечка памяти в transform_phrase.
* 🪲 [v13.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.6.1) Исправлена утечка памяти в версии 13.6.0.
* 🪲 [v13.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.2) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) Исправлены дополнительные проблемы, связанные с fuzz-тестированием полнотекстового поиска.
* 🪲 [v13.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.5.1) [ Issue #3560](https://github.com/manticoresoftware/manticoresearch/issues/3560) Исправлен случай, когда OPTIMIZE TABLE мог зависнуть навсегда при работе с данными KNN.
* 🪲 [v13.4.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.2) [ Issue #2544](https://github.com/manticoresoftware/manticoresearch/issues/2544) Исправлена проблема, когда добавление столбца float_vector могло повредить индексы.
* 🪲 [v13.4.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.4.1) [ Issue #3651](https://github.com/manticoresoftware/manticoresearch/issues/3651) Добавлено fuzz-тестирование для разбора полнотекстовых запросов и исправлены несколько найденных проблем.
* 🪲 [v13.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.3.1) [ Issue #3583](https://github.com/manticoresoftware/manticoresearch/issues/3583) Исправлен сбой при использовании сложных булевых фильтров с подсветкой.
* 🪲 [v13.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.7) [ Issue #3481](https://github.com/manticoresoftware/manticoresearch/issues/3481) Исправлен сбой при одновременном использовании HTTP update, распределенных таблиц и неверного кластера репликации.
* 🪲 [v13.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.6) [ PR #3567](https://github.com/manticoresoftware/manticoresearch/pull/3567) Обновлена зависимость manticore-backup до версии 1.9.6.
* 🪲 [v13.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.5) [ PR #3565](https://github.com/manticoresoftware/manticoresearch/pull/3565) Исправлена настройка CI для улучшения совместимости Docker-образа.
* 🪲 [v13.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.4) Исправлена обработка длинных токенов. Некоторые специальные токены (например, regex-шаблоны) могли создавать слишком длинные слова, теперь они сокращаются перед использованием.

## Версия 13.2.3
**Выпущено**: 8 июля 2025

### Ломающие изменения
* ⚠️ [PR #3586](https://github.com/manticoresoftware/manticoresearch/pull/3586) Поддержка Debian 10 (Buster) прекращена. Debian 10 достиг конца жизненного цикла 30 июня 2024 года. Пользователям рекомендуется обновиться до Debian 11 (Bullseye) или Debian 12 (Bookworm).
* ⚠️ [v13.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.0.0) Обновлён API библиотеки KNN для поддержки пустых значений [float_vector](https://manual.manticoresearch.com/Creating_a_table/Data_types#Float-vector). Это обновление не меняет формат данных, но увеличивает версию API Manticore Search / MCL.
* ⚠️ [v12.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.0) [ PR #3516](https://github.com/manticoresoftware/manticoresearch/pull/3516) Исправлена ошибка с неправильными ID исходных и целевых строк при обучении и построении KNN-индекса. Это обновление не меняет формат данных, но увеличивает версию API Manticore Search / MCL.
* ⚠️ [v11.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.0) Добавлена поддержка новых функций векторного поиска KNN, таких как квантование, пересчет оценок и oversampling. Эта версия меняет формат данных KNN и синтаксис SQL [KNN_DIST()](https://manual.manticoresearch.com/Searching/KNN?client=SQL#KNN-vector-search). Новая версия может читать старые данные, но старые версии не смогут прочитать новый формат.

### Новые функции и улучшения
* 🆕 [v13.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.0) [ PR #3549](https://github.com/manticoresoftware/manticoresearch/issues/3525) Исправлена проблема с `@@collation_database`, вызывавшая несовместимость с некоторыми версиями mysqldump
* 🆕 [v13.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.1.0) [ Issue #3489](https://github.com/manticoresoftware/manticoresearch/issues/3489) Исправлена ошибка в Fuzzy Search, мешавшая парсингу некоторых SQL-запросов
* 🆕 [v12.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.1.0) [ Issue #3426](https://github.com/manticoresoftware/manticoresearch/issues/3426) Добавлена поддержка операционных систем RHEL 10
* 🆕 [v11.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.1.0) Добавлена поддержка пустых float-векторов в [KNN поиске](https://manual.manticoresearch.com/Searching/KNN#KNN-vector-search)
* 🆕 [v10.2.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.0) [ Issue #3252](https://github.com/manticoresoftware/manticoresearch/issues/3252) [log_level](https://manual.manticoresearch.com/Server_settings/Setting_variables_online#SET) теперь также управляет уровнем детализации логов Buddy

### Исправления ошибок
* 🪲 [v13.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.3) [ PR #3556](https://github.com/manticoresoftware/manticoresearch/pull/3556) Исправлен парсинг параметра "oversampling" в JSON-запросах
* 🪲 [v13.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.2) Исправлена ошибка при логировании сбоев на Linux и FreeBSD путем удаления использования Boost stacktrace
* 🪲 [v13.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/13.2.1) [ Issue #3298](https://github.com/manticoresoftware/manticoresearch/issues/3298) Исправлен сбой логирования при запуске внутри контейнера
* 🪲 [v12.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.2)  Улучшена точность статистики по таблицам за счет учёта в микросекундах
* 🪲 [v12.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/12.0.1) [ PR #3522](https://github.com/manticoresoftware/manticoresearch/pull/3522) Исправлен сбой при фасетировании по MVA в объединённом запросе
* 🪲 [v11.0.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.3) [ Issue #3502](https://github.com/manticoresoftware/manticoresearch/issues/3502) Исправлен сбой, связанный с квантованием векторного поиска
* 🪲 [v11.0.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.2) [ Issue #3493](https://github.com/manticoresoftware/manticoresearch/issues/3493) Изменено отображение CPU в SHOW THREADS на целочисленное
* 🪲 [v11.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/11.0.1)  Исправлены пути для колонночных и вторичных библиотек
* 🪲 [v10.2.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.7)  Добавлена поддержка MCL 5.0.5, включая исправление имени файла библиотеки эмбеддингов
* 🪲 [v10.2.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.6) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Применено ещё одно исправление, связанное с issue #3469
* 🪲 [v10.2.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.4) [ Issue #3469](https://github.com/manticoresoftware/manticoresearch/issues/3469) Исправлена проблема, при которой HTTP-запросы с bool-запросами возвращали пустые результаты
* 🪲 [v10.2.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.5)  Изменено имя файла по умолчанию для библиотеки эмбеддингов и добавлены проверки поля 'from' в KNN-векторах
* 🪲 [v10.2.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.3) [ PR #3464](https://github.com/manticoresoftware/manticoresearch/pull/3464) Обновлён Buddy до версии 3.30.2, включающее [PR #565 по использованию памяти и логированию ошибок](https://github.com/manticoresoftware/manticoresearch-buddy/pull/565)
* 🪲 [v10.2.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.2) [ Issue #3470](https://github.com/manticoresoftware/manticoresearch/pull/3470) Исправлены фильтры JSON строк, null-фильтры и вопросы сортировки в JOIN-запросах
* 🪲 [v10.2.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.2.1)  Исправлена ошибка в `dist/test_kit_docker_build.sh`, из-за которой отсутствовал коммит Buddy
* 🪲 [v10.1.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.4)  Исправлен сбой при группировке по MVA в объединённом запросе
* 🪲 [v10.1.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.3) [ Issue #3434](https://github.com/manticoresoftware/manticoresearch/issues/3434) Исправлена ошибка фильтрации пустых строк
* 🪲 [v10.1.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.2) [ PR #3452](https://github.com/manticoresoftware/manticoresearch/pull/3452) Обновлён Buddy до версии 3.29.7, решающей [Buddy #507 - ошибка при мультизапросах с нечетким поиском](https://github.com/manticoresoftware/manticoresearch-buddy/issues/507) и [Buddy #561 - исправление для метрик rate](https://github.com/manticoresoftware/manticoresearch-buddy/pull/561), требуется для [Helm Release 10.1.0](https://github.com/manticoresoftware/manticoresearch-helm/releases/tag/manticoresearch-10.1.0)
* 🪲 [v10.1.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.1)  Обновлён Buddy до версии 3.29.4, решая [#3388 - "Неопределённый ключ массива 'Field'"](https://github.com/manticoresoftware/manticoresearch/issues/3388) и [Buddy #547 - layouts='ru' может не работать](https://github.com/manticoresoftware/manticoresearch-buddy/issues/547)

## Версия 10.1.0
**Выпущена**: 9 июня 2025 года

Эта версия представляет собой обновление с новыми возможностями, одним ломаюшим изменением и многочисленными улучшениями стабильности и исправлениями ошибок. Изменения направлены на расширение возможностей мониторинга, улучшение функционала поиска и исправление различных критических проблем, влияющих на стабильность и производительность системы.

**Начиная с версии 10.1.0, поддержка CentOS 7 прекращена. Пользователям рекомендуется перейти на поддерживаемую операционную систему.**

### Ломаюшие изменения
* ⚠️ [v10.0.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.0) [ Issue #540](https://github.com/manticoresoftware/manticoresearch-buddy/issues/540) ЛОМАНУТЫЙ CHANGE: установлено `layouts=''` по умолчанию для [нечеткого поиска](Searching/Spell_correction.md#Fuzzy-Search)

### Новые возможности и улучшения
* 🆕 [v10.1.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.1.0) [ Issue #537](https://github.com/manticoresoftware/manticoresearch-buddy/issues/537) Добавлен встроенный [экспортер Prometheus](Node_info_and_management/Node_status.md#Prometheus-Exporter)
* 🆕 [v9.8.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.0) [ Issue #3409](https://github.com/manticoresoftware/manticoresearch/issues/3409) Добавлена команда [ALTER TABLE tbl REBUILD KNN](Updating_table_schema_and_settings.md#Rebuilding-a-KNN-index)
* 🆕 [v9.7.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.0) [ Issue #1778](https://github.com/manticoresoftware/manticoresearch/issues/1778) Добавлена автоматическая генерация эмбеддингов (официально пока не объявляется, так как код в основной ветке, но требует дополнительного тестирования)
* 🆕 [v9.6.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.0)  Повышена версия API KNN для поддержки автоэмбеддингов
* 🆕 [v9.5.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.0) [ Issue #1894](https://github.com/manticoresoftware/manticoresearch/issues/1894) Улучшено восстановление кластера: периодическое сохранение `seqno` для более быстрого перезапуска узла после сбоя
* 🆕 [v9.4.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.4.0) [ Issue #2400](https://github.com/manticoresoftware/manticoresearch/issues/2400) Добавлена поддержка последней версии [Logstash](Integration/Logstash.md#Integration-with-Logstash) и Beats

### Исправления ошибок
* 🪲 [v10.0.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/10.0.1) Исправлена обработка форм слов: пользовательские формы теперь переопределяют автоматически сгенерированные; добавлены тестовые случаи для test 22
* 🪲 [v9.8.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.2) Исправление: обновлен deps.txt для включения исправлений упаковки в MCL, связанных с библиотекой embeddings
* 🪲 [v9.8.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.8.1) Исправление: обновлен deps.txt с исправлениями упаковки для MCL и библиотеки embeddings
* 🪲 [v9.7.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.3) [ Issue #3306](https://github.com/manticoresoftware/manticoresearch/issues/3306) Исправлен сбой с сигналом 11 во время индексирования
* 🪲 [v9.7.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.2) [ Issue #3109](https://github.com/manticoresoftware/manticoresearch/issues/3109) Исправлена проблема, когда несуществующие `@@variables` всегда возвращали 0
* 🪲 [v9.7.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.7.1) [ Issue #3377](https://github.com/manticoresoftware/manticoresearch/issues/3377) Исправлен сбой, связанный с [remove_repeats()](Functions/Searching_and_ranking_functions.md#REMOVE_REPEATS%28%29)
* 🪲 [v9.6.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.3) [ PR #3411](https://github.com/manticoresoftware/manticoresearch/pull/3411) Исправление: использование динамического определения версий для телеметрических метрик
* 🪲 [v9.6.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.2) Исправление: небольшое исправление в выводе [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)
* 🪲 [v9.6.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.6.1) Исправление: сбой при создании таблицы с атрибутом KNN без модели
* 🪲 [v9.5.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.16) [ Issue #3342](https://github.com/manticoresoftware/manticoresearch/issues/3342) Исправлена проблема, при которой `SELECT ... FUZZY=0` не всегда отключал нечёткий поиск
* 🪲 [v9.5.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.15) [ PR #3397](https://github.com/manticoresoftware/manticoresearch/pull/3397) Добавлена поддержка [MCL](https://github.com/manticoresoftware/columnar) 4.2.2; исправлены ошибки со старыми форматами хранения
* 🪲 [v9.5.14](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.14) [ Issue #3392](https://github.com/manticoresoftware/manticoresearch/issues/3392) Исправлена некорректная обработка строк в HTTP JSON ответах
* 🪲 [v9.5.13](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.13) [ Issue #3356](https://github.com/manticoresoftware/manticoresearch/issues/3356) Исправлен сбой в сложном случае полнотекстового запроса (общий подтермин)
* 🪲 [v9.5.12](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.12) Исправлена опечатка в сообщении об ошибке автосброса чанка на диск
* 🪲 [v9.5.11](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.11) [ Issue #3195](https://github.com/manticoresoftware/manticoresearch/issues/3195) Улучшен [автосброс чанка на диск](Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#Flushing-RAM-chunk-to-disk): пропуск сохранения, если выполняется оптимизация
* 🪲 [v9.5.10](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.10) [ Issue #3313](https://github.com/manticoresoftware/manticoresearch/issues/3313) Исправлена проверка дублирования ID для всех дисковых чанков в RT таблице с помощью [indextool](Miscellaneous_tools.md#indextool)
* 🪲 [v9.5.9](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.9) [ Issue #3132](https://github.com/manticoresoftware/manticoresearch/issues/3132) Добавлена поддержка сортировки `_random` в JSON API
* 🪲 [v9.5.8](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.8) [ Issue #3382](https://github.com/manticoresoftware/manticoresearch/issues/3382) Исправлена проблема: невозможность использования uint64 ID документа через JSON HTTP API
* 🪲 [v9.5.7](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.7) [ Issue #3385](https://github.com/manticoresoftware/manticoresearch/issues/3385) Исправлены некорректные результаты при фильтрации по `id != value`
* 🪲 [v9.5.6](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.6) [ PR #538](https://github.com/manticoresoftware/manticoresearch-buddy/pull/538) Исправлена критическая ошибка с нечётким сопоставлением в некоторых случаях
* 🪲 [v9.5.5](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.5) [ Issue #3199](https://github.com/manticoresoftware/manticoresearch/issues/3199) Исправлено декодирование пробелов в параметрах HTTP запросов Buddy (например, `%20` и `+`)
* 🪲 [v9.5.4](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.4) [ Issue #3133](https://github.com/manticoresoftware/manticoresearch/issues/3133) Исправлена некорректная сортировка по `json.field` в фасетном поиске
* 🪲 [v9.5.3](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.3) [ Issue #3091](https://github.com/manticoresoftware/manticoresearch/issues/3091) Исправлены несоответствия результатов поиска для разделителей в SQL и JSON API
* 🪲 [v9.5.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.2) [ Issue #2819](https://github.com/manticoresoftware/manticoresearch/issues/2819) Повышена производительность: заменено `DELETE FROM` на `TRUNCATE` для распределённых таблиц
* 🪲 [v9.5.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.5.1) [ Issue #3080](https://github.com/manticoresoftware/manticoresearch/issues/3080) Исправлен сбой при фильтрации с псевдонимом `geodist()` с JSON атрибутами

## Версия 9.3.2
Выпущена: 2 мая 2025 года

В этом релизе включены многочисленные исправления ошибок и улучшения стабильности, улучшенный учёт использования таблиц, а также усовершенствования управления памятью и ресурсами.

❤️ Особая благодарность [@cho-m](https://github.com/cho-m) за исправление совместимости сборки с Boost 1.88.0 и [@benwills](https://github.com/benwills) за улучшение документации по `stored_only_fields`.

* 🪲 [v9.3.2](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.2) Исправлена проблема, при которой в столбце "Show Threads" активность ЦП отображалась как число с плавающей запятой вместо строки; также исправлена ошибка парсинга результата PyMySQL из-за неправильного типа данных.
* 🪲 [v9.3.1](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.1) [ Issue #3343](https://github.com/manticoresoftware/manticoresearch/issues/3343) Исправлено оставление файлов `tmp.spidx` при прерывании процесса оптимизации.
* 🆕 [v9.3.0](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.3.0) [ PR #3337](https://github.com/manticoresoftware/manticoresearch/pull/3337) Добавлен счетчик команд на таблицу и детальная статистика использования таблиц.
* 🪲 [v9.2.39](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.39) [ Issue #3236](https://github.com/manticoresoftware/manticoresearch/issues/3236) Исправление: предотвращено повреждение таблиц за счет удаления сложных обновлений чанков. Использование функций ожидания внутри последовательного рабочего потока ломало последовательную обработку, что могло приводить к повреждению таблиц.
	Перереализован автосброс. Удалена внешняя очередь опроса, чтобы избежать ненужных блокировок таблиц. Добавлено условие "маленькая таблица": если количество документов ниже 'предела маленькой таблицы' (8192) и не используется Вторичный индекс (SI), сброс пропускается.

* 🪲 [v9.2.38](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.38) Исправление: пропуск создания Вторичного индекса (SI) для фильтров с `ALL`/`ANY` по спискам строк, не влияя на JSON-атрибуты.
* 🪲 [v9.2.37](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.37) [ Issue #2898](https://github.com/manticoresoftware/manticoresearch/issues/2898) Добавлена поддержка обратных кавычек для системных таблиц.
* 🪲 [v9.2.36](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.36) Исправление: использование плейсхолдера для операций с кластером в старом коде. В парсере теперь четко разделяются поля для имен таблиц и кластеров.
* 🪲 [v9.2.35](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.35) Исправление: сбой при удалении кавычек с одинарной `'`.
* 🪲 [v9.2.34](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.34) [ Issue #3090](https://github.com/manticoresoftware/manticoresearch/issues/3090) Исправление: обработка больших ID документов (раньше могла не находить их).
* 🪲 [v9.2.33](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.33) Исправление: использование беззнаковых целых для размеров битовых векторов.
* 🪲 [v9.2.32](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.32) Исправление: уменьшение пикового потребления памяти во время слияния. Пояснения: сопоставления docid-to-rowid теперь используют 12 байт на документ вместо 16. Пример: 24 ГБ ОЗУ на 2 миллиарда документов вместо 36 ГБ.
* 🪲 [v9.2.31](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.31) [ Issue #3238](https://github.com/manticoresoftware/manticoresearch/issues/3238) Исправление: неправильное значение `COUNT(*)` в больших реальном времени таблицах.
* 🪲 [v9.2.30](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.30) Исправление: неопределенное поведение при обнулении строковых атрибутов.
* 🪲 [v9.2.29](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.29) Незначительное исправление: улучшен текст предупреждения.
* 🪲 [v9.2.28](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.28) [ Issue #3290](https://github.com/manticoresoftware/manticoresearch/issues/3290) Улучшение: расширена функциональность `indextool --buildidf`.
* 🪲 [v9.2.27](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.27) [ Issue #3032](https://github.com/manticoresoftware/manticoresearch/issues/3032) При интеграции с Kafka теперь можно создавать источник для конкретного партиции Kafka.
* 🪲 [v9.2.26](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.26) [ Issue #3301](https://github.com/manticoresoftware/manticoresearch/issues/3301) Исправление: `ORDER BY` и `WHERE` по `id` могли вызывать ошибки Out Of Memory (OOM).
* 🪲 [v9.2.25](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.25) [ Issue #3171](https://github.com/manticoresoftware/manticoresearch/issues/3171) Исправление: сбой из-за ошибки сегментации при использовании grouper с несколькими JSON-атрибутами на RT-таблице с несколькими дисковыми чанками.
* 🪲 [v9.2.24](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.24) [ Issue #3246](https://github.com/manticoresoftware/manticoresearch/issues/3246) Исправление: запросы `WHERE string ANY(...)` не работали после сброса чанка в RAM.
* 🪲 [v9.2.23](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.23) [ PR #518](https://github.com/manticoresoftware/manticoresearch-buddy/pull/518) Незначительные улучшения синтаксиса автоматического шардинга.
* 🪲 [v9.2.22](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.22) [ Issue #2763](https://github.com/manticoresoftware/manticoresearch/issues/2763) Исправление: глобальный файл idf не загружался при использовании `ALTER TABLE`.
* 🪲 [v9.2.21](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.21) Исправление: глобальные idf-файлы могут быть большими. Теперь мы освобождаем таблицы раньше, чтобы избежать удержания ненужных ресурсов.
* 🪲 [v9.2.20](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.20) [ PR #3277](https://github.com/manticoresoftware/manticoresearch/pull/3277) Улучшение: лучшая валидация опций шардинга.

* 🪲 [v9.2.19](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.19) [ PR #3275](https://github.com/manticoresoftware/manticoresearch/pull/3275) Исправление: совместимость сборки с Boost 1.88.0.
* 🪲 [v9.2.18](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.18) [ Issue #3228](https://github.com/manticoresoftware/manticoresearch/issues/3228) Исправление: сбой при создании распределенной таблицы (проблема с некорректным указателем).

* 🪲 [v9.2.17](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.17) [ PR #3272](https://github.com/manticoresoftware/manticoresearch/pull/3272) Исправление: проблема с многострочным нечёткм `FACET`.
* 🪲 [v9.2.16](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.16) [ Issue #3063](https://github.com/manticoresoftware/manticoresearch/issues/3063) Исправление: ошибка в вычислении расстояния при использовании функции `GEODIST`.
* 🪲 [v9.2.15](https://github.com/manticoresoftware/manticoresearch/releases/tag/9.2.15) [ Issue #3027](https://github.com/manticoresoftware/manticoresearch/issues/3027) Небольшое улучшение: поддержка формата фильтра `query_string` из Elastic.

## Версия 9.2.14
Выпущено: 28 марта 2025

### Небольшие изменения
* [Коммит ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) Реализован флаг `--mockstack` для вычисления требований к стэку рекурсивных операций. Новый режим `--mockstack` анализирует и сообщает необходимые размеры стэка для оценки рекурсивных выражений, операций сопоставления шаблонов, обработки фильтров. Вычисленные требования к стэку выводятся в консоль для отладки и оптимизации.
* [Issue #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) По умолчанию включён [boolean_simplify](Searching/Options.md#boolean_simplify).
* [Issue #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) Добавлена новая опция конфигурации: `searchd.kibana_version_string`, которая может быть полезна при использовании Manticore с определёнными версиями Kibana или OpenSearch Dashboards, ожидающими конкретную версию Elasticsearch.
* [Issue #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) Исправлена работа [CALL SUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) с двухсимвольными словами.
* [Issue #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) Улучшен [нечёткий поиск](Searching/Spell_correction.md#Fuzzy-Search): ранее он иногда не мог найти "defghi" при поиске "def ghi", если существовал другой подходящий документ.
* ⚠️ BREAKING [Issue #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) Для согласованности в некоторых HTTP JSON ответах `_id` заменён на `id`. Убедитесь, что ваше приложение обновлено соответствующим образом.
* ⚠️ BREAKING [Issue #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) Добавлена проверка `server_id` при присоединении к кластеру для обеспечения уникальности ID каждого узла. Операция `JOIN CLUSTER` теперь может завершаться ошибкой с сообщением о дублировании [server_id](Server_settings/Searchd.md#server_id), если присоединяющийся узел имеет тот же `server_id`, что и существующий узел в кластере. Для решения этой проблемы убедитесь, что каждый узел в репликационном кластере имеет уникальный [server_id](Server_settings/Searchd.md#server_id). Перед попыткой присоединения к кластеру можно изменить значение по умолчанию [server_id](Server_settings/Searchd.md#server_id) в разделе "searchd" вашего конфигурационного файла на уникальное. Это изменение обновляет протокол репликации. Если вы используете репликационный кластер, необходимо:
  - Сначала корректно остановить все ваши узлы
  - Затем запустить последний остановленный узел с параметром `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Дополнительную информацию смотрите в разделе о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).

### Исправления ошибок
* [Коммит 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) Исправлен сбой, вызванный потерей планировщика после ожидания; теперь специфические планировщики, такие как `serializer`, корректно восстанавливаются.
* [Коммит c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) Исправлена ошибка, когда веса из правой присоединённой таблицы не могли использоваться в `ORDER BY`.
* [Issue #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0: исправлена ошибка вызова `lower_bound` с `const knn::DocDist_t*&`. ❤️ Спасибо [@Azq2](https://github.com/Azq2) за PR.
* [Issue #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) Исправлена проблема с обработкой имён таблиц в верхнем регистре при авто-схеме вставок.
* [Issue #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) Исправлен сбой при декодировании некорректного входа base64.
* [Issue #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) Исправлены две связанные проблемы с KNN-индексом при `ALTER`: для векторов с плавающей точкой теперь сохраняются оригинальные размеры, и KNN-индексы теперь правильно создаются.
* [Issue #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) Исправлен сбой при построении вторичного индекса на пустом JSON-столбце.
* [Issue #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) Исправлен сбой, вызванный дублирующимися записями.
* [Issue #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) Исправлено: опция `fuzzy=1` не могла использоваться вместе с `ranker` или `field_weights`.
* [Issue #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) Исправлена ошибка, при которой `SET GLOBAL timezone` не имел эффекта.
* [Issue #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) Исправлена проблема, при которой значения текстовых полей могли теряться при использовании ID больше 2^63.
* [Issue #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) Исправлено: оператор `UPDATE` теперь корректно учитывает настройку `query_log_min_msec`.
* [Issue #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) Исправлена состязательная ситуация при сохранении реального дискового блока, которая могла привести к сбою `JOIN CLUSTER`.


## Версия 7.4.6
Выпущено: 28 февраля 2025

### Важные изменения
* [Issue #832](https://github.com/manticoresoftware/manticoresearch/issues/832) Интеграция с [Kibana](Integration/Kibana.md) для более лёгкой и эффективной визуализации данных.

### Небольшие изменения
* [Issue #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) Исправлены различия в точности чисел с плавающей запятой между arm64 и x86_64.
* [Issue #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) Внедрены оптимизации производительности для пакетной обработки join.
* [Issue #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) Внедрены оптимизации производительности для EstimateValues в гистограммах.
* [Issue #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) Добавлена поддержка Boost 1.87.0. ❤️ Спасибо, [@cho-m](https://github.com/cho-m), за PR.
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) Оптимизировано повторное использование блоков данных при создании фильтров с несколькими значениями; добавлены min/max в метаданные атрибутов; реализована предварительная фильтрация значений фильтра на основе min/max.

### Исправления ошибок
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) Исправлена обработка выражений в объединённых запросах при использовании атрибутов из обеих таблиц (левой и правой); исправлена опция index_weights для правой таблицы.
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) Использование `avg()` в запросе `SELECT ... JOIN` могло приводить к некорректным результатам; теперь исправлено.
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) Исправлен неправильный набор результатов, вызванный неявным отсечением при включённом пакетном join.
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) Исправлен сбой демона при завершении работы во время активного слияния чанков.
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) Исправлена проблема, при которой `IN(...)` мог выдавать неправильные результаты.
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) Установка `max_iops` / `max_iosize` в версии 7.0.0 могла ухудшать производительность индексирования; теперь исправлено.
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) Исправлена утечка памяти в кэше join-запросов.
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) Исправлена обработка опций запроса в объединённых JSON-запросах.
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) Исправлена проблема с командой ATTACH TABLE.
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) Исправлены несоответствия в сообщениях об ошибках.
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) Установка `diskchunk_flush_write_timeout=-1` для таблицы не отключала сброс индекса; теперь исправлено.
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) Устранено дублирование записей после массовой замены больших ID.
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) Исправлен сбой демона, вызванный полнотекстовым запросом с одним оператором `NOT` и ранжировщиком выражений.
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) Исправлена потенциальная уязвимость в библиотеке CJSON. ❤️ Спасибо, [@tabudz](https://github.com/tabudz), за PR.

## Версия 7.0.0
Выпущена: 30 января 2025

### Основные изменения
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) Добавлены новые функции [нечёткого поиска (Fuzzy Search)](Searching/Spell_correction.md#Fuzzy-Search) и [автодополнения (Autocomplete)](Searching/Autocomplete.md#CALL-AUTOCOMPLETE) для упрощения поиска.
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Интеграция с Kafka](Integration/Kafka.md#Syncing-from-Kafka).
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) Введены [вторичные индексы для JSON](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes).
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) Обновления и поиски во время обновлений больше не блокируются слиянием чанков.
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) Автоматический [сброс чанков на диск (disk chunk flush)](Server_settings/Searchd.md#diskchunk_flush_write_timeout) для RT-таблиц для улучшения производительности; теперь мы автоматически сбрасываем RAM-чанки на диск, предотвращая проблемы с производительностью, вызванные отсутствием оптимизаций в RAM-чанках, которые иногда могли приводить к нестабильности в зависимости от размера чанка.
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) Опция [Scroll](Searching/Pagination.md#Scroll-Search-Option) для удобной постраничной навигации.
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) Интеграция с [Jieba](https://github.com/fxsjy/jieba) для улучшения [токенизации китайского языка](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

### Мелкие изменения
* ⚠️ ИЗМЕНЕНИЕ СОВМЕСТИМОСТИ [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) Исправлена поддержка `global_idf` в RT-таблицах. Требуется пересоздание таблиц.
* ⚠️ ИЗМЕНЕНИЕ СОВМЕСТИМОСТИ [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Из внутреннего набора символов `cjk` удалены тайские символы. Обновите определения наборов символов соответственно: если у вас `cjk,non_cjk` и тайские символы важны, замените на `cjk,thai,non_cjk` или `cont,non_cjk`, где `cont` — новое обозначение для всех языков с непрерывными скриптами (то есть `cjk` + `thai`). Измените существующие таблицы с помощью [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode).
* ⚠️ ИЗМЕНЕНИЕ СОВМЕСТИМОСТИ [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) теперь совместимы с распределёнными таблицами. Это повышает версию протокола master/agent. Если вы используете Manticore Search в распределённой среде с несколькими инстансами, сначала обновите агенты, затем мастера.
* ⚠️ BREAKING [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) Изменено название столбца с `Name` на `Variable name` для PQ [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables).
* ⚠️ BREAKING [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) Введена [поблочная регистрация binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) с новыми опциями: [binlog_common](Logging/Binary_logging.md#Binary-logging-strategies), [binlog](Logging/Binary_logging.md#Per-table-binary-logging-configuration) для `create table` / `alter table`. Перед обновлением до новой версии необходимо корректно завершить работу экземпляра Manticore.
* ⚠️ BREAKING [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) Исправлено неправильное сообщение об ошибке при присоединении узла к кластеру с неверной версией протокола репликации. Это изменение обновляет протокол репликации. Если вы используете кластер репликации, необходимо:
  - Сначала корректно остановить все ваши узлы
  - Затем запустить последний остановленный узел с `--new-cluster`, используя инструмент `manticore_new_cluster` в Linux.
  - Подробнее читайте о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* ⚠️ BREAKING [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) Добавлена поддержка нескольких таблиц в [`ALTER CLUSTER ADD` и `DROP`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster). Это изменение также затрагивает протокол репликации. Обратитесь к предыдущему разделу для рекомендаций по обновлению.
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) Исправлена проблема dlopen на Macos.
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) Изменено значение по умолчанию cutoff для OPTIMIZE TABLE в таблицах с KNN индексами для улучшения производительности поиска.
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) Добавлена поддержка `COUNT(DISTINCT)` для `ORDER BY` в `FACET` и `GROUP BY`.
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) Улучшена читаемость в [логировании](Logging/Server_logging.md#Server-logging) слияния чанков.
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка [DBeaver](Integration/DBeaver.md).
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) Реализованы вторичные индексы для функций [POLY2D()](Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29).
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP-запросы теперь поддерживают `Content-Encoding: gzip`.
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) Добавлена команда `SHOW LOCKS`.
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) Разрешён обход ограничения [searchd.max_connections](Server_settings/Searchd.md#max_connections) для запросов Buddy к демону.
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) Добавлена поддержка объединения таблиц через JSON HTTP интерфейс.
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) Логирование успешно обработанных запросов через Buddy в их оригинальной форме.
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) Добавлен специальный режим для запуска `mysqldump` для реплицируемых таблиц.
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) Улучшено переименование внешних файлов при копировании для операторов `CREATE TABLE` и `ALTER TABLE`.
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) Обновлено значение по умолчанию [searchd.max_packet_size](Server_settings/Searchd.md#max_packet_size) до 128MB.
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) Добавлена поддержка [IDF boost modifier](Searching/Full_text_matching/Operators.md#IDF-boost-modifier) в JSON ["match"](Searching/Full_text_matching/Basic_usage.md#match).
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) Улучшена синхронизация записи [binlog](Logging/Binary_logging.md#Binary-logging) для предотвращения ошибок.
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) Включена поддержка zlib в пакетах для Windows.
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) Добавлена поддержка команды SHOW TABLE INDEXES.
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) Установлены метаданные сессии для ответов Buddy.
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) Разрешение до миллисекунд для агрегатов на совместимых конечных точках.
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) Изменены сообщения об ошибках при операциях с кластером, если репликация не запускается.
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) Новые [метрики производительности в SHOW STATUS](Node_info_and_management/Node_status.md#Query-Time-Statistics): min/max/avg/95-й/99-й перцентили по типу запроса за последние 1, 5 и 15 минут.
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) Все вхождения `index` заменены на `table` в запросах и ответах.
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) Добавлен столбец `distinct` в результаты агрегирования на HTTP `/sql` endpoint.
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) Реализовано автоматическое определение типов данных, импортируемых из Elasticsearch.
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) Добавлена поддержка сопоставления строк в выражениях сравнения полей JSON с учётом колляции.
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) Добавлена поддержка выражения `uuid_short` в списке select.
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search теперь запускает Buddy напрямую без обертки `manticore-buddy`.
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) Различающиеся сообщения об ошибках для отсутствующих таблиц и таблиц, не поддерживающих операции вставки.
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 теперь статически встроен в `searchd`.
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) Добавлена инструкция `CALL uuid_short` для генерации последовательностей с несколькими значениями `uuid_short`.
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) Добавлены отдельные опции для правой таблицы в операции JOIN.
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) Улучшена производительность агрегации HTTP JSON, чтобы соответствовать `GROUP BY` в SphinxQL.
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) Добавлена поддержка `fixed_interval` в запросах, связанных с датами в Kibana.
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) Реализована пакетная обработка для JOIN-запросов, что улучшает производительность некоторых JOIN-запросов в сотни и даже тысячи раз.
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) Включена возможность использования веса присоединенной таблицы в fullscan-запросах.
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) Исправлен логирование для join-запросов.
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) Исключения Buddy скрыты из лога `searchd` в режиме без отладки.
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) Завершение работы демона с сообщением об ошибке, если пользователь указывает неверные порты для слушателя репликации.

### Исправления ошибок
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) Исправлено: Неправильные результаты возвращались в JOIN-запросах с более чем 32 колонками.
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) Исправлена проблема, когда соединение таблиц не работало при использовании двух json-атрибутов в условии.
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) Исправлен неправильный total_relation в мультизапросах с [cutoff](Searching/Options.md#cutoff).
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) Исправлена фильтрация по `json.string` в правой таблице при [соединении таблиц](Searching/Joining.md).
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) Разрешено использование `null` для всех значений во всех POST HTTP JSON конечных точках (insert/replace/bulk). В этом случае используется значение по умолчанию.
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) Оптимизировано потребление памяти за счет настройки выделения буфера сети [the max_packet_size](Server_settings/Searchd.md#max_packet_size) для начального зондирования сокета.
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) Исправлена вставка unsigned int в атрибут bigint через JSON-интерфейс.
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) Исправлена работа вторичных индексов с exclude фильтрами и включенным pseudo_sharding.
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) Исправлена ошибка в [manticore_new_cluster](Starting_the_server/Manually.md#searchd-command-line-options).
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) Исправлен сбой демона при неправильном запросе `_update`.
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) Исправлена неспособность гистограмм обрабатывать value фильтры с исключениями.
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) Исправлены [knn](Searching/KNN.md#KNN-vector-search) запросы к распределённым таблицам.
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) Улучшена обработка exclude фильтров на кодировании таблиц в columnar accessor.
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) Исправлен парсер выражений, который не учитывал переопределённый `thread_stack`.
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) Исправлен сбой при клонировании columnar IN выражения.
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) Исправлена проблема инверсии в итераторе bitmap, которая вызывала сбой.
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) Исправлена проблема, из-за которой некоторые пакеты Manticore автоматически удалялись `unattended-upgrades`.
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) Улучшена обработка запросов из инструмента DbForge MySQL.
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) Исправлено экранирование специальных символов в `CREATE TABLE` и `ALTER TABLE`.  ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) Исправлена взаимная блокировка при обновлении blob-атрибута во "замороженном" индексе. Взаимная блокировка возникала из-за конфликтующих блокировок при попытке разморозить индекс. Это могло вызвать сбой в manticore-backup.
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) Команда `OPTIMIZE` теперь выдаёт ошибку, если таблица заморожена.
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) Разрешено использование имён функций в качестве имён колонок.
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) Исправлен сбой демона при запросе настроек таблицы с неизвестным дисковым чанком.
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) Исправлена проблема, из-за которой `searchd` зависал при остановке после `FREEZE` и `FLUSH RAMCHUNK`.
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) Удалены связанные с датой/временем токены (и регулярные выражения) из зарезервированных слов.
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) Исправлен сбой при использовании `FACET` с более чем 5 полями сортировки.
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) Исправлена ошибка восстановления `mysqldump` с включенным `index_field_lengths`.
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) Исправлен сбой при выполнении команды `ALTER TABLE`.
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) Исправлена DLL MySQL в Windows-пакете для корректной работы индексации.
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) Исправлена ошибка компиляции GCC. ❤️ Спасибо, [@animetosho](https://github.com/animetosho), за PR.
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) Исправлена проблема экранирования в [_update](Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial).
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) Исправлен сбой индексации при объявлении нескольких атрибутов или полей с одинаковым именем.
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) Исправлен сбой демона при неверном преобразовании для вложенных bool-запросов для связанных конечных точек "compat" поиска.
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) Исправлено расширение в фразах с модификаторами.
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) Исправлен сбой демона при использовании полнотекстового оператора [ZONE](Searching/Full_text_matching/Operators.md#ZONE-limit-operator) или [ZONESPAN](Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator).
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) Исправлена генерация инфиксов для plain и RT таблиц с словарём ключевых слов.
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) Исправлен ответ с ошибкой в запросе `FACET`; установлен порядок сортировки по умолчанию `DESC` для `FACET` с `COUNT(*)`.
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) Исправлен сбой демона в Windows при старте.
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) Исправлено усечение запросов для HTTP конечных точек `/sql` и `/sql?mode=raw`; сделаны запросы с этих конечных точек консистентными без необходимости заголовка `query=`.
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) Исправлена проблема, когда авто-схема создаёт таблицу, но одновременно завершается с ошибкой.
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) Исправлена библиотека HNSW для поддержки загрузки нескольких KNN индексов.
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) Исправлена проблема заморозки при одновременном возникновении нескольких условий.
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) Исправлен сбой с фатальной ошибкой при использовании `OR` с KNN поиском.
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) Исправлена проблема, когда `indextool --mergeidf *.idf --out global.idf` удалял выходной файл после создания.
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) Исправлен сбой демона при подзапросе с `ORDER BY` строкой в внешнем запросе.
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) Исправлен сбой при обновлении атрибута типа float вместе с атрибутом типа string.
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) Исправлена проблема, когда несколько стоп-слов из токенизаторов `lemmatize_xxx_all` увеличивали `hitpos` последующих токенов.
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) Исправлен сбой на `ALTER ... ADD COLUMN ... TEXT`.
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) Исправлена проблема, когда обновление атрибута blob в замороженной таблице с хотя бы одним RAM чанком вызывает ожидание последующих запросов `SELECT` до разморозки таблицы.
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) Исправлен пропуск кеширования запросов для запросов с упакованными факторами.
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Теперь Manticore сообщает об ошибке при неизвестном действии вместо сбоя на запросах `_bulk`.
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) Исправлен возврат ID вставленного документа для HTTP конечной точки `_bulk`.
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) Исправлен сбой в группировщике при работе с несколькими таблицами, одна из которых пустая, а другая имеет разное количество совпавших записей.
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) Исправлен сбой в сложных запросах `SELECT`.
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) Добавлено сообщение об ошибке, если аргумент `ALL` или `ANY` в выражении `IN` не является JSON-атрибутом.
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) Исправлен сбой демона при обновлении MVA в больших таблицах.
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) Исправлен сбой при ошибке токенизации с `libstemmer`. ❤️ Спасибо, [@subnix](https://github.com/subnix), за PR.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, когда объединённый вес из правой таблицы некорректно работал в выражениях.
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) Исправлена проблема, при которой вес таблицы с правым соединением не работал в выражениях.
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) Исправлен сбой `CREATE TABLE IF NOT EXISTS ... WITH DATA`, когда таблица уже существует.
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) Исправлена ошибка с неопределённым ключом массива "id" при подсчёте по KNN с идентификатором документа.
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) Исправлена функциональность `REPLACE INTO cluster_name:table_name`.
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) Исправлена критическая ошибка при запуске контейнера Manticore Docker с `--network=host`.

## Version 6.3.8
Релиз: 22 ноября 2024

Версия 6.3.8 продолжает серию 6.3 и содержит только исправления ошибок.

### Исправления ошибок

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) Исправлен расчёт доступных потоков при ограничении конкурентности запроса настройками `threads` или `max_threads_per_query`.

## manticore-extra v1.1.20

Релиз: 7 октября 2024

### Исправления ошибок

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) Решена проблема, при которой утилита `unattended-upgrades`, автоматически устанавливающая обновления пакетов в системах на базе Debian, ошибочно помечала несколько пакетов Manticore, включая `manticore-galera`, `manticore-executor` и `manticore-columnar-lib`, для удаления. Это происходило из-за того, что `dpkg` ошибочно считал виртуальный пакет `manticore-extra` избыточным. Были внесены изменения, чтобы `unattended-upgrades` больше не пыталась удалять важные компоненты Manticore.

## Version 6.3.6
Релиз: 2 августа 2024

Версия 6.3.6 продолжает серию 6.3 и содержит только исправления ошибок.

### Исправления ошибок

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) Исправлен сбой, введённый в версии 6.3.4, который мог возникать при работе с выражениями и распределёнными или многими таблицами.
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) Исправлен сбой демона или внутренняя ошибка при преждевременном завершении из-за `max_query_time` при запросе нескольких индексов.

## Version 6.3.4
Релиз: 31 июля 2024

Версия 6.3.4 продолжает серию 6.3 и включает незначительные улучшения и исправления ошибок.

### Незначительные изменения
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) Добавлена поддержка [DBeaver](Integration/DBeaver.md#Integration-with-DBeaver).
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) Улучшено экранирование разделителей в формах слов и исключениях.
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) Добавлены операторы сравнения строк в выражения списка SELECT.
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) Добавлена поддержка null-значений в bulk-запросах, похожих на Elastic.
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) Добавлена поддержка mysqldump версии 9.
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) Улучшена обработка ошибок в HTTP JSON запросах с JSON path к узлу, где возникает ошибка.

### Исправления ошибок
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) Исправлено снижение производительности в запросах с шаблонами (wildcard) при большом количестве совпадений, когда disk_chunks > 1.
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) Исправлен сбой в выражениях списка SELECT MVA MIN или MAX для пустых массивов MVA.
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) Исправлена некорректная обработка бесконечного диапазона запроса в Kibana.
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) Исправлен фильтр соединения по колонковым атрибутам из правой таблицы, когда атрибут отсутствует в списке SELECT.
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) Исправлено дублирование спецификатора 'static' в Manticore 6.3.2.
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) Исправлен LEFT JOIN, возвращающий неподходящие записи при использовании MATCH() по правой таблице.
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) Исправлено сохранение disk chunk в RT индексе с `hitless_words`.
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) Свойство 'aggs_node_sort' теперь можно добавлять в любом порядке среди других свойств.
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) Исправлена ошибка порядка full-text и filter в JSON запросе.
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) Исправлена ошибка, связанная с неправильным JSON ответом на длинные UTF-8 запросы.
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) Исправлен расчёт выражений presort/prefilter, зависящих от объединённых атрибутов.
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) Изменён метод вычисления размера данных для метрик: теперь данные читаются из файла `manticore.json` вместо проверки общего размера каталога данных.
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) Добавлена обработка запросов валидации от Vector.dev.

## Version 6.3.2
Релиз: 26 июня 2024

Версия 6.3.2 продолжает серию 6.3 и содержит несколько исправлений ошибок, часть из которых была обнаружена после выхода версии 6.3.0.

### Изменения, нарушающие совместимость
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) Обновлены значения aggs.range для использования числового типа.

### Исправления ошибок
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) Исправлена группировка по stored check при слиянии rset.
* [Коммит 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) Исправлена ошибка сбоя демона при выполнении запроса с символами подстановки в RT индексе с использованием словаря CRC и включённым `local_df`.
* [Проблема #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) Исправлен сбой в JOIN при использовании `count(*)` без GROUP BY.
* [Проблема #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) Исправлено отсутствие предупреждения в JOIN при попытке группировки по полю полнотекстового поиска.
* [Проблема #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) Исправлена проблема, когда добавление атрибута через `ALTER TABLE` не учитывало параметры KNN.
* [Проблема #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) Исправлен сбой при удалении пакета `manticore-tools` для Redhat в версии 6.3.0.
* [Проблема #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) Исправлены ошибки с JOIN и несколькими выражениями FACET, возвращающими неверные результаты.
* [Проблема #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) Исправлена ошибка ALTER TABLE, возникающая при работе с таблицей, находящейся в кластере.
* [Проблема #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) Исправлена передача исходного запроса в buddy из интерфейса SphinxQL.
* [Проблема #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) Улучшено расширение символов подстановки в `CALL KEYWORDS` для RT индекса с дисковыми чанками.
* [Проблема #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) Исправлено зависание при некорректных запросах `/cli`.
* [Проблема #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Решены проблемы, из-за которых параллельные запросы к Manticore могли застревать.
* [Проблема #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) Исправлено зависание при выполнении `drop table if exists t; create table t` через `/cli`.

### Связанные с репликацией
* [Проблема #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) Добавлена поддержка формата `cluster:name` в HTTP эндпоинте `/_bulk`.

## Версия 6.3.0
Выпущено: 23 мая 2024

### Основные изменения
* [Проблема #839](https://github.com/manticoresoftware/manticoresearch/issues/839) Реализован тип данных [float_vector](Creating_a_table/Data_types.md#Float-vector); реализован [поиск в векторах](Searching/KNN.md#KNN-vector-search).
* [Проблема #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) [INNER/LEFT JOIN](Searching/Joining.md) (**бета-версия**).
* [Проблема #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) Реализовано автоопределение форматов даты для полей типа [timestamp](Creating_a_table/Data_types.md#Timestamps).
* [Проблема #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) Лицензия Manticore Search изменена с GPLv2-or-later на GPLv3-or-later.
* [Коммит 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Запуск Manticore на Windows теперь требует Docker для запуска Buddy.
* [Проблема #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) Добавлен полнотекстовый оператор [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator).
* [Проблема #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) Добавлена поддержка Ubuntu Noble 24.04.
* [Коммит 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) Переработка операций с временем для повышения производительности и добавления новых функций даты/времени:
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - Возвращает текущую дату в локальной временной зоне
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - Возвращает номер квартала года из аргумента timestamp
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - Возвращает название дня недели для переданного timestamp
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - Возвращает название месяца для переданного timestamp
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - Возвращает целочисленный индекс дня недели для переданного timestamp
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - Возвращает номер дня в году для переданного timestamp
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - Возвращает номер года и код дня первой недели текущей недели для переданного timestamp
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - Возвращает количество дней между двумя переданными timestamp
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - Форматирует часть даты из аргумента timestamp
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - Форматирует часть времени из аргумента timestamp
  - [timezone](Server_settings/Searchd.md#timezone) - Временная зона, используемая функциями даты и времени.
* [Коммит 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) Добавлены агрегаты [range](Searching/Faceted_search.md#Facet-over-set-of-ranges), [histogram](Searching/Faceted_search.md#Facet-over-histogram-values), [date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges) и [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) в HTTP интерфейс и аналогичные выражения в SQL.

### Незначительные изменения
* [Проблема #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) Поддержка версий Filebeat 8.10 - 8.11.
* [Проблема #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table).
* [Проблема #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) Добавлена возможность копирования таблиц с помощью SQL-оператора [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:).
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) Оптимизирован [алгоритм уплотнения таблицы](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table): Ранее как ручной процесс [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE), так и автоматический [auto_optimize](Server_settings/Searchd.md#auto_optimize) сначала выполняли слияние чанков для обеспечения ограничения по количеству, а затем удаляли удалённые документы из всех других чанков, содержащих удалённые документы. Этот подход иногда был слишком ресурсоёмким и был отключён. Теперь слияние чанков происходит исключительно в соответствии с настройкой [progressive_merge](Server_settings/Common.md#progressive_merge). Однако чанки с большим количеством удалённых документов имеют более высокую вероятность объединения.
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) Добавлена защита от загрузки вторичного индекса более новой версии.
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) Частичная замена через [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE).
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) Обновлены размеры буферов для слияния по умолчанию: `.spa` (скалярные атрибуты): 256КБ -> 8МБ; `.spb` (blob-атрибуты): 256КБ -> 8МБ; `.spc` (колоночные атрибуты): 1МБ, без изменений; `.spds` (docstore): 256КБ -> 8МБ; `.spidx` (вторичные индексы): буфер 256КБ -> лимит памяти 128МБ; `.spi` (словарь): 256КБ -> 16МБ; `.spd` (doclists): 8МБ, без изменений; `.spp` (hitlists): 8МБ, без изменений; `.spe` (skiplists): 256КБ -> 8МБ.
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) Добавлена [композитная агрегация](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once) через JSON.
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) Отключён PCRE.JIT из-за проблем с некоторыми шаблонами регулярных выражений и отсутствия значительного прироста скорости.
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) Добавлена поддержка vanilla Galera v.3 (api v25) (`libgalera_smm.so` из MySQL 5.x).
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) Изменён суффикс метрики с `_rate` на `_rps`.
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) Улучшена документация о поддержке HA балансировщика.
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) В сообщениях об ошибках изменено `index` на `table`; исправлена корректировка сообщения об ошибке парсера bison.
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) Поддержка `manticore.tbl` как имени таблицы.
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) Поддержка запуска индексатора через systemd ([документация](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)). ❤️ Спасибо, [@subnix](https://github.com/subnix) за PR.
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) Поддержка вторичных индексов в GEODIST().
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) Упрощён [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS).
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) Добавлена поддержка значений по умолчанию (`agent_connect_timeout` и `agent_query_timeout`) для оператора `create distributed table`.
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) Добавлена опция поиска [expansion_limit](Searching/Options.md#expansion_limit), которая переопределяет `searchd.expansion_limit`.
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) Реализация [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) для преобразования int в bigint.
* [Issue #1456](https://github.com/manticoresoftware/manticoresearch/issues/1456) Метапримечания в ответе MySQL.
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION).
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) Поддержка [удаления документов по массиву id](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents) через JSON.
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) Улучшено сообщение об ошибке "unsupported value type".
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) Добавлена версия Buddy в `SHOW STATUS`.
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) Оптимизация запросов на совпадение в случае отсутствия документов для ключевого слова.
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) Добавлено преобразование в булев атрибут из строковых значений "true" и "false" при публикации данных.
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) Добавлена таблица и опция searchd [access_dict](Server_settings/Searchd.md#access_dict).
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) Добавлены новые опции: [expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) и [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits) в секции searchd конфига; сделан порог слияния мелких терминов расширенных терминов настраиваемым.
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) Добавлен показ времени последней команды в `@@system.sessions`.
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) Обновлён протокол Buddy до версии 2.
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) Реализованы дополнительные форматы запросов к эндпойнту `/sql` для упрощения интеграции с библиотеками.
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) Добавлен раздел Info к SHOW BUDDY PLUGINS.
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) Улучшено потребление памяти в `CALL PQ` при больших пакетах.
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) Переключение компилятора с Clang 15 на Clang 16.
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) Добавлено сравнение строк. ❤️ Спасибо, [@etcd](https://github.com/etcd) за PR.
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) Добавлена поддержка объединённых хранимых полей.
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) Логирование host:port клиента в query-log.
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) Исправлена некорректная ошибка.
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) Введена поддержка уровней детализации для [плана запроса в формате JSON](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan).
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) Отключено логирование запросов от Buddy, если не установлен `log_level=debug`.
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) Поддержка Linux Mint 21.3.
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore не мог быть собран с Mysql 8.3+.
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) Команда `DEBUG DEDUP` для чанков таблиц реального времени, где могут возникать дубликаты после присоединения plain-таблицы с дубликатами.
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) Добавлено время в [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES).
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) Обработка колонки `@timestamp` как timestamp.
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) Реализована логика включения/отключения плагинов buddy.
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) Обновлен composer на более свежую версию с исправленными последними CVE.
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Небольшая оптимизация systemd-юнита Manticore, связанная с `RuntimeDirectory`.
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) Добавлена поддержка rdkafka и обновление до PHP 8.3.3.
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) Поддержка [присоединения](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) RT-таблицы. Новая команда [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table).

### Ломающее изменения и устаревшие возможности
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) Исправлена проблема расчёта IDF. `local_df` теперь используется по умолчанию. Улучшен протокол поиска master-agent (обновлена версия). Если вы запускаете Manticore Search в распределённой среде с несколькими инстансами, сначала обновляйте агенты, затем мастера.
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) Добавлена репликация распределённых таблиц и обновлён протокол репликации. Если вы используете кластер репликации, необходимо:
  - Сначала корректно остановить все ваши узлы
  - Затем запустить последний остановленный узел с параметром `--new-cluster`, используя утилиту `manticore_new_cluster` в Linux.
  - Подробнее про [перезапуск кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API эндпойнт алиасы `/json/*` устарели.
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) Изменена [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) на [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan) в JSON, добавлен профилинг запросов для JSON.
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup больше не создаёт бэкапы `plugin_dir`.
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) Миграция Buddy на Swoole для улучшения производительности и стабильности. При переходе на новую версию убедитесь, что все пакеты Manticore обновлены.
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) Все основные плагины объединены в Buddy, изменена базовая логика.
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) Идентификаторы документов в ответах `/search` теперь обрабатываются как числа.
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) Добавлен Swoole, отключён ZTS, удалено расширение parallel.
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) Переопределение в `charset_table` не работало в некоторых случаях.

### Изменения, связанные с репликацией
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) Исправлена ошибка репликации при SST больших файлов.
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) Добавлен механизм повторных попыток для команд репликации; исправлен сбой присоединения репликации в условиях загруженной сети с потерями пакетов.
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) Сообщение FATAL в репликации изменено на WARNING.
* [Коммит 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) Исправлен расчет `gcache.page_size` для кластеров репликации без таблиц или с пустыми таблицами; также исправлено сохранение и загрузка опций Galera.
* [Коммит a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) Добавлена возможность пропускать команду репликации обновления узлов на узле, который присоединяется к кластеру.
* [Коммит c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) Исправлена блокировка при репликации во время обновления blob-атрибутов и замены документов.
* [Коммит e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) Добавлены опции конфигурации searchd [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout), [replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout), [replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay), [replication_retry_count](Server_settings/Searchd.md#replication_retry_count) для управления сетью во время репликации, аналогично `searchd.agent_*`, но с другими значениями по умолчанию.
* [Проблема #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) Исправлен повторный запрос репликации узлов после пропуска некоторых узлов и ошибки разрешения имен этих узлов.
* [Проблема #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) Исправлен уровень подробности журнала репликации в `show variables`.
* [Проблема #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) Исправлена проблема репликации для узла-присоединяющегося при подключении к кластеру после перезапуска pod в Kubernetes.
* [Проблема #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) Исправлено долгое ожидание репликации для изменения в пустом кластере с недопустимым именем узла.

### Исправления ошибок
* [Коммит 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) Исправлена очистка неиспользуемых совпадений в `count distinct`, которая могла приводить к сбою.
* [Проблема #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) Бинарный лог теперь записывается с гранулярностью транзакций.
* [Проблема #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) Исправлена ошибка, связанная с 64-битными ID, которая могла вызвать ошибку "Malformed packet" при вставке через MySQL, приводящую к [повреждению таблиц и дублированию ID](https://github.com/manticoresoftware/manticoresearch/issues/2090).
* [Проблема #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) Исправлена вставка дат так, как будто они были в UTC, вместо локального часового пояса.
* [Проблема #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) Исправлен сбой, возникавший при поиске в таблице реального времени с непустым `index_token_filter`.
* [Проблема #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) Изменена фильтрация дубликатов в колонковом хранилище RT для исправления сбоев и неверных результатов запросов.
* [Коммит 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) Исправлено повреждение памяти html-стриппером после обработки присоединенного поля.
* [Коммит 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) Избежан возврат потока назад после flush для предотвращения проблем с коммуникацией с mysqldump.
* [Коммит 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) Не ожидается завершение preread, если он не был запущен.
* [Коммит 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) Исправлено деление большой строки вывода Buddy на несколько строк в логе searchd.
* [Коммит 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) Перемещено предупреждение интерфейса MySQL о неудачном заголовке уровня подробности `debugv`.
* [Коммит 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) Исправлена гонка при выполнении множества операций управления кластерами; запрещено создавать несколько кластеров с одинаковым именем или путем.
* [Коммит 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) Исправлен неявный обрез в полнотекстовых запросах; MatchExtended разделен на шаблонную часть D.
* [Коммит 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) Исправлено несоответствие `index_exact_words` между индексированием и загрузкой таблицы в демон.
* [Коммит 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) Исправлено отсутствие сообщения об ошибке при удалении недействительного кластера.
* [Коммит 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) Исправлен конфликт CBO со связкой очереди; исправлен CBO с псевдо-шардированием RT.
* [Коммит 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) При запуске без библиотеки и параметров вторичного индекса (SI) ранее появлялось вводящее в заблуждение предупреждение 'WARNING: secondary_indexes set but failed to initialize secondary library'.
* [Коммит 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) Исправлена сортировка попаданий в режиме кворума.
* [Коммит 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) Исправлена проблема с заглавными опциями в плагине ModifyTable.
* [Коммит 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) Исправлено восстановление из дампа с пустыми JSON-значениями (представленными как NULL).
* [Коммит a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) Исправлен тайм-аут SST на узле-присоединяющемся при получении SST с использованием pcon.
* [Коммит b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) Исправлен сбой при выборе псевдонима строкового атрибута.
* [Коммит c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) Добавлено преобразование запроса терма в `=term` полнотекстового запроса с полем `morphology_skip_fields`.
* [Коммит cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) Добавлен отсутствующий ключ конфигурации (skiplist_cache_size).
* [Коммит cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) Исправлен сбой в ранжировщике выражений при большом сложном запросе.
* [Коммит e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) Исправлена проблема CBO полнотекста с недопустимыми подсказками индекса.
* [Коммит eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) Прерван предварительный чтение при завершении работы для ускорения выключения.
* [Коммит f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) Изменён расчёт стека для полнотекстовых запросов, чтобы избежать сбоя при сложном запросе.
* [Проблема #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) Исправлен сбой индексатора при индексировании SQL-источника с несколькими столбцами с одинаковыми именами.
* [Проблема #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) Возвращается 0 вместо <empty> для несуществующих системных переменных.
* [Проблема #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) Исправлена ошибка indextool при проверке внешних файлов таблицы RT.
* [Проблема #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) Исправлена ошибка разбора запроса из-за многоформного слова внутри фразы.
* [Проблема #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) Добавлен повтор воспроизведения пустых бинарных логов со старыми версиями бинарных логов.
* [Проблема #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) Исправлено удаление последнего пустого бинарного лога.
* [Проблема #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) Исправлены некорректные относительные пути (преобразуемые в абсолютные относительно каталога запуска демона) после изменений `data_dir`, влияющих на текущий рабочий каталог при запуске демона.
* [Проблема #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) Замедление в hn_small: получение/кэширование информации о CPU при запуске демона.
* [Проблема #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) Исправлено предупреждение о пропущенном внешнем файле при загрузке индекса.
* [Проблема #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) Исправлен сбой в глобальных группировщиках при освобождении атрибутов с указателями на данные.
* [Проблема #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS не работает.
* [Проблема #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) Исправлено замещение параметра `agent_query_timeout` для каждой таблицы на параметр по умолчанию.
* [Проблема #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) Исправлен сбой группировщика и ранжировщика при использовании `packedfactors()` с несколькими значениями на совпадение.
* [Проблема #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore падает при частом обновлении индекса.
* [Проблема #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) Исправлен сбой при очистке разобранного запроса после ошибки разбора.
* [Проблема #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) Исправлено направление HTTP JSON-запросов к buddy.
* [Проблема #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) Корневое значение JSON-атрибута не могло быть массивом. Исправлено.
* [Проблема #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) Исправлен сбой при воссоздании таблицы внутри транзакции.
* [Проблема #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) Исправлено расширение коротких форм русских лемм.
* [Проблема #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) Исправлено использование JSON и STRING атрибутов в выражении [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()).
* [Проблема #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) Исправлен группировщик для множества псевдонимов к JSON полям.
* [Проблема #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) Неверное total_related в dev: исправлен неявный cutoff vs limit; добавлено лучшее обнаружение полного сканирования в JSON запросах.
* [Проблема #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) Исправлено использование JSON и STRING атрибутов во всех выражениях даты.
* [Проблема #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) Сбой при использовании LEVENSHTEIN().
* [Проблема #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) Исправлена порча памяти после ошибки разбора поискового запроса с подсветкой.
* [Проблема #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) Отключено расширение подстановочных знаков для терминов короче `min_prefix_len` / `min_infix_len`.
* [Проблема #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) Изменено поведение, чтобы не регистрировать ошибку, если Buddy успешно обрабатывает запрос.
* [Проблема #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) Исправлено total в метаданных поискового запроса для запросов с установленным limit.
* [Проблема #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) Невозможно использовать таблицу с заглавными буквами через JSON в plain режиме.
* [Проблема #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) Предоставлено значение по умолчанию для `SPH_EXTNODE_STACK_SIZE`.
* [Проблема #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) Исправлен лог SphinxQL для отрицательного фильтра с ALL/ANY для MVA-атрибута.
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) Исправлено применение docid killlists из других индексов. ❤️ Спасибо, [@raxoft](https://github.com/raxoft), за PR.
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) Исправлены пропущенные совпадения из-за преждевременного выхода при полном сканировании необработанного индекса (без итераторов индексов); удалён cutoff из итератора plain row.
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) Исправлена ошибка `FACET` при запросе распределённой таблицы с агентом и локальными таблицами.
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) Исправлен сбой при оценке гистограммы для больших значений.
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) сбой при alter table tbl add column col uint.
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) Пустой результат для условия `WHERE json.array IN (<value>)`.
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) Исправлена проблема с TableFormatter при отправке запроса на `/cli`.
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) `CREATE TABLE` не выдавал ошибку при отсутствии файла wordforms.
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) Порядок атрибутов в RT-таблицах теперь соответствует порядку в конфигурации.
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) HTTP bool запрос с условием 'should' возвращал некорректные результаты.
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) Сортировка по строковым атрибутам не работает с `SPH_SORT_ATTR_DESC` и `SPH_SORT_ATTR_ASC`.
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) Отключён HTTP-заголовок `Expect: 100-continue` для curl запросов к Buddy.
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) сбой, вызванный псевдонимом GROUP BY.
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) SQL мета-сводка показывает неправильное время на Windows.
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) Исправлено падение производительности при однотерминовом запросе с JSON.
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) Несовместимые фильтры не вызывали ошибку на `/search`.
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) Исправлены операции `ALTER CLUSTER ADD` и `JOIN CLUSTER` для ожидания друг друга, чтобы предотвратить состояние гонки, когда `ALTER` добавляет таблицу в кластер, в то время как донор отправляет таблицы узлу присоединения.
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) Некорректная обработка запросов `/pq/{table}/*`.
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` не работал в некоторых случаях.
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) Исправлена проблема с восстановлением MVA в некоторых случаях.
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) Исправлен сбой indextool при завершении работы, если используется с MCL.
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) Исправлено ненужное декодирование URL для запросов `/cli_json`.
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) изменена логика установки plugin_dir при запуске демона.
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) исключения alter table ... не срабатывали.
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) Manticore завершался с `signal 11` при вставке данных.
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) Уменьшено ограничение по пропускной способности для [low_priority](Searching/Options.md#low_priority).
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Ошибка при mysqldump + mysql restore.
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) Исправлено некорректное создание распределённой таблицы в случае отсутствия локальной таблицы или неправильного описания агента; теперь возвращается сообщение об ошибке.
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) Реализован счётчик `FREEZE` для избежания проблем с freeze/unfreeze.
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) Соблюдение тайм-аута запроса в OR-узлах. Ранее `max_query_time` мог не работать в некоторых случаях.
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) Ошибка при переименовании new в current [manticore.json].
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) Полнотекстовый запрос мог игнорировать hint `SecondaryIndex` CBO.
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) Исправлен `expansion_limit` для среза итогового набора результатов по ключевым словам вызова с нескольких дисковых или RAM-чунк.
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) неправильные внешние файлы.
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) После остановки Manticore могли оставаться запущенными несколько процессов manticore-executor.
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) Сбой при использовании расстояния Левенштейна.
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) Ошибка после многократного запуска max оператора на пустом индексе.
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) сбой на multi-group с JSON.field.
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) Manticore сбрасывался на некорректный запрос к _update.
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) Исправлена проблема с компараторами фильтров строк для закрытых диапазонов в JSON-интерфейсе.
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) `alter` завершался с ошибкой, когда путь data_dir находился на символической ссылке.
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) Улучшена специальная обработка запросов SELECT в mysqldump для обеспечения совместимости получаемых операторов INSERT с Manticore.
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) Тайские символы были в неправильных наборах символов.
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) Сбой при использовании SQL с зарезервированным словом.
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) Таблицы с wordforms не удавалось импортировать.
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) Исправлен сбой, который происходил, если параметр engine был установлен в 'columnar', а дублирующиеся ID добавлены через JSON.
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) Корректное сообщение об ошибке при попытке вставить документ без схемы и без имен столбцов.
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) Авто-схема для многострочных вставок могла не сработать.
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) Добавлено сообщение об ошибке при индексировании, если атрибут id объявлен в источнике данных.
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Сбой кластера Manticore.
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) optimize.php завершался с ошибкой, если присутствовала таблица percolate.
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) Исправлены ошибки при развёртывании на Kubernetes.
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) Исправлена некорректная обработка одновременных запросов к Buddy.

### Связано с manticore-backup
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) Установлен VIP HTTP порт по умолчанию, если доступен.
Различные улучшения: улучшена проверка версий и потоковая декомпрессия ZSTD; добавлены запросы у пользователя при несовпадении версий во время восстановления; исправлено некорректное поведение запросов при восстановлении для разных версий; усовершенствована логика декомпрессии для чтения напрямую из потока, а не в оперативную память; добавлен флаг `--force`
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) Добавлен показ версии резервной копии после запуска Manticore search для выявления проблем на данном этапе.
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) Обновлено сообщение об ошибке при неудачном соединении с демоном.
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) Исправлена ошибка при преобразовании абсолютных корневых путей бэкапа в относительные и удалена проверка на возможность записи при восстановлении для разрешения восстановления из разных путей.
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) Добавлена сортировка итератора файлов для обеспечения последовательности в различных ситуациях.
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) Резервное копирование и восстановление нескольких конфигураций.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлен defattr, чтобы избежать необычных прав пользователя в файлах после установки на RHEL.
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Добавлено дополнительное chown для установки файлов по умолчанию под пользователя root в Ubuntu.

### Связано с MCL (columnar, secondary, knn libs)
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) Добавлена поддержка векторного поиска.
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) Исправлена очистка временных файлов при прерванной настройке построения вторичного индекса. Это решает проблему с превышением демоном лимита открытых файлов при создании файлов `tmp.spidx`.
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) Используется отдельная библиотека streamvbyte для columnar и SI.
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) Добавлено предупреждение, что колоннарное хранение не поддерживает json атрибуты.
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) Исправлена распаковка данных в SI.
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) Исправлен сбой при сохранении дискового чанка с смешанным построчным и колоннарным хранением.
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) Исправлен итератор SI, которому подавалась подсказка на уже обработанный блок.
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) Обновление не работало для столбца rowwise MVA с колоннарным движком.
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) Исправлен сбой при агрегации к колоннарному атрибуту, используемому в `HAVING`.
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) Исправлен сбой ранжировщика `expr` при использовании колоннарного атрибута.

### Связано с Docker
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) Поддержка [plain indexation](https://github.com/manticoresoftware/docker#building-plain-tables) через переменные окружения.
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) Улучшена гибкость конфигурации через переменные окружения.
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) Улучшены процессы [резервного копирования и восстановления](https://github.com/manticoresoftware/docker#backup-and-restore) для Docker.
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) Улучшен entrypoint для обработки восстановления из бэкапа только при первом запуске.
* [Коммит a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) Исправлен вывод логирования запросов в stdout.
* [Проблема #38](https://github.com/manticoresoftware/docker/issues/38) Отключены предупреждения BUDDY, если EXTRA не установлен.
* [Проблема #71](https://github.com/manticoresoftware/docker/pull/71) Исправлено имя хоста в `manticore.conf.sh`.

## Версия 6.2.12
Выпущено: 23 августа 2023 года

Версия 6.2.12 продолжает серию 6.2 и устраняет проблемы, выявленные после релиза 6.2.0.

### Исправления ошибок
* ❗[Проблема #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) «Manticore 6.2.0 не запускается через systemctl на Centos 7»: изменён параметр `TimeoutStartSec` с `infinity` на `0` для лучшей совместимости с Centos 7.
* ❗[Проблема #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) «Сбой после обновления с 6.0.4 до 6.2.0»: добавлена возможность повтора для пустых binlog-файлов из более старых версий binlog.
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) «исправлена опечатка в searchdreplication.cpp»: исправлена опечатка в `searchdreplication.cpp`: beggining -> beginning.
* [Проблема #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) «Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1»: понижен уровень детализации предупреждения интерфейса MySQL о заголовке до logdebugv.
* [Проблема #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) «join cluster зависает, когда node_address не удаётся разрешить»: улучшен повтор при репликации, если некоторые узлы недоступны и разрешение имён не удаётся. Это должно решить проблемы в узлах Kubernetes и Docker, связанные с репликацией. Улучшено сообщение об ошибке при сбоях старта репликации и внесены изменения в тест модели 376. Также добавлено понятное сообщение об ошибке при сбоях разрешения имён.
* [Проблема #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) «Отсутствует отображение в нижний регистр для "Ø" в charset non_cjk»: скорректировано отображение символа 'Ø'.
* [Проблема #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) «searchd оставляет binlog.meta и binlog.001 после чистой остановки»: обеспечено корректное удаление последнего пустого binlog-файла.
* [Коммит 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): исправлена проблема сборки `Thd_t` на Windows, связанная с ограничениями по атомарному копированию.
* [Коммит 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): исправлена проблема с FT CBO по сравнению с `ColumnarScan`.
* [Коммит c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): внесены исправления в тест 376 и добавлена замена для ошибки `AF_INET` в тесте.
* [Коммит cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): решена проблема взаимной блокировки при репликации при обновлении blob-атрибутов по сравнению с заменой документов. Также удалена rlock индекса во время коммита, так как он уже заблокирован на более базовом уровне.

### Незначительные изменения
* [Коммит 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) Обновлена информация о эндпоинтах `/bulk` в руководстве.

### MCL
* Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) версии 2.2.4

## Версия 6.2.0
Выпущено: 4 августа 2023 года

### Основные изменения
* Оптимизатор запросов был усовершенствован для поддержки полнотекстовых запросов, что значительно улучшает эффективность и производительность поиска.
* Интеграции с:
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) — для [создания логических резервных копий](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) с помощью `mysqldump`
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) и [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) для визуализации данных, хранящихся в Manticore
  - [HeidiSQL](https://www.heidisql.com/) и [DBForge](https://www.devart.com/dbforge/) для облегчения разработки с Manticore
* Мы начали использовать [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions), что упрощает для участников сообщества использование того же процесса непрерывной интеграции (CI), который применяет основная команда при подготовке пакетов. Все задачи могут запускаться на хостах GitHub, что облегчает тестирование изменений в вашей форке Manticore Search.
* Мы начали использовать [CLT](https://github.com/manticoresoftware/clt) для тестирования сложных сценариев. Например, теперь мы можем гарантировать, что пакет, собранный после коммита, можно корректно установить на всех поддерживаемых дистрибутивах Linux. Command Line Tester (CLT) предоставляет удобный способ записи тестов в интерактивном режиме и их последующего воспроизведения.
* Существенное повышение производительности операции подсчёта уникальных элементов за счёт комбинации хеш-таблиц и HyperLogLog.
* Включено многопоточное выполнение запросов, содержащих вторичные индексы, при этом количество потоков ограничено числом физических ядер процессора. Это должно значительно ускорить выполнение запросов.
* `pseudo_sharding` был скорректирован так, чтобы ограничиваться числом свободных потоков. Это обновление значительно повышает пропускную способность.
* Теперь пользователи могут указывать [движок хранения атрибутов по умолчанию](Server_settings/Searchd.md#engine) через настройки конфигурации, что обеспечивает лучшую настройку в соответствии с требованиями рабочей нагрузки.
* Поддержка [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/) с многочисленными исправлениями ошибок и улучшениями [в вторичных индексах](Server_settings/Searchd.md#secondary_indexes).

### Незначительные изменения
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): HTTP-эндпоинт [/pq](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) теперь служит псевдонимом для HTTP-эндпоинта `/json/pq`.
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): Обеспечена многобайтовая совместимость для `upper()` и `lower()`.
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): Для запросов `count(*)` теперь возвращается заранее вычисленное значение вместо сканирования индекса.
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): Теперь можно использовать `SELECT` для произвольных вычислений и отображения `@@sysvars`. В отличие от предыдущего состояния, теперь вы не ограничены одним вычислением. Поэтому запросы вроде `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` вернут все столбцы. Обратите внимание, что необязательный 'limit' всегда будет игнорироваться.
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): Реализован заглушечный запрос `CREATE DATABASE`.
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): При выполнении `ALTER TABLE table REBUILD SECONDARY` вторичные индексы теперь всегда перестраиваются, даже если атрибуты не обновлялись.
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): Сортировщики, использующие заранее вычисленные данные, теперь определяются до применения CBO, чтобы избежать лишних вычислений CBO.
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): Реализована и используется имитация стека полнотекстовых выражений для предотвращения сбоев демона.
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): Добавлен быстрый путь кода для клонирования совпадений, которые не используют атрибуты string/mvas/json.
* [Commit a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): Добавлена поддержка команды `SELECT DATABASE()`. Однако она всегда возвращает `Manticore`. Это важно для интеграции с разными MySQL-инструментами.
* [Commit bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): Изменён формат ответа эндпоинта [/cli](Connecting_to_the_server/HTTP.md#/cli), добавлен эндпоинт `/cli_json`, который функционирует как предыдущий `/cli`.
* [Commit d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): Параметр `thread_stack` теперь можно изменять во время работы с помощью оператора `SET`. Доступны как локальные для сессии, так и глобальные для демона варианты. Текущие значения можно просмотреть в выводе `show variables`.
* [Commit d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): В код CBO интегрирована логика для более точной оценки сложности выполнения фильтров по строковым атрибутам.
* [Commit e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): Улучшена оценка стоимости DocidIndex, что повысило общую производительность.
* [Commit f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): Метрики нагрузки, аналогичные 'uptime' в Linux, теперь видны в команде `SHOW STATUS`.
* [Commit f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): Порядок полей и атрибутов для `DESC` и `SHOW CREATE TABLE` теперь соответствует `SELECT * FROM`.
* [Commit f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): Разные внутренние парсеры теперь при выдаче ошибок предоставляют свой внутренний мнемонический код (например, `P01`). Это помогает определить, какой парсер вызвал ошибку, и скрывает несущественные внутренние детали.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) "Sometimes CALL SUGGEST does not suggest a correction of a single letter typo": Улучшено поведение [SUGGEST/QSUGGEST](Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) для коротких слов: добавлена опция `sentence` для отображения всего предложения.
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) "Percolate index does not search properly by exact phrase query when stemming enabled": Запрос перколяции изменён для поддержки модификатора точного терма, улучшена функциональность поиска.
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) "DATE FORMATTING methods": добавлено выражение списка выбора [date_format()](Functions/Date_and_time_functions.md#DATE_FORMAT()), которое открывает функцию `strftime()`.
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) "Sorting buckets via HTTP JSON API": введено необязательное [свойство сортировки](Searching/Faceted_search.md#HTTP-JSON) для каждого bucket агрегатов в HTTP-интерфейсе.
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) "Improve error logging of JSON insert api failure - "unsupported value type"": Эндпоинт `/bulk` теперь сообщает количество обработанных и необработанных строк (документов) в случае ошибки.
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) "CBO hints don't support multiple attributes": Подсказки индекса теперь поддерживают несколько атрибутов.
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) "Add tags to http search query": В [ответы HTTP PQ](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) добавлены теги.
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) "buddy should not create table in parallel": Исправлена проблема с ошибками при параллельном выполнении CREATE TABLE. Теперь одновременно может выполняться только одна операция `CREATE TABLE`.
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) "добавить поддержку @ в именах столбцов".
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) "Запросы к набору данных такси медленные при ps=1": логика CBO была улучшена, а разрешение гистограммы по умолчанию установлено в 8k для большей точности на атрибутах с случайно распределёнными значениями.
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) "Исправление CBO vs fulltext на наборе данных hn": реализована улучшенная логика для определения когда использовать пересечение итераторов bitmap и когда использовать очередь с приоритетом.
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) "columnar: изменить интерфейс итератора на однократный вызов" : Итераторы по колонкам теперь используют один вызов `Get`, заменяющий предыдущие двухшаговые вызовы `AdvanceTo` + `Get` для получения значения.
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) "Ускорение вычисления агрегатов (удалить CheckReplaceEntry?)": вызов `CheckReplaceEntry` был удалён из сортировщика групп для ускорения расчёта агрегатных функций.
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) "create table read_buffer_docs/hits не распознаёт синтаксис k/m/g": опции `read_buffer_docs` и `read_buffer_hits` в `CREATE TABLE` теперь поддерживают синтаксис k/m/g.
* [Language packs](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) для английского, немецкого и русского теперь легко устанавливаются на Linux командой `apt/yum install manticore-language-packs`. На macOS используйте команду `brew install manticoresoftware/tap/manticore-language-packs`.
* Порядок полей и атрибутов теперь согласован между операциями `SHOW CREATE TABLE` и `DESC`.
* Если при выполнении запросов `INSERT` не хватает места на диске, новые запросы `INSERT` будут завершаться с ошибкой до освобождения достаточного количества пространства.
* Добавлена функция преобразования типов [UINT64()](Functions/Type_casting_functions.md#UINT64%28%29).
* Эндпоинт `/bulk` теперь обрабатывает пустые строки как команду [commit](Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK). Подробнее [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents).
* Добавлены предупреждения для [недопустимых подсказок индекса](Searching/Options.md#Query-optimizer-hints), обеспечивающие большую прозрачность и возможность устранения ошибок.
* При использовании `count(*)` с одним фильтром запросы теперь используют предрассчитанные данные из вторичных индексов, если они доступны, что значительно ускоряет время выполнения.

### ⚠️ Ломающие изменения
* ⚠️ Таблицы, созданные или изменённые в версии 6.2.0, не могут быть прочитаны старыми версиями.
* ⚠️ Идентификаторы документов теперь обрабатываются как беззнаковые 64-битные целые числа при индексировании и операциях INSERT.
* ⚠️ Синтаксис подсказок оптимизатора запросов обновлён. Новый формат — `/*+ SecondaryIndex(uid) */`. Обратите внимание, что старый синтаксис больше не поддерживается.
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): использование `@` в названиях таблиц запрещено для предотвращения конфликтов синтаксиса.
* ⚠️ Строковые поля/атрибуты, отмеченные как `indexed` и `attribute`, теперь рассматриваются как одно поле в операциях `INSERT`, `DESC` и `ALTER`.
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): библиотеки MCL больше не загружаются на системах без поддержки SSE 4.2.
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): [agent_query_timeout](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) был сломан. Исправлен и теперь работает.

### Исправления ошибок
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) "Crash on DROP TABLE": решена проблема, вызывающая длительное ожидание завершения операций записи (оптимизация, сохранение дискового чанка) в RT таблице при выполнении DROP TABLE. Добавлено предупреждение о том, что каталог таблицы не пуст после DROP TABLE.
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): добавлена поддержка колоннарных атрибутов, отсутствовавшая в коде для группировки по нескольким атрибутам.
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) Исправлена возможная причина сбоя при переполнении места на диске — теперь ошибки записи в binlog обрабатываются корректно.
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): исправлен сбой, который иногда происходил при использовании нескольких колоннарных скан-итераторов (или итераторов вторичного индекса) в запросе.
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): фильтры не удалялись при использовании сортировщиков, применяющих предрассчитанные данные. Исправлено.
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): обновлен код CBO для более точных оценок запросов с фильтрами по построчным атрибутам, исполняемым в нескольких потоках.
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) "фатальный сбой дампа в кластере Kubernetes": исправлен неисправный фильтр Блума для корневого объекта JSON; исправлен сбой демона из-за фильтрации по JSON-полю.
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) Исправлен сбой демона, вызванный некорректным конфигом `manticore.json`.
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) Исправлен json range filter для поддержки значений int64.
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) `.sph` файлы могли повреждаться при `ALTER`. Исправлено.
* [Коммит 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): Добавлен общий ключ для репликации оператора replace для решения ошибки `pre_commit`, возникающей при репликации replace с нескольких мастер-узлов.
* [Коммит 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) исправил проблемы с проверкой bigint в функциях, таких как 'date_format()'.
* [Коммит 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): Итераторы больше не отображаются в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), когда сортировщики используют предварительно вычисленные данные.
* [Коммит a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): Обновлен размер стека fulltext-узла для предотвращения сбоев при выполнении сложных fulltext-запросов.
* [Коммит a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): Исправлена ошибка, вызывавшая сбой при репликации обновлений с использованием атрибутов JSON и строк.
* [Коммит b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): Строитель строк обновлен для использования 64-битных целых чисел во избежание сбоев при работе с большими наборами данных.
* [Коммит c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): Исправлен сбой, возникший при подсчёте distinct по нескольким индексам.
* [Коммит d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): Исправлена проблема, при которой запросы по дисковым чанкам RT-индексов могли выполняться в нескольких потоках, даже если `pseudo_sharding` был отключен.
* [Коммит d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) Множество значений, возвращаемых командой `show index status`, изменено и теперь зависит от типа используемого индекса.
* [Коммит e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) Исправлена ошибка HTTP при обработке массовых запросов и проблема с тем, что ошибка не возвращалась клиенту из сетевого цикла.
* [Коммит f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) использование расширенного стека для PQ.
* [Коммит fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) Обновлен вывод ранжировщика экспорта, чтобы соответствовать [packedfactors()](Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29).
* [Коммит ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): Исправлена проблема со списком строк в фильтре журнала запросов SphinxQL.
* [Проблема #589](https://github.com/manticoresoftware/manticoresearch/issues/589) "Определение charset, похоже, зависит от порядка кодов": Исправлено некорректное сопоставление charset для дубликатов.
* [Проблема #811](https://github.com/manticoresoftware/manticoresearch/issues/811) "Сопоставление нескольких слов в формах слова мешает поиску фраз с китайскими знаками препинания между ключевыми словами": Исправлена позиция токена ngram в фразовом запросе с формами слов.
* [Проблема #834](https://github.com/manticoresoftware/manticoresearch/issues/834) "Знак равенства в поисковом запросе нарушает запрос": Обеспечена возможность экранировать точный символ и исправлено двойное точное расширение опцией `expand_keywords`.
* [Проблема #864](https://github.com/manticoresoftware/manticoresearch/issues/864) "Конфликт исключений/стоп-слов"
* [Проблема #910](https://github.com/manticoresoftware/manticoresearch/issues/910) "Сбой Manticore при вызове call snippets() с libstemmer_fr и index_exact_words": Исправлены внутренние конфликты, вызывавшие сбои при вызове `SNIPPETS()`.
* [Проблема #946](https://github.com/manticoresoftware/manticoresearch/issues/946) "Дублированные записи при SELECT": Исправлена проблема дублирования документов в результатах для запроса с опцией `not_terms_only_allowed` к RT-индексу с удалёнными документами.
* [Проблема #967](https://github.com/manticoresoftware/manticoresearch/issues/967) "Использование JSON-аргументов в UDF-функциях приводит к сбою": Исправлен сбой демона при обработке поиска с включённым pseudo-sharding и UDF с JSON-аргументом.
* [Проблема #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) "count(*) в FEDERATED": Исправлен сбой демона при выполнении запроса через движок `FEDERATED` с агрегатом.
* [Проблема #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) Исправлена несовместимость колонки `rt_attr_json` с колоночным хранением.
* [Проблема #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) "* удаляется из поискового запроса опцией ignore_chars": Исправлено, чтобы подстановочные знаки в запросе не затрагивались `ignore_chars`.
* [Проблема #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) "indextool --check не работает, если есть распределённая таблица": indextool теперь совместим с экземплярами, содержащими 'distributed' и 'template' индексы в json-конфиге.
* [Проблема #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) "Определённый select на конкретном наборе RT приводит к сбою searchd": Исправлен сбой демона при запросе с packedfactors и большим внутренним буфером.
* [Проблема #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) "С опцией not_terms_only_allowed удалённые документы игнорируются"
* [Проблема #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) "indextool --dumpdocids не работает": Восстановлена работоспособность команды `--dumpdocids`.
* [Проблема #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) "indextool --buildidf не работает": indextool теперь закрывает файл после завершения globalidf.
* [Проблема #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) "Count(*) пытается рассматриваться как набор схемы в удалённых таблицах": Исправлена проблема, когда демон отправлял сообщение об ошибке для запросов к распределённому индексу, если агент возвращал пустой набор результатов.
* [Проблема #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) "FLUSH ATTRIBUTES зависает при threads=1".
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "Потеря соединения с MySQL сервером во время запроса - manticore 6.0.5": Исправлены сбои, происходившие при использовании нескольких фильтров для колоннарных атрибутов.
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "Чувствительность к регистру при фильтрации JSON строк": Скорректирована сортировка для правильной работы фильтров, используемых в HTTP поисковых запросах.
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "Соответствие в неправильном поле": Исправлен ущерб, связанный с `morphology_skip_fields`.
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "системные удаленные команды через API должны учитывать g_iMaxPacketSize": Внесены изменения для обхода проверки `max_packet_size` для команд репликации между узлами. Кроме того, последняя ошибка кластера добавлена в отображение статуса.
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "временные файлы остались после неудачной оптимизации": Исправлена проблема с оставлением временных файлов после ошибки в процессе слияния или оптимизации.
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "добавлена переменная окружения для таймаута запуска buddy": Добавлена переменная окружения `MANTICORE_BUDDY_TIMEOUT` (по умолчанию 3 секунды) для управления временем ожидания сообщения buddy при запуске демона.
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "переполнение int при сохранении метаданных PQ": Смягчено чрезмерное потребление памяти демоном при сохранении большого индекса PQ.
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "Невозможно воссоздать RT таблицу после изменения внешнего файла": Исправлена ошибка изменения с пустой строкой для внешних файлов; исправлено оставление внешних файлов RT индекса после изменения внешних файлов.
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "SELECT statement sum(value) as value работает некорректно": Исправлена проблема, когда выражение списка select с псевдонимом могло скрывать атрибут индекса; также исправлен sum на count в int64 для целых чисел.
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "Избегать привязки к localhost в репликации": Обеспечено, что репликация не привязывается к localhost для имен хостов с несколькими IP.
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "ответ mysql клиенту не удался для данных > 16Mb": Исправлена проблема возврата пакета SphinxQL размером более 16Mb клиенту.
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) "неправильная ссылка в "пути к внешним файлам должны быть абсолютными": Исправлено отображение полного пути к внешним файлам в `SHOW CREATE TABLE`.
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "debug сборка падает на длинных строках в сниппетах": Теперь длинные строки (>255 символов) разрешены в тексте, обрабатываемом функцией `SNIPPET()`.
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "случайный сбой при use-after-delete в опросе kqueue (master-agent)": Исправлены сбои, когда мастер не может подключиться к агенту на системах с kqueue (FreeBSD, MacOS и др.).
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "слишком долгое подключение к самому себе": При подключении с мастера к агентам на MacOS/BSD теперь используется единый таймаут соединения+запроса вместо только соединения.
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq (json meta) с не достигнутыми встроенными синонимами не загружается": Исправлен флаг встроенных синонимов в pq.
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "Разрешить некоторым функциям (sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday) использовать неявно повышенные значения аргументов".
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "Включить многопоточный SI в полном сканировании, но ограничить потоки": В код CBO внедрено улучшение для лучшего прогнозирования производительности многопоточного вторичного индекса при использовании в полнотекстовом запросе.
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "запросы count(*) остаются медленными после использования precalc sorters": Итераторы теперь не инициализируются при использовании сортировщиков с предрассчитанными данными, избегая пагубного влияния на производительность.
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "журнал запросов в sphinxql не сохраняет оригинальные запросы для MVA": Теперь логируются `all()/any()`.

## Версия 6.0.4
Выпущена: 15 марта 2023

### Новые возможности
* Улучшена интеграция с Logstash, Beats и т.п., включая:
  - Поддержка версий Logstash 7.6 - 7.15, Filebeat 7.7 - 7.12
  - Поддержка авто-схем.
  - Добавлена обработка bulk-запросов в формате, похожем на Elasticsearch.
* [Buddy commit ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Логирование версии Buddy при запуске Manticore.

### Исправления ошибок
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) Исправлен некорректный символ в метаданных поиска и ключевых словах для bigram индекса.
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) Отклоняются HTTP-заголовки в нижнем регистре.
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) Исправлена утечка памяти в демоне при чтении вывода консоли Buddy.
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) Исправлено неожиданное поведение знака вопроса.
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) Исправлена гонка в таблицах приведенных к нижнему регистру токенизатора, вызывающая сбой.
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) Исправлена обработка bulk-записей в JSON интерфейсе для документов с id, явно установленным в null.
* [Коммит 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) Исправлена статистика терминов в CALL KEYWORDS для нескольких одинаковых терминов.
* [Коммит f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Конфигурация по умолчанию теперь создается установщиком Windows; пути больше не заменяются во время выполнения.
* [Коммит 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Коммит cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) Исправлены проблемы репликации для кластера с узлами в нескольких сетях.
* [Коммит 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) Исправлена HTTP-точка /pq, теперь является псевдонимом HTTP-точки /json/pq.
* [Коммит 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) Исправлен сбой демона при перезапуске Buddy.
* [Коммит Buddy fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) Отображение оригинальной ошибки при получении некорректного запроса.
* [Коммит Buddy db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) Разрешены пробелы в пути резервной копии и добавлена магия в regexp для поддержки одинарных кавычек.

## Версия 6.0.2
Выпущена: 10 фев 2023

### Исправления ошибок
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / ошибка сегментации при поиске по фасетам с большим количеством результатов
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - ВНИМАНИЕ: встроенное значение KNOWN_CREATE_SIZE (16) меньше измеренного (208). Рассмотрите возможность исправления значения!
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Сбой plain-индекса в Manticore 6.0.0
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - потеря нескольких распределённых индексов при перезапуске демона
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - состояние гонки в таблицах приведения к нижнему регистру токенизатора

## Версия 6.0.0
Выпущена: 7 фев 2023

Начиная с этого релиза, Manticore Search поставляется с Manticore Buddy — демоном-сайдкаром, написанным на PHP, который обрабатывает высокоуровневую функциональность, не требующую сверхнизкой задержки или высокой пропускной способности. Manticore Buddy работает за кулисами, и вы можете даже не заметить, что он запущен. Несмотря на свою невидимость для конечного пользователя, было большой задачей сделать Manticore Buddy легко устанавливаемым и совместимым с основным демоном на C++. Это крупное изменение позволит команде разработать широкий спектр новых высокоуровневых функций, таких как оркестрация шардов, контроль доступа и аутентификация, а также различные интеграции, например mysqldump, DBeaver, Grafana mysql connector. Сейчас он уже обрабатывает [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES), [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) и [Auto schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

В этот релиз также включено более 130 исправлений ошибок и множество функций, многие из которых можно считать важными.

### Основные изменения
* 🔬 Экспериментально: теперь можно выполнять Elasticsearch-совместимые [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) и [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON-запросы, что позволяет использовать Manticore с такими инструментами, как Logstash (версия < 7.13), Filebeat и другими инструментами из семейства Beats. Включено по умолчанию. Вы можете отключить их с помощью `SET GLOBAL ES_COMPAT=off`.
* Поддержка [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) с многочисленными исправлениями и улучшениями в [вторичных индексах](Server_settings/Searchd.md#secondary_indexes). **⚠️ Критическое изменение**: вторичные индексы включены по умолчанию с этого релиза. Обязательно выполните [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) при обновлении с Manticore 5. Подробнее см. ниже.
* [Коммит c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) Авто-схема: теперь можно пропустить создание таблицы, просто вставить первый документ, и Manticore автоматически создаст таблицу на основе его полей. Подробнее об этом читайте [здесь](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema). Это можно включать/выключать с помощью [searchd.auto_schema](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).
* Полная переработка [оптимизатора на основе стоимости](Searching/Cost_based_optimizer.md), который уменьшает время отклика запросов во многих случаях.
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) Оценка производительности параллелизма в CBO.
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO теперь учитывает [вторичные индексы](Server_settings/Searchd.md#secondary_indexes) и может работать умнее.
  - [Коммит cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) Статистика кодирования колоннарных таблиц/полей теперь хранится в метаданных, чтобы помочь CBO принимать более умные решения.
  - [Коммит 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) Добавлены подсказки CBO для тонкой настройки его поведения.
* [Телеметрия](Telemetry.md#Telemetry): с радостью сообщаем о добавлении телеметрии в этот релиз. Эта функция позволяет собирать анонимные и деперсонализированные метрики, которые помогут нам улучшить производительность и опыт пользователей нашего продукта. Будьте уверены, все собранные данные **полностью анонимны и не будут связаны с какой-либо личной информацией**. Эту функцию можно [легко отключить](Telemetry.md#Telemetry) в настройках при необходимости.
* [Коммит 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) для перестроения вторичных индексов в любое нужное время, например:
  - при миграции с Manticore 5 на новую версию,
  - когда вы сделали [UPDATE](Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE) (то есть [обновление на месте, а не замену](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)) атрибута в индексе
* [Проблема #821](https://github.com/manticoresoftware/manticoresearch/issues/821) Новый инструмент `manticore-backup` для [резервного копирования и восстановления экземпляра Manticore](Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL-команда [BACKUP](Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) для создания резервных копий изнутри Manticore.
* SQL-команда [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) как простой способ увидеть выполняющиеся запросы, а не потоки.
* [Проблема #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL-команда `KILL` для завершения долгого выполнения `SELECT`.
* Динамический `max_matches` для агрегирующих запросов для повышения точности и снижения времени ответа.

### Мелкие изменения
* [Проблема #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL-команды [FREEZE/UNFREEZE](Securing_and_compacting_a_table/Freezing_a_table.md) для подготовки таблицы real-time/plain к резервному копированию.
* [Коммит c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) Новые настройки `accurate_aggregation` и `max_matches_increase_threshold` для контролируемой точности агрегации.
* [Проблема #718](https://github.com/manticoresoftware/manticoresearch/issues/718) Поддержка знаковых отрицательных 64-битных ID. Обратите внимание, что ID > 2^63 до сих пор нельзя использовать, но теперь можно использовать ID в диапазоне от -2^63 до 0.
* Поскольку мы недавно добавили поддержку вторичных индексов, возникла путаница, так как "индекс" мог означать вторичный индекс, полнотекстовый индекс или plain/real-time `index`. Чтобы уменьшить путаницу, мы переименуем последний в "таблицу". Следующие SQL и командные команды затронуты этим изменением. Их старые версии устарели, но всё ещё работают:
  - `index <table name>` => `table <table name>`,
  - `searchd -i / --index` => `searchd -t / --table`,
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`,
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`,
  - `FLUSH RTINDEX` => `FLUSH TABLE`,
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`,
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`,
  - `RELOAD INDEX` => `RELOAD TABLE`,
  - `RELOAD INDEXES` => `RELOAD TABLES`.

  Мы не планируем делать старые формы недействительными, но для обеспечения совместимости с документацией рекомендуем менять названия в вашем приложении. В будущих выпусках будет изменено переименование "index" в "table" в выводе различных SQL и JSON команд.
* Запросы с состоянием в UDF теперь вынуждены выполняться в одном потоке.
* [Проблема #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) Рефакторинг всего связанного с планированием времени как предпосылка для параллельного слияния чанков.
* **⚠️ ИЗМЕНЕНИЕ, ЛОМАЮЩЕЕ СОВМЕСТИМОСТЬ**: Формат хранения колонок был изменён. Необходимо перестроить те таблицы, которые имеют колоночные атрибуты.
* **⚠️ ИЗМЕНЕНИЕ, ЛОМАЮЩЕЕ СОВМЕСТИМОСТЬ**: Формат файлов вторичных индексов был изменён, поэтому если вы используете вторичные индексы для поиска и в конфигурационном файле у вас установлен `searchd.secondary_indexes = 1`, будьте внимательны — новая версия Manticore **пропустит загрузку таблиц с вторичными индексами**. Рекомендуется:
  - Перед обновлением изменить `searchd.secondary_indexes` на 0 в конфигурационном файле.
  - Запустить экземпляр. Manticore загрузит таблицы с предупреждением.
  - Выполнить `ALTER TABLE <table name> REBUILD SECONDARY` для каждого индекса, чтобы перестроить вторичные индексы.

  Если у вас кластер с репликацией, нужно выполнить `ALTER TABLE <table name> REBUILD SECONDARY` на всех узлах или следовать [этой инструкции](Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables), но выполнять `ALTER .. REBUILD SECONDARY` вместо `OPTIMIZE`.
* **⚠️ ИЗМЕНЕНИЕ, ЛОМАЮЩЕЕ СОВМЕСТИМОСТЬ**: Версия binlog была обновлена, так что binlog из предыдущих версий не будет применяться. Важно обеспечить чистую остановку Manticore Search при обновлении. Это означает, что после остановки предыдущего экземпляра не должно оставаться binlog файлов в `/var/lib/manticore/binlog/`, кроме `binlog.meta`.
* [Проблема #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`: теперь можно увидеть настройки из конфигурационного файла изнутри Manticore.
* [Проблема #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](Server_settings/Setting_variables_online.md#SET) включает/выключает отслеживание времени CPU; [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) теперь не показывает статистику ЦП при выключенном отслеживании времени CPU.
* [Проблема #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) Сегменты RAM чанков RT таблиц теперь могут сливаться во время сброса RAM чанка.
* [Проблема #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) Добавлен прогресс вторичного индекса в вывод [indexer](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).
* [Проблема #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) Ранее Manticore мог удалить запись таблицы из списка индексов, если не мог начать её обслуживать при запуске. Новое поведение — сохранять её в списке и пытаться загрузить при следующем запуске.
* [indextool --docextract](Miscellaneous_tools.md#indextool) возвращает все слова и попадания, принадлежащие запрошенному документу.
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) Переменная окружения `dump_corrupt_meta` включает дамп повреждённых метаданных таблицы в лог в случае, если searchd не может загрузить индекс.
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` теперь показывает `max_matches` и статистику псевдо-шардинга.
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) Улучшено сообщение об ошибке вместо запутывающего "Index header format is not json, will try it as binary...".
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) Путь к украинскому лемматизатору изменён.
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) Добавлена статистика по вторичным индексам в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META).
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON-интерфейс теперь легко визуализировать с помощью Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification.
* **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ**: Протокол репликации изменён. Если вы используете кластер репликации, при обновлении до Manticore 5 необходимо:
  - сначала корректно остановить все ноды
  - затем запустить ноду, остановленную последней, с параметром `--new-cluster` (для Linux запустить инструмент `manticore_new_cluster`).
  - подробнее о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).

### Изменения, связанные с Manticore Columnar Library
* Рефакторинг интеграции вторичных индексов с колончатым хранилищем.
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Оптимизация Manticore Columnar Library, снижающая время отклика за счёт частичной предварительной оценки min/max.
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) В случае прерывания слияния дискового чанка демон теперь очищает временные файлы MCL.
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) Версии библиотек Columnar и secondary выводятся в лог при аварии.
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) Добавлена поддержка быстрого перематывания doclist для вторичных индексов.
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) Запросы вида `select attr, count(*) from plain_index` (без фильтрации) теперь выполняются быстрее при использовании MCL.
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) @@autocommit в HandleMysqlSelectSysvar для совместимости с .net-коннектором MySQL выше версии 8.25
* **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ**: [MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL: добавлен SSE код для сканирования колонок. Теперь MCL требует как минимум SSE4.2.

### Изменения, связанные с упаковкой
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ**: Прекращена поддержка Debian Stretch и Ubuntu Xenial.
* Поддержка RHEL 9, включая Centos 9, Alma Linux 9 и Oracle Linux 9.
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) Поддержка Debian Bookworm.
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) Упаковка: сборки arm64 для Linux и MacOS.
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) Многоархитектурный (x86_64 / arm64) образ Docker.
* [Упрощённая сборка пакетов для участников](Installation/Compiling_from_sources.md#Building-using-CI-Docker).
* Теперь возможно устанавливать конкретную версию через APT.
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Установщик для Windows (ранее был только архив).
* Переход на компиляцию с использованием CLang 15.
* **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ**: Пользовательские формулы Homebrew, включая формулу для Manticore Columnar Library. Для установки Manticore, MCL и других необходимых компонентов используйте команду `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`.

### Исправления ошибок
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) Поле с именем `text`
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id не может быть не bigint
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER и поле с именем "text"
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) Возможная ошибка: HTTP (JSON) offset и limit влияют на результаты фасетов
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd зависает/аварийно завершает работу при интенсивной нагрузке
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ индекс выходит за пределы памяти
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` был сломан с времён Sphinx. Исправлено.
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL: сбой при select при слишком большом количестве ft полей
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field нельзя сохранять
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) Краш при использовании LEVENSHTEIN()
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Неожиданный краш Manticore и невозможность нормального перезапуска
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) Вызов CALL KEYWORDS через /sql возвращает управляющий символ, который ломает json
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb не может создать таблицу FEDERATED
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) WARNING: dlopen() failed: /usr/bin/lib_manticore_columnar.so: невозможно открыть файл объекта: Нет такого файла или каталога
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) Manticore аварийно завершается при поиске с ZONESPAN через api
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) неправильный вес при использовании нескольких индексов и facet distinct
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL group query зависает после повторной обработки SQL индекса
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL: Indexer падает в 5.0.2 и manticore-columnar-lib 1.15.4
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED возвращает пустой набор (MySQL 8.0.28)
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) select COUNT DISTINCT на 2 индексах при нулевом результате вызывает внутреннюю ошибку
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) КРЕШ при запросе delete
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL: Ошибка с длинным текстовым полем
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex: Поведение ограничения агрегации поиска не соответствует ожиданиям
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) Возвращаемые результаты Hits являются объектом Nonetype даже для поисков, которые должны вернуть несколько результатов
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) Краш при использовании Attribute и Stored Field в выражении SELECT
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) таблица становится невидимой после краша
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) Два отрицательных терма в поисковом запросе вызывают ошибку: запрос не вычисляется
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c не работает через json query_string
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) pseudo_sharding с совпадением запроса
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 функция min/max работает не как ожидалось...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) Поле "weight" разбирается некорректно
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Сервис Manticore падает при запуске и продолжает перезапускаться
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a, smth работает неправильно
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) Searchd крашится при использовании expr в ранжировщике, но только для запросов с двумя близостями
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action не работает
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL: Manticore падает при выполнении запроса и другие сбои во время восстановления кластера.
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE выводит без обратных кавычек
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) Теперь возможно выполнять запросы к Manticore из Java через JDBC коннектор
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) Проблемы ранжирования bm25f
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) Конфигурационные индексы зависают в watchdog в состоянии первой загрузки
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) Ошибка сегментирования при сортировке данных facet
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) крах на CONCAT(TO_STRING)
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) В некоторых случаях простой одиночный select мог вызвать зависание всего инстанса, из-за чего нельзя было войти в него или выполнить любой другой запрос, пока выполнялся select.
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Краш индекса
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) Неправильный подсчет из facet distinct
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) LCS неправильно вычисляется во встроенном ранжировщике sph04
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 dev падает
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) FACET с json в движке columnar вызывает краш
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL: 5.0.3 краш из-за вторичного индекса
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit в HandleMysqlSelectSysvar
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) Исправление распределения thread-chunk в RT индексах
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) Исправление распределения thread-chunk в RT индексах
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) неправильное значение max_query_time по умолчанию
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) Краш при использовании regex выражения в многопоточном исполнении
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) Нарушена обратная совместимость индексов
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool сообщает об ошибке при проверке атрибутов columnar
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) утечка памяти при клонировании json группировщиков
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) Утечка памяти при клонировании функции levenshtein
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) Сообщение об ошибке теряется при загрузке метаданных
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) Пропаганда ошибок из динамических индексов/подключей и системных переменных
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) Сбой при count distinct по столбцовому строковому полю в столбцовом хранилище
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: min(pickup_datetime) из taxi1 вызывает сбой
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) Пустой JSON запрос excludes удаляет колонки из списка select
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) Вторичные задачи, выполняемые на текущем планировщике, иногда вызывают ненормальные побочные эффекты
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) Сбой с facet distinct и разными схемами
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL: Столбцовый rt индекс повреждается после запуска без столбцовой библиотеки
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) Неявный cutoff не работает в json
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) Проблема с Columnar grouper
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) Невозможно удалить последнее поле из индекса
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) Неправильное поведение после --new-cluster
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) "columnar library not loaded", но библиотека не требуется
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) Нет ошибки для запроса delete
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) Исправлено расположение ICU data file в сборках для Windows
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) Проблема отправки Handshake
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) Отображение id в show create table
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Сбой / Сегментационная ошибка при Facet search с большим количеством результатов.
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT индекс: searchd «зависает» навсегда при вставке многих документов и полном заполнении RAMchunk
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) Поток зависает при остановке, если репликация занята между узлами
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) Смешивание float и int в фильтре JSON range могло заставить Manticore игнорировать фильтр
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) Float фильтры в JSON были неточными
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) Отмена неподтверждённых транзакций при изменении индекса (иначе возможен сбой)
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) Синтаксическая ошибка запроса при использовании обратного слэша
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) workers_clients могут быть неверны в SHOW STATUS
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) Исправлен сбой при слиянии ram сегментов без docstores
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) Исправлено пропущенное условие ALL/ANY для equals JSON фильтра
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) Репликация могла завершиться ошибкой `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)`, если searchd запускался из каталога без прав записи.
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) С версии 4.0.2 лог сбоев содержал только смещения. Этот коммит исправляет это.

## Версия 5.0.2
Релиз: 30 мая 2022

### Исправления ошибок
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - неправильный размер стека мог вызывать сбой.

## Версия 5.0.0
Релиз: 18 мая 2022


### Основные новые возможности
* 🔬 Поддержка [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/), которая включает бета-версию [Вторичных индексов](Server_settings/Searchd.md#secondary_indexes). Построение вторичных индексов включено по умолчанию для обычных и real-time столбцовых и построчных индексов (если используется [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)), но для включения поиска по ним нужно установить `secondary_indexes = 1` либо в конфигурационном файле, либо с помощью [SET GLOBAL](Server_settings/Setting_variables_online.md). Новая функциональность поддерживается во всех ОС, кроме старых Debian Stretch и Ubuntu Xenial.
* [Режим только для чтения](Security/Read_only.md): теперь можно задавать слушатели, обрабатывающие только запросы на чтение и отбрасывающие любые записи.
* Новый эндпоинт [/cli](Connecting_to_the_server/HTTP.md#/cli) для ещё более простого выполнения SQL-запросов через HTTP.
* Ускорённый пакетный INSERT/REPLACE/DELETE через JSON по HTTP: ранее можно было передавать несколько команд записи через HTTP JSON протокол, но они обрабатывались по одной, теперь они выполняются как одна транзакция.
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) Поддержка [Вложенных фильтров](Searching/Filters.md#Nested-bool-query) в JSON протоколе. Ранее в JSON нельзя было записать такие вещи как `a=1 and (b=2 or c=3)`: `must` (AND), `should` (OR) и `must_not` (NOT) работали только на верхнем уровне. Теперь их можно вкладывать.
* Поддержка [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) в протоколе HTTP. Теперь вы можете использовать передачу с разбиением на чанки в вашем приложении для отправки больших пакетов с меньшим потреблением ресурсов (поскольку вычисление `Content-Length` не требуется). На серверной стороне Manticore теперь всегда обрабатывает входящие HTTP-данные в потоковом режиме, не дожидаясь полной передачи всего пакета, как было раньше, что:
  - уменьшает пиковое использование оперативной памяти, снижая риск OOM
  - сокращает время отклика (наши тесты показали уменьшение на 11% для обработки пакета размером 100МБ)
  - позволяет обойти [max_packet_size](Server_settings/Searchd.md#max_packet_size) и передавать пакеты, гораздо превышающие максимальное допустимое значение `max_packet_size` (128МБ), например, 1ГБ за раз.
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) Поддержка интерфейса HTTP с `100 Continue`: теперь вы можете передавать большие пакеты из `curl` (включая библиотеки curl, используемые в различных языках программирования), которые по умолчанию выставляют `Expect: 100-continue` и некоторое время ждут перед фактической отправкой пакета. Ранее нужно было добавлять заголовок `Expect: `, теперь это не требуется.

  <details>

  Ранее (обратите внимание на время отклика):

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

  Сейчас:
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

* **⚠️ ВАЖНОЕ ИЗМЕНЕНИЕ**: [Псевдо-шардинг](Server_settings/Searchd.md#pseudo_sharding) включён по умолчанию. Если вы хотите его отключить, убедитесь, что добавили `pseudo_sharding = 0` в секцию `searchd` вашего конфигурационного файла Manticore.
* Наличие хотя бы одного полнотекстового поля в индексе real-time/plain больше не обязательно. Теперь вы можете использовать Manticore даже в случаях, не связанных с полнотекстовым поиском.
* [Быстрая выборка](Creating_a_table/Data_types.md#fast_fetch) для атрибутов на основе [Manticore Columnar Library](https://github.com/manticoresoftware/columnar): запросы вида `select * from <columnar table>` теперь работают намного быстрее, особенно если в схеме много полей.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Неявный [cutoff](Searching/Options.md#cutoff). Manticore теперь не тратит время и ресурсы на обработку данных, которые не нужны в результирующем наборе. Минус в том, что это влияет на `total_found` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) и [hits.total](Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) в JSON-выводе. Теперь оно точно отображается только если вы увидите `total_relation: eq`, а `total_relation: gte` означает, что фактическое количество документов, подходящих под критерии, больше, чем значение `total_found`, которое вы получили. Чтобы сохранить прежнее поведение, можно использовать опцию поиска `cutoff=0`, что заставит `total_relation` всегда быть `eq`.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Все полнотекстовые поля теперь по умолчанию [хранятся](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields). Чтобы сделать все поля не хранимыми (то есть вернуть прежнее поведение), нужно задать `stored_fields = ` (пустое значение).
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON теперь поддерживает [опции поиска](Searching/Options.md#General-syntax).

### Незначительные изменения
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Изменение формата мета-файлов индекса. Ранее мета-файлы (`.meta`, `.sph`) были в бинарном формате, теперь это просто json. Новая версия Manticore будет автоматически конвертировать старые индексы, но:
  - вы можете получить предупреждение вида `WARNING: ... syntax error, unexpected TOK_IDENT`
  - вы не сможете запускать индекс на прежних версиях Manticore, убедитесь, что у вас есть резервная копия
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Поддержка состояния сессии с помощью [HTTP keep-alive](Connecting_to_the_server/HTTP.md#Keep-alive). Это делает HTTP-соединение состоянием, если клиент это поддерживает. Например, используя новый эндпойнт [/cli](Connecting_to_the_server/HTTP.md#/cli) и HTTP keep-alive (включён по умолчанию во всех браузерах), можно вызывать `SHOW META` после `SELECT`, и это будет работать так же, как через mysql. Ранее HTTP-заголовок `Connection: keep-alive` тоже поддерживался, но всего лишь приводил к повторному использованию того же соединения. С этой версии он также делает сессию состоянием.
* Теперь можно указать `columnar_attrs = *`, чтобы определить все ваши атрибуты как колоночные в [plain mode](Read_this_first.md#Real-time-mode-vs-plain-mode), что удобно при большом списке.
* Быстрее репликация SST
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: Протокол репликации изменён. Если у вас рабочий кластер репликации, при обновлении на Manticore 5 нужно:
  - сначала корректно остановить все узлы
  - затем запустить последний остановленный узел с параметром `--new-cluster` (в Linux используйте инструмент `manticore_new_cluster`).
  - подробности смотрите в разделе о [перезапуске кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster).
* Улучшения репликации:
  - Быстрее SST
  - Устойчивость к шумам, что полезно при нестабильной сети между узлами репликации
  - Улучшенное логирование
* Улучшение безопасности: Manticore теперь слушает `127.0.0.1` вместо `0.0.0.0`, если в конфигурации вообще не указан параметр `listen`. Хотя в стандартной поставке Manticore Search указан `listen`, и не типично иметь конфигурацию без него, такая возможность существует. Ранее Manticore слушал на `0.0.0.0`, что небезопасно, теперь он слушает на `127.0.0.1`, который обычно не доступен из Интернета.
* Быстрее агрегация по колоночным атрибутам.
* Повышена точность функции `AVG()`: ранее для агрегаций внутри использовался тип `float`, теперь используется `double`, что значительно повышает точность.
* Улучшена поддержка JDBC MySQL драйвера.
* Поддержка `DEBUG malloc_stats` для [jemalloc](https://github.com/jemalloc/jemalloc).
* [optimize_cutoff](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) теперь доступен как настройка на уровне таблицы, которую можно задать при CREATE или ALTER таблицы.
* **⚠️ КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ**: [query_log_format](Server_settings/Searchd.md#query_log_format) теперь **по умолчанию `sphinxql`**. Если вы привыкли к формату `plain`, нужно добавить `query_log_format = plain` в файл конфигурации.
* Значительное улучшение потребления памяти: теперь Manticore значительно меньше использует ОЗУ при длинных и интенсивных нагрузках на insert/replace/optimize, если используются хранимые поля.
* Значение по умолчанию для [shutdown_timeout](Server_settings/Searchd.md#shutdown_timeout) увеличено с 3 секунд до 60 секунд.
* [Коммит ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) Поддержка Java mysql connector >= 6.0.3: в [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) изменён способ подключения к mysql, что сломало совместимость с Manticore. Новое поведение теперь поддерживается.
* [Коммит 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) отключено сохранение нового дискового чанка при загрузке индекса (например, при запуске searchd).
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) Поддержка glibc >= 2.34.
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) подсчёт 'VIP' соединений отдельно от обычных (не-VIP). Ранее VIP-соединения учитывались в лимите `max_connections`, что могло вызывать ошибку превышения лимита для обычных соединений. Теперь VIP-соединения не учитываются в лимите. Текущее число VIP-соединений можно увидеть в `SHOW STATUS` и `status`.
* [ID](Creating_a_table/Data_types.md#Document-ID) теперь можно указывать явно.
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) поддержка компрессии zstd для mysql proto

### ⚠️ Другие незначительные критические изменения
* ⚠️ Формула BM25F была слегка обновлена для улучшения релевантности поиска. Это влияет только на результаты поиска при использовании функции [BM25F()](Functions/Searching_and_ranking_functions.md#BM25F%28%29), поведение формулы ранжирования по умолчанию не меняется.
* ⚠️ Изменено поведение REST [/sql](Connecting_to_the_server/HTTP.md#mode=raw) endpoint: `/sql?mode=raw` теперь требует экранирования и возвращает массив.
* ⚠️ Изменён формат ответа на запросы `/bulk` INSERT/REPLACE/DELETE:
  - ранее каждая подзапрос была отдельной транзакцией и возвращала отдельный ответ
  - теперь весь пакет считается одной транзакцией, которая возвращает один ответ
* ⚠️ Опции поиска `low_priority` и `boolean_simplify` теперь требуют указания значения (`0/1`): раньше можно было писать `SELECT ... OPTION low_priority, boolean_simplify`, теперь нужно писать `SELECT ... OPTION low_priority=1, boolean_simplify=1`.
* ⚠️ Если вы используете старые клиенты [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php), [python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) или [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java), пожалуйста, перейдите по соответствующей ссылке и найдите обновлённую версию. **Старые версии не полностью совместимы с Manticore 5.**
* ⚠️ HTTP JSON запросы теперь логируются в другом формате в режиме `query_log_format=sphinxql`. Ранее логировалась только часть полнотекстового поиска, теперь логируется как есть.

### Новые пакеты
* **⚠️ СУЩЕСТВЕННОЕ ИЗМЕНЕНИЕ:** из-за новой структуры при обновлении до Manticore 5 рекомендуется удалить старые пакеты перед установкой новых:
  - Для RPM-систем: `yum remove manticore*`
  - Для Debian и Ubuntu: `apt remove manticore*`
* Новая структура deb/rpm пакетов. Ранее поставлялись:
  - `manticore-server` с `searchd` (основной демоны поиска) и всем необходимым для него
  - `manticore-tools` с `indexer` и `indextool`
  - `manticore` включающий всё
  - `manticore-all` RPM как мета-пакет, ссылающийся на `manticore-server` и `manticore-tools`

  Новая структура:
  - `manticore` - deb/rpm мета-пакет, который устанавливает всё вышеуказанное как зависимости
  - `manticore-server-core` - `searchd` и всё, что нужно для запуска отдельно
  - `manticore-server` - файлы systemd и другие вспомогательные скрипты
  - `manticore-tools` - `indexer`, `indextool` и другие инструменты
  - `manticore-common` - файл конфигурации по умолчанию, стандартный каталог данных, стандартные стоп-слова
  - `manticore-icudata`, `manticore-dev`, `manticore-converter` изменились мало
  - `.tgz` пакет, включающий все пакеты
* Поддержка Ubuntu Jammy
* Поддержка Amazon Linux 2 через [YUM репозиторий](Installation/RHEL_and_Centos.md#YUM-repository)

### Исправления ошибок
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) Случайный сбой при использовании UDF функции
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) Ошибка недостатка памяти при индексации RT индекса
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) Существенные изменения в sphinxql-парсере версии 3.6.0, 4.2.0
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) FATAL: недостаток памяти (не удалось выделить 9007199254740992 байт)
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) Строки некорректно передаются в UDF функции
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) Сбой searchd после попытки добавить текстовый столбец в RT индекс
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Индексатор не мог найти все столбцы
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) Группировка по json.boolean работает неправильно
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) Ошибки команд indextool, связанных с индексом (например, --dumpdict)
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) Поля исчезают из выборки
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) Несовместимость заголовка Content-Type в .NET HttpClient при использовании `application/x-ndjson`
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) Вычисление длины поля
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) В create/insert into/drop columnar таблице обнаружена утечка памяти
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) Пустой столбец в результатах при определённых условиях
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) Сбой демона при запуске
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) Демон зависает при запуске
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) Сбой при SST
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) Атрибут JSON помечен как columnar, когда engine='columnar'
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) Репликация слушает на 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * не работает с csvpipe
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) Сбой при выборе float в columnar в rt
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool изменяет rt индекс во время проверки
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) Необходима проверка пересечений диапазонов портов у слушателей
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) Логирование оригинальной ошибки в случае сбоя сохранения RT индекса на диск
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) Сообщается только об одной ошибке для конфигурации RE2
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) Изменения в потреблении оперативной памяти в коммите 5463778558586d2508697fa82e71d657ac36510f
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) Третий узел не становится непервичным кластером после грязного перезапуска
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) Счётчик обновлений увеличивается на 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) Новая версия 4.2.1 портит индекс, созданный в 4.2.0 с использованием морфологии
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) Отсутствует экранирование в ключах json /sql?mode=raw
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) Использование функции скрывает другие значения
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) Утечка памяти из-за строки в FixupAttrForNetwork
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) Утечка памяти в 4.2.0 и 4.2.1 связана с кешем docstore
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) Странный пинг-понг со сохранёнными полями через сеть
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) lemmatizer_base сбрасывается в пустое, если не указан в разделе 'common'
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding замедляет SELECT по id
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) DEBUG malloc_stats выводит нули при использовании jemalloc
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) Drop/add столбец делает значение невидимым
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) Нельзя добавить столбец bit(N) в колонарную таблицу
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) "cluster" обнуляется при запуске в manticore.json
* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP действия не отражаются в SHOW STATUS
* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) отключение pseudo_sharding для запросов с одним ключевым словом с низкой частотой
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) исправлено слияние сохранённых атрибутов и индекса
* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) обобщены выборщики distinct значений; добавлены специализированные выборщики distinct для колонарных строк
* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) исправлен выбор null целочисленных атрибутов из docstore
* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` мог быть указан дважды в логе запросов

## Версия 4.2.0, 23 декабря 2021

### Основные новые функции
* **Поддержка псевдо-шардинга для real-time индексов и полнотекстовых запросов**. В предыдущем выпуске мы добавили ограниченную поддержку псевдо-шардинга. Начиная с этой версии вы можете получить все преимущества псевдо-шардинга и вашего многоядерного процессора, просто включив [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding). Самое классное — вам не нужно ничего менять в индексах или запросах, просто включите его, и если у вас есть свободные ресурсы ЦП, они будут использованы для снижения времени отклика. Поддерживаются как обычные, так и real-time индексы для полнотекстовых, фильтрационных и аналитических запросов. Например, вот как включение псевдо-шардинга может сделать среднее **время отклика большинства запросов примерно в 10 раз меньше** на [наборе данных отобранных комментариев Hacker news](https://zenodo.org/record/45901/), умноженном в 100 раз (116 миллионов документов в обычном индексе).

![Pseudo sharding on vs off in 4.2.0](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) теперь поддерживается.

<!-- example pq_transactions_4.2.0 -->
* Транзакции PQ теперь атомарные и изолированные. Ранее поддержка транзакций PQ была ограничена. Это обеспечивает гораздо **быстрее REPLACE в PQ**, особенно когда нужно заменить много правил одновременно. Подробности производительности:

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

Вставка **1 миллиона правил PQ занимает 48 секунд**, а **REPLACE 40 тысяч правил** партиями по 10 тысяч — 406 секунд.

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

Вставка **1 миллиона правил PQ занимает 34 секунды**, а **REPLACE их** партиями по 10 тысяч — 23 секунды.

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
* [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) теперь доступен как параметр конфигурации в разделе `searchd`. Полезно, когда вы хотите ограничить количество RT-чанков во всех ваших индексах до определённого числа глобально.
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) точный [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) и [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) по нескольким локальным физическим индексам (real-time/обычным) с идентичными наборами/порядком полей.
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) поддержка bigint для `YEAR()` и других функций временной метки.
* [Коммит 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) Адаптивный [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Ранее Manticore Search собирал ровно до `rt_mem_limit` данных перед сохранением нового дискового чанка на диск, и во время сохранения продолжал собирать до 10% больше (т.н. двойной буфер) для минимизации возможной приостановки вставки. Если этот лимит тоже исчерпывался, добавление новых документов блокировалось до полного сохранения дискового чанка на диск. Новый адаптивный лимит основан на том, что теперь у нас есть [auto-optimize](Server_settings/Searchd.md#auto_optimize), поэтому не страшно, если дисковые чанки не полностью соответствуют `rt_mem_limit` и начинают сброс чанка на диск раньше. Теперь мы собираем до 50% `rt_mem_limit` и сохраняем это как дисковый чанк. При сохранении мы смотрим на статистику (сколько мы сохранили, сколько новых документов пришло во время сохранения) и пересчитываем исходный коэффициент, который будет использоваться в следующий раз. Например, если мы сохранили 90 миллионов документов, а во время сохранения пришло еще 10 миллионов, коэффициент составляет 90%, значит, в следующий раз мы можем собрать до 90% от `rt_mem_limit` перед началом сброса следующего дискового чанка. Значение коэффициента вычисляется автоматически от 33.3% до 95%.
* [Проблема #628](https://github.com/manticoresoftware/manticoresearch/issues/628) [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib) для источника PostgreSQL. Спасибо, [Dmitry Voronin](https://github.com/dimv36) за [вклад](https://github.com/manticoresoftware/manticoresearch/pull/630).
* [Коммит 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` и `--version`. Ранее можно было увидеть версию индексера, но `-v`/`--version` не поддерживались.
* [Проблема #662](https://github.com/manticoresoftware/manticoresearch/issues/662) бесконечный лимит mlock по умолчанию при запуске Manticore через systemd.
* [Коммит 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) spinlock -> очередь операций для coro rwlock.
* [Коммит 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) переменная окружения `MANTICORE_TRACK_RT_ERRORS` полезна для отладки повреждений RT-сегментов.

### Ломающее изменения
* Версия binlog была увеличена, бинлог предыдущей версии не будет воспроизведён, поэтому убедитесь, что вы корректно остановили Manticore Search во время обновления: после остановки предыдущего экземпляра в `/var/lib/manticore/binlog/` не должно быть файлов binlog, кроме `binlog.meta`.
* [Коммит 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) новый столбец "chain" в `show threads option format=all`. Отображает стек некоторых тикетов информации задач, наиболее полезен для целей профилирования, так что если вы парсите вывод `show threads`, обратите внимание на новый столбец.
* `searchd.workers` устарел с версии 3.5.0, теперь он помечен как deprecated, если он всё ещё есть в вашем конфигурационном файле, при запуске вы получите предупреждение. Manticore Search запустится, но с предупреждением.
* Если вы используете PHP и PDO для доступа к Manticore, вам нужно установить `PDO::ATTR_EMULATE_PREPARES`.

### Исправления ошибок
* ❗[Проблема #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 медленнее, чем Manticore 3.6.3. Версия 4.0.2 была быстрее предыдущих в плане массовых вставок, но значительно медленнее для вставки одиночных документов. Исправлено в версии 4.2.0.
* ❗[Коммит 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT индекс мог повредиться при интенсивной замене REPLACE, или мог аварийно завершиться.
* [Коммит 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) исправлены средние значения при объединении группировщиков и сортировщика group N; исправлено слияние агрегатов.
* [Коммит 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` мог аварийно завершиться.
* [Коммит 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) проблема истощения ОЗУ, вызванная UPDATE.
* [Коммит 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) демон мог зависнуть при INSERT.
* [Коммит 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) демон мог зависнуть при остановке.
* [Коммит f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) демон мог упасть при остановке.
* [Коммит 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) демон мог зависнуть при аварии.
* [Коммит f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) демон мог упасть при запуске пытаясь переподключиться к кластеру с неверным списком узлов.
* [Коммит 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) распределённый индекс мог быть полностью забыт в режиме RT, если не удавалось разрешить одного из агентов при запуске.
* [Проблема #683](https://github.com/manticoresoftware/manticoresearch/issues/683) атрибут bit(N) с engine='columnar' не работает.
* [Проблема #682](https://github.com/manticoresoftware/manticoresearch/issues/682) создание таблицы не проходит, но папка создаётся.
* [Проблема #663](https://github.com/manticoresoftware/manticoresearch/issues/663) Конфигурация не проходит с ошибкой: неизвестное имя ключа 'attr_update_reserve'.
* [Проблема #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore падает на пакетных запросах.
* [Проблема #679](https://github.com/manticoresoftware/manticoresearch/issues/679) Пакетные запросы снова вызывают аварии с версией v4.0.3.
* [Коммит f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) исправил сбой демона при старте при попытке повторного присоединения к кластеру с неверным списком узлов
* [Проблема #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 не принимает подключения после серии вставок
* [Проблема #635](https://github.com/manticoresoftware/manticoresearch/issues/635) Запрос FACET с ORDER BY JSON.field или строковым атрибутом мог вызывать сбой
* [Проблема #634](https://github.com/manticoresoftware/manticoresearch/issues/634) Сбой SIGSEGV при запросе с packedfactors
* [Коммит 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields не поддерживался при create table

## Версия 4.0.2, 21 сентября 2021

### Основные новые возможности
- **Полная поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**. Ранее Manticore Columnar Library поддерживалась только для plain-индексов. Теперь она поддерживается:
  - в индексах реального времени для `INSERT`, `REPLACE`, `DELETE`, `OPTIMIZE`
  - в репликации
  - в `ALTER`
  - в `indextool --check`
- **Автоматическая компактность индексов** ([Проблема #478](https://github.com/manticoresoftware/manticoresearch/issues/478)). Наконец, нет необходимости вызывать OPTIMIZE вручную или через crontask или иные механизмы автоматизации. Manticore теперь делает это автоматически и по умолчанию. Вы можете настроить порог компактности по умолчанию через глобальную переменную [optimize_cutoff](Server_settings/Setting_variables_online.md).
- **Переработка системы снимков и блокировок чанков**. Эти изменения могут сначала быть незаметны снаружи, но значительно улучшают поведение многих процессов в индексах реального времени. Короче говоря, ранее большинство операций изменения данных Manticore сильно зависели от блокировок, теперь мы используем снимки дисковых чанков.
- **Значительно более быстрая пакетная вставка (bulk INSERT) в индекс реального времени**. Например, на [сервере AX101 от Hetzner](https://www.hetzner.com/dedicated-rootserver/ax101) с SSD, 128 ГБ ОЗУ и AMD Ryzen™ 9 5950X (16*2 ядра) **с версией 3.6.0 можно было получить 236К документов в секунду** в таблицу со схемой `name text, email string, description text, age int, active bit(1)` (по умолчанию `rt_mem_limit`, размер партии 25000, 16 параллельных вставщиков, всего вставлено 16 миллионов документов). В версии 4.0.2 при том же уровне параллелизма/размера партии/количества достигается **357К документов в секунду**.

  <details>

  - операции чтения (например, SELECT, репликация) выполняются со снимками
  - операции, которые меняют только внутреннюю структуру индекса без изменения схемы/документов (например, слияние RAM-сегментов, сохранение дисковых чанков, слияние дисковых чанков) выполняются с использованием снимков только для чтения и в конце заменяют существующие чанки
  - UPDATE и DELETE выполняются по существующим чанкам, но в случае слияния записи накапливаются и затем применяются к новым чанкам
  - UPDATE последовательно захватывают эксклюзивную блокировку для каждого чанка. Слияния получают совместную блокировку при начале этапа сбора атрибутов из чанка. Таким образом, в каждый момент времени только одна операция (слияние или обновление) имеет доступ к атрибутам чанка.
  - когда слияние достигает этапа, когда нужны атрибуты, оно устанавливает специальный флаг. После окончания UPDATE проверяет этот флаг, и если он установлен, то всё обновление сохраняется в специальной коллекции. В конце, когда слияние завершается, оно применяет обновления к новорожденному дисковому чанку.
  - ALTER выполняется через эксклюзивную блокировку
  - репликация выполняется как обычная операция чтения, но дополнительно сохраняет атрибуты перед SST и запрещает обновления во время SST

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) теперь может добавлять/удалять полное текстовое поле** (в режиме RT). Ранее можно было только добавлять/удалять атрибут.
- 🔬 **Экспериментально: псевдо-шардинг для запросов полнотекстового поиска** — позволяет параллелить любой запрос без полнотекстового поиска. Вместо ручной подготовки шардов теперь можно просто включить новую опцию [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) и ожидать до `CPU cores` раз уменьшения времени ответа для запросов без полнотекстового поиска. Учтите, что это может занять все существующие ядра CPU, так что если вам важна не только задержка, но и пропускная способность — используйте с осторожностью.

### Мелкие изменения
<!-- example -->
- Поддержка Linux Mint и Ubuntu Hirsute Hippo через [APT репозиторий](Installation/Debian_and_Ubuntu.md#APT-repository)
- более быстрая обновление по id через HTTP в больших индексах в некоторых случаях (зависит от распределения id)
- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) — добавлено кэширование для lemmatizer-uk


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
- [кастомные флаги запуска для systemd](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd). Теперь не нужно запускать searchd вручную, если требуется запуск Manticore с конкретным флагом запуска
- новая функция [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29), вычисляющая расстояние Левенштейна
- добавлены новые [флаги запуска searchd](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` и `--replay-flags=ignore-all-errors`, которые позволяют запускать searchd при повреждённом binlog
- [Проблема #621](https://github.com/manticoresoftware/manticoresearch/issues/621) — отображение ошибок из RE2
- более точный [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) для распределённых индексов, состоящих из локальных plain-индексов
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) для удаления дубликатов при фасетном поиске
- [модификатор точной формы](Searching/Full_text_matching/Operators.md#Exact-form-modifier) теперь не требует [morphology](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) и работает для индексов с включённым [infix/prefix](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) поиском

### Ломающие изменения
- новая версия может читать старые индексы, но старые версии не могут читать индексы Manticore 4
- убрана неявная сортировка по id. Сортируйте явно, если требуется
- значение по умолчанию для `charset_table` изменено с `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` на `non_cjk`
- `OPTIMIZE` теперь происходит автоматически. Если это не нужно, убедитесь, что в секции `searchd` конфигурационного файла установлено `auto_optimize=0`
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` устарели, теперь они удалены
- для участников разработки: теперь мы используем компилятор Clang для сборок на Linux, так как по нашим тестам он позволяет собрать более быстродействующий Manticore Search и Manticore Columnar Library
- если [max_matches](Searching/Options.md#max_matches) не указан в поисковом запросе, он неявно обновляется до минимально необходимого значения ради производительности нового колоночного хранилища. Это может повлиять на метрику `total` в [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), но не на `total_found`, которая показывает фактическое количество найденных документов.

### Миграция с Manticore 3
- убедитесь, что вы корректно остановили Manticore 3:
  - никаких binlog файлов не должно быть в `/var/lib/manticore/binlog/` (только `binlog.meta` должен находиться в директории)
  - иначе индексы, по которым Manticore 4 не может воспроизвести binlog, не будут запущены
- новая версия может читать старые индексы, но старые версии не могут читать индексы Manticore 4, поэтому сделайте резервную копию, если хотите иметь возможность легко откатиться на старую версию
- если вы используете кластер с репликацией, убедитесь, что:
  - сначала корректно остановлены все ваши узлы
  - затем запущен последний остановленный узел с параметром `--new-cluster` (используйте утилиту `manticore_new_cluster` в Linux)
  - подробнее читайте про [перезапуск кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster)

### Исправления ошибок
- Исправлено множество проблем с репликацией:
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - исправлен сбой при SST на узле присоединения с активным индексом; добавлена проверка sha1 на узле присоединения при записи частей файлов для ускорения загрузки индекса; добавлена ротация изменённых файлов индекса на узле присоединения при загрузке индекса; добавлено удаление файлов индекса на узле присоединения при замене активного индекса новым с узла донора; добавлены точки лога репликации на узле донора для отправки файлов и частей
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - сбой при JOIN CLUSTER в случае некорректного адреса
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - при первоначальной репликации большого индекса узел присоединения мог завершиться с ошибкой `ERROR 1064 (42000): invalid GTID, (null)`, донор мог стать неотзывчивым, когда присоединялся другой узел
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - хеш мог рассчитываться неправильно для большого индекса, что могло привести к сбою репликации
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - репликация прерывалась при перезапуске кластера
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` не отображал параметр `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd высокого CPU при простое примерно через день
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - немедленный сброс .meta
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - файл manticore.json очищался
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait не срабатывал под root. Исправлено поведение systemctl (раньше показывалась ошибка ExecStop и не дожидался корректной остановки searchd)
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE против SHOW STATUS. `command_insert`, `command_replace` и другие показывали неверные метрики
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - `charset_table` для обычного индекса имел неверное значение по умолчанию
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - новые дисковые части не блокировались в памяти (mlocked)
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - узел кластера Manticore падал при невозможности разрешить имя узла
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - репликация обновлённого индекса могла приводить к неопределённому состоянию
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - индексатор мог зависать при индексации обычного источника индекса с json-атрибутом
- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - исправлен фильтр с оператором "не равно" в PQ индексе
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - исправлен выбор окон (select windows) в запросах по списку с более чем 1000 совпадений. Ранее `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` не работал
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - HTTPS-запрос к Manticore мог вызывать предупреждение "max packet size(8388608) exceeded"
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 мог зависать после нескольких обновлений строковых атрибутов


## Версия 3.6.0, 3 мая 2021
**Релиз технического обслуживания перед Manticore 4**

### Основные новые возможности
- Поддержка [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) для обычных индексов. Новая настройка [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) для обычных индексов
- Поддержка [Украинского лемматизатора](https://github.com/manticoresoftware/lemmatizer-uk)
- Полностью переработанные гистограммы. При построении индекса Manticore также строит гистограммы для каждого поля, которые затем используются для более быстрой фильтрации. В версии 3.6.0 алгоритм был полностью пересмотрен, и вы можете получить более высокую производительность при большом объёме данных и большом количестве фильтраций.

### Незначительные изменения
- инструмент `manticore_new_cluster [--force]`, полезный для перезапуска кластера репликации через systemd
- [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) для `indexer --merge`
- [новый режим](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- добавлена [поддержка экранирования пути JSON](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL) с помощью обратных кавычек
- [indextool --check](Miscellaneous_tools.md#indextool) может работать в режиме RT
- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) для SELECT/UPDATE
- идентификатор чанка для объединённого дискового чанка теперь уникален
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### Оптимизации
- [ускоренный разбор JSON](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606), наши тесты показывают снижение задержки на 3-4% для запросов вида `WHERE json.a = 1`
- недокументированная команда `DEBUG SPLIT` как предпосылка для автоматического шардинга/ребаланса

### Исправления ошибок
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - неточные и нестабильные результаты FACET
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - странное поведение при использовании MATCH: пострадавшим от этой проблемы необходимо перестроить индекс, поскольку ошибка возникала на этапе построения индекса
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - периодические аварийные дампы при выполнении запросов с функцией SNIPPET()
- Оптимизации стека, полезные для обработки сложных запросов:
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - CRASH DUMP при выполнении SELECT
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - определение размера стека для деревьев фильтров
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - обновление с использованием условия IN не применяется корректно
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - SHOW STATUS сразу после CALL PQ возвращает - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - Исправлена статическая сборка бинарника
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - ошибка в мультизапросах
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - невозможно использовать необычные имена для столбцов при использовании 'create table'
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - сбой демона при повторе binlog с обновлением строкового атрибута; версия binlog установлена в 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - исправлено определение рамки стека выражений во время выполнения (тест 207)
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - фильтр индекса percolate и теги были пусты для пустого сохранённого запроса (тест 369)
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - сбои потока SST репликации при сети с большой задержкой и высокой ошибочностью (репликация между разными дата-центрами); версия команды репликации обновлена до 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - блокировка joiner кластера при операциях записи после присоединения к кластеру (тест 385)
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - совпадение с подстановочными знаками с точным модификатором (тест 321)
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - контрольные точки docid против docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - непоследовательное поведение indexer при разборе некорректного xml
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - сохранённый запрос percolate с NOTNEAR выполняется бесконечно (тест 349)
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - неправильный вес для фразы, начинающейся с подстановочного знака
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - запрос percolate с подстановочными знаками генерирует термины без полезной нагрузки при совпадении, что вызывает перемешанные попадания и нарушает совпадение (тест 417)
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - исправлен расчёт 'total' в случае параллельного запроса
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - сбой на Windows при множественных одновременных сессиях у демона
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - некоторые настройки индекса не могли быть реплицированы
- [Коммит 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - При высокой частоте добавления новых событий netloop иногда зависает из-за того, что атомарное событие 'kick' обрабатывается один раз для нескольких событий одновременно, из-за чего теряются их фактические действия
статус запроса, а не статус сервера
- [Коммит d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - Новый сброшенный на диск чанк может быть утерян при фиксации
- [Коммит 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - неточная 'net_read' в профайлере
- [Коммит f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - Проблема с Percolate для арабского (тексты справа налево)
- [Коммит 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - id некорректно выбирался при дублировании имени столбца
- [Коммит refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) сетевых событий для исправления сбоя в редких случаях
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) исправление в `indextool --dumpheader`
- [Коммит ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE работал неправильно со сохраняемыми полями

### Ломающие изменения:
- Новый формат binlog: перед обновлением необходимо корректно остановить Manticore
- Формат индекса слегка изменился: новая версия без проблем читает ваши существующие индексы, но если вы решите понизить версию с 3.6.0 до более старой, более новые индексы станут нечитаемыми
- Изменение формата репликации: не реплицируйте с более старой версии на 3.6.0 и наоборот, переключайтесь на новую версию на всех узлах одновременно
- Опция `reverse_scan` устарела. Убедитесь, что вы не используете эту опцию в ваших запросах начиная с версии 3.6.0, иначе они не будут работать
- Начиная с этого релиза мы больше не поддерживаем сборки для RHEL6, Debian Jessie и Ubuntu Trusty. Если для вас это критично, [свяжитесь с нами](https://manticoresearch.com/contact-us/)

### Устаревшие функции
- Больше нет неявной сортировки по id. Если вы на нее полагаетесь, обновите свои запросы соответствующим образом
- Опция поиска `reverse_scan` устарела

## Версия 3.5.4, 10 декабря 2020

### Новые возможности
- Новые клиенты для Python, Javascript и Java теперь доступны в общей доступности и хорошо документированы в этом руководстве.
- автоматическое удаление дискового чанка RT-индекса. Эта оптимизация позволяет автоматически удалять дисковый чанк при [OPTIMIZing](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) RT-индекса, когда чанк явно больше не нужен (все документы удалены). Ранее требовалось слияние, теперь чанк можно удалить мгновенно. Опция [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) игнорируется, т.е. даже если фактически ничего не сливается, устаревший дисковый чанк удаляется. Это полезно, если вы поддерживаете политику хранения в индексе и удаляете старые документы. Теперь компактация таких индексов будет быстрее.
- [standalone NOT](Searching/Options.md#not_terms_only_allowed) как опция для SELECT

### Небольшие изменения
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) Новая опция [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) полезна, если вы запускаете `indexer --all` и в конфигурационном файле есть не только plain-индексы. Без `ignore_non_plain=1` будет предупреждение и соответствующий код выхода.
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) и [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) позволяют визуализировать план выполнения полнотекстового запроса. Полезно для понимания сложных запросов.

### Устаревшие функции
- `indexer --verbose` устарел, так как не добавлял полезной информации в вывод индексатора
- Для дампа бэктрейса watchdog теперь следует использовать сигнал `USR2` вместо `USR1`

### Исправления ошибок
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) кириллический символ точка в режиме вызова сниппетов не подсвечивается
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY expression select = критический сбой
- [Коммит 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) статус searchd показывает Segmentation fault при работе в кластере
- [Коммит 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' не обрабатывает чанки >9
- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) Ошибка, приводящая к сбою Manticore
- [Коммит fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) Конвертер создаёт повреждённые индексы
- [Коммит eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 vs CALL SNIPPETS()
- [Коммит ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) count distinct возвращает 0 при низком max_matches на локальном индексе
- [Коммит 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) При использовании агрегации сохраняемые тексты не возвращаются в подборках


## Версия 3.5.2, 1 октября 2020

### Новые возможности

* OPTIMIZE сокращает число дисковых чанков до заданного количества (по умолчанию `2* No. of cores`), а не до одного. Оптимальное число чанков можно контролировать с помощью опции [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).
* Оператор NOT теперь можно использовать автономно. По умолчанию он отключен, так как случайные одиночные запросы NOT могут быть медленными. Его можно включить, установив новую директиву searchd [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) в значение `0`.
* Новая настройка [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) определяет, сколько потоков может использовать запрос. Если директива не установлена, запрос может использовать потоки до значения [threads](Server_settings/Searchd.md#threads).
Для каждого запроса `SELECT` количество потоков можно ограничить с помощью [OPTION threads=N](Searching/Options.md#threads), переопределяя глобальную настройку `max_threads_per_query`.
* Индексы Percolate теперь можно импортировать с помощью [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md).
* HTTP API `/search` получает базовую поддержку [фасетирования](Searching/Faceted_search.md#HTTP-JSON)/[группировки](Searching/Grouping.md) через новый узел запроса `aggs`.

### Незначительные изменения

* Если директива replication listen не объявлена, движок будет пытаться использовать порты после заданного порта 'sphinx', до порта 200.
* Для подключений SphinxSE или клиентов SphinxAPI необходимо явно указать `listen=...:sphinx`.
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) выводит новые метрики: `killed_documents`, `killed_rate`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists` и `disk_mapped_cached_hitlists`.
* SQL команда `status` теперь выводит `Queue\Threads` и `Tasks\Threads`.

### Устаревшие функции:

* `dist_threads` полностью устарел, searchd будет записывать предупреждение, если директива по-прежнему используется.

### Docker

Официальный Docker-образ теперь основан на Ubuntu 20.04 LTS

### Упаковка

Помимо обычного пакета `manticore`, вы также можете установить Manticore Search по компонентам:

- `manticore-server-core` - предоставляет `searchd`, man-страницу, каталог логов, API и galera модуль. Также установит `manticore-common` как зависимость.
- `manticore-server` - предоставляет скрипты автоматизации для ядра (init.d, systemd) и обертку `manticore_new_cluster`. Также установит `manticore-server-core` как зависимость.
- `manticore-common` - предоставляет конфигурацию, стоп-слова, общие документы и скелетные папки (datadir, modules и пр.)
- `manticore-tools` - содержит вспомогательные инструменты ( `indexer`, `indextool` и др.), их man-страницы и примеры. Также установит `manticore-common` как зависимость.
- `manticore-icudata` (RPM) или `manticore-icudata-65l` (DEB) - содержит файл данных ICU для использования морфологии icu.
- `manticore-devel` (RPM) или `manticore-dev` (DEB) - содержит заголовочные файлы для разработки UDF.

### Исправления ошибок

1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) Сбой демона при группировщике в RT индексе с разными чанками
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) Быстрый путь для пустых удалённых документов
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) Обнаружение фрейма стека выражения во время выполнения
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Сопоставление более 32 полей в percolate индексах
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) Диапазон портов для replication listen
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) Показать create table для pq
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) Поведение HTTPS порта
8. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) Перемешивание строк docstore при замене
9. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) Уровень сообщения о недоступности TFO изменён на 'info'
10. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) Сбой при некорректном использовании strcmp
11. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) Добавление индекса в кластер с системными (стоп-словами) файлами
12. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) Слияние индексов с большими словарями; оптимизация RT больших дисковых чанков
13. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool может выгружать метаданные из текущей версии
14. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) Ошибка в порядке группировки в GROUP N
15. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) Явная очистка для SphinxSE после рукопожатия
16. [Commit 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) Избегание копирования больших описаний, когда это не требуется
17. [Commit 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) Отрицательное время в show threads
18. [Commit f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) Плагин фильтра токенов против нулевых дельт позиции
19. [Commit a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) Изменение 'FAIL' на 'WARNING' при множественных совпадениях

## Версия 3.5.0, 22 июля 2020

### Основные новые функции:
* Этот релиз занял так много времени, потому что мы усердно работали над переходом мультизадачности с потоков на **корутины**. Это упрощает конфигурацию и делает параллелизацию запросов гораздо более прямолинейной: Manticore теперь использует заданное количество потоков (см. новую настройку [threads](Server_settings/Searchd.md#threads)), а новый режим гарантирует выполнение задачи самым оптимальным образом.
* Изменения в [выделении текста (highlighting)](Searching/Highlighting.md#Highlighting-options):
  - любое выделение текста, которое работает с несколькими полями (`highlight({},'field1, field2'`) или `highlight` в JSON-запросах) теперь по умолчанию применяет ограничения по полям.
  - любое выделение, работающее с обычным текстом (`highlight({}, string_attr)` или `snippet()`) теперь применяет ограничения ко всему документу.
  - [ограничения по полям](Searching/Highlighting.md#limits_per_field) можно переключить на глобальные ограничения с помощью опции `limits_per_field=0` (по умолчанию `1`).
  - [allow_empty](Searching/Highlighting.md#allow_empty) теперь равно `0` по умолчанию для выделения через HTTP JSON.

* Один и тот же порт [теперь можно использовать](Server_settings/Searchd.md#listen) для http, https и бинарного API (для принятия соединений от удалённого экземпляра Manticore). Всё ещё требуется `listen = *:mysql` для соединений по протоколу mysql. Manticore теперь автоматически определяет тип клиента, пытающегося подключиться, за исключением MySQL (из-за ограничений протокола).

* В RT-режиме поле теперь может быть одновременно [текстовым и строковым атрибутом](Creating_a_table/Data_types.md#String) — [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331).

  В [обычном режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) это называется `sql_field_string`. Теперь оно доступно и в [RT-режиме](Read_this_first.md#Real-time-mode-vs-plain-mode) для индексов в реальном времени. Пример использования:

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
* Теперь вы можете [выделять текст в строковых атрибутах](Searching/Highlighting.md#Highlighting-via-SQL).
* Поддержка SSL и сжатия для SQL интерфейса.
* Поддержка команды mysql-клиента [`status`](Node_info_and_management/Node_status.md#STATUS).
* [Репликация](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) теперь может реплицировать внешние файлы (stopwords, exceptions и др.).
* Оператор фильтра [`in`](Searching/Filters.md#Set-filters) теперь доступен через HTTP JSON интерфейс.
* [`expressions`](Searching/Expressions.md#expressions) в HTTP JSON.
* [Теперь можно менять `rt_mem_limit` на лету](https://github.com/manticoresoftware/manticoresearch/issues/344) в RT-режиме, то есть использовать `ALTER ... rt_mem_limit=<new value>`.
* Теперь доступны [отдельные таблицы символов CJK](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology): `chinese`, `japanese` и `korean`.
* [thread_stack](Server_settings/Searchd.md#thread_stack) теперь ограничивает максимальный размер стека потока, а не начальный.
* Улучшен вывод `SHOW THREADS`.
* Отображение прогресса длительных `CALL PQ` в `SHOW THREADS`.
* cpustat, iostat, coredump можно изменять во время работы с помощью [SET](Server_settings/Setting_variables_online.md#SET).
* Реализовано `SET [GLOBAL] wait_timeout=NUM`.

### Ломающие изменения:
* **Формат индекса изменён.** Индексы, построенные в версии 3.5.0, не могут быть загружены версиями Manticore ниже 3.5.0, но Manticore 3.5.0 понимает старые форматы.
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) (т.е. без указания списка столбцов) ранее ожидал ровно `(query, tags)` в значениях. Теперь это изменено на `(id,query,tags,filters)`. id можно установить в 0, если хотите, чтобы он генерировался автоматически.
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) — новое значение по умолчанию для выделения через HTTP JSON интерфейс.
* Разрешены только абсолютные пути для внешних файлов (stopwords, exceptions и др.) в `CREATE TABLE`/`ALTER TABLE`.

### Устаревание:
* `ram_chunks_count` был переименован в `ram_chunk_segments_count` в `SHOW INDEX STATUS`.
* `workers` устарел. Сейчас существует только один режим работы с воркерами.
* `dist_threads` устарел. Все запросы теперь максимально параллельны (ограничены `threads` и `jobs_queue_size`).
* `max_children` устарел. Используйте [threads](Server_settings/Searchd.md#threads) для установки числа потоков, которые будет использовать Manticore (по умолчанию равно числу ядер CPU).
* `queue_max_length` устарел. Вместо него, если действительно необходимо, используйте [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) для тонкой настройки размера внутренней очереди заданий (по умолчанию без ограничений).
* Все эндпоинты `/json/*` теперь доступны без `/json/`, например, `/search`, `/insert`, `/delete`, `/pq` и т.д.
* Значение `field`, означающее "полнотекстовое поле", было переименовано в "text" в `describe`.
  <!-- more -->
  **3.4.2:**
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | field  | indexed stored |
  +-------+--------+----------------+
  ```
  **3.5.0**:
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | text   | indexed stored |
  +-------+--------+----------------+
  ```
  <!-- \more -->
* Кириллическая `и` больше не сопоставляется с `i` в `non_cjk` charset_table (которая стоит по умолчанию), так как это слишком сильно влияло на русские стеммеры и лемматизаторы.
* `read_timeout`. Используйте вместо него [network_timeout](Server_settings/Searchd.md#network_timeout), который контролирует и чтение, и запись.


### Пакеты

* Официальный пакет Ubuntu Focal 20.04
* Имя deb-пакета изменено с `manticore-bin` на `manticore`

### Исправления ошибок:
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) утечка памяти у searchd
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) Незначительный выход за границы при чтении в сниппетах
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) Опасная запись в локальную переменную для аварийных запросов
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) Незначительная утечка памяти сортировщика в тесте 226
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) Большая утечка памяти в тесте 226
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) Кластер показывает, что ноды синхронизированы, но `count(*)` показывает разные числа
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметика: дублирующиеся и иногда пропадающие предупреждающие сообщения в логах
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Косметика: (null) в имени индекса в логах
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) Невозможно получить больше 70 млн результатов
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) Невозможно вставить правила PQ с синтаксисом no-columns
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) Ввод документа в индекс в кластере выдает вводящую в заблуждение ошибку
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` и `json/update` возвращают идентификаторы в экспоненциальной форме
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) Обновление JSON скалярных свойств и MVA в одном запросе
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` не работает в режиме RT
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `ALTER RECONFIGURE` в режиме RT должно быть запрещено
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `rt_mem_limit` сбрасывается в 128M после перезапуска searchd
17. highlight() иногда зависает
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) Не удалось использовать код U+ в режиме RT
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) Не удалось использовать подстановочный знак в wordforms в режиме RT
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) Исправлен `SHOW CREATE TABLE` с несколькими файлами wordform
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON-запрос без поля "query" приводит к сбою searchd
22. Официальный докер Manticore [official docker](https://hub.docker.com/r/manticoresearch/manticore) не может индексировать из MySQL 8
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert требует id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` не работает для PQ
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` некорректно работает для PQ
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) Конец строки в настройках в show index status
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) Пустой заголовок в "highlight" в HTTP JSON ответе
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) Ошибка парсинга в `CREATE TABLE LIKE`
29. [Commit 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) Сбой RT под нагрузкой
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) Потерян лог сбоя при падении RT дискового чанка
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) Импорт таблицы не удался и закрывает соединение
32. [Commit 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` портит PQ индекс
33. [Commit 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) Проблемы с перезагрузкой searchd после смены типа индекса
34. [Commit 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) Демон падает при импорте таблицы с отсутствующими файлами
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) Сбой при select с несколькими индексами, group by и ranker = none
36. [Commit c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` не подсвечивает строки в строковых атрибутах
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` не сортирует по строковому атрибуту
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) Ошибка в случае отсутствия каталога данных
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* не поддерживается в режиме RT
40. [Commit 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) Плохие JSON объекты в строках: 1. `CALL PQ` возвращает "Bad JSON objects in strings: 1", когда json превышает определенное значение.
41. [Commit 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) Несогласованность в RT-режиме. В некоторых случаях нельзя удалить индекс, так как он неизвестен, и нельзя создать, так как каталог не пуст.
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) Сбой при select
43. [Commit 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M выдает предупреждение на поле размером 2М
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) Ошибка при выполнении условий запроса
45. [Коммит dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) Простой поиск по 2 терминам находит документ, содержащий только один термин
46. [Коммит 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) В PQ было невозможно сопоставить json с заглавными буквами в ключах
47. [Коммит 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) Индексатор падает при csv+docstore
48. [Issue #363](https://github.com/manticoresoftware/manticoresearch/issues/363) использование `[null]` в json атрибуте на centos 7 вызывает поврежденные вставленные данные
49. Критический [Issue #345](https://github.com/manticoresoftware/manticoresearch/issues/345) Записи не вставляются, count() случайное, "replace into" возвращает OK
50. max_query_time слишком сильно замедляет SELECTы
51. [Issue #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Ошибка связи master-agent на Mac OS
52. [Issue #328](https://github.com/manticoresoftware/manticoresearch/issues/328) Ошибка при подключении к Manticore с Connector.Net/Mysql 8.0.19
53. [Коммит daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) Исправлено экранирование \0 и оптимизирована производительность
54. [Коммит 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) Исправлен count distinct vs json
55. [Коммит 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) Исправлен сбой при drop table на другом узле
56. [Коммит 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) Исправлены сбои при интенсивном вызове pq


## Версия 3.4.2, 10 апреля 2020
### Критические исправления ошибок
* [Коммит 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) исправлена ошибка, из-за которой RT индекс из старой версии не мог индексировать данные

## Версия 3.4.0, 26 марта 2020
### Основные изменения
* сервер работает в 2 режимах: rt-режим и обычный режим
   *   rt-режим требует data_dir и отсутствие описания индексов в конфиге
   *   в обычном режиме индексы определяются в конфиге; использование data_dir запрещено
* репликация доступна только в rt-режиме

### Мелкие изменения
* charset_table по умолчанию alias non_cjk
* в rt-режиме полнотекстовые поля индексируются и сохраняются по умолчанию
* полнотекстовые поля в rt-режиме переименованы с 'field' в 'text'
* ALTER RTINDEX переименован в ALTER TABLE
* TRUNCATE RTINDEX переименован в TRUNCATE TABLE

### Функции
* поля только для хранения
* SHOW CREATE TABLE, IMPORT TABLE

### Улучшения
* гораздо более быстрый lockless PQ
* /sql может выполнять любой тип SQL-запроса в режиме mode=raw
* алиас mysql для протокола mysql41
* состояние по умолчанию state.sql в data_dir

### Исправления ошибок
* [Коммит a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) исправлен сбой при неправильном синтаксисе поля в highlight()
* [Коммит 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) исправлен сбой сервера при репликации RT индекса с docstore
* [Коммит 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) исправлен сбой highlight для индекса с опцией infix или prefix и индекса без включенных полей для хранения
* [Коммит 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) исправлена ложная ошибка про пустой docstore и поиск по dock-id для пустого индекса
* [Коммит a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправлен #314 SQL команда insert с завершающей точкой с запятой
* [Коммит 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) удалено предупреждение о несоответствии слова(слов) запроса
* [Коммит b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) исправлены запросы в сниппетах, разделённые через ICU
* [Коммит 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) исправлена гонка при поиске/добавлении в кеш блоков docstore
* [Коммит f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) исправлена утечка памяти в docstore
* [Коммит a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) исправлен #316 LAST_INSERT_ID возвращает пустое значение на INSERT
* [Коммит 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) исправлен #317 json/update HTTP endpoint для поддержки массива для MVA и объекта для JSON атрибута
* [Коммит e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) исправлен всплеск сбросов индексатора rt без явного id

## Версия 3.3.0, 4 февраля 2020
### Функции
* Параллельный поиск в Real-Time индексе
* Команда EXPLAIN QUERY
* конфигурационный файл без определения индексов (альфа версия)
* команды CREATE/DROP TABLE (альфа версия)
* indexer --print-rt - может читать из источника и выводить INSERTы для Real-Time индекса

### Улучшения
* Обновлены стеммеры Snowball до версии 2.0
* LIKE фильтр для SHOW INDEX STATUS
* улучшено использование памяти при высоком max_matches
* SHOW INDEX STATUS добавляет ram_chunks_count для RT индексов
* lockless PQ
* изменён LimitNOFILE на 65536


### Исправления ошибок
* [Коммит 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) добавлена проверка схемы индекса на дублирование атрибутов #293
* [Коммит a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) исправлен сбой в hitless terms
* [Коммит 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) исправлен разрыв docstore после ATTACH
* [Коммит d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) исправлена проблема docstore в распределённой настройке
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) заменить FixedHash на OpenHash в сортировщике
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) исправить атрибуты с дублирующимися именами в определении индекса
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) исправить html_strip в HIGHLIGHT()
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) исправить макрос passage в HIGHLIGHT()
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) исправить проблемы с двойным буфером при создании RT индекса для маленьких или больших дисковых чанков
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) исправить удаление события для kqueue
* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) исправить сохранение дискового чанка для большого значения rt_mem_limit у RT индекса
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) исправить переполнение float при индексировании
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) исправить ошибку вставки документа с отрицательным ID в RT индекс
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) исправить сбой сервера на ranker fieldmask
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) исправить сбой при использовании кеша запросов
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) исправить сбой при использовании RAM-сегментов RT индекса с параллельными вставками

## Версия 3.2.2, 19 декабря 2019
### Возможности
* Автоинкремент ID для RT индексов
* Поддержка подсветки для docstore через новую функцию HIGHLIGHT(), доступную также в HTTP API
* SNIPPET() может использовать специальную функцию QUERY(), которая возвращает текущий MATCH запрос
* новая опция field_separator для функций подсветки.

### Улучшения и изменения
* отложенная выборка сохранённых полей для удалённых узлов (значительно повышает производительность)
* строки и выражения больше не нарушают оптимизации мультизапросов и FACET
* сборка для RHEL/CentOS 8 теперь использует mysql libclient из mariadb-connector-c-devel
* файл данных ICU теперь поставляется вместе с пакетами, icu_data_dir удалён
* файлы сервиса systemd включают политику 'Restart=on-failure'
* indextool теперь может проверять RT индексы онлайн
* конфигурация по умолчанию теперь /etc/manticoresearch/manticore.conf
* сервис на RHEL/CentOS переименован в 'manticore' вместо 'searchd'
* удалены опции query_mode и exact_phrase для сниппетов

### Исправления ошибок
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) исправить сбой при SELECT запросе через HTTP интерфейс
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) исправить сохранение дисковых чанков у RT индекса без пометки некоторых документов как удалённых
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) исправить сбой при поиске по нескольким индексам или мультизапросам с dist_threads
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) исправить сбой при генерации infix для длинных терминов с широкими utf8 кодовыми точками
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) исправить гонку при добавлении сокета в IOCP
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) исправить проблему с bool запросами и json списком select
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) исправить проверку indextool с сообщением о неправильном смещении skiplist, проверка doc2row lookup
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) исправить генерацию плохого индекса у индексатора из-за отрицательного смещения skiplist при большом объёме данных
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) исправить преобразование JSON только числовых в строки и преобразование JSON строки в число в выражениях
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) исправить выход indextool с кодом ошибки при установке нескольких команд в командной строке
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) исправить #275 ошибочное состояние binlog при ошибке "нет места на диске"
* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) исправить #279 сбой при фильтре IN для JSON атрибута
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) исправить #281 неправильный вызов закрытия канала
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) исправить зависание сервера при вызове PQ с рекурсивным JSON атрибутом, закодированным как строка
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) исправить выход за пределы списка документов в узле multiand
* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) исправить получение публичной информации потока
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) исправить блокировки кеша docstore

## Версия 3.2.0, 17 октября 2019
### Возможности
* Хранение документов
* новые директивы stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level

### Улучшения и изменения
* улучшена поддержка SSL
* обновлена встроенная кодировка non_cjk
* отключено логирование операторов UPDATE/DELETE как SELECT в журнале запросов
* пакеты для RHEL/CentOS 8

### Исправления ошибок
* [Коммит 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) исправлен сбой при замене документа в дисковом чанке RT индекса
* [Коммит 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) исправлена ошибка \#269 LIMIT N OFFSET M
* [Коммит 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) исправлена обработка DELETE-запросов с явно заданным id или списком id для пропуска поиска
* [Коммит 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) исправлен неправильный индекс после удаления события в netloop в поллере windowspoll
* [Коммит 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) исправлено округление float в JSON через HTTP
* [Коммит 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) исправлена проверка пустого пути для удаленных сниппетов; исправлены тесты на Windows
* [Коммит aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) исправлена перезагрузка конфигурации для работы на Windows так же, как на Linux
* [Коммит 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) исправлена ошибка \#194, PQ теперь работает с морфологией и стеммерами
* [Коммит 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) исправлено управление устаревшими сегментами RT

## Версия 3.1.2, 22 августа 2019
### Особенности и улучшения
* Экспериментальная поддержка SSL для HTTP API
* фильтр по полям для CALL KEYWORDS
* max_matches для /json/search
* автоматический расчет размера по умолчанию для Galera gcache.size
* улучшена поддержка FreeBSD

### Исправления ошибок
* [Коммит 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) исправлена репликация RT индекса на узел, где существует такой же RT индекс с другим путем
* [Коммит 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) исправлено перенос расписания сброса для индексов без активности
* [Коммит d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) улучшено переназначение сброса индексов RT/PQ
* [Коммит d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) исправлена ошибка \#250 опции индекса index_field_lengths для TSV и CSV piped источников
* [Коммит 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) исправлен неправильный отчет indextool о проверке блочного индекса на пустом индексе
* [Коммит 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) исправлен пустой список select в журнале запросов Manticore SQL
* [Коммит 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) исправлен ответ индексатора на -h/--help

## Версия 3.1.0, 16 июля 2019
### Особенности и улучшения
* репликация для RealTime индексов
* ICU-токенизатор для китайского языка
* новая опция морфологии icu_chinese
* новая директива icu_data_dir
* транзакции с несколькими запросами для репликации
* LAST_INSERT_ID() и @session.last_insert_id
* LIKE 'pattern' для SHOW VARIABLES
* Множественный INSERT документов для percolate индексов
* Добавлены парсеры времени для конфигурации
* внутренний менеджер задач
* mlock для компонентов doc и hit lists
* изоляция пути сниппетов (jail snippets path)

### Удаления
* поддержка библиотеки RLP прекращена в пользу ICU; все директивы rlp* удалены
* обновление ID документа через UPDATE отключено

### Исправления ошибок
* [Коммит f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) исправлены ошибки в concat и group_concat
* [Коммит b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) исправлен тип атрибута uid запроса в percolate индексе на BIGINT
* [Коммит 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) предотвращен сбой при неудаче предварительного выделения нового дискового чанка
* [Коммит 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) добавлен отсутствующий тип данных timestamp для ALTER
* [Коммит f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) исправлен сбой из-за неправильного чтения mmap
* [Коммит 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) исправлен хэш блокировки кластеров в репликации
* [Коммит ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) исправлен утечка провайдеров в репликации
* [Коммит 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) исправлена ошибка \#246 неопределенного sigmask в индексаторе
* [Коммит 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) исправлена гонка при отчетах netloop
* [Коммит a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) убран нулевой разрыв для балансировщика стратегий HA

## Версия 3.0.2, 31 мая 2019
### Улучшения
* добавлены mmap-ридеры для списков документов и попаданий (docs и hit lists)
* теперь ответ HTTP-эндпоинта `/sql` совпадает с ответом `/json/search`
* новые директивы `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* новая директива `server_id` для настроек репликации

### Удаления
* удален HTTP-эндпоинт `/search`

### Устаревшие возможности
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` заменены директивами `access_*`

### Исправления ошибок
* [Коммит 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) разрешены имена атрибутов, начинающиеся с цифр, в списке select
* [Коммит 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) исправлены MVAs в UDF, исправлено алиасинг MVA
* [Коммит 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) исправлена \#187 ошибка при использовании запроса с SENTENCE
* [Коммит 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) исправлена \#143 поддержка () вокруг MATCH()
* [Коммит 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) исправлено сохранение состояния кластера в операторе ALTER cluster
* [Коммит 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) исправлен сбой сервера при ALTER index с blob-атрибутами
* [Коммит 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) исправлена \#196 фильтрация по id
* [Коммит 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) исключен поиск по шаблонным индексам
* [Коммит 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) исправлен тип колонки id на обычный bigint в ответе SQL


## Версия 3.0.0, 6 мая 2019
### Новые возможности и улучшения
* Новое хранилище индексов. Атрибуты не-скалярного типа теперь не ограничены размером 4 ГБ на индекс
* Директива attr_update_reserve
* Строки, JSON и MVAs можно обновлять через UPDATE
* killlists применяются при загрузке индекса
* Директива killlist_target
* ускорение мульти AND-поисков
* улучшена средняя производительность и использование оперативной памяти
* инструмент convert для обновления индексов, сделанных с 2.x
* функция CONCAT()
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* Директива node_address
* список нод выводится в SHOW STATUS

### Изменения в поведении
* для индексов с killlists сервер не переключает индексы в порядке, определённом в конфиге, а следует цепочке целей killlist
* порядок индексов в поиске больше не определяет порядок применения killlists
* Идентификаторы документов теперь signed big integers

### Удалённые директивы
* docinfo (теперь всегда extern), inplace_docinfo_gap, mva_updates_pool

## Версия 2.8.2 GA, 2 апреля 2019
### Новые возможности и улучшения
* Репликация Galera для percolate индексов
* Опция OPTION morphology

### Заметки по компиляции
Минимальная версия Cmake теперь 3.13. Для компиляции требуются boost и libssl
развивающие библиотеки.

### Исправления ошибок
* [Коммит 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) исправлен сбой при множественных звёздочках в списке select для запроса в множество распределённых индексов
* [Коммит 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) исправлена [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) большая пачка через интерфейс Manticore SQL
* [Коммит 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) исправлен [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) сбой сервера при RT optimize с обновлённым MVA
* [Коммит edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) исправлен сбой сервера при удалении binlog из-за удаления RT индекса после перезагрузки конфига на SIGHUP
* [Коммит bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) исправлены payload плагинов аутентификации mysql handshake
* [Коммит 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) исправлен [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) параметр phrase_boundary в RT индексе
* [Коммит 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) исправлен [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) дедлок при ATTACH индекса к самому себе
* [Коммит 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) исправлено сохранение пустого meta в binlog после сбоя сервера
* [Коммит 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) исправлен сбой сервера из-за строки в сортировщике от RT индекса с дисковыми чанками

## Версия 2.8.1 GA, 6 марта 2019
### Новые возможности и улучшения
* SUBSTRING_INDEX()
* Поддержка SENTENCE и PARAGRAPH для percolate запросов
* systemd генератор для Debian/Ubuntu; добавлен LimitCORE для разрешения core dump

### Исправления ошибок
* [Коммит 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) исправлен сбой сервера на режиме match all и пустом полнотекстовом запросе
* [Коммит daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) исправлен сбой при удалении статической строки
* [Коммит 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) исправлен код выхода при неудаче indextool с FATAL
* [Коммит 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) исправлен [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) отсутствие совпадений для префиксов из-за неверной проверки exact form
* [Коммит 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) исправлен [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) перезагрузки настроек конфигурации для RT индексов
* [Коммит e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) исправлен сбой сервера при доступе к большой JSON строке
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) исправлено: поле PQ в JSON-документе, изменённом index stripper, вызвало неправильное совпадение в поле-сиблинг
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) исправлен сбой сервера при разборе JSON в сборках для RHEL7
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) исправлен сбой при json unescaping, когда слэш стоит на краю
* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) исправлена опция 'skip_empty' для пропуска пустых документов без предупреждения о том, что они невалидны как json
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) исправлено [#140](https://github.com/manticoresoftware/manticoresearch/issues/161): вывод 8 знаков после запятой для чисел с плавающей точкой, когда 6 недостаточно для точности
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) исправлено создание пустого jsonobj
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) исправлено [#160](https://github.com/manticoresoftware/manticoresearch/issues/161): пустой mva выводит NULL вместо пустой строки
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) исправлен сбой сборки без pthread_getname_np
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) исправлен сбой сервера при остановке с потоками thread_pool

## Версия 2.8.0 GA, 28 января 2019
### Улучшения
* Распределённые индексы для percolate индексов
* Новые опции и изменения в CALL PQ:
    * skip_bad_json
    * режим (sparsed/sharded)
    * JSON-документы можно передавать как JSON-массив
    * shift
    * Имена столбцов 'UID', 'Documents', 'Query', 'Tags', 'Filters' переименованы в 'id', 'documents', 'query', 'tags', 'filters'
* DESCRIBE pq TABLE
* SELECT FROM pq WHERE UID больше не поддерживается, используйте 'id'
* SELECT по pq индексам теперь наравне с обычными индексами (например, можно фильтровать правила через REGEX())
* ANY/ALL можно использовать для тегов PQ
* выражения автоматически конвертируются для JSON полей без явного кастинга
* встроенная таблица символов 'non_cjk' и ngram_chars 'cjk'
* встроенные коллекции стоп-слов для 50 языков
* в декларации стоп-слов несколько файлов можно разделять запятыми
* CALL PQ может принимать JSON-массив документов

### Исправления ошибок
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) исправлен утечка памяти, связанная с csjon
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) исправлен сбой из-за отсутствующего значения в json
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) исправлено сохранение пустого мета для RT индекса
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) исправлен пропавший флаг формы (exact) для последовательности лемматизатора
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) исправлено переполнение для строковых атрибутов > 4M, теперь используется saturate вместо overflow
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) исправлен сбой сервера при SIGHUP с отключённым индексом
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) исправлен сбой сервера при одновременных API командах статуса сессии
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) исправлен сбой сервера при удалении запроса в RT индекс с фильтрами по полям
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) исправлен сбой сервера при CALL PQ для распределённого индекса с пустым документом
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) исправлено обрезание сообщения об ошибке Manticore SQL длиннее 512 символов
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) исправлен сбой при сохранении percolate индекса без binlog
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) исправлена неработающая HTTP-интерфейс в OSX
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) исправлено ошибочное сообщение indextool при проверке MVA
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) исправлена блокировка записи при FLUSH RTINDEX, чтобы не блокировать весь индекс во время сохранения и при регулярном flush из rt_flush_period
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) исправлена зависшая команда ALTER percolate index, ожидающая загрузки поиска
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) исправлено max_children для использования значения по умолчанию из количества потоков thread_pool при значении 0
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) исправлена ошибка индексирования данных в индекс с плагином index_token_filter вместе с стоп-словами и stopword_step=0
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) исправлен сбой при отсутствии lemmatizer_base, когда в определениях индекса используются aot лемматизаторы

## Версия 2.7.5 GA, 4 декабря 2018
### Улучшения
* Функция REGEX
* limit/offset для JSON API поиска
* точки профилирования для qcache

### Исправления ошибок
* [Коммит eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) исправлен сбой сервера при FACET с несколькими широкими типами атрибутов
* [Коммит d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) исправлена неявная группировка в основном списке select запроса FACET
* [Коммит 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) исправлен сбой при запросе с GROUP N BY
* [Коммит 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) исправлена взаимная блокировка при обработке сбоя в операциях с памятью
* [Коммит 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) исправлено потребление памяти indextool во время проверки
* [Коммит 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) удалён ненужный include gmock, так как upstream сам решает эту проблему

## Версия 2.7.4 GA, 1 ноября 2018
### Улучшения
* SHOW THREADS для удалённых распределённых индексов теперь выводит оригинальный запрос вместо вызова API
* SHOW THREADS новая опция `format=sphinxql` выводит все запросы в формате SQL
* SHOW PROFILE выводит дополнительный этап `clone_attrs`

### Исправления ошибок
* [Коммит 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) исправлен сбой сборки с libc без malloc_stats, malloc_trim
* [Коммит f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) исправлены специальные символы внутри слов для результата CALL KEYWORDS
* [Коммит 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) исправлен сломанный CALL KEYWORDS к распределённому индексу через API или удалённому агенту
* [Коммит fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) исправлено распространение agent_query_timeout распределённому индексу к агентам как max_query_time
* [Коммит 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) исправлен счётчик общих документов на дисковом куске, который влияло на команду OPTIMIZE и ломало расчёт веса
* [Коммит dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) исправлены множественные tail hits в RT индексе от blended
* [Коммит eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) исправлена взаимная блокировка при ротации

## Версия 2.7.3 GA, 26 сентября 2018
### Улучшения
* Опция sort_mode для CALL KEYWORDS
* DEBUG на VIP соединении может выполнять 'crash <пароль>' для намеренного действия SIGEGV на сервере
* DEBUG может выполнять 'malloc_stats' для дампа статистики malloc в searchd.log и 'malloc_trim' для вызова malloc_trim()
* Улучшен backtrace, если gdb установлен в системе

### Исправления ошибок
* [Коммит 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) исправлен сбой или ошибка при переименовании в Windows
* [Коммит 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) исправлены сбои сервера на 32-битных системах
* [Коммит ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) исправлен сбой или зависание сервера на пустом выражении SNIPPET
* [Коммит b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) исправлена сломанная непрогрессивная оптимизация и исправлена прогрессивная оптимизация, чтобы не создавать kill-list для самого старого дискового куска
* [Коммит 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) исправлен неправильный ответ queue_max_length для SQL и API в режиме потокового пула
* [Коммит ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) исправлен сбой при добавлении полносканирующего запроса в PQ индекс с установленными опциями regexp или rlp
* [Коммит f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) исправлен сбой при последовательных вызовах PQ
* [Коммит 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) рефакторинг AcquireAccum
* [Коммит 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) исправлена утечка памяти после вызова pq
* [Коммит 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) косметический рефакторинг (стиль c++11 c-строк, значения по умолчанию, nullptr)
* [Коммит 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) исправлена утечка памяти при попытке вставить дубликат в PQ индекс
* [Коммит 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) исправлен сбой при использовании IN на JSON поле с большими значениями
* [Коммит 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) исправлен сбой сервера на запросе CALL KEYWORDS к RT индексу с установленным лимитом расширения
* [Коммит 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) исправлен недопустимый фильтр для запроса PQ matches
* [Коммит 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) введён небольшой аллокатор объектов для ptr attrs
* [Коммит 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) рефакторинг ISphFieldFilter до варианта с подсчётом ссылок
* [Коммит 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) исправлен UB/SIGSEGV при использовании strtod для неоконченных строк
* [Коммит 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) исправлена утечка памяти при обработке json resultset
* [Коммит e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) исправлено чтение за пределами блока памяти при применении атрибута add
* [Коммит fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) исправлен рефакторинг CSphDict для flavours с подсчетом ссылок
* [Коммит fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) исправлена утечка внутреннего типа AOT наружу
* [Коммит 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) исправлена утечка памяти в управлении токенизатором
* [Коммит 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) исправлена утечка памяти в grouper
* [Коммит 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) специальное освобождение/копирование для динамических указателей в matches (утечка памяти grouper)
* [Коммит b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) исправлена утечка памяти динамических строк для RT
* [Коммит 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) рефакторинг grouper
* [Коммит b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) небольшой рефакторинг (c++11 c-trs, некоторый рефакторинг форматирования)
* [Коммит 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) рефакторинг ISphMatchComparator в flavour с подсчетом ссылок
* [Коммит b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) приватизация cloner
* [Коммит efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) упрощение native little-endian для MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID
* [Коммит 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) добавлена поддержка valgrind в ubertests
* [Коммит 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) исправлен краш из-за гонки флага 'success' на соединении
* [Коммит 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) переключение epoll на edge-triggered режим
* [Коммит 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) исправлено выражение IN с форматированием как у фильтра at
* [Коммит bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) исправлен краш в RT индексе при коммите документа с большим docid
* [Коммит ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) исправлены опции без аргументов в indextool
* [Коммит 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) исправлена утечка памяти расширенного ключевого слова
* [Коммит 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) исправлена утечка памяти json grouper
* [Коммит 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) исправлена утечка глобальных пользовательских переменных
* [Коммит 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) исправлена утечка динамических строк при раннем отбрасывании матчей
* [Коммит 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) исправлена утечка в length(<expression>)
* [Коммит 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) исправлена утечка памяти из-за strdup() в парсере
* [Коммит 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) исправлен рефакторинг парсера выражений с точным соблюдением подсчета ссылок

## Версия 2.7.2 GA, 27 августа 2018
### Улучшения
* совместимость с клиентами MySQL 8
* [TRUNCATE](Emptying_a_table.md) С WITH RECONFIGURE
* удален счетчик памяти в SHOW STATUS для RT индексов
* глобальный кеш мульти-агентов
* улучшен IOCP в Windows
* VIP соединения для HTTP протокола
* Manticore SQL команда [DEBUG](Reporting_bugs.md#DEBUG), которая может выполнять различные подкоманды
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1 хэш пароля, необходимый для вызова `shutdown` через команду DEBUG
* новые статистики для SHOW AGENT STATUS (_ping, _has_perspool, _need_resolve)
* опция --verbose индексатора теперь принимает \[debugvv\] для вывода отладочных сообщений

### Исправления
* [Коммит 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) удален wlock при оптимизации
* [Коммит 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) исправлен wlock при перезагрузке настроек индекса
* [Коммит b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) исправлена утечка памяти при запросах с JSON фильтром
* [Коммит 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) исправлены пустые документы в PQ result set
* [Коммит 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) исправлена путаница задач из-за удаленной
* [Коммит cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) исправлен неправильный подсчет удаленного хоста
* [Коммит 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) исправлена утечка памяти разобранных описателей агентов
* [Коммит 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) исправлен утечка в поиске
* [Коммит 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) косметические изменения в явных/встроенных c-трансах, использовании override/final
* [Коммит 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) исправлена утечка json в локальной/удалённой схемах
* [Коммит 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) исправлена утечка json сортирующего столбца выражения в локальной/удалённой схемах
* [Коммит c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) исправлена утечка const alias
* [Коммит 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) исправлена утечка потока preread
* [Коммит 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) исправлена блокировка при выходе из-за зависшего ожидания в netloop
* [Коммит adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) исправлено зависание поведения 'ping' при смене HA агента на обычный хост
* [Коммит 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) отделена сборка мусора для dashboard storage
* [Коммит 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) исправлена проблема с ref-counted ptr
* [Коммит 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) исправлен аварийный сбой indextool при отсутствии индекса
* [Коммит 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) исправлено имя вывода превышающих атрибутов/полей при индексировании xmlpipe
* [Коммит cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) исправлено значение индексатора по умолчанию, если в конфиге нет секции indexer
* [Коммит e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) исправлены неверные встроенные стоп-слова в дисковом чанке RT индекса после рестарта сервера
* [Коммит 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) исправлен пропуск фантомных (уже закрытых, но не полностью удалённых из poller) соединений
* [Коммит f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) исправлены смешанные (осиротевшие) сетевые задачи
* [Коммит 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) исправлен сбой при чтении действия после записи
* [Коммит 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) исправлены сбои searchd при запуске тестов на Windows
* [Коммит e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) исправлена обработка кода EINPROGRESS при обычном connect()
* [Коммит 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) исправлены тайм-ауты соединений при работе с TFO

## Версия 2.7.1 GA, 4 июля 2018
### Улучшения
* улучшена производительность масок при сопоставлении множества документов на PQ
* добавлена поддержка fullscan запросов на PQ
* добавлена поддержка MVA атрибутов на PQ
* добавлена поддержка regexp и RLP для percolate индексов

### Исправления ошибок
* [Коммит 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) исправлена потеря строки запроса
* [Коммит 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) исправлена пустая информация в SHOW THREADS
* [Коммит 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) исправлен сбой при сопоставлении с оператором NOTNEAR
* [Коммит 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) исправлено сообщение об ошибке при некорректном фильтре на удаление PQ


## Версия 2.7.0 GA, 11 июня 2018
### Улучшения
* уменьшено количество системных вызовов чтобы избежать влияния патчей Meltdown и Spectre
* внутренний переписыватель управления локальными индексами
* рефакторинг удалённых сниппетов
* полная перезагрузка конфигурации
* все соединения с узлами теперь независимые
* улучшения протокола
* для Windows коммуникация переключена с wsapoll на IO completion ports
* TFO можно использовать для связи между мастером и узлами
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) теперь выводит версию сервера и mysql_version_string
* добавлен параметр `docs_id` для документов, вызываемых в CALL PQ.
* в фильтрах запросов percolate теперь допустимы выражения
* распределённые индексы могут работать с FEDERATED
* фиктивные SHOW NAMES COLLATE и `SET wait_timeout` (для лучшей совместимости с ProxySQL)

### Исправления ошибок
* [Коммит 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) исправлено добавление неравенства в теги PQ
* [Коммит 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) исправлено добавление поля ID документа в JSON-документ при CALL PQ
* [Коммит 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) исправлена обработка flush в PW индексе
* [Коммит c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) исправлена фильтрация PQ по JSON и строковым атрибутам
* [Коммит 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) исправлен разбор пустой JSON строки
* [Коммит 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) исправлен сбой при мультизапросах с OR фильтрами
* [Коммит 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) исправлен indextool для использования общей секции конфигурации (опция lemmatizer_base) для команд (dumpheader)
* [Коммит 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) исправлена пустая строка в наборе результатов и фильтре
* [Коммит 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) исправлены отрицательные значения идентификаторов документов
* [Коммит 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) исправлена длина обрезки слова для очень длинных проиндексированных слов
* [Коммит 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) исправлен поиск множества документов по wildcard-запросам в PQ


## Версия 2.6.4 GA, 3 мая 2018
### Новые возможности и улучшения
* Поддержка движка MySQL FEDERATED [support](Extensions/FEDERATED.md)
* Пакеты MySQL теперь возвращают флаг SERVER_STATUS_AUTOCOMMIT, обеспечивая совместимость с ProxySQL
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - включение TCP Fast Open для всех слушателей
* indexer --dumpheader теперь может выгружать заголовок RT из файла .meta
* скрипт сборки cmake для Ubuntu Bionic

### Исправления ошибок
* [Коммит 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) исправлены некорректные записи в кэше запросов для RT индекса;
* [Коммит 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) исправлена потеря настроек индекса после бесшовной ротации
* [Коммит 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) исправлены установки длины инфикса и префикса; добавлено предупреждение о неподдерживаемой длине инфикса
* [Коммит 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) исправлен порядок автоматической очистки RT индексов
* [Коммит 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) исправлены проблемы в схеме набора результатов для индексов с несколькими атрибутами и запросами к нескольким индексам
* [Коммит b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) исправлено потеряние некоторых хитов при пакетной вставке с дубликатами документов
* [Коммит 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) исправлен сбой optimize при слиянии дисковых чанков RT индекса с большим количеством документов

## Версия 2.6.3 GA, 28 марта 2018
### Улучшения
* jemalloc при компиляции. Если jemalloc присутствует в системе, его можно включить с помощью флага cmake `-DUSE_JEMALLOC=1`

### Исправления ошибок
* [Коммит 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) исправлена опция логирования expand_keywords в журнале запросов Manticore SQL
* [Коммит caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) исправлен HTTP-интерфейс для корректной обработки запросов большого размера
* [Коммит e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) исправлен сбой сервера при DELETE в RT индексе с включённой опцией index_field_lengths
* [Коммит cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) исправлена опция cpustats в команде searchd cli для работы на неподдерживаемых системах
* [Коммит 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) исправлено сопоставление подстрок utf8 с заданными минимальными длинами


## Версия 2.6.2 GA, 23 февраля 2018
### Улучшения
* улучшена производительность [Percolate Queries](Searching/Percolate_query.md) при использовании оператора NOT и при обработке пакетных документов.
* [percolate_query_call](Searching/Percolate_query.md) теперь может использовать несколько потоков в зависимости от значения [dist_threads](Server_settings/Searchd.md#threads)
* новый оператор полнотекстового соответствия NOTNEAR/N
* LIMIT для SELECT на перколяторных индексах
* [expand_keywords](Searching/Options.md#expand_keywords) теперь принимает 'start','exact' (где 'start,exact' эквивалентно '1')
* ranged-main-query для [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field), использующий ограниченный запрос, определённый sql_query_range

### Исправления ошибок
* [Коммит 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) исправлен сбой при поиске в RAM-сегментах; устранена взаимная блокировка при сохранении дискового чанка с двойным буфером; устранена взаимная блокировка при сохранении дискового чанка во время optimize
* [Коммит 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) исправлен сбой индексатора при xml-схеме с пустым именем атрибута
* [Коммит 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) исправлено неправильное удаление не принадлежащего pid-файла
* [Коммит a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) устранён случайный оставшийся орфанный fifo в временной папке
* [Коммит 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) исправлен пустой набор результатов FACET с ошибочной NULL строкой
* [Коммит 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) исправлена сломанная блокировка индекса при запуске сервера как службы Windows
* [Коммит be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) исправлены неверные библиотеки iconv для mac os
* [Коммит 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) исправлен неправильный подсчёт count(\*)


## Версия 2.6.1 GA, 26 января 2018
### Улучшения
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) в случае агентов с зеркалами даёт значение количества повторных попыток на зеркало вместо на агента, общее количество попыток на агента равно agent_retry_count\*mirrors.
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) теперь может задаваться для каждого индекса, переопределяя глобальное значение. Добавлен псевдоним [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count).
* retry_count может быть задан в определении агента, и значение представляет количество повторных попыток на агента.
* Перколяционные запросы теперь доступны в HTTP JSON API по адресу [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
* Добавлены опции -h и -v (помощь и версия) в исполняемые файлы.
* Добавлена поддержка [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) для индексов Real-Time.

### Исправления ошибок
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) исправил ranged-main-query для корректной работы с sql_range_step при использовании в поле MVA.
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) исправил проблему с застреванием системного цикла blackhole и кажущейся отключённостью агентов blackhole.
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) исправил id запросов для согласованности, исправил дублирование id для сохранённых запросов.
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) исправил падение сервера при выключении из различных состояний.
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) исправлены тайм-ауты на длинных запросах.
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) переработан опрос сети master-agent на системах с поддержкой kqueue (Mac OS X, BSD).


## Версия 2.6.0, 29 декабря 2017
### Новые возможности и улучшения
* [HTTP JSON](Connecting_to_the_server/HTTP.md): JSON-запросы теперь могут выполнять проверку равенства для атрибутов, MVA и JSON-атрибуты могут использоваться при вставках и обновлениях, обновления и удаления через JSON API теперь можно выполнять и для распределённых индексов.
* [Перколяционные запросы](Searching/Percolate_query.md).
* Удалена поддержка 32-битных docid из кода. Также удалён весь код, который конвертировал/загружал устаревшие индексы с 32-битными docid.
* [Морфология только для определённых полей](https://github.com/manticoresoftware/manticore/issues/7). Добавлена новая директива индекса morphology_skip_fields, позволяющая задавать список полей, для которых морфология не применяется.
* [expand_keywords теперь может быть директивой, задаваемой во время выполнения запроса через оператор OPTION](https://github.com/manticoresoftware/manticore/issues/8).

### Исправления ошибок
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) исправлено падение в отладочной сборке сервера (и возможно неопределённое поведение в релизе) при сборке с rlp.
* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) исправлена оптимизация RT-индекса с включённой опцией progressive, при которой списки удаления объединялись в неправильном порядке.
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee) мелкое падение на macOS.
* множество мелких исправлений после тщательного статического анализа кода.
* другие мелкие исправления ошибок.

### Обновление
В этом релизе изменён внутренний протокол связи между мастерами и агентами. Если вы используете Manticoresearch в распределённой среде с несколькими экземплярами, убедитесь, что при обновлении сначала обновляются агенты, а затем мастера.

## Версия 2.5.1, 23 ноября 2017
### Новые возможности и улучшения
* JSON-запросы протокола [HTTP API](Connecting_to_the_server/HTTP.md). Поддерживаются операции поиска, вставки, обновления, удаления и замены. Команды по манипуляции данными можно также выполнять пакетно, но есть некоторые текущие ограничения — MVA и JSON-атрибуты нельзя использовать для вставок, замен или обновлений.
* Команда [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES).
* Команда [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md).
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) теперь показывает прогресс оптимизации, ротации или сброса.
* GROUP N BY корректно работает с MVA-атрибутами.
* агентам blackhole теперь выделена отдельная нить, чтобы они больше не влияли на запросы мастера.
* реализован счётчик ссылок на индексы, чтобы избежать задержек из-за ротаций и высокой нагрузки.
* реализовано хэширование SHA1, пока не доступно для внешнего использования.
* исправления для компиляции на FreeBSD, macOS и Alpine.

### Исправления ошибок
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) регрессия фильтра с блочным индексом.
* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) переименование PAGE_SIZE -> ARENA_PAGE_SIZE для совместимости с musl.
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) отключение googletests для cmake версии ниже 3.1.0.
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) ошибка привязки сокета при перезапуске сервера.
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) исправлено падение сервера при выключении.
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) исправлено отображение потоков для системного потока blackhole.
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) переработана проверка конфигурации iconv, исправляет сборку на FreeBSD и Darwin.

## Версия 2.4.1 GA, 16 октября 2017
### Новые возможности и улучшения
* Оператор OR в WHERE между фильтрами атрибутов.
* Режим обслуживания ( SET MAINTENANCE=1).
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) доступны на распределённых индексах.
* [Группировка в UTC](Server_settings/Searchd.md#grouping_in_utc).
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) для настройки прав доступа к пользовательским файлам журнала.
* Веса полей могут быть нулевыми или отрицательными.
* [max_query_time](Searching/Options.md#max_query_time) теперь может влиять на полные сканирования
* добавлены [net_wait_tm](Server_settings/Searchd.md#net_wait_tm), [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) и [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) для тонкой настройки сетевого потока (в случае workers=thread_pool)
* COUNT DISTINCT работает с поиском по фасетам
* IN можно использовать с JSON-массивами с плавающей точкой
* мультизапросная оптимизация больше не ломается из-за целочисленных/чисел с плавающей точкой выражений
* [SHOW META](Node_info_and_management/SHOW_META.md) показывает строку `multiplier`, когда используется мультизапросная оптимизация

### Компиляция
Manticore Search строится с помощью cmake, минимальная версия gcc для компиляции — 4.7.2.

### Папки и служба
* Manticore Search запускается от пользователя `manticore`.
* Папка данных по умолчанию теперь `/var/lib/manticore/`.
* Папка логов по умолчанию теперь `/var/log/manticore/`.
* Папка для pid файлов по умолчанию теперь `/var/run/manticore/`.

### Исправления багов
* [Коммит a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) исправил оператор SHOW COLLATION, который ломал java-коннектор
* [Коммит 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) исправил сбои при обработке распределенных индексов; добавлены блокировки для хеша распределенного индекса; удалены операторы перемещения и копирования из агента
* [Коммит 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) исправил сбои при обработке распределенных индексов из-за параллельных переподключений
* [Коммит e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) исправил сбой в обработчике сбоев при сохранении запроса в лог сервера
* [Коммит 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) исправил сбой с пулом атрибутов в мультизапросах
* [Коммит 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) исправил уменьшение размера core-файла за счет предотвращения включения страниц индекса в core-файл
* [Коммит 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) исправил сбои searchd при запуске, если указаны неверные агенты
* [Коммит 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) исправил ошибку индексатора в sql_query_killlist запросе
* [Коммит 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) исправил fold_lemmas=1 против количества попаданий
* [Коммит cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) исправил непоследовательное поведение html_strip
* [Коммит e406](https://github.com/manticoresoftware/manticore/commit/e406761) исправил потерю новых настроек optimize rt index; исправил утечки блокировок при optimize с опцией sync;
* [Коммит 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) исправил обработку ошибочных мультизапросов
* [Коммит 2645](https://github.com/manticoresoftware/manticore/commit/2645230) исправил зависимость результатов от порядка мультизапросов
* [Коммит 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) исправил сбой сервера на мультизапросе с некорректным запросом
* [Коммит f353](https://github.com/manticoresoftware/manticore/commit/f353326) исправил блокировку с shared на exclusive
* [Коммит 3754](https://github.com/manticoresoftware/manticore/commit/3754785) исправил сбой сервера для запроса без индексов
* [Коммит 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) исправил взаимоблокировку сервера

## Версия 2.3.3, 06 июля 2017
* Брендинг Manticore
