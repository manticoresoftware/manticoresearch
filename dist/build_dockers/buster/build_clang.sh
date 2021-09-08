#!/bin/bash
. distr.txt
docker build -t ${distr}_clang:12 clang_12
