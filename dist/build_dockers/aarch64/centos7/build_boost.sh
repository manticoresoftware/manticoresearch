#!/bin/bash
. distr.txt
docker build -t ${distr}_boost:175 boost_175
