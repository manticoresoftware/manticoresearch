#!/bin/bash

# this is root script which should be run to do everything
# build for debian bookworm (12), released ????

distr=bookworm
image=debian:$distr
. debian.sh
