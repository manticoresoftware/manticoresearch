#!/bin/bash

for distr in bionic buster centos7 centos8 focal stretch xenial; do
    cd $distr
    # . build_base.sh
    . build.sh
    docker push registry.gitlab.com/manticoresearch/dev/${distr}_cmake:320a
#    docker manifest create registry.gitlab.com/manticoresearch/dev/${distr}_cmake:320 registry.gitlab.com/manticoresearch/dev/${distr}_cmake:320 registry.gitlab.com/manticoresearch/dev/${distr}_cmake:320a
#    docker manifest push registry.gitlab.com/manticoresearch/dev/${distr}_cmake:320
#    docker pull registry.gitlab.com/manticoresearch/dev/${distr}_cmake:320
#    docker rmi registry.gitlab.com/manticoresearch/dev/${distr}_cmake:320a
    cd ..
done

# commented out lines implies that image for x64 is created and pushed as tag 320. They create multi-arch images,
# so that final image:320 will pull amd64 or arm64 depending on platform where it is called