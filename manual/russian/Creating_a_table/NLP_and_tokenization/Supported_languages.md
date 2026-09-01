# Поддерживаемые языки

Manticore поддерживает широкий набор языков, а базовая поддержка включена для большинства языков через `charset_table = non_cont` (это значение по умолчанию). Также можно использовать опцию `non_cjk`, которая является псевдонимом `non_cont`: `charset_table = non_cjk`.

Для многих языков Manticore предоставляет файл [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords), который можно использовать для повышения релевантности поиска.

Кроме того, для нескольких языков доступна расширенная морфология, которая может заметно повысить релевантность поиска за счет словарной лемматизации или алгоритмов стемминга для более качественного сегментирования и нормализации.

В таблице ниже перечислены все поддерживаемые языки и указано, как включить:
* базовую поддержку (столбец "Supported")
* stopwords (столбец "Stopwords file name")
* расширенную морфологию (столбец "Advanced morphology")

| Язык | Поддержка | Имя файла stopwords | Расширенная морфология | Примечания |
| - | - | - | - | - |
| Afrikaans | charset_table=non_cont | af | - | |
| Arabic | charset_table=non_cont | ar | morphology=stem_ar (Arabic stemmer); morphology=libstemmer_ar  | |
| Armenian | charset_table=non_cont | hy | - | |
| Assamese | укажите charset_table вручную | - | - | |
| Basque | charset_table=non_cont | eu | - | |
| Bengali | charset_table=non_cont | bn | - | |
| Bishnupriya | укажите charset_table вручную | - | - | |
| Buhid | укажите charset_table вручную | - | - | |
| Bulgarian | charset_table=non_cont | bg | - | |
| Catalan | charset_table=non_cont | ca | morphology=libstemmer_ca | |
| Chinese using [ICU](https://icu.unicode.org/) | charset_table=chinese | zh | morphology=icu_chinese | Точнее, чем использование ngrams |
| Chinese using [Jieba](https://github.com/fxsjy/jieba) | charset_table=chinese | zh | morphology=jieba_chinese, requires package `manticore-language-packs` | Точнее, чем использование ngrams |
| Chinese using ngrams| ngram_chars=chinese | zh | ngram_chars=1 | Индексация быстрее, но качество поиска может быть ниже |
| Croatian | charset_table=non_cont | hr | - | |
| Kurdish | charset_table=non_cont | ckb | - | |
| Czech | charset_table=non_cont | cz | morphology=stem_cz (Czech stemmer) | |
| Danish | charset_table=non_cont | da | morphology=libstemmer_da | |
| Dutch | charset_table=non_cont | nl | morphology=libstemmer_nl | |
| English | charset_table=non_cont | en | morphology=lemmatize_en (единственная корневая форма); morphology=lemmatize_en_all (все корневые формы); morphology=stem_en (English stemmer Портера); morphology=stem_enru (English and Russian stemmers Портера); morphology=libstemmer_en (English from libstemmer)  | |
| Esperanto | charset_table=non_cont | eo | - | |
| Estonian | charset_table=non_cont | et | - | |
| Finnish | charset_table=non_cont | fi | morphology=libstemmer_fi | |
| French | charset_table=non_cont | fr | morphology=libstemmer_fr | |
| Galician | charset_table=non_cont | gl | - | |
| Garo | укажите charset_table вручную | - | - | |
| German | charset_table=non_cont | de | morphology=lemmatize_de (единственная корневая форма); morphology=lemmatize_de_all (все корневые формы); morphology=lemmatize_de_v2 (единственная корневая форма с нормализацией sharp-s); morphology=lemmatize_de_v2_all (все корневые формы с нормализацией sharp-s); morphology=libstemmer_de | Для вариантов AOT v2 используйте charset_table=non_cont,german |
| Greek | charset_table=non_cont | el | morphology=libstemmer_el | |
| Hebrew | charset_table=non_cont | he | - | |
| Hindi | charset_table=non_cont | hi | morphology=libstemmer_hi | |
| Hmong | укажите charset_table вручную | - | - | |
| Ho | укажите charset_table вручную | - | - | |
| Hungarian | charset_table=non_cont | hu | morphology=libstemmer_hu | |
| Indonesian | charset_table=non_cont | id | morphology=libstemmer_id | |
| Irish | charset_table=non_cont | ga | morphology=libstemmer_ga | |
| Italian | charset_table=non_cont | it | morphology=libstemmer_it | |
| Japanese | ngram_chars=japanese | - | ngram_chars=japanese ngram_len=1 | Требуется сегментация на основе ngram |
| Komi | укажите charset_table вручную | - | - | |
| Korean | ngram_chars=korean | - | ngram_chars=korean ngram_len=1 | Требуется сегментация на основе ngram  |
| Large Flowery Miao | укажите charset_table вручную | - | - | |
| Latin | charset_table=non_cont | la | - | |
| Latvian | charset_table=non_cont | lv | - | |
| Lithuanian | charset_table=non_cont | lt | morphology=libstemmer_lt | |
| Maba | укажите charset_table вручную | - | - | |
| Maithili | укажите charset_table вручную | - | - | |
| Marathi | укажите charset_table вручную | - | - | |
| Marathi | charset_table=non_cont | mr | - | |
| Mende | укажите charset_table вручную | - | - | |
| Mru | укажите charset_table вручную | - | - | |
| Myene | укажите charset_table вручную | - | - | |
| Nepali | укажите charset_table вручную | - | morphology=libstemmer_ne | |
| Ngambay | укажите charset_table вручную | - | - | |
| Norwegian | charset_table=non_cont | no | morphology=libstemmer_no | |
| Odia | укажите charset_table вручную | - | - | |
| Persian | charset_table=non_cont | fa | - | |
| Polish | charset_table=non_cont | pl | - | |
| Portuguese | charset_table=non_cont | pt | morphology=libstemmer_pt | |
| Romanian | charset_table=non_cont | ro | morphology=libstemmer_ro | |
| Russian | charset_table=non_cont | ru | morphology=lemmatize_ru (единственная корневая форма); morphology=lemmatize_ru_all (все корневые формы); morphology=stem_ru (Russian stemmer Портера); morphology=stem_enru (English and Russian stemmers Портера); morphology=libstemmer_ru (из libstemmer) | |
| Santali | укажите charset_table вручную | - | - | |
| Sindhi | укажите charset_table вручную | - | - | |
| Slovak | charset_table=non_cont | sk | - | |
| Slovenian | charset_table=non_cont | sl | - | |
| Somali | charset_table=non_cont | so | - | |
| Sotho | charset_table=non_cont | st | - | |
| Spanish | charset_table=non_cont | es | morphology=libstemmer_es | |
| Swahili | charset_table=non_cont | sw | - | |
| Swedish | charset_table=non_cont | sv | morphology=libstemmer_sv | |
| Sylheti | укажите charset_table вручную | - | - | |
| Tamil | укажите charset_table вручную | - | morphology=libstemmer_ta | |
| Thai | charset_table=thai | th | - | |
| Turkish | charset_table=non_cont | tr | morphology=libstemmer_tr | |
| Ukrainian | charset_table=non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491 | - | morphology=lemmatize_uk (единственная корневая форма); morphology=lemmatize_uk_all (все корневые формы) | Переопределите `charset_table`, чтобы сохранить `і`, `ї` и `ґ` |
| Vietnamese | charset_table=non_cont | - | - | Использует латинское письмо. Вьетнамские диакритические знаки (ă, â, ê, ô, ơ, ư, đ и тоны) по умолчанию автоматически приводятся к базовым латинским символам, поэтому "tiếng" совпадает с "tieng" без дополнительной настройки. |
| Yoruba | charset_table=non_cont | yo | - | |
| Zulu | charset_table=non_cont | zu | - |  |

### Немецкая морфология AOT и sharp s

Стандартный `non_cont` charset преобразует немецкие умляуты в их базовые ASCII-буквы и заменяет `ß` на `s` до выполнения морфологии. Чтобы немецкая морфология AOT получила эти символы, добавьте встроенный псевдоним `german` после `non_cont`:

```ini
charset_table = non_cont, german
```

Псевдоним сохраняет `ä`, `ö`, `ü` и `ß`, а также приводит их заглавные варианты к строчным, включая `ẞ` к `ß`. Он меняет только сопоставление символов в токенизаторе; режим морфологии автоматически не выбирает.

| Значение морфологии | Поведение |
| - | - |
| `lemmatize_de` | Существующее поведение немецкой AOT с одной корневой формой |
| `lemmatize_de_all` | Существующее поведение немецкой AOT со всеми корневыми формами |
| `lemmatize_de_v2` | Поведение с одной корневой формой и нормализацией sharp s |
| `lemmatize_de_v2_all` | Поведение со всеми корневыми формами и нормализацией sharp s |

Следовательно, конфигурация v2 включает и значение морфологии, и charset, который сохраняет немецкие символы, например:

```ini
morphology = lemmatize_de_v2
charset_table = non_cont, german
```

Варианты v2 нормализуют `ß` и `ẞ` в `ss` перед поиском в словаре, а также нормализуют `ß`, встречающийся в сгенерированных словарных леммах, в `ss`. В v2 формы вроде `Straße`/`Strasse`/`STRAẞE` и `GUSSE`/`GÜSSE`/`GUß`/`GUSS` эквивалентны при обычном полнотекстовом поиске по целым словам. Это намеренно также убирает различия вроде `Maße` и `Masse` при обычном полнотекстовом сопоставлении.

При `index_exact_words=1` Manticore также сохраняет токенизированную форму до морфологии. Поэтому запросы по точному слову, такие как `MATCH('=straße')` и `MATCH('=strasse')`, могут различать формы `ß` и `ss`. Точные слова по-прежнему подчиняются сопоставлениям `charset_table`: с псевдонимом `german`, например, `Straße` и `STRAẞE` обе имеют точную форму `straße`, потому что регистр приводится к нижнему до морфологии.

Существующим таблицам, которые используют `lemmatize_de` или `lemmatize_de_all`, после обновления демона ничего делать не нужно. Миграция существующей таблицы на любое значение v2 изменяет сгенерированные термины, поэтому перед выдачей в работу перестройте обычную таблицу или заново загрузите все документы в новую RT-таблицу. Изменение морфологии уже заполненной RT-таблицы, а также использование merge, attach или optimize не преобразует уже проиндексированные термины. Существующие проверки dictionary-settings могут отклонить attach между таблицами с разными значениями морфологии там, где эти проверки применяются, но это не механизм миграции.

Оба значения v2 используют тот же `de.pak`, что и существующие значения; обновление language pack не требуется. Демоны, выпущенные до появления имен опций v2, не могут загрузить таблицу v2, поэтому откат таблицы v2 требует восстановления или перестройки таблицы с существующим значением морфологии. Таблицы, которые используют `lemmatize_de` или `lemmatize_de_all`, этим ограничением не затронуты.

<!-- proofread -->
