# ---------- macosbrew ----------

message ( STATUS "Installing for MacOS via Brew" )
set ( SPLIT_SYMBOLS 1 )

# configure specific stuff
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64" )
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64" )

# generate config files

file ( READ "manticore.conf.in" _MINCONF )
string ( REPLACE "@CONFDIR@/log/searchd.pid" "@RUNDIR@/searchd.pid" _MINCONF "${_MINCONF}" )
string ( REPLACE "@CONFDIR@/log" "@LOGDIR@" _MINCONF "${_MINCONF}" )
file ( WRITE "${MANTICORE_BINARY_DIR}/manticore.conf.in" "${_MINCONF}" )
unset ( _MINCONF )
set ( CONFDIR "${CMAKE_INSTALL_LOCALSTATEDIR}/manticore" )
set ( RUNDIR "${CMAKE_INSTALL_LOCALSTATEDIR}/run/manticore" )
set ( LOGDIR "${CMAKE_INSTALL_LOCALSTATEDIR}/log/manticore" )
configure_file ( "manticore.conf.in" "${MANTICORE_BINARY_DIR}/manticore.conf" @ONLY )

# install specific stuff
configure_file ( "${MANTICORE_BINARY_DIR}/manticore.conf.in" "${MANTICORE_BINARY_DIR}/manticore.conf.dist" @ONLY )
INSTALL ( FILES ${MANTICORE_BINARY_DIR}/manticore.conf.dist
                DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/manticore COMPONENT doc RENAME manticore.conf )
install ( DIRECTORY misc/stopwords DESTINATION ${CMAKE_INSTALL_DATADIR}/${PACKAGE_NAME} COMPONENT doc)
install ( FILES ${ICU_SRC}/source/data/in/icudt65l.dat DESTINATION ${CMAKE_INSTALL_DATADIR}/${PACKAGE_NAME}/icu COMPONENT doc)

# data and log dirs are created by brew formula

