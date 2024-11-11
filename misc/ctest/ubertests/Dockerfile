FROM ubuntu:jammy
ENV DEBIAN_FRONTEND noninteractive
ENV DISTR jammy

RUN apt-get -y update && apt-get install -y wget

RUN wget https://repo.manticoresearch.com/manticore-dev-repo.noarch.deb && \
    dpkg -i manticore-dev-repo.noarch.deb && \
    apt-get update && apt-get install -y \
    manticore-tzdata \
    manticore-language-packs \
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
    ca-certificates \
    ninja-build \
    lsb-release \
    software-properties-common \
    php \
    php-mysql \
    php-curl \
    php-xml \
    php-dompdf \
    python3 \
    ssh \
    mysql-server \
    xsltproc \
    openssl \
    python3-dev

RUN cd / && wget https://dl.min.io/client/mc/release/linux-amd64/mc \
    && chmod +x mc && \
    mv mc /usr/local/bin/ && \
    mc update || [ $? -eq 1 ]

# boost
ARG boostminorver=75

# boost*tar.gz originates from https://boostorg.jfrog.io/artifactory/main/release/1.${boostminorver}.0/source/boost_1_${boostminorver}_0.tar.gz
RUN wget https://repo.manticoresearch.com/repository/misc/boost_1_${boostminorver}_0.tar.gz \
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

# Ubertests settings file
ARG TARGETPLATFORM
RUN if [ "$TARGETPLATFORM" = "linux/arm64" ]; then ARCHITECTURE=aarch64; else ARCHITECTURE=x86-64bit; fi \
&& wget --no-check-certificate -q -O /odbc.tar.gz https://dev.mysql.com/get/Downloads/Connector-ODBC/8.3/mysql-connector-odbc-8.3.0-linux-glibc2.28-${ARCHITECTURE}.tar.gz \
    && tar -zxf /odbc.tar.gz \
    && rm /odbc.tar.gz \
    && mkdir -p /var/run/mysqld && chmod a+rwX /var/run/mysqld \
    && { mysqld & } && sleep 30 \
    && mysql -e 'CREATE DATABASE test; CREATE USER test@localhost; GRANT ALL PRIVILEGES ON test.* TO test@localhost;' \
    && mysqladmin shutdown \
&& <<EOF cat > /root/.sphinx
(
    "db-host"=>"127.0.0.1",
    "db-user"=>"test",
    'lemmatizer_base'=>'/work/aot',
    'odbc_driver'=>'/mysql-connector-odbc-8.3.0-linux-glibc2.28-${ARCHITECTURE}/lib/libmyodbc8w.so;CHARSET=utf8'
)
EOF

# ODBC notes:
# * Urls for various odbc mysql driver versions:
#   - https://dev.mysql.com/get/Downloads/Connector-ODBC/8.3/mysql-connector-odbc-8.3.0-linux-glibc2.28-x86-64bit.tar.gz
#   - https://dev.mysql.com/get/Downloads/Connector-ODBC/8.3/mysql-connector-odbc-8.3.0-linux-glibc2.28-aarch64.tar.gz
#   - https://dev.mysql.com/get/Downloads/Connector-ODBC/5.3/mysql-connector-odbc-5.3.9-linux-ubuntu16.04-x86-64bit.tar.gz
# * If test 228 fails with broken encoding - try to remove ';CHARSET=utf8' ending in the odbc_driver line above. M.b. better is to probe once on local instance before pushing to dockerhub

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

RUN git config --global --add safe.directory '*'
ADD entry_point.sh /
VOLUME [ "/work" ]
WORKDIR /work
ENTRYPOINT ["bash", "/entry_point.sh"]
CMD []

# docker buildx create --name manticore_build --platform linux/amd64,linux/arm64
# docker buildx use manticore_build
# docker buildx build --no-cache --platform linux/amd64,linux/arm64 --push -t manticoresearch/ubertests_ctest:3263_20241005 .

# deprecated:
# docker build -t registry.gitlab.com/manticoresearch/dev/ubertests_ctest:3263 .
# docker push registry.gitlab.com/manticoresearch/dev/ubertests_ctest:3263
