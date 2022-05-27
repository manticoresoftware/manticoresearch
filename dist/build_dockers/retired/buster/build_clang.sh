#!/bin/bash
. distr.txt
docker build -t ${distr}_clang:13 clang_13
