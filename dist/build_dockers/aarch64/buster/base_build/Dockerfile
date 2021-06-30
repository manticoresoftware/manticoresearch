FROM debian:buster
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y \
    libmariadb-dev-compat \
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

ENV DISTR buster
ENV CXXFLAGS "-Wno-error=stringop-truncation"
CMD []

# docker build -t base_build:buster .