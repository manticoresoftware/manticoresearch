# Поддерживаемые языки

Manticore поддерживает широкий спектр языков, с базовой поддержкой, включенной для большинства языков через `charset_table = non_cont` (что является значением по умолчанию). Опция `non_cjk`, которая является псевдонимом для `non_cont`, также может быть использована: `charset_table = non_cjk`.

Для многих языков Manticore предоставляет файл [стоп-слов](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords), который может быть использован для улучшения релевантности поиска.

Дополнительно, расширенная морфология доступна для нескольких языков, что может значительно улучшить релевантность поиска, используя основанные на словаре алгоритмы лемматизации или стемминга для лучшей сегментации и нормализации.

Таблица ниже перечисляет все поддерживаемые языки и указывает, как их включить:
* базовая поддержка (столбец "Поддерживается")
* стоп-слова (столбец "Имя файла со стоп-словами")
* расширенная морфология (столбец "Расширенная морфология")

| Язык | Поддерживается | Имя файла со стоп-словами | Расширенная морфология | Примечания |
| - | - | - | - | - |
| Африкаанс | charset_table=non_cont | af | - | |
| Арабский | charset_table=non_cont | ar | morphology=stem_ar (арабский стеммер); morphology=libstemmer_ar  | |
| Армянский | charset_table=non_cont | hy | - | |
| Ассамский | specify charset_table specify charset_table manually | - | - | |
| Баскский | charset_table=non_cont | eu | - | |
| Бенгали | charset_table=non_cont | bn | - | |
| Бишнуприа | specify charset_table manually | - | - | |
| Бухид | specify charset_table manually | - | - | |
| Болгарский | charset_table=non_cont | bg | - | |
| Каталонский | charset_table=non_cont | ca | morphology=libstemmer_ca | |
| Китайский с использованием [ICU](https://icu.unicode.org/) | charset_table=chinese | zh | morphology=icu_chinese | Более точно, чем использование ngrams |
| Китайский с использованием [Jieba](https://github.com/fxsjy/jieba) | charset_table=chinese | zh | morphology=jieba_chinese, требует пакет `manticore-language-packs` | Более точно, чем использование ngrams |
| Китайский с использованием ngrams| ngram_chars=chinese | zh | ngram_chars=1 | Быстрое индексирование, но производительность поиска может быть не так хороша |
| Хорватский | charset_table=non_cont | hr | - | |
| Курдский | charset_table=non_cont | ckb | - | |
| Чешский | charset_table=non_cont | cz | morphology=stem_cz (чешский стеммер) | |
| Датский | charset_table=non_cont | da | morphology=libstemmer_da | |
| Нидерландский | charset_table=non_cont | nl | morphology=libstemmer_nl | |
| Английский | charset_table=non_cont | en | morphology=lemmatize_en (единственная корневая форма); morphology=lemmatize_en_all (все корневые формы); morphology=stem_en (стеммер Портера для английского); morphology=stem_enru (стеммеры Портера для английского и русского); morphology=libstemmer_en (английский из libstemmer)  | |
| Эсперанто | charset_table=non_cont | eo | - | |
| Эстонский | charset_table=non_cont | et | - | |
| Финский | charset_table=non_cont | fi | morphology=libstemmer_fi | |
| Французский | charset_table=non_cont | fr | morphology=libstemmer_fr | |
| Галисийский | charset_table=non_cont | gl | - | |
| Гаро | specify charset_table manually | - | - | |
| Немецкий | charset_table=non_cont | de | morphology=lemmatize_de (единственная корневая форма); morphology=lemmatize_de_all (все корневые формы); morphology=libstemmer_de | |
| Греческий | charset_table=non_cont | el | morphology=libstemmer_el | |
| Иврит | charset_table=non_cont | he | - | |
| Хинди | charset_table=non_cont | hi | morphology=libstemmer_hi | |
| Хмонг | specify charset_table manually | - | - | |
| Хо | specify charset_table manually | - | - | |
| Венгерский | charset_table=non_cont | hu | morphology=libstemmer_hu | |
| Индонезийский | charset_table=non_cont | id | morphology=libstemmer_id | |
| Ирландский | charset_table=non_cont | ga | morphology=libstemmer_ga | |
| Итальянский | charset_table=non_cont | it | morphology=libstemmer_it | |
| Японский | ngram_chars=japanese | - | ngram_chars=japanese ngram_len=1 | Требует сегментацию на основе ngram |
| Коми | specify charset_table manually | - | - | |
| Корейский | ngram_chars=korean | - | ngram_chars=korean ngram_len=1 | Требует сегментацию на основе ngram  |
| Большой цветной мiao | specify charset_table manually | - | - | |
| Латынь | charset_table=non_cont | la | - | |
| Латышский | charset_table=non_cont | lv | - | |
| Литовский | charset_table=non_cont | lt | morphology=libstemmer_lt | |
| Маба | specify charset_table manually | - | - | |
| Майтхили | specify charset_table manually | - | - | |
| Марати | specify charset_table manually | - | - | |
| Марати | charset_table=non_cont | mr | - | |
| Менде | specify charset_table manually | - | - | |
| Мру | specify charset_table manually | - | - | |
| Мьене | specify charset_table manually | - | - | |
| Непали | specify charset_table manually | - | morphology=libstemmer_ne | |
| Нгамбай | specify charset_table manually | - | - | |
| Норвежский | charset_table=non_cont | no | morphology=libstemmer_no | |
| Одиа | specify charset_table manually | - | - | |
| Персидский | charset_table=non_cont | fa | - | |
| Поляки | charset_table=non_cont | pl | - | |
| Португальский | charset_table=non_cont | pt | morphology=libstemmer_pt | |
| Румынский | charset_table=non_cont | ro | morphology=libstemmer_ro | |
| Русский | charset_table=non_cont | ru | morphology=lemmatize_ru (единственная корневая форма); morphology=lemmatize_ru_all (все корневые формы); morphology=stem_ru (стеммер Портера для русского); morphology=stem_enru (стеммеры Портера для английского и русского); morphology=libstemmer_ru (из libstemmer) | |
| Сантали | specify charset_table manually | - | - | |
| Синди | specify charset_table manually | - | - | |
| Словацкий | charset_table=non_cont | sk | - | |
| Словенский | charset_table=non_cont | sl | - | |
| Сомали | charset_table=non_cont | so | - | |
| Сото | charset_table=non_cont | st | - | |
| Испанский | charset_table=non_cont | es | morphology=libstemmer_es | |
| Свахили | charset_table=non_cont | sw | - | |
| Шведский | charset_table=non_cont | sv | morphology=libstemmer_sv | |
| Силхети | specify charset_table manually | - | - | |
| Тамильский | specify charset_table manually | - | morphology=libstemmer_ta | |
| Тайский | charset_table=thai | th | - | |
| Турецкий | charset_table=non_cont | tr | morphology=libstemmer_tr | |
| Украинский | charset_table=non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491   | - | morphology=lemmatize_uk_all | Требует [установки](../../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) украинского лемматизатора |
| Йоруба | charset_table=non_cont | yo | - | |
| Зулу | charset_table=non_cont | zu | - |  |
<!-- proofread -->
