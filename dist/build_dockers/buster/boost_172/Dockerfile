FROM base_build:buster

ARG boostminorver=72
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.${boostminorver}.0/source/boost_1_${boostminorver}_0.tar.gz \
    && tar -zxf boost_1_${boostminorver}_0.tar.gz && rm boost_1_${boostminorver}_0.tar.gz \
    && cd boost_1_${boostminorver}_0 \
    && ./bootstrap.sh \
    && ./b2 install  --with-context --with-system runtime-link=static \
    && cd .. && rm -rf boost_1_${boostminorver}_0

CMD []

# docker build -t buster_boost:172 .
