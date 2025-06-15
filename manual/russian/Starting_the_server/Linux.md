# Запуск Manticore в Linux

### Запуск и остановка с использованием systemd

После установки служба Manticore Search не запускается автоматически. Чтобы запустить Manticore, выполните следующую команду:

```shell
sudo systemctl start manticore
```

Чтобы остановить Manticore, выполните следующую команду:

```shell
sudo systemctl stop manticore
```

Служба Manticore настроена на запуск при загрузке. Вы можете проверить это, выполнив:

```shell
sudo systemctl is-enabled manticore
```

Если вы хотите отключить автоматический запуск Manticore при загрузке, выполните:

```bash
sudo systemctl disable manticore
```

Чтобы включить запуск Manticore при загрузке, выполните:

```bash
sudo systemctl enable manticore
```

Процесс `searchd` записывает информацию о запуске в журнал `systemd`. Если ведение журналов systemd включено, вы можете просмотреть записанную информацию с помощью следующей команды:

```shell
sudo journalctl -u manticore
```

### Пользовательские флаги запуска с использованием systemd

`systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS` позволяет указать пользовательские флаги запуска, с которыми должен запускаться демон Manticore Search. Полный список смотрите [здесь](../Starting_the_server/Manually.md#searchd-command-line-options).

Например, чтобы запустить Manticore с уровнем журналирования debug, выполните:
```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS='--logdebug'
systemctl restart manticore
```

Чтобы отменить это, выполните:
```bash
systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS=''
systemctl restart manticore
```

Обратите внимание, что переменные окружения systemd сбрасываются при перезагрузке сервера.

### Запуск и остановка с использованием service

Manticore можно запускать и останавливать с помощью команд service:

```shell
sudo service manticore start
sudo service manticore stop
```

Чтобы включить sysV службу при загрузке на системах RedHat, выполните:

```shell
chkconfig manticore on
```

Чтобы включить sysV службу при загрузке на системах Debian (включая Ubuntu), выполните:

```shell
update-rc.d manticore defaults
```

Обратите внимание, что `searchd` запускается системой инициализации от имени пользователя `manticore`, и все файлы, созданные сервером, будут принадлежать этому пользователю. Если `searchd` запускается, например, от имени пользователя root, права доступа к файлам изменятся, что может привести к проблемам при повторном запуске `searchd` в виде службы.

<!-- proofread -->

