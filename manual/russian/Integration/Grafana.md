# Интеграция с Grafana

> ПРИМЕЧАНИЕ: Интеграция с Grafana требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

[Grafana](https://grafana.com/) — это платформа с открытым исходным кодом для визуализации данных и мониторинга, которая позволяет создавать интерактивные панели и графики. Manticore Search интегрируется с Grafana с помощью стандартного MySQL-коннектора, что позволяет визуализировать данные поиска, контролировать показатели производительности и анализировать тренды в реальном времени.

В настоящее время тестируются и поддерживаются версии Grafana с 10.0 по 12.3.

## Предварительные требования

Перед интеграцией Manticore Search с Grafana убедитесь, что:

1. Manticore Search (версия 6.2.0 или выше) правильно установлен и настроен на вашем сервере. Обратитесь к [официальному руководству по установке Manticore Search](../Installation/Installation.md) за помощью.
2. Grafana настроена на вашей системе. Следуйте [официальному руководству по установке Grafana](https://grafana.com/docs/grafana/latest/setup-grafana/installation/) для инструкции по установке.

## Подключение Manticore Search к Grafana

Чтобы подключить Manticore Search к Grafana:

1. Войдите в панель управления Grafana и нажмите на "Configuration" (иконка шестерёнки) в левой боковой панели.
2. Выберите "Data Sources" и нажмите "Add data source".
3. В списке доступных источников данных выберите "MySQL".
4. На странице настроек укажите следующие данные:
   - **Name**: название источника данных (например, "Manticore Search")
   - **Host**: имя хоста или IP-адрес сервера Manticore Search (с портом MySQL, по умолчанию: `localhost:9306`)
   - **Database**: оставить пустым или указать имя базы данных
   - **User**: имя пользователя с доступом к Manticore Search (по умолчанию: `root`)
   - **Password**: пароль для указанного пользователя (по умолчанию пусто)
5. Нажмите "Save & Test" для проверки подключения.

## Создание визуализаций и панелей

После подключения Manticore Search к Grafana вы можете создавать панели и визуализации:

1. В панели Grafana нажмите на значок "+" в левой боковой панели и выберите "New dashboard".
2. Нажмите кнопку "Add visualization", чтобы начать настройку графика.
3. Выберите источник данных Manticore Search, подключённый через MySQL-коннектор.
4. Выберите тип графика, который хотите создать (например, Time Series, Bar chart, Candlestick, Pie chart).
5. Используйте конструктор запросов Grafana или напишите SQL-запрос для получения данных из таблиц Manticore Search.
6. Настройте внешний вид графика, подписи и другие параметры по необходимости.
7. Нажмите "Apply", чтобы сохранить визуализацию на панели.

## Пример использования

Вот простой пример с данными временных рядов. Сначала создайте таблицу и загрузите пример данных:

```sql
CREATE TABLE btc_usd_trading (
  id bigint,
  time timestamp,
  open float,
  high float,
  low float,
  close float
);
```

Загрузите данные:
```bash
curl -sSL https://gist.githubusercontent.com/donhardman/df109ba6c5e690f73198b95f3768e73f/raw/0fab3aee69d7007fad012f4e97f38901a64831fb/btc_usd_trading.sql | mysql -h0 -P9306
```

В Grafana вы можете создать:
- **График временных рядов**: визуализировать изменение цены во времени
- **Свечной график (Candlestick chart)**: отображать открытие, максимум, минимум и закрытие для финансовых данных
- **Агрегационные графики**: использовать функции COUNT, AVG, MAX, MIN

Примеры запросов:
```sql
-- Time series query
SELECT time, close FROM btc_usd_trading ORDER BY time;

-- Aggregation query
SELECT DATE(time) as date, AVG(close) as avg_price 
FROM btc_usd_trading 
GROUP BY date 
ORDER BY date;
```

## Поддерживаемые функции

При работе с Manticore Search через Grafana вы можете:

- Выполнять SQL-запросы к таблицам Manticore Search
- Использовать агрегатные функции: COUNT, AVG, MAX, MIN
- Применять операции GROUP BY и ORDER BY
- Фильтровать данные с помощью операторов WHERE
- Получать метаданные таблиц через `information_schema.tables`
- Создавать различные типы визуализаций, поддерживаемых Grafana

## Ограничения

- Некоторые расширенные функции MySQL могут быть недоступны при работе с Manticore Search через Grafana.
- Доступны только функции, поддерживаемые Manticore Search. Смотрите [SQL справочник](../Searching/Full_text_matching/Basic_usage.md) для подробностей.

## Ссылки

Для дополнительной информации и подробных руководств:
- [Блог по интеграции с Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)
- [Официальная документация Grafana](https://grafana.com/docs/)
