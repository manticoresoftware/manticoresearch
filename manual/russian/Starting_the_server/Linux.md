# Запуск Manticore в Linux

### Запуск и остановка с помощью systemd

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

Чтобы включить автозапуск Manticore при загрузке, выполните:

```bash
sudo systemctl enable manticore
```

Процесс `searchd` записывает информацию о запуске в журнал `systemd`. Если ведение журнала systemd включено, вы можете просмотреть записанную информацию с помощью следующей команды:

```shell
sudo journalctl -u manticore
```

### Пользовательские параметры запуска с помощью systemd

`systemctl set-environment _ADDITIONAL_SEARCHD_PARAMS` позволяет указать пользовательские флаги запуска, с которыми следует запускать демон Manticore Search. Полный список смотрите [здесь](../Starting_the_server/Manually.md#searchd-command-line-options).

Например, чтобы запустить Manticore с уровнем логирования debug, можно выполнить:
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

### Запуск и остановка с помощью service

Manticore можно запускать и останавливать с помощью команд service:

```shell
sudo service manticore start
sudo service manticore stop
```

Чтобы включить службу sysV для автозапуска в системах RedHat, выполните:

```shell
chkconfig manticore on
```

Чтобы включить службу sysV для автозапуска в системах Debian (включая Ubuntu), выполните:

```shell
update-rc.d manticore defaults
```

Обратите внимание, что `searchd` запускается системой инициализации от имени пользователя `manticore`, и все файлы, созданные сервером, будут принадлежать этому пользователю. Если `searchd` запустить, например, от имени root, права на файлы изменятся, что может вызвать проблемы при повторном запуске `searchd` в качестве службы.

<!-- proofread -->

