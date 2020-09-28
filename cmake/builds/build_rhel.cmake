# ---------- rhel ----------
# Above line is mandatory!
# rules to build rpm package for Red Hat/ Centos

message ( STATUS "Will create RPM for generic RedHat/Centos" )
set ( WITH_SYSTEMD ON )
include ( builds/CommonRpm )
LIST ( APPEND PKGSUFFIXES "rhel" )