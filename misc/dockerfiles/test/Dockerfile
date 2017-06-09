FROM ubuntu:16.04

WORKDIR /tmp
RUN apt-get update && apt-get install -y \
    autoconf automake libtool g++ \
    build-essential \
    git \
    libexpat1-dev \
    libmysqlclient-dev \
    mysql-client \
    php php-curl php-mysql php-xml \
    python \
    zip && \
    apt-get clean

RUN mkdir -p /search
RUN mkdir -p /search_src
RUN mkdir -p /aot
