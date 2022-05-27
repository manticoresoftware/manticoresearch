FROM debian:stretch
ENV DEBIAN_FRONTEND noninteractive

# note, no boost, no icu here, since we build them from sources
RUN apt-get update && apt-get install -y \
    default-libmysqlclient-dev \
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
&& rm -rf /var/lib/apt/lists/*

ENV DISTR stretch
CMD []

# docker build -t base_build:stretch .