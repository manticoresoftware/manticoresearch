FROM centos8_cmake:320

RUN yum -y install llvm-toolset-11.0.0-1.module_el8.4.0+587+5187cac0
ENV CC clang-11
ENV CXX clang++-11

# to avoid warning: unknown warning option '-Werror=stringop-truncation'; did you mean '-Werror=string-conversion'? [-Wunknown-warning-option]
# previously it's set to "-Wno-error=stringop-truncation"
ENV CXXFLAGS ""

CMD []
