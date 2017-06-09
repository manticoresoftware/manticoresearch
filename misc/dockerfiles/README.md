# Introduction

This is a direcectory with dockerized smoke tests of search engine.
It depends only on docker and docker-compose.

### Docker and composer installation

docker installation
~~~
$ sudo apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
$ sudo add-apt-repository "deb https://apt.dockerproject.org/repo/  ubuntu-$(lsb_release -cs)  main"
$ sudo apt-get update
$ sudo apt-get install docker-engine
~~~
docker-compose installation
~~~
$ sudo curl -o /usr/local/bin/docker-compose -L "https://github.com/docker/compose/releases/download/1.11.2/docker-compose-$(uname -s)-$(uname -m)"
$ sudo chmod +x /usr/local/bin/docker-compose
~~~
Add user to docker group to be able to run docker commands
~~~
$ sudo usermod -aG docker ${USER}
~~~
Check of applications
~~~
$ docker version
$ docker-compose version
~~~
# Next commands must be run at the root directory of search project

# Build
~~~
$ docker-compose -f docker-compose-test.yml build
~~~
build should be done only once at source clone or checkout at the box or in case Dockerfile files got changed. That creates docker images.

# Tests
These starts docker docntainers and issue tests inside containers. Multiple containers might be started at same time.

Regular test for id64 build
~~~
$ docker-compose -f docker-compose-test.yml up test
~~~
Tests for id32 build
~~~
$ docker-compose -f docker-compose-test.yml up test32
~~~
Tests of id64 RT pass
~~~
$ docker-compose -f docker-compose-test.yml up test_rt
~~~
To shutdown test containers
~~~
$ docker-compose -f docker-compose-test.yml down
~~~