FROM ubuntu:bionic
ENV DEBIAN_FRONTEND noninteractive
ENV DISTR bionic

RUN apt-get update && apt-get install -y \
    libmysqlclient-dev \
    libexpat-dev \
    libpq-dev \
    unixodbc-dev \
    flex \
    bison \
    git \
    build-essential \
    libjemalloc-dev \
    libssl-dev \
    wget \
    ca-certificates \
    ninja-build \
    lsb-release \
    software-properties-common \
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
    python-dev

RUN cd / && wget https://dl.min.io/client/mc/release/linux-amd64/mc \
    && chmod +x mc && \
    mv mc /usr/local/bin/ && \
    mc update || [ $? -eq 1 ]

# boost
ARG boostminorver=75
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.${boostminorver}.0/source/boost_1_${boostminorver}_0.tar.gz \
    && tar -zxf boost_1_${boostminorver}_0.tar.gz && rm boost_1_${boostminorver}_0.tar.gz \
    && cd boost_1_${boostminorver}_0 \
    && ./bootstrap.sh \
    && ./b2 install  --with-context --with-system --with-filesystem \
    && cd .. && rm -rf boost_1_${boostminorver}_0

# cmake
ARG cmakever="3.26.3"
RUN cd / \
    && wget https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakever}-linux-$(uname -m).tar.gz \
    && tar -zxf cmake-${cmakever}-linux-$(uname -m).tar.gz \
    && rm cmake-${cmakever}-linux-$(uname -m).tar.gz \
    && export arch=$(uname -m)
ENV PATH $PATH:/cmake-${cmakever}-linux-x86_64/bin:/cmake-${cmakever}-linux-aarch64/bin

# clang
RUN wget https://apt.llvm.org/llvm.sh && \
chmod +x llvm.sh && \
./llvm.sh 13

ENV CC clang-13
ENV CXX clang++-13

# mysql
RUN wget --no-check-certificate -q -O /odbc.tar.gz https://dev.mysql.com/get/Downloads/Connector-ODBC/5.3/mysql-connector-odbc-5.3.9-linux-ubuntu16.04-x86-64bit.tar.gz \
    && tar -zxf /odbc.tar.gz \
    && rm /odbc.tar.gz \
    && mkdir -p /var/run/mysqld && chmod a+rwX /var/run/mysqld \
    && { mysqld & } && sleep 30 \
    && mysql -e 'CREATE DATABASE test; CREATE USER test@localhost; GRANT ALL PRIVILEGES ON test.* TO test@localhost;' \
    && mysqladmin shutdown

# Note: for mysql 8 change url above to https://dev.mysql.com/get/Downloads/Connector-ODBC/8.0/mysql-connector-odbc-8.0.27-linux-glibc2.12-x86-64bit.tar.gz,
# and 'odbc_driver'=>'/mysql-connector-odbc-8.0.21-linux-glibc2.12-x86-64bit/lib/libmyodbc8w.so;CHARSET=utf8' line in file 'sphinx'

# AOT
RUN mkdir -p /work/aot && cd /work/aot && \
wget https://repo.manticoresearch.com/repository/morphology/de.pak.tar.xz -q && \
tar -Jxf de.pak.tar.xz && \
wget https://repo.manticoresearch.com/repository/morphology/en.pak.tar.xz -q && \
tar -Jxf en.pak.tar.xz && \
wget https://repo.manticoresearch.com/repository/morphology/ru.pak.tar.xz -q && \
tar -Jxf ru.pak.tar.xz && \
rm *.tar.xz

# Cleanup
RUN rm -rf /var/lib/apt/lists/*

ADD sphinx /root/.sphinx
ADD entry_point.sh /
VOLUME [ "/work" ]
WORKDIR /work
ENTRYPOINT ["bash", "/entry_point.sh"]
CMD []

# docker build -t registry.gitlab.com/manticoresearch/dev/ubertests_ctest:3263 .
# docker push registry.gitlab.com/manticoresearch/dev/ubertests_ctest:3263
# docker buildx create --name manticore_build --platform linux/amd64,linux/arm64
# docker buildx use manticore_build
# docker buildx build --platform linux/amd64,linux/arm64 --push -t manticoresearch/ubertests_ctest:3263 .
