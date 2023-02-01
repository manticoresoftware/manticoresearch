FROM ubuntu:jammy

RUN mkdir /tmp/nsis_build && apt-get update && apt-get install -y curl git nsis

COPY ["nsis_src_parser.sh", "nsisscript.nsi", "/"]
COPY ["nsisscript.nsi", "/tmp/nsis_build"]
COPY ["nsisunz.dll", "/usr/share/nsis/Plugins/x86-unicode/"]

