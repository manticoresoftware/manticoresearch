#!/bin/bash
. distr.txt
docker build -t build_base:${distr} build_base
