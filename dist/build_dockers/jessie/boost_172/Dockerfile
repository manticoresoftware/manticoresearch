FROM base_build:jessie

ARG boostminorver=72
# unfortunately wget doesn't work properly in jessie, so it will be only 'fat' build with layer of boost archive left
ADD https://boostorg.jfrog.io/artifactory/main/release/1.${boostminorver}.0/source/boost_1_${boostminorver}_0.tar.gz /
RUN tar -zxf boost_1_${boostminorver}_0.tar.gz \
        && cd boost_1_${boostminorver}_0 \
        && ./bootstrap.sh \
        && ./b2 install  --with-context --with-system runtime-link=static \
        && cd .. && rm -rf boost_1_${boostminorver}_0

CMD []

# docker build -t jessie_boost:172 .
