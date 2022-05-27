FROM registry.gitlab.com/manticoresearch/dev/ubuntu_debian:any

RUN apt-get update && apt-get install -y \
    rpm \
    elfutils \
&& rm -rf /var/lib/apt/lists/*

# docker build -t centos_rhel:any -t registry.gitlab.com/manticoresearch/dev/centos_rhel:any .
