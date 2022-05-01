FROM ubuntu:jammy
ENV DEBIAN_FRONTEND noninteractive

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
    file \
&& rm -rf /var/lib/apt/lists/*

RUN cd / && wget https://dl.min.io/client/mc/release/linux-amd64/mc \
    && chmod +x mc && mv mc /usr/local/bin/

ENV DISTR jammy
ENV CXXFLAGS "-Wno-error=deprecated-copy"
CMD []

# docker build -t base_build:jammy .
