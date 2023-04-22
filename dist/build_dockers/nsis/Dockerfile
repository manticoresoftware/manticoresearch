FROM ubuntu:jammy

RUN mkdir /tmp/nsis_build && apt-get update && apt-get install -y curl git nsis

COPY ["nsisunz.dll", "/usr/share/nsis/Plugins/x86-unicode/"]


# docker buildx create --name manticore_build --platform linux/amd64,linux/arm64
# docker buildx use manticore_build
# docker buildx build --platform linux/amd64,linux/arm64 --push -t manticoresearch/build_nsis:1.0.0 .
