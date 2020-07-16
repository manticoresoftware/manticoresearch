#!/bin/bash
. distr.txt
docker build -t ${distr}_cmake:317 cmake_317
