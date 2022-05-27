FROM base_build_arm:bionic

ARG boostminorver=75
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.${boostminorver}.0/source/boost_1_${boostminorver}_0.tar.gz \
    && tar -zxf boost_1_${boostminorver}_0.tar.gz && rm boost_1_${boostminorver}_0.tar.gz \
    && cd boost_1_${boostminorver}_0 \
    && ./bootstrap.sh \
    && ./b2 install  --with-context --with-system \
    && cd .. && rm -rf boost_1_${boostminorver}_0

CMD []

# docker build -t bionic_boost_arm:175 .
