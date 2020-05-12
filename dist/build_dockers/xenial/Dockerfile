# docker build -t registry.gitlab.com/manticoresearch/dev/xenial_cmake314 .

FROM ubuntu:xenial

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y \
    libmysqlclient-dev \
    libexpat-dev \
    libpq-dev \
    unixodbc-dev \
    libicu-dev \
    flex \
    bison \
    git \
    build-essential \
    libssl-dev \
    libboost-system-dev \
    libboost-program-options-dev \
&& rm -rf /var/lib/apt/lists/*

# add cmake as separate layer
# file taken from https://github.com/Kitware/CMake/releases/download/v3.14.5/cmake-3.14.5-Linux-x86_64.tar.gz
ADD cmake-3.14.5-Linux-x86_64.tar.gz /
ENV PATH $PATH:/cmake-3.14.5-Linux-x86_64/bin
ENV DISTR xenial

CMD []
