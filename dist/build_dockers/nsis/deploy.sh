#!/bin/sh

docker build --no-cache -t registry.gitlab.com/manticoresearch/dev/build_nsis:0.2.0 .
docker push registry.gitlab.com/manticoresearch/dev/build_nsis:0.2.0
