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
Это не удалит ваши данные. Если вы вносили изменения в файл конфигурации, он будет сохранен в `/etc/manticoresearch/manticore.conf.rpmsave`.

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
Это не удалит ваши данные или файл конфигурации.

Если вы ищете отдельные пакеты, пожалуйста, найдите их [здесь](https://manticoresearch.com/install/#separate-packages).

Для получения дополнительной информации об установке смотрите [ниже](../Installation/Debian_and_Ubuntu.md).

<!-- request MacOS -->

``` bash
brew install manticoresoftware/tap/manticoresearch manticoresoftware/tap/manticore-extra
```

Пожалуйста, найдите больше информации об установке [ниже](../Installation/MacOS.md).

<!-- request Windows -->

1. Скачайте [Manticore Search Installer](https://repo.manticoresearch.com/repository/manticoresearch_windows/release/x64/manticore-6.0.4-230314-1a3a4ea82-x64.exe) и запустите его. Следуйте инструкциям по установке.
2. Выберите директорию для установки.
3. Выберите компоненты, которые вы хотите установить. Мы рекомендуем установить все из них.
4. Manticore поставляется с предварительно настроенным файлом `manticore.conf` в [режиме RT](https://manual.manticoresearch.com/Read_this_first.md#Real-time-mode-vs-plain-mode). Дополнительная конфигурация не требуется.

Для получения дополнительной информации об установке смотрите [ниже](../Installation/Windows.md#Installing-Manticore-on-Windows).

<!-- request Docker -->

Однострочная команда для песочницы (не рекомендуется для производственного использования):
``` bash
docker run --name manticore --rm -d manticoresearch/manticore && echo "Waiting for Manticore docker to start. Consider mapping the data_dir to make it start faster next time" && until docker logs manticore 2>&1 | grep -q "accepting connections"; do sleep 1; echo -n .; done && echo && docker exec -it manticore mysql && docker stop manticore
```

Это запустит контейнер Manticore и дождется его загрузки. После запуска он открывает сеанс клиента MySQL. Когда вы выходите из клиента MySQL, контейнер Manticore прекращает работу и удаляется, не оставляя храненных данных. Для получения информации о том, как использовать Manticore в условиях реального производства, смотрите следующий раздел.

Для производственного использования:
``` bash
docker run --name manticore -v $(pwd)/data:/var/lib/manticore -p 127.0.0.1:9306:9306 -p 127.0.0.1:9308:9308 -d manticoresearch/manticore
```

Эта настройка позволит использовать библиотеку Manticore Columnar и Manticore Buddy, и запустить Manticore на портах 9306 для соединений MySQL и 9308 для всех остальных соединений, используя `./data/` как назначенную директорию данных.

Узнайте больше о производственном использовании [в документации](https://github.com/manticoresoftware/docker#production-use).

<!-- request Kubernetes -->

``` bash
helm repo add manticoresearch https://helm.manticoresearch.com
# Update values.yaml if needed
helm install manticore -n manticore --create-namespace manticoresearch/manticoresearch
```

Вы можете найти больше информации об установке графика Helm [в документации](https://github.com/manticoresoftware/manticoresearch-helm#installation).

<!-- end -->

<!-- proofread -->
