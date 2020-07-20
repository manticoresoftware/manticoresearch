#!/bin/bash

for distr in bionic buster centos6 centos7 centos8 focal jessie stretch trusty xenial; do
    cd $distr
    # . build_base.sh
    . build.sh
    docker push registry.gitlab.com/manticoresearch/dev/${distr}_cmake:317
    docker push registry.gitlab.com/manticoresearch/dev/${distr}_cmake:latest
    cd ..
done