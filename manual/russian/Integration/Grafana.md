# Интеграция с Grafana

> ПРИМЕЧАНИЕ: Интеграция с Grafana требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

[Grafana](https://grafana.com/) — это платформа с открытым исходным кодом для визуализации данных и мониторинга, которая позволяет создавать интерактивные панели и графики. Manticore Search интегрируется с Grafana с использованием стандартного MySQL-коннектора, что позволяет визуализировать данные поиска, отслеживать метрики производительности и анализировать тренды в режиме реального времени.

В настоящее время протестированы и поддерживаются версии Grafana 10.0–12.4.

## Требования

Перед интеграцией Manticore Search с Grafana убедитесь, что:

1. Manticore Search (версии 6.2.0 или выше) правильно установлен и настроен на вашем сервере. Руководство по установке можно найти в [официальном руководстве по установке Manticore Search](../Installation/Installation.md).
2. Grafana установлена на вашей системе. Следуйте инструкции из [официального руководства по установке Grafana](https://grafana.com/docs/grafana/latest/setup-grafana/installation/).

## Подключение Manticore Search к Grafana

Чтобы подключить Manticore Search к Grafana:

1. Войдите в свою панель управления Grafana и нажмите на «Configuration» (иконка шестерёнки) в левой боковой панели.
2. Выберите «Data Sources» и нажмите «Add data source».
3. Выберите «MySQL» из списка доступных источников данных.
4. На странице настроек укажите следующие данные:
   - **Name**: Название источника данных (например, «Manticore Search»)
   - **Host**: Имя хоста или IP адрес сервера Manticore Search (с портом MySQL, по умолчанию: `localhost:9306`)
   - **Database**: Оставьте пустым или укажите имя вашей базы данных
   - **User**: Имя пользователя, имеющего доступ к Manticore Search (по умолчанию: `root`)
   - **Password**: Пароль для указанного пользователя (по умолчанию: пустой)
5. Нажмите «Save & Test», чтобы проверить подключение.

## Создание визуализаций и панелей

После подключения Manticore Search к Grafana вы можете создавать панели и визуализации:

1. В панели Grafana нажмите на значок «+» в левой боковой панели и выберите «New dashboard».
2. Нажмите кнопку «Add visualization», чтобы начать настройку графика.
3. Выберите источник данных Manticore Search, который вы подключили через MySQL-коннектор.
4. Выберите тип графика, который хотите создать (например, Time Series, Bar chart, Candlestick, Pie chart).
5. Используйте конструктор запросов Grafana или напишите SQL-запрос для получения данных из таблиц Manticore Search.
6. Настройте внешний вид графика, подписи и другие параметры по необходимости.
7. Нажмите «Apply», чтобы сохранить визуализацию на панели.

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

В Grafana вы можете создавать:
- **График временных рядов**: Визуализировать изменения цен со временем
- **Свечный график**: Отображать значения открытия, максимума, минимума и закрытия для финансовых данных
- **Агрегационные графики**: Использовать функции COUNT, AVG, MAX, MIN

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
- Фильтровать данные с помощью WHERE
- Получать метаданные таблиц через `information_schema.tables`
- Создавать различные типы визуализаций, поддерживаемые Grafana

## Ограничения

- Некоторые продвинутые функции MySQL могут быть недоступны при работе с Manticore Search через Grafana.
- Доступны только функции, поддерживаемые Manticore Search. Подробнее смотрите в [справочнике по SQL](../Searching/Full_text_matching/Basic_usage.md).

## Ссылки

Для получения дополнительной информации и подробных инструкций:
- [Пост в блоге об интеграции с Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/)
- [Официальная документация Grafana](https://grafana.com/docs/)
