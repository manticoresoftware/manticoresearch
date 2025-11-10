# Установка

<!-- example installation expanded -->

<!-- request RHEL, Centos, Alma, Amazon, Oracle -->

``` bash
sudo yum install https://repo.manticoresearch.com/manticore-repo.noarch.rpm
sudo yum install manticore manticore-extra
```

Если вы обновляетесь с более старой версии, рекомендуется сначала удалить старые пакеты, чтобы избежать конфликтов, вызванных обновленной структурой пакетов:
```bash
sudo yum --setopt=tsflags=noscripts remove manticore*
```
Это не удалит ваши данные. Если вы вносили изменения в конфигурационный файл, он будет сохранен в `/etc/manticoresearch/manticore.conf.rpmsave`.

Если вы ищете отдельные пакеты, пожалуйста, найдите их [здесь](https://manticoresearch.com/install/#separate-packages).

Для получения дополнительной информации об установке смотрите [ниже](../Installation/RHEL_and_Centos.md).

<!-- request Debian, Ubuntu, Mint -->

``` bash
wget https://repo.manticoresearch.com/manticore-repo.noarch.deb
sudo dpkg -i manticore-repo.noarch.deb
sudo apt update
sudo apt install manticore manticore-extra
```

Если вы обновляетесь до Manticore 6 с более старой версии, рекомендуется сначала удалить старые пакеты, чтобы избежать конфликтов, вызванных обновленной структурой пакетов:
```bash
sudo apt remove manticore*
```
Это не удалит ваши данные или конфигурационный файл.

Если вы ищете отдельные пакеты, пожалуйста, найдите их [здесь](https://manticoresearch.com/install/#separate-packages).

Для получения дополнительной информации об установке смотрите [ниже](../Installation/Debian_and_Ubuntu.md).

<!-- request MacOS -->

``` bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

Пожалуйста, найдите более подробную информацию об установке [ниже](../Installation/MacOS.md).

<!-- request Windows -->

1. Скачайте [установщик Manticore Search](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-6.0.4-230314-1a3a4ea82-x64.exe) и запустите его. Следуйте инструкциям по установке.
2. Выберите каталог для установки.
3. Выберите компоненты, которые хотите установить. Рекомендуется установить все.
4. Manticore поставляется с преднастроенным файлом `manticore.conf` в [RT режиме](https://manual.manticoresearch.com/Read_this_first.md#Real-time-mode-vs-plain-mode). Дополнительная настройка не требуется.

Для получения дополнительной информации об установке смотрите [ниже](../Installation/Windows.md#Installing-Manticore-on-Windows).

<!-- request Docker -->

Однострочник для песочницы (не рекомендуется для использования в продакшене):
``` bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

Это запускает контейнер Manticore и ждет его загрузки. После запуска он открывает сессию клиента MySQL. При выходе из клиента MySQL контейнер Manticore останавливается и удаляется, не оставляя данных. Для информации о том, как использовать Manticore в рабочей среде, смотрите следующий раздел.

Для использования в продакшене:
``` bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

Эта настройка активирует Manticore Columnar Library и Manticore Buddy, и запустит Manticore на портах 9306 для MySQL соединений и 9308 для всех остальных, используя `./data/` в качестве каталога данных.

Подробнее о продакшен использовании читайте [в документации](https://github.com/manticoresoftware/docker#production-use).

<!-- request Kubernetes -->

``` bash
helm repo add manticoresearch https://helm.manticoresearch.com
# Update values.yaml if needed
helm install manticore -n manticore --create-namespace manticoresearch/manticoresearch
```

Более подробную информацию об установке Helm chart вы можете найти [в документации](https://github.com/manticoresoftware/manticoresearch-helm#installation).

<!-- end -->

<!-- proofread -->

