#!/bin/bash
. distr.txt
docker build -t ${distr}_clang:7 clang_7
