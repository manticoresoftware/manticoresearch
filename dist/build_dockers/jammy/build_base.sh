#!/bin/bash
. distr.txt
docker build -t base_build:${distr} -t registry.gitlab.com/manticoresearch/dev/base_build:${distr} base_build
