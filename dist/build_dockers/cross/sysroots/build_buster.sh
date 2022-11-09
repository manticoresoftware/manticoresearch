#!/bin/bash

# this is root script which should be run to do everything
# build for debian buster (10), released 2019

distr=buster
image=debian:$distr
. debian.sh
