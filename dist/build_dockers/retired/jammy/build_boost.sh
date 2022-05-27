#!/bin/bash
. distr.txt
docker build -t ${distr}_boost:178 -t registry.gitlab.com/manticoresearch/dev/${distr}_boost:178 boost_178
