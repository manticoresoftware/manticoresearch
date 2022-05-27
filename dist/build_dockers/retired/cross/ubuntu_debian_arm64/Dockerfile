FROM registry.gitlab.com/manticoresearch/dev/build_cross:clang13_cmake323

# Topmost layer. Put actual sysroots for arm64 and activate entrypoint
COPY *_aarch64.tar.xz /sysroot/
ENV arch aarch64

ENTRYPOINT ["bash", "/sysroot/root/entry_point.sh"]
CMD []

# docker build -t ubuntu_debian:arm64 -t registry.gitlab.com/manticoresearch/dev/ubuntu_debian:arm64 .
