#!/bin/bash

# Версии MariaDB и MySQL
versions=("mariadb:10.5" "mariadb:10.6" "mariadb:10.7" "mariadb:10.8" "mariadb:10.9" "mariadb:10.10" "mariadb:10.11" "mariadb:11.0" "mariadb:11.1" "mariadb:11.2" "mariadb:11.3" "mariadb:latest" "mysql:5.6" "mysql:5.7" "mysql:8.0" "mysql:8.2" "mysql:latest")

# Перебор всех версий
for version in "${versions[@]}"; do
    echo "Тестирование $version"

    # Запуск контейнера
	docker pull -q $version > /dev/null
    docker run -d --network=test_network --name db-test -e MYSQL_ROOT_PASSWORD=my-secret-pw $version


    # Ожидание запуска
    sleep 15

    # Выполнение mysqldump
    docker exec db-test mariadb-dump -hmanticore -P9306 manticore > dump.sql

    # Проверка на наличие ошибок
    if [ -s dump.sql ]; then
        echo "test $version выполнен успешно"
    else
        echo "Ошибка при выполнении test для $version"
    fi

    # Остановка и удаление контейнера (заглушить вывод )
    docker stop db-test > /dev/null
    docker rm db-test > /dev/null
	rm dump.sql
done
