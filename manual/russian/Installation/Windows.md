# Установка Manticore на Windows

Manticore можно установить на Windows несколькими способами. Мы рекомендуем использовать WSL (Подсистема Windows для Linux), так как она поддерживает репликацию и не требует Docker. Ниже приведены рекомендуемые и альтернативные методы.

### Установка или включение WSL2

Для установки Manticore Search на Windows сначала необходимо включить Подсистему Windows для Linux. WSL2 позволяет запускать Linux-бинарные файлы нативно на Windows. Для работы этого метода требуется Windows 10 версии 2004 и выше или Windows 11.

Следуйте [официальному руководству Microsoft](https://docs.microsoft.com/en-us/windows/wsl/install) для пошаговой установки WSL2.

### Установка Manticore

Для установки Manticore на Windows через WSL2 обратитесь к разделу [Установка на Debian и Ubuntu](../Installation/Debian_and_Ubuntu.md).

> ПРИМЕЧАНИЕ: Установка Manticore через WSL2 — рекомендуемый метод, так как он обеспечивает лучшую совместимость по сравнению с использованием нативных пакетов Windows.

## Установка Manticore как нативных бинарных файлов Windows

В качестве альтернативы вы можете установить Manticore как нативные бинарные файлы Windows, для которых требуется Docker для [Manticore Buddy](../Installation/Manticore_Buddy.md#Manticore-Buddy), выполнив следующие шаги:

1. Установите Docker Desktop и запустите его.
2. Скачайте установщик Manticore Search (ссылку можно найти на [странице установки](http://manticoresearch.com/install/)) и запустите его. Следуйте инструкциям по установке.
3. Выберите каталог для установки.
4. Выберите компоненты, которые хотите установить. Рекомендуется установить все.
5. Manticore поставляется с преднастроенным файлом `manticore.conf` в [RT режиме](../Read_this_first.md#Real-time-mode-vs-plain-mode). Дополнительная настройка не требуется.

> ПРИМЕЧАНИЕ: Нативные бинарные файлы Windows не поддерживают [репликацию](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication).

### Установка как службы Windows

Для установки сервера Manticore Search как службы Windows выполните:

```bat
\path\to\searchd.exe --install --config \path\to\config --servicename Manticore
```

Обязательно используйте полный путь к файлу конфигурации, иначе `searchd.exe` не сможет его найти при запуске в качестве службы.

После установки службу можно запустить через оснастку Службы в Microsoft Management Console.

После запуска вы можете получить доступ к Manticore через интерфейс командной строки MySQL:

```bat
mysql -P9306 -h127.0.0.1
```

Обратите внимание, что в большинстве примеров в этом руководстве мы используем `-h0` для подключения к локальному хосту, но в Windows необходимо явно использовать `localhost` или `127.0.0.1`.

<!-- proofread -->

