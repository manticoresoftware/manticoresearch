# Ротация журналов запросов и сервера

Manticore Search принимает сигнал USR1 для повторного открытия файлов журнала сервера и запросов.

Официальные [пакеты DEB и RPM](https://manticoresearch.com/install/) устанавливают конфигурационный файл Logrotate для всех файлов в каталоге логов по умолчанию.

Простая конфигурация logrotate для файлов журналов выглядит следующим образом:

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

Кроме того, доступна SQL-команда `FLUSH LOGS`, которая работает так же, как и системный сигнал USR1. Она инициирует повторное открытие файлов журнала searchd и журнала запросов, что позволяет реализовать ротацию файлов журналов. Команда не блокирует выполнение (то есть возвращается немедленно).
<!-- proofread -->

