FROM build_base:buster

ADD https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz /
RUN tar -zxf boost_1_72_0.tar.gz && rm boost_1_72_0.tar.gz \
&& cd boost_1_72_0 \
&& ./bootstrap.sh \
&& ./b2 install  --with-context --with-system runtime-link=static \
&& cd / && rm -rf boost_1_72_0

CMD []

# docker build -t buster_boost:172 .
