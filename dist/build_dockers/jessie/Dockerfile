# docker build -t registry.gitlab.com/manticoresearch/dev/jessie_cmake317 .

FROM debian:jessie

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
# wget on jessie doesn't work. Guess: ssl (for https) is too old.
RUN mkdir /boost
ADD https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz /boost
RUN cd /boost \
&& tar -zxf boost_1_72_0.tar.gz && cd boost_1_72_0 \
&& ./bootstrap.sh \
&& ./b2 --with-context \
&& ./b2 install --with-context \
&& cd / && rm -rf boost

# add cmake as separate layer
RUN wget https://github.com/Kitware/CMake/releases/download/v3.17.3/cmake-3.17.3-Linux-x86_64.tar.gz \
&& tar -zxf cmake-3.17.3-Linux-x86_64.tar.gz \
&& rm cmake-3.17.3-Linux-x86_64.tar.gz
#ADD https://github.com/Kitware/CMake/releases/download/v3.17.3/cmake-3.17.3-Linux-x86_64.tar.gz /
ENV PATH $PATH:/cmake-3.17.3-Linux-x86_64/bin

# cloud client - expires every 2 months, put at the highest level
RUN wget https://dl.min.io/client/mc/release/linux-amd64/mc \
&& mv mc /usr/local/bin/ \
&& chmod a+x /usr/local/bin/mc

ENV DISTR jessie

CMD []
