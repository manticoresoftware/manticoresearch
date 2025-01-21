FROM ubuntu:jammy
ENV DEBIAN_FRONTEND noninteractive

# toolchain stuff
ARG cmakever="3.31.2"
RUN apt-get update && apt-get install -y --no-install-recommends  \
    curl \
    git \
    build-essential \
    ninja-build \
    xsltproc \
    && rm -rf /var/lib/apt/lists/* \
    && git config --global --add safe.directory '*' \
    && cd / && curl -k -L https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakever}-linux-x86_64.tar.gz | tar -zx
ENV PATH $PATH:/cmake-${cmakever}-linux-x86_64/bin

# build dependencies
ARG boostminorver=80
RUN apt-get update && apt-get install -y --no-install-recommends \
    libmysqlclient-dev \
    libexpat-dev \
    libpq-dev \
    unixodbc-dev \
    libjemalloc-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/* \
    && cd / && curl -k -L https://boostorg.jfrog.io/artifactory/main/release/1.${boostminorver}.0/source/boost_1_${boostminorver}_0.tar.gz | tar -zx \
    && cd boost_1_${boostminorver}_0 \
    && ./bootstrap.sh \
    && ./b2 install  --with-context --with-system --with-filesystem \
    && cd - && rm -rf /boost_1_${boostminorver}_0

# test suite tools
# llvm need for llvm-gcov paired to clang-16 in build env
ENV llvm=16
RUN apt-get update && apt-get install -y --no-install-recommends \
    mysql-server \
    php \
    php-mysql \
    php-curl \
    php-xml \
    php-dompdf \
    python3 \
    gnupg \
    ca-certificates \
    && curl -k -L https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-${llvm} main" >> /etc/apt/sources.list.d/llvm.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends llvm-${llvm} \
    && rm -rf /var/lib/apt/lists/* \
    && mkdir -p /var/run/mysqld && chmod a+rwX /var/run/mysqld \
    && { mysqld & } && sleep 5 \
    && mysql -e "CREATE USER test@localhost; \
    CREATE DATABASE test; GRANT ALL PRIVILEGES ON test.* TO test@localhost; \
    CREATE DATABASE test1; GRANT ALL PRIVILEGES ON test1.* TO test@localhost; \
    CREATE DATABASE test2; GRANT ALL PRIVILEGES ON test2.* TO test@localhost; \
    CREATE DATABASE test3; GRANT ALL PRIVILEGES ON test3.* TO test@localhost; \
    CREATE DATABASE test4; GRANT ALL PRIVILEGES ON test4.* TO test@localhost; \
    CREATE DATABASE test5; GRANT ALL PRIVILEGES ON test5.* TO test@localhost; \
    CREATE DATABASE test6; GRANT ALL PRIVILEGES ON test6.* TO test@localhost; \
    CREATE DATABASE test7; GRANT ALL PRIVILEGES ON test7.* TO test@localhost; \
    CREATE DATABASE test8; GRANT ALL PRIVILEGES ON test8.* TO test@localhost; \
    CREATE DATABASE test9; GRANT ALL PRIVILEGES ON test9.* TO test@localhost;" \
    && mysqladmin shutdown
ADD resource.json /opt/work/resource.json
ENV CTEST_RESOURCE /opt/work/resource.json
ENV PATH $PATH:/usr/lib/llvm-${llvm}/bin

# testing environment
RUN curl -O -k https://repo.manticoresearch.com/manticore-dev-repo.noarch.deb \
    && dpkg -i manticore-dev-repo.noarch.deb \
    && rm manticore-dev-repo.noarch.deb \
    && apt-get update && apt-get install -y --no-install-recommends \
    manticore-tzdata \
    && rm -rf /var/lib/apt/lists/* \
    && cd / && curl -k -L https://dev.mysql.com/get/Downloads/Connector-ODBC/8.3/mysql-connector-odbc-8.3.0-linux-glibc2.28-x86-64bit.tar.gz | tar -zx \
    && mkdir -p /work/aot && cd /work/aot \
    && curl -k -L https://repo.manticoresearch.com/repository/morphology/de.pak.tar.xz | tar -Jx \
    && curl -k -L https://repo.manticoresearch.com/repository/morphology/en.pak.tar.xz | tar -Jx \
    && curl -k -L https://repo.manticoresearch.com/repository/morphology/ru.pak.tar.xz | tar -Jx

ADD sphinx /root/.sphinx
ADD entry_point.sh /
VOLUME [ "/work" ]
WORKDIR /work
ENTRYPOINT ["bash", "/entry_point.sh"]
CMD []

# docker build -t registry.gitlab.com/manticoresearch/dev/ubertests_public:331 .
# docker tag registry.gitlab.com/manticoresearch/dev/ubertests_public:331 manticoresearch/ubertests_public:331
# docker push manticoresearch/ubertests_public:331