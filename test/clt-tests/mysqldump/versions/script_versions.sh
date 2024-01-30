#!/bin/bash

# Версии MariaDB и MySQL
versions=("mariadb:latest" "mariadb:10.5" "mariadb:10.6")

    # Создание сети
    docker network create test_network

# Перебор всех версий
for version in "${versions[@]}"; do
    echo "Тестирование $version"

    # Запуск контейнера
    docker run -d --network=test_network --name db-test -e MYSQL_ROOT_PASSWORD=my-secret-pw $version

    # Ожидание запуска
    sleep 15

    # Выполнение mysqldump
    docker exec db-test mariadb -hmanticore -P9306 -e "show status";

    # Проверка на наличие ошибок
    if [ $? -eq 0 ]; then
        echo "mysqldump $version выполнен успешно"
    else
        echo "Ошибка при выполнении mysqldump для $version"
    fi

    # Остановка и удаление контейнера
    docker stop db-test
    docker rm db-test
done
