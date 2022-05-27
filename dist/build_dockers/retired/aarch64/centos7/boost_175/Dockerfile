FROM base_build:centos7

ARG boostminorver=75
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.${boostminorver}.0/source/boost_1_${boostminorver}_0.tar.gz \
    && tar -zxf boost_1_${boostminorver}_0.tar.gz && rm boost_1_${boostminorver}_0.tar.gz \
    && cd boost_1_${boostminorver}_0 \
    && ./bootstrap.sh \
    && ./b2 install  --with-context --with-system \
    && cd .. && rm -rf boost_1_${boostminorver}_0

CMD []

# docker build -t centos7_boost:175 .
# ./b2 install --with-context --with-chrono  --with-coroutine --with-fiber --with-stacktrace --with-system --with-program_options