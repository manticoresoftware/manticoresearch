FROM centos7_cmake:320

RUN yum install -y \
        centos-release-scl-rh \
        llvm-toolset-7.0-clang \
&& scl enable llvm-toolset-7.0 bash \
&& yum clean all \
&& rm -rf /var/cache/yum

ENV PATH /opt/rh/llvm-toolset-7.0/root/usr/bin:$PATH
ENV LD_LIBRARY_PATH /opt/rh/llvm-toolset-7.0/root/usr/lib64/:$LD_LIBRARY_PATH

ENV CC=clang-7
ENV CXX=clang++

CMD []
