# Установка

<!-- example installation expanded -->

<!-- request RHEL, Centos, Alma, Amazon, Oracle, Debian, Ubuntu, Mint, MacOS -->

``` bash
curl https://manticoresearch.com | sh
```

Если вы ищете отдельные пакеты, вы можете найти их [здесь](https://manticoresearch.com/install/#separate-packages).

Подробнее об установке в Linux на базе RPM см. [здесь](../Installation/RHEL_and_Centos.md).

Подробнее об установке в Linux на базе DEBIAN см. [здесь](../Installation/Debian_and_Ubuntu.md).

Подробнее об установке в macOS см. [ниже](../Installation/MacOS.md).

Подробнее о быстром установщике выполните

``` bash
curl https://manticoresearch.com | sh -s help
```

<!-- request Windows -->

1. Загрузите [Установщик Manticore Search](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-6.0.4-230314-1a3a4ea82-x64.exe) и запустите его. Следуйте инструкциям по установке.
2. Выберите каталог для установки.
3. Выберите компоненты, которые хотите установить. Мы рекомендуем установить все.
4. Manticore поставляется с предварительно настроенным файлом `manticore.conf` в [RT режиме](https://manual.manticoresearch.com/Read_this_first.md#Real-time-mode-vs-plain-mode). Дополнительная настройка не требуется.

Для получения подробной информации об установке смотрите [ниже](../Installation/Windows.md#Installing-Manticore-on-Windows).

<!-- request Docker -->

Однострочная команда для песочницы (не рекомендуется для использования в продакшене):
``` bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

Эта команда запускает контейнер Manticore и ожидает его запуска. После запуска она открывает сессию MySQL клиента. Когда вы выходите из MySQL клиента, контейнер Manticore останавливается и удаляется, не оставляя данных. Подробности по использованию Manticore в рабочем продакшен-среде смотрите в следующем разделе.

Для использования в продакшене:
``` bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

Эта настройка активирует Manticore Columnar Library и Manticore Buddy, а также запустит Manticore на портах 9306 для MySQL соединений и 9308 для всех остальных, используя `./data/` в качестве директории для данных.

Подробнее о работе в продакшене читайте [в документации](https://github.com/manticoresoftware/docker#production-use).

<!-- request Kubernetes -->

``` bash
helm repo add manticoresearch https://helm.manticoresearch.com
# Update values.yaml if needed
helm install manticore -n manticore --create-namespace manticoresearch/manticoresearch
```

Больше информации об установке Helm чарта вы можете найти [в документации](https://github.com/manticoresoftware/manticoresearch-helm#installation).

<!-- end -->

<!-- proofread -->
