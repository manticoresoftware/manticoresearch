#!/bin/bash
. distr.txt
docker build -t ${distr}_clang:11 clang_11
