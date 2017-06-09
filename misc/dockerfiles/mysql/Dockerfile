FROM mysql:5.7

# default values
ENV MYSQL_ROOT_PASSWORD test
ENV MYSQL_USER test
ENV MYSQL_PASSWORD test
ENV MYSQL_DATABASE test

VOLUME /var/lib/mysql

ENTRYPOINT ["docker-entrypoint.sh"]

CMD ["mysqld"]
