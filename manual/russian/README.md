* [☝ Введение](Introduction.md)
* [❗ Прочитайте это сначала](Read_this_first.md)
* [1️⃣ Установка](Installation/Installation.md)
    * [• Docker](Installation/Docker.md)
    * [• RedHat и Centos](Installation/RHEL_and_Centos.md)
    * [• Debian и Ubuntu](Installation/Debian_and_Ubuntu.md)
    * [• MacOS](Installation/MacOS.md)
    * [• Windows](Installation/Windows.md)
    * [• Компиляция из исходников](Installation/Compiling_from_sources.md)
    * [• Manticore Buddy](Installation/Manticore_Buddy.md)
    * [• Миграция из Sphinx](Installation/Migration_from_Sphinx.md)
* [🔰 Быстрый старт](Quick_start_guide.md)
* [2️⃣ Запуск сервера](Starting_the_server.md)
    * [• В Linux](Starting_the_server/Linux.md)
    * [• Вручную](Starting_the_server/Manually.md)
    * [• В Docker](Starting_the_server/Docker.md)
    * [• В Windows](Starting_the_server/Windows.md)
    * [• В MacOS](Starting_the_server/MacOS.md)
* [3️⃣ Создание таблицы](Creating_a_table.md)
    * [⪢ Типы данных](Creating_a_table/Data_types.md)
        * [• Хранение атрибутов построчно и поколоночно](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)
    * [⪢ Создание локальной таблицы](Creating_a_table/Local_tables.md)
        * [✔ Таблица реального времени](Creating_a_table/Local_tables/Real-time_table.md)
        * [• Plain таблица](Creating_a_table/Local_tables/Plain_table.md)
        * [• Настройки Plain и real-time таблиц](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md)
        * [• Percolate таблица](Creating_a_table/Local_tables/Percolate_table.md)
        * [• Шаблонная таблица](Creating_a_table/Local_tables/Template_table.md)
    * [⪢ NLP и токенизация]
        * [• Токенизация данных](Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
        * [• Поддерживаемые языки](Creating_a_table/NLP_and_tokenization/Supported_languages.md)
        * [• Языки с непрерывным скриптом](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)
        * [• Низкоуровневая токенизация](Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md)
        * [• Настройки поиска с подстановочными знаками](Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md)
        * [• Игнорирование стоп-слов](Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md)
        * [• Формы слов](Creating_a_table/NLP_and_tokenization/Wordforms.md)
        * [• Исключения](Creating_a_table/NLP_and_tokenization/Exceptions.md)
        * [• Морфология](Creating_a_table/NLP_and_tokenization/Morphology.md)
        * [• Расширенная HTML токенизация](Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md)
    * [⪢ Создание распределённой таблицы](Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md)
        * [• Создание локальной распределённой таблицы](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)
        * [• Удалённые таблицы](Creating_a_table/Creating_a_distributed_table/Remote_tables.md)
* [• Список таблиц](Listing_tables.md)
* [• Удаление таблицы](Deleting_a_table.md)
* [• Очистка таблицы](Emptying_a_table.md)
* [⪢ Создание кластера](Creating_a_cluster/Creating_a_cluster.md)
    * [Добавление нового узла](Creating_a_cluster/Adding_a_new_node.md)
    * [⪢ Удалённые узлы](Creating_a_cluster/Remote_nodes.md)
        * [Мирроринг](Creating_a_cluster/Remote_nodes/Mirroring.md)
        * [Балансировка нагрузки](Creating_a_cluster/Remote_nodes/Load_balancing.md)
    * [⪢ Настройка репликации](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)
        * [Создание кластера репликации](Creating_a_cluster/Setting_up_replication/Creating_a_replication_cluster.md)
        * [Присоединение к кластеру репликации](Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md)
        * [Удаление кластера репликации](Creating_a_cluster/Setting_up_replication/Deleting_a_replication_cluster.md)
        * [Добавление и удаление таблицы из кластера репликации](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md)
        * [Управление узлами репликации](Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md)
        * [Статус кластера репликации](Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md)
        * [Перезапуск кластера](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md)
        * [Восстановление кластера](Creating_a_cluster/Setting_up_replication/Cluster_recovery.md)
* [4️⃣ Подключение к серверу](Connecting_to_the_server.md)
    * [Протокол MySQL](Connecting_to_the_server/MySQL_protocol.md)
    * [HTTP](Connecting_to_the_server/HTTP.md)
    * [SQL через HTTP](Connecting_to_the_server/HTTP.md#SQL-over-HTTP)
* [⪢ Создание и изменение данных](Data_creation_and_modification/Data_creation_and_modification.md)
    * [⪢ Добавление документов в таблицу]
        * [✔ Добавление документов в таблицу реального времени](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
        * [Добавление правил в перколяционную таблицу](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md)
    * [⪢ Добавление данных из внешних хранилищ](Data_creation_and_modification/Adding_data_from_external_storages.md)
        * [Создание Plain таблиц](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)
        * [⪢ Получение из баз данных]
            * [Введение](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Introduction.md)
            * [Подключение к базе данных](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md)
            * [Выполнение запросов на извлечение](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md)
            * [Обработка полученных данных](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md)








































































































































