#!/bin/bash

# this is root script which should be run to do everything
# build for debian bullseye (11), released 2021

distr=bullseye
image=debian:$distr
. debian.sh
