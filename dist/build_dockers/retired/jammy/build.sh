#!/bin/bash
. distr.txt
# uncomment for first use, or better run build_base.sh manually
# . build_base.sh
. build_clang.sh
. build_boost.sh
. build_cmake.sh
docker tag ${distr}_cmake:323 registry.gitlab.com/manticoresearch/dev/${distr}_cmake:latest

echo "docker push registry.gitlab.com/manticoresearch/dev/base_build:${distr}"
echo "docker push registry.gitlab.com/manticoresearch/dev/${distr}_clang:13"
echo "docker push registry.gitlab.com/manticoresearch/dev/${distr}_boost:178"
echo "docker push registry.gitlab.com/manticoresearch/dev/${distr}_cmake:323"
echo "docker push registry.gitlab.com/manticoresearch/dev/${distr}_cmake:latest"

