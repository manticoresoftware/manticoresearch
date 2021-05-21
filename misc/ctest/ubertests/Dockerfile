# production system for ubertesting. Look base at dist/build_dockers/bionic/cmake_320

FROM registry.gitlab.com/manticoresearch/dev/bionic_cmake:320
RUN apt-get update && apt-get install -y --no-install-recommends -q \
    php \
    php-mysql \
    php-curl \
    php-xml \
    php-dompdf \
    python \
    ssh \
    mysql-server \
    xsltproc \
    openssl \
    python-dev \
    && rm -rf /var/lib/apt/lists/* \
    && wget --no-check-certificate -q -O /odbc.tar.gz https://dev.mysql.com/get/Downloads/Connector-ODBC/5.3/mysql-connector-odbc-5.3.9-linux-ubuntu16.04-x86-64bit.tar.gz \
    && tar -zxf /odbc.tar.gz \
    && rm /odbc.tar.gz \    
    && mkdir -p /var/run/mysqld && chmod a+rwX /var/run/mysqld \
    && { mysqld & } && sleep 3 \
    && mysql -e 'CREATE DATABASE test; CREATE USER test@localhost; GRANT ALL PRIVILEGES ON test.* TO test@localhost;' \
    && mysqladmin shutdown

# update stuff that may exipre
RUN apt-get update && apt-get install -y \
    ca-certificates \
&& rm -rf /var/lib/apt/lists/* \
&& mc update

ADD sphinx /root/.sphinx
ADD entry_point.sh /
VOLUME [ "/work" ]
WORKDIR /work
ENTRYPOINT ["bash", "/entry_point.sh"]
CMD []

# docker build -t registry.gitlab.com/manticoresearch/dev/ubertests_public:320 .