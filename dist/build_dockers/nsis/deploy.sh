#!/bin/sh

docker build --no-cache -t registry.gitlab.com/manticoresearch/dev/build_nsis:0.1.3 .
docker push registry.gitlab.com/manticoresearch/dev/build_nsis:0.1.3
