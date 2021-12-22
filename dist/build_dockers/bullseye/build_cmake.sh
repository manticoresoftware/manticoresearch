#!/bin/bash
. distr.txt
docker build -t ${distr}_cmake:320 cmake_320
