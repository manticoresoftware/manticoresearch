# Установка Manticore на Windows

Manticore можно установить на Windows несколькими способами. Мы рекомендуем использовать WSL (Windows Subsystem for Linux), так как он поддерживает репликацию и не требует Docker. Ниже приведены рекомендуемые и альтернативные методы.

### Установка или включение WSL2

Чтобы установить Manticore Search на Windows, сначала необходимо включить Windows Subsystem for Linux. WSL2 позволяет запускать бинарные файлы Linux нативно на Windows. Для этого метода требуется версия Windows 10 не ниже 2004 или Windows 11.

Следуйте [официальному руководству Microsoft](https://docs.microsoft.com/en-us/windows/wsl/install) для поэтапных инструкций по установке WSL2.

### Установка Manticore

Для установки Manticore на Windows через WSL2 обратитесь к разделу [Debian и Ubuntu Installation](../Installation/Debian_and_Ubuntu.md).

> ПРИМЕЧАНИЕ: Установка Manticore через WSL2 является рекомендуемым методом, так как обеспечивает лучшую совместимость по сравнению с использованием нативных пакетов Windows.

## Установка Manticore в виде нативных бинарных файлов Windows

В качестве альтернативы можно установить Manticore в виде нативных бинарных файлов Windows, для которых требуется Docker для [Manticore Buddy](../Installation/Manticore_Buddy.md#Manticore-Buddy), выполнив следующие шаги:

1. Установите Docker Desktop и запустите его.
2. Скачайте установщик Manticore Search (ссылку можно найти на [странице установки](http://manticoresearch.com/install/)) и запустите его. Следуйте инструкциям по установке.
3. Выберите каталог для установки.
4. Выберите компоненты для установки. Мы рекомендуем установить все.
5. Manticore поставляется с преднастроенным файлом `manticore.conf` в [RT режиме](../Read_this_first.md#Real-time-mode-vs-plain-mode). Дополнительная настройка не требуется.

> ПРИМЕЧАНИЕ: Нативные бинарные файлы Windows не поддерживают [репликацию](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication).

### Установка как службы Windows

Чтобы установить сервер Manticore Search как службу Windows, выполните:

```bat
\path\to\searchd.exe --install --config \path\to\config --servicename Manticore
```

Убедитесь, что используете полный путь к конфигурационному файлу, иначе `searchd.exe` не сможет его найти при запуске службы.

После установки службу можно запустить из консоли служб (Services snap-in) Microsoft Management Console.

После запуска вы можете получить доступ к Manticore через интерфейс командной строки MySQL:

```bat
mysql -P9306 -h127.0.0.1
```

Обратите внимание, что в большинстве примеров этого руководства мы используем `-h0` для подключения к локальному хосту, но в Windows необходимо явно использовать `localhost` или `127.0.0.1`.

<!-- proofread -->

