cmake_minimum_required ( VERSION 3.20 )

ADD_LIBRARY ( repli_stuff STATIC wsrep_cxx.cpp wsrep_cxx.h
		api_command_cluster.h api_command_cluster.cpp
		cluster_delete.h cluster_delete.cpp
		cluster_file_reserve.h cluster_file_reserve.cpp
		cluster_file_send.h cluster_file_send.cpp
		cluster_get_nodes.h cluster_get_nodes.cpp
		cluster_index_add_local.h cluster_index_add_local.cpp
		cluster_synced.h cluster_synced.cpp
		cluster_update_nodes.h cluster_update_nodes.cpp
		grastate.h grastate.cpp
		commit_monitor.h commit_monitor.cpp
		common.h
		configuration.cpp
		nodes.h nodes.cpp
		portrange.h portrange.cpp
		receiver_ctx.h receiver_ctx.cpp
		replicate_index.h replicate_index.cpp
		serialize.h serialize.cpp
		send_files.h send_files.cpp
		cluster_commands.h
		recv_state.h recv_state.cpp
)

target_include_directories ( repli_stuff PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( repli_stuff PRIVATE lextra )

# fool protect: galera should be built with the same wsrep version.
# It that is not true, here on linking with galera_wsrep we'll have config-time error
FILE ( STRINGS "wsrep_cxx.cpp" _wsrep LIMIT_COUNT 1 REGEX "INTERFACE_VERSION" )
STRING ( REGEX MATCH "[0-9]+" INTERFACE_VERSION "${_wsrep}" )
set_target_properties ( repli_stuff PROPERTIES WSREP_API_VERSION ${INTERFACE_VERSION} )
#target_link_libraries ( repli_stuff PRIVATE galera::galera_wsrep )