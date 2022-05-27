#!/bin/bash

for distr in bionic buster centos7 centos8 focal stretch xenial; do
    cd $distr
    # . build_base.sh
    . build.sh
    docker push registry.gitlab.com/manticoresearch/dev/${distr}_cmake:320
    docker push registry.gitlab.com/manticoresearch/dev/${distr}_cmake:latest
    cd ..
done