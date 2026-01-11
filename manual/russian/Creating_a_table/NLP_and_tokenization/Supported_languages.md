# Поддерживаемые языки

Manticore поддерживает широкий спектр языков, при этом базовая поддержка включена для большинства языков через `charset_table = non_cont` (что является значением по умолчанию). Также можно использовать опцию `non_cjk`, которая является синонимом для `non_cont`: `charset_table = non_cjk`.

Для многих языков Manticore предоставляет файл с [стоп-словами](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords), который можно использовать для улучшения релевантности поиска.

Кроме того, для некоторых языков доступна продвинутая морфология, которая может значительно улучшить релевантность поиска, используя лемматизацию на основе словаря или алгоритмы стемминга для лучшей сегментации и нормализации.

В таблице ниже перечислены все поддерживаемые языки и указано, как включить:
* базовую поддержку (столбец "Supported")
* стоп-слова (столбец "Stopwords file name")
* продвинутую морфологию (столбец "Advanced morphology")

| Язык | Поддерживается | Имя файла стоп-слов | Расширенная морфология | Примечания |
| - | - | - | - | - |
| Африкаанс | charset_table=non_cont | af | - | |
| Арабский | charset_table=non_cont | ar | morphology=stem_ar (арабский стеммер); morphology=libstemmer_ar  | |
| Армянский | charset_table=non_cont | hy | - | |
| Ассамский | указать charset_table вручную | - | - | |
| Баскский | charset_table=non_cont | eu | - | |
| Бенгальский | charset_table=non_cont | bn | - | |
| Биснуприя | указать charset_table вручную | - | - | |
| Бухид | указать charset_table вручную | - | - | |
| Болгарский | charset_table=non_cont | bg | - | |
| Каталанский | charset_table=non_cont | ca | morphology=libstemmer_ca | |
| Китайский с использованием [ICU](https://icu.unicode.org/) | charset_table=chinese | zh | morphology=icu_chinese | Более точно, чем использование ngrams |
| Китайский с использованием [Jieba](https://github.com/fxsjy/jieba) | charset_table=chinese | zh | morphology=jieba_chinese, требуется пакет `manticore-language-packs` | Более точно, чем использование ngrams |
| Китайский с использованием ngrams| ngram_chars=chinese | zh | ngram_chars=1 | Быстрая индексация, но производительность поиска может быть не такой хорошей |
| Хорватский | charset_table=non_cont | hr | - | |
| Курдский | charset_table=non_cont | ckb | - | |
| Чешский | charset_table=non_cont | cz | morphology=stem_cz (чешский стеммер) | |
| Датский | charset_table=non_cont | da | morphology=libstemmer_da | |
| Голландский | charset_table=non_cont | nl | morphology=libstemmer_nl | |
| Английский | charset_table=non_cont | en | morphology=lemmatize_en (одна основная форма); morphology=lemmatize_en_all (все основные формы); morphology=stem_en (стеммер Портера для английского); morphology=stem_enru (стеммеры Портера для английского и русского); morphology=libstemmer_en (английский из libstemmer)  | |
| Эсперанто | charset_table=non_cont | eo | - | |
| Эстонский | charset_table=non_cont | et | - | |
| Финский | charset_table=non_cont | fi | morphology=libstemmer_fi | |
| Французский | charset_table=non_cont | fr | morphology=libstemmer_fr | |
| Галисийский | charset_table=non_cont | gl | - | |
| Гаро | указать charset_table вручную | - | - | |
| Немецкий | charset_table=non_cont | de | morphology=lemmatize_de (одна основная форма); morphology=lemmatize_de_all (все основные формы); morphology=libstemmer_de | |
| Греческий | charset_table=non_cont | el | morphology=libstemmer_el | |
| Иврит | charset_table=non_cont | he | - | |
| Хинди | charset_table=non_cont | hi | morphology=libstemmer_hi | |
| Хмонг | указать charset_table вручную | - | - | |
| Хо | указать charset_table вручную | - | - | |
| Венгерский | charset_table=non_cont | hu | morphology=libstemmer_hu | |
| Индонезийский | charset_table=non_cont | id | morphology=libstemmer_id | |
| Ирландский | charset_table=non_cont | ga | morphology=libstemmer_ga | |
| Итальянский | charset_table=non_cont | it | morphology=libstemmer_it | |
| Японский | ngram_chars=japanese | - | ngram_chars=japanese ngram_len=1 | Требуется сегментация на основе ngram |
| Коми | указать charset_table вручную | - | - | |
| Корейский | ngram_chars=korean | - | ngram_chars=korean ngram_len=1 | Требуется сегментация на основе ngram  |
| Большой Цветочный Мяо | указать charset_table вручную | - | - | |
| Латинский | charset_table=non_cont | la | - | |
| Латышский | charset_table=non_cont | lv | - | |
| Литовский | charset_table=non_cont | lt | morphology=libstemmer_lt | |
| Маба | указать charset_table вручную | - | - | |
| Майтхили | указать charset_table вручную | - | - | |
| Маратхи | указать charset_table вручную | - | - | |
| Маратхи | charset_table=non_cont | mr | - | |
| Менде | указать charset_table вручную | - | - | |
| Мру | указать charset_table вручную | - | - | |
| Миене | указать charset_table вручную | - | - | |
| Непальский | указать charset_table вручную | - | morphology=libstemmer_ne | |
| Нгамбай | указать charset_table вручную | - | - | |
| Норвежский | charset_table=non_cont | no | morphology=libstemmer_no | |
| Ория | указать charset_table вручную | - | - | |
| Персидский | charset_table=non_cont | fa | - | |
| Польский | charset_table=non_cont | pl | - | |
| Португальский | charset_table=non_cont | pt | morphology=libstemmer_pt | |
| Румынский | charset_table=non_cont | ro | morphology=libstemmer_ro | |
| Русский | charset_table=non_cont | ru | morphology=lemmatize_ru (одна основная форма); morphology=lemmatize_ru_all (все основные формы); morphology=stem_ru (стеммер Портера для русского); morphology=stem_enru (стеммеры Портера для английского и русского); morphology=libstemmer_ru (из libstemmer) | |
| Сантали | указать charset_table вручную | - | - | |
| Синдхи | указать charset_table вручную | - | - | |
| Словацкий | charset_table=non_cont | sk | - | |
| Словенский | charset_table=non_cont | sl | - | |
| Сомалийский | charset_table=non_cont | so | - | |
| Сесото | charset_table=non_cont | st | - | |
| Испанский | charset_table=non_cont | es | morphology=libstemmer_es | |
| Суахили | charset_table=non_cont | sw | - | |
| Шведский | charset_table=non_cont | sv | morphology=libstemmer_sv | |
| Сильхети | указать charset_table вручную | - | - | |
| Тамильский | указать charset_table вручную | - | morphology=libstemmer_ta | |
| Тайский | charset_table=thai | th | - | |
| Турецкий | charset_table=non_cont | tr | morphology=libstemmer_tr | |
| Украинский | charset_table=non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491   | - | morphology=lemmatize_uk_all | Требуется [установка](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) украинского лемматизатора |
| Йоруба | charset_table=non_cont | yo | - | |
| Зулу | charset_table=non_cont | zu | - |  |
<!-- proofread -->

