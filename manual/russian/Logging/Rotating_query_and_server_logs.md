# Ротация логов запросов и сервера

Manticore Search принимает сигнал USR1 для повторного открытия файлов логов сервера и запросов.

Официальные [DEB и RPM пакеты](https://manticoresearch.com/install/) устанавливают файл конфигурации Logrotate для всех файлов в папке логов по умолчанию.

Простая конфигурация logrotate для файлов логов выглядит так:

```ini
/var/log/manticore/*.log {
       weekly
       rotate 10
       copytruncate
       delaycompress
       compress
       notifempty
       missingok
}
```

## FLUSH LOGS

```sql
mysql> FLUSH LOGS;
Query OK, 0 rows affected (0.01 sec)
```

Кроме того, доступна SQL команда `FLUSH LOGS`, которая работает так же, как системный сигнал USR1. Она инициирует повторное открытие файлов логов searchd и логов запросов, позволяя вам реализовать ротацию файлов логов. Команда неблокирующая (т.е., возвращает управление немедленно).
<!-- proofread -->

