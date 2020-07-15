FROM ubuntu:trusty
ENV DEBIAN_FRONTEND noninteractive

# note, no boost, no icu here, since we build them from sources
RUN apt-get update && apt-get install -y \
    libmysqlclient-dev \
    libexpat-dev \
    libpq-dev \
    unixodbc-dev \
    flex \
    bison \
    git \
    build-essential \
    libssl-dev \
    wget \
&& rm -rf /var/lib/apt/lists/*

# things that sometimes expire - on separate layer
RUN apt-get update && apt-get install -y \
    ca-certificates \
&& rm -rf /var/lib/apt/lists/*

# boost on separate layer
ADD https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz /
RUN tar -zxf boost_1_72_0.tar.gz && rm boost_1_72_0.tar.gz \
&& cd boost_1_72_0 \
&& ./bootstrap.sh \
&& ./b2 --with-context \
&& ./b2 install --with-context \
&& cd / && rm -rf boost_1_72_0

# add cmake as separate layer
ADD https://github.com/Kitware/CMake/releases/download/v3.17.3/cmake-3.17.3-Linux-x86_64.tar.gz /
RUN tar -zxf cmake-3.17.3-Linux-x86_64.tar.gz && rm cmake-3.17.3-Linux-x86_64.tar.gz
#ADD https://github.com/Kitware/CMake/releases/download/v3.17.3/cmake-3.17.3-Linux-x86_64.tar.gz /
ENV PATH $PATH:/cmake-3.17.3-Linux-x86_64/bin

# cloud client - expires every 2 months, put at the highest level
ADD https://dl.min.io/client/mc/release/linux-amd64/mc /
RUN chmod +x mc && mv mc /usr/local/bin/

ENV DISTR trusty
CMD []

# docker build -t registry.gitlab.com/manticoresearch/dev/trusty_cmake317 .