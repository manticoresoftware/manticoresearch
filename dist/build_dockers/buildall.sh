#!/bin/bash

for distr in bionic; do
    cd $distr
    # . build_base.sh
    . build.sh
    cd ..
done