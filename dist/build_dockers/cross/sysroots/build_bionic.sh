#!/bin/bash

# this is root script which should be run to do everything
# build for ubuntu bionic (18.04 LTS)

distr=bionic
image=ubuntu:$distr
. debian.sh
