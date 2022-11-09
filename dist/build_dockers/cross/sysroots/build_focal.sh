#!/bin/bash

# this is root script which should be run to do everything
# build for ubuntu focal (20.04 LTS)

distr=focal
image=ubuntu:$distr
. debian.sh
