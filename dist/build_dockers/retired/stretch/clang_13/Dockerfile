FROM stretch_cmake:320

RUN apt -y update && \
apt -y install apt-transport-https ca-certificates && \
apt install -y lsb-release wget software-properties-common && \
wget https://apt.llvm.org/llvm.sh && \
chmod +x llvm.sh && \
./llvm.sh 13 && \
rm -rf /var/lib/apt/lists/*

ENV CC clang-13
ENV CXX clang++-13

CMD []
