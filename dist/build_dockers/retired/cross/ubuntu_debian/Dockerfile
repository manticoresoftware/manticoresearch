FROM registry.gitlab.com/manticoresearch/dev/build_cross:clang13_cmake323

# Topmost layer. Fetch actual sysroot for any arch, and activate entrypoint
ADD entry_point.sh /sysroot/root/
ENTRYPOINT ["bash", "/sysroot/root/entry_point.sh"]
CMD []

# docker build -t ubuntu_debian:any -t registry.gitlab.com/manticoresearch/dev/ubuntu_debian:any .
