FROM base_build:centos7

ARG boostminorver=72
RUN wget https://dl.bintray.com/boostorg/release/1.${boostminorver}.0/source/boost_1_${boostminorver}_0.tar.gz \
    && tar -zxf boost_1_${boostminorver}_0.tar.gz && rm boost_1_${boostminorver}_0.tar.gz \
    && cd boost_1_${boostminorver}_0 \
    && ./bootstrap.sh \
    && ./b2 install  --with-context --with-system runtime-link=static \
    && cd .. && rm -rf boost_1_${boostminorver}_0

CMD []

# docker build -t centos7_boost:172 .
