# Таблицы-шаблоны

<!-- example template -->
Таблица-шаблон — это особый тип таблицы в Manticore, который не хранит никаких данных и не создает файлов на диске. Несмотря на это, она может иметь те же [настройки НЛП](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Natural-language-processing-specific-settings), что и обычная или реального времени таблица. Таблицы-шаблоны могут использоваться для следующих целей:

* В качестве шаблона для наследования настроек в [режиме Plain](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29), упрощая конфигурационный файл Manticore.
* Генерация ключевых слов с помощью команды [CALL KEYWORDS](../../Searching/Autocomplete.md#CALL-KEYWORDS).
* Подсветка произвольной строки с помощью команды [CALL SNIPPETS](../../Searching/Highlighting.md#CALL-SNIPPETS).


<!-- intro -->
##### Создание таблицы-шаблона через конфигурационный файл:

<!-- request CONFIG -->

```ini
table template {
  type = template
  morphology = stem_en
  wordforms = wordforms.txt
  exceptions = exceptions.txt
  stopwords = stopwords.txt
}
```
<!-- end -->
<!-- proofread -->

