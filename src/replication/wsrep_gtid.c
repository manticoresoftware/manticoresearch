/* Copyright (C) 2013 Codership Oy <info@codersihp.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*! @file Helper functions to deal with GTID string representations */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "wsrep_api.h"

/*!
 * Read GTID from string
 * @return length of GTID string representation or -EINVAL in case of error
 */
int
wsrep_gtid_scan(const char* str, size_t str_len, wsrep_gtid_t* gtid)
{
    unsigned int offset;
    char* endptr;

    if ((offset = wsrep_uuid_scan(str, str_len, &gtid->uuid)) > 0 &&
        offset < str_len && str[offset] == ':') {
        ++offset;
        if (offset < str_len)
        {
            errno = 0;
            gtid->seqno = strtoll(str + offset, &endptr, 0);

            if (errno == 0) {
                offset = endptr - str;
                return offset;
            }
        }
    }
    *gtid = WSREP_GTID_UNDEFINED;
    return -EINVAL;
}

/*!
 * Write GTID to string
 * @return length of GTID stirng representation of -EMSGSIZE if string is too
 *         short
 */
int
wsrep_gtid_print(const wsrep_gtid_t* gtid, char* str, size_t str_len)
{
    unsigned int offset, ret;
    if ((offset = wsrep_uuid_print(&gtid->uuid, str, str_len)) > 0)
    {
        ret = snprintf(str + offset, str_len - offset,
                       ":%" PRId64, gtid->seqno);
        if (ret <= str_len - offset) {
            return (offset + ret);
        }

    }

    return -EMSGSIZE;
}
