#!/bin/bash
. distr.txt
docker build -t ${distr}_cmake:323 -t registry.gitlab.com/manticoresearch/dev/${distr}_cmake:323 cmake_323
