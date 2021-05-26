# Copyright (c) 2018, Codership Oy. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

CMAKE_MINIMUM_REQUIRED(VERSION 3.17)

ADD_LIBRARY(wsrep STATIC wsrep_gtid.c wsrep_uuid.c wsrep_loader.c wsrep_dummy.c)
target_compile_definitions(wsrep PUBLIC HAVE_WSREP)

# fool protect: galera should be built with the same wsrep version.
# It that is not true, here on linking with galera_wsrep we'll have config-time error
FILE (STRINGS "wsrep_api.h" _wsrep LIMIT_COUNT 1 REGEX "WSREP_INTERFACE_VERSION")
STRING (REGEX MATCH "[0-9]+" WSREP_API "${_wsrep}")
set_target_properties (wsrep PROPERTIES WSREP_API_VERSION ${WSREP_API})
target_link_libraries (wsrep PRIVATE galera::galera_wsrep)

add_library ( manticore::wsrep ALIAS wsrep )

