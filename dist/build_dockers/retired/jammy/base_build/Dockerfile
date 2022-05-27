FROM ubuntu:jammy
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y \
    file \
    flex \
    bison \
    git \
    build-essential \
    wget \
    ninja-build \
    libmysqlclient-dev \
    libexpat-dev \
    libpq-dev \
    unixodbc-dev \
    libjemalloc-dev \
    libssl-dev \
    libzstd-dev \
&& rm -rf /var/lib/apt/lists/*

ENV DISTR jammy
ENV CXXFLAGS "-Wno-error=deprecated-copy"
CMD []

# docker build -t base_build:jammy -t registry.gitlab.com/manticoresearch/dev/base_build:jammy .

