#!/bin/bash
. distr.txt
docker build -t ${distr}_boost:172 boost_172
