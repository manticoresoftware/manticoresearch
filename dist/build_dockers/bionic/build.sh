#!/bin/bash
. distr.txt
# uncomment for first use, or better run build_base.sh manually
# . build_base.sh
. build_boost.sh
. build_cmake.sh
. build_clang.sh
docker tag ${distr}_cmake:320 registry.gitlab.com/manticoresearch/dev/${distr}_cmake:320
docker tag ${distr}_cmake:320 registry.gitlab.com/manticoresearch/dev/${distr}_cmake:latest
docker tag ${distr}_clang:13 registry.gitlab.com/manticoresearch/dev/${distr}_clang:13
echo "docker push registry.gitlab.com/manticoresearch/dev/${distr}_cmake:320"
echo "docker push registry.gitlab.com/manticoresearch/dev/${distr}_cmake:latest"
echo "docker push registry.gitlab.com/manticoresearch/dev/${distr}_clang:13"
