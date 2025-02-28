FROM ubuntu:jammy
ENV DEBIAN_FRONTEND=noninteractive DISTR=jammy arch=x86_64

# toolchain stuff
ARG cmakever="3.31.2"
RUN apt-get update && apt-get install -y --no-install-recommends  \
        curl \
        git \
        build-essential \
        ninja-build \
        flex \
        bison \
    && rm -rf /var/lib/apt/lists/* \
    && git config --global --add safe.directory '*' \
    && cd / && curl -k -L https://github.com/Kitware/CMake/releases/download/v${cmakever}/cmake-${cmakever}-linux-${arch}.tar.gz | tar -zx
ENV PATH=$PATH:/cmake-${cmakever}-linux-${arch}/bin CTEST_CMAKE_GENERATOR=Ninja CMAKE_GENERATOR=Ninja

# build dependencies
ARG boostminorver=80
RUN apt-get update && apt-get install -y --no-install-recommends \
        libmysqlclient-dev \
        libexpat-dev \
        libpq-dev \
        unixodbc-dev \
        libjemalloc-dev \
    && rm -rf /var/lib/apt/lists/* \
    && cd / && curl -k -L https://archives.boost.io/release/1.${boostminorver}.0/source/boost_1_${boostminorver}_0.tar.gz | tar -zx \
    && cd boost_1_${boostminorver}_0 \
    && ./bootstrap.sh \
    && ./b2 install  --with-context --with-system --with-filesystem \
    && cd - && rm -rf /boost_1_${boostminorver}_0

# llvm
ENV llvm=16
RUN apt-get update && apt-get install -y --no-install-recommends \
        gnupg \
        ca-certificates \
    && curl -k -L https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-${llvm} main" >> /etc/apt/sources.list.d/llvm.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends  \
        clang-$llvm \
    && rm -rf /var/lib/apt/lists/*
ENV PATH=$PATH:/usr/lib/llvm-${llvm}/bin CC=/usr/bin/clang-${llvm} CXX=/usr/bin/clang++-${llvm}

# vcpkg
ENV VCPKG_ROOT=/vcpkg VCPKG_DISABLE_METRICS=1 VCPKG_FORCE_SYSTEM_BINARIES=1 PATH=/vcpkg:$PATH
RUN apt-get update && apt-get install -y --no-install-recommends \
        curl \
        zip \
        unzip \
        tar \
        pkg-config \
    && rm -rf /var/lib/apt/lists/* \
    && cd / && git clone https://github.com/microsoft/vcpkg.git \
    && cd vcpkg \
    && ./bootstrap-vcpkg.sh \
    && vcpkg install vcpkg-cmake vcpkg-cmake-config vcpkg-cmake-get-vars

# staff for test suite
RUN apt-get update && apt-get install -y --no-install-recommends \
        mysql-server \
        php \
        php-mysql \
        php-curl \
        php-xml \
        python3 \
    && rm -rf /var/lib/apt/lists/* \
    && mkdir -p /var/run/mysqld \
    && { mysqld & } && sleep 5 \
    && mysql -e "CREATE USER test@localhost; CREATE DATABASE test; GRANT ALL PRIVILEGES ON test.* TO test@localhost;" \
    && mysqladmin shutdown

# latest valgrind via linuxbrew
RUN NONINTERACTIVE=1 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)" \
    && eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv)" \
    && brew install valgrind \
    && ln -s /home/linuxbrew/.linuxbrew/bin/valgrind /usr/bin/ \
    && ln -s /home/linuxbrew/.linuxbrew/include/valgrind /usr/include/ \
    && apt-get update && apt-get install -y --no-install-recommends \
        libc6-dbg \
    && rm -rf /var/lib/apt/lists/*

# testing environment
RUN curl -O -k https://repo.manticoresearch.com/manticore-dev-repo.noarch.deb \
    && dpkg -i manticore-dev-repo.noarch.deb \
    && rm manticore-dev-repo.noarch.deb \
    && apt-get update && apt-get install -y --no-install-recommends \
        manticore-tzdata \
    && rm -rf /var/lib/apt/lists/* \
    && cd / && curl -k -L https://dev.mysql.com/get/Downloads/Connector-ODBC/8.3/mysql-connector-odbc-8.3.0-linux-glibc2.28-x86-64bit.tar.gz | tar -zx \
    && mkdir -p /aot && cd /aot \
    && curl -k -L https://repo.manticoresearch.com/repository/morphology/de.pak.tar.xz | tar -Jx \
    && curl -k -L https://repo.manticoresearch.com/repository/morphology/en.pak.tar.xz | tar -Jx \
    && curl -k -L https://repo.manticoresearch.com/repository/morphology/ru.pak.tar.xz | tar -Jx

ADD --link sphinx /root/.sphinx
ADD --link entry_point.sh /
VOLUME [ "/work" ]
WORKDIR /work
ENTRYPOINT ["/bin/bash", "/entry_point.sh"]
CMD []

# docker build -t manticore_valgrind:jammy .
# docker image tag manticore_valgrind:jammy manticoresearch/manticore_valgrind:jammy
# docker push manticoresearch/manticore_valgrind:jammy
