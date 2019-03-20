/* Copyright (C) 2009-2010 Codership Oy <info@codersihp.com>

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

/*! @file Dummy wsrep API implementation. */

#include "wsrep_api.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>

/*! Dummy backend context. */
typedef struct wsrep_dummy
{
    wsrep_log_cb_t log_fn;
    char* options;
} wsrep_dummy_t;

/* Get pointer to wsrep_dummy context from wsrep_t pointer */
#define WSREP_DUMMY(_p) ((wsrep_dummy_t *) (_p)->ctx)

/* Trace function usage a-la DBUG */
#define WSREP_DBUG_ENTER(_w) do {                                       \
        if (WSREP_DUMMY(_w)) {                                          \
            if (WSREP_DUMMY(_w)->log_fn)                                \
                WSREP_DUMMY(_w)->log_fn(WSREP_LOG_DEBUG, __FUNCTION__); \
        }                                                               \
    } while (0)


static void dummy_free(wsrep_t *w)
{
    WSREP_DBUG_ENTER(w);
    if (WSREP_DUMMY(w)->options) {
        free(WSREP_DUMMY(w)->options);
        WSREP_DUMMY(w)->options = NULL;
    }
    free(w->ctx);
    w->ctx = NULL;
}

static wsrep_status_t dummy_init (wsrep_t* w,
                                  const struct wsrep_init_args* args)
{
    WSREP_DUMMY(w)->log_fn = args->logger_cb;
    WSREP_DBUG_ENTER(w);
    if (args->options) {
        WSREP_DUMMY(w)->options = strdup(args->options);
    }
    return WSREP_OK;
}

static uint64_t dummy_capabilities (wsrep_t* w __attribute__((unused)))
{
    return 0;
}

static wsrep_status_t dummy_options_set(
    wsrep_t* w,
    const char* conf)
{
    WSREP_DBUG_ENTER(w);
    if (WSREP_DUMMY(w)->options) {
        free(WSREP_DUMMY(w)->options);
        WSREP_DUMMY(w)->options = NULL;
    }
    if (conf) {
        WSREP_DUMMY(w)->options = strdup(conf);
    }
    return WSREP_OK;
}

static char* dummy_options_get (wsrep_t* w)
{
    WSREP_DBUG_ENTER(w);
    if (WSREP_DUMMY(w)->options)
        return strdup(WSREP_DUMMY(w)->options);
    return NULL;
}

static wsrep_status_t dummy_connect(
    wsrep_t* w,
    const char*  name      __attribute__((unused)),
    const char*  url       __attribute__((unused)),
    const char*  donor     __attribute__((unused)),
    wsrep_bool_t bootstrap __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_disconnect(wsrep_t* w)
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_recv(wsrep_t* w,
                                 void*    recv_ctx __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_replicate(
    wsrep_t* w,
    const wsrep_conn_id_t   conn_id    __attribute__((unused)),
    wsrep_ws_handle_t*      ws_handle  __attribute__((unused)),
    uint32_t                flags      __attribute__((unused)),
    wsrep_trx_meta_t*       meta       __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_pre_commit(
    wsrep_t* w,
    const wsrep_conn_id_t   conn_id    __attribute__((unused)),
    wsrep_ws_handle_t*      ws_handle  __attribute__((unused)),
    uint32_t                flags      __attribute__((unused)),
    wsrep_trx_meta_t*       meta       __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_replicate_pre_commit(
    wsrep_t* w,
    const wsrep_conn_id_t   conn_id    __attribute__((unused)),
    wsrep_ws_handle_t*      ws_handle  __attribute__((unused)),
    uint32_t                flags      __attribute__((unused)),
    wsrep_trx_meta_t*       meta       __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_interim_commit(
    wsrep_t* w,
    wsrep_ws_handle_t*  ws_handle  __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_post_commit(
    wsrep_t* w,
    wsrep_ws_handle_t*  ws_handle  __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_applier_pre_commit(
    wsrep_t* w,
    void*    trx_handle    __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_applier_interim_commit(
    wsrep_t* w,
    void*    trx_handle    __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_applier_post_commit(
    wsrep_t* w,
    void*    trx_handle    __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_post_rollback(
    wsrep_t* w,
    wsrep_ws_handle_t*  ws_handle  __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_replay_trx(
    wsrep_t* w,
    wsrep_ws_handle_t*  ws_handle  __attribute__((unused)),
    void*               trx_ctx    __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_abort_pre_commit(
    wsrep_t* w,
    const wsrep_seqno_t  bf_seqno __attribute__((unused)),
    const wsrep_trx_id_t trx_id   __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_append_key(
    wsrep_t* w,
    wsrep_ws_handle_t*     ws_handle  __attribute__((unused)),
    const wsrep_key_t*     key        __attribute__((unused)),
    const size_t           key_num    __attribute__((unused)),
    const wsrep_key_type_t key_type   __attribute__((unused)),
    const bool             copy       __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_append_data(
    wsrep_t* w,
    wsrep_ws_handle_t*      ws_handle  __attribute__((unused)),
    const struct wsrep_buf* data       __attribute__((unused)),
    const size_t            count      __attribute__((unused)),
    const wsrep_data_type_t type       __attribute__((unused)),
    const bool              copy       __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_causal_read(
    wsrep_t* w,
    wsrep_gtid_t* gtid __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_free_connection(
    wsrep_t* w,
    const wsrep_conn_id_t  conn_id   __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_to_execute_start(
    wsrep_t* w,
    const wsrep_conn_id_t   conn_id __attribute__((unused)),
    const wsrep_key_t*      key     __attribute__((unused)),
    const size_t            key_num __attribute__((unused)),
    const struct wsrep_buf* data    __attribute__((unused)),
    const size_t            count   __attribute__((unused)),
    wsrep_trx_meta_t*       meta    __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_to_execute_end(
    wsrep_t* w,
    const wsrep_conn_id_t  conn_id   __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_preordered_collect(
    wsrep_t*                 w,
    wsrep_po_handle_t*       handle    __attribute__((unused)),
    const struct wsrep_buf*  data      __attribute__((unused)),
    size_t                   count     __attribute__((unused)),
    wsrep_bool_t             copy      __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_preordered_commit(
    wsrep_t*                 w,
    wsrep_po_handle_t*       handle    __attribute__((unused)),
    const wsrep_uuid_t*      source_id __attribute__((unused)),
    uint32_t                 flags     __attribute__((unused)),
    int                      pa_range  __attribute__((unused)),
    wsrep_bool_t             commit    __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_sst_sent(
    wsrep_t* w,
    const wsrep_gtid_t* state_id  __attribute__((unused)),
    const int           rcode     __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_sst_received(
    wsrep_t* w,
    const wsrep_gtid_t* state_id  __attribute__((unused)),
    const void*         state     __attribute__((unused)),
    const size_t        state_len __attribute__((unused)),
    const int           rcode     __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_snapshot(
    wsrep_t* w,
    const void*  msg        __attribute__((unused)),
    const size_t msg_len    __attribute__((unused)),
    const char*  donor_spec __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static struct wsrep_stats_var dummy_stats[] = {
    { NULL, WSREP_VAR_STRING, { 0 } }
};

static struct wsrep_stats_var* dummy_stats_get (wsrep_t* w)
{
    WSREP_DBUG_ENTER(w);
    return dummy_stats;
}

static void dummy_stats_free (
    wsrep_t* w,
    struct wsrep_stats_var* stats __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
}

static void dummy_stats_reset (wsrep_t* w)
{
    WSREP_DBUG_ENTER(w);
}

static void dummy_fetch_pfs_info (
    wsrep_t* w, wsrep_node_info_t* nodes __attribute__((unused)),
    uint32_t size __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
}

static wsrep_seqno_t dummy_pause (wsrep_t* w)
{
    WSREP_DBUG_ENTER(w);
    return -ENOSYS;
}

static wsrep_status_t dummy_resume (wsrep_t* w)
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_desync (wsrep_t* w)
{
    WSREP_DBUG_ENTER(w);
    return WSREP_NOT_IMPLEMENTED;
}

static wsrep_status_t dummy_resync (wsrep_t* w)
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static wsrep_status_t dummy_lock (wsrep_t* w,
                                  const char* s __attribute__((unused)),
                                  bool        r __attribute__((unused)),
                                  uint64_t    o __attribute__((unused)),
                                  int64_t     t __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_NOT_IMPLEMENTED;
}

static wsrep_status_t dummy_unlock (wsrep_t* w,
                                    const char* s __attribute__((unused)),
                                    uint64_t    o __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return WSREP_OK;
}

static bool dummy_is_locked (wsrep_t* w,
                             const char*   s __attribute__((unused)),
                             uint64_t*     o __attribute__((unused)),
                             wsrep_uuid_t* t __attribute__((unused)))
{
    WSREP_DBUG_ENTER(w);
    return false;
}

static wsrep_t dummy_iface = {
    WSREP_INTERFACE_VERSION,
    &dummy_init,
    &dummy_capabilities,
    &dummy_options_set,
    &dummy_options_get,
    &dummy_connect,
    &dummy_disconnect,
    &dummy_recv,
    &dummy_replicate,
    &dummy_pre_commit,
    &dummy_replicate_pre_commit,
    &dummy_interim_commit,
    &dummy_post_commit,
    &dummy_applier_pre_commit,
    &dummy_applier_interim_commit,
    &dummy_applier_post_commit,
    &dummy_post_rollback,
    &dummy_replay_trx,
    &dummy_abort_pre_commit,
    &dummy_append_key,
    &dummy_append_data,
    &dummy_causal_read,
    &dummy_free_connection,
    &dummy_to_execute_start,
    &dummy_to_execute_end,
    &dummy_preordered_collect,
    &dummy_preordered_commit,
    &dummy_sst_sent,
    &dummy_sst_received,
    &dummy_snapshot,
    &dummy_stats_get,
    &dummy_stats_free,
    &dummy_stats_reset,
    &dummy_fetch_pfs_info,
    &dummy_pause,
    &dummy_resume,
    &dummy_desync,
    &dummy_resync,
    &dummy_lock,
    &dummy_unlock,
    &dummy_is_locked,
    WSREP_NONE,
    WSREP_INTERFACE_VERSION,
    "Codership Oy <info@codership.com>",
    &dummy_free,
    NULL,
    NULL
};

int wsrep_dummy_loader(wsrep_t* w)
{
    if (!w)
        return EINVAL;

    *w = dummy_iface;

    // allocate private context
    if (!(w->ctx = malloc(sizeof(wsrep_dummy_t))))
        return ENOMEM;

    // initialize private context
    WSREP_DUMMY(w)->log_fn = NULL;
    WSREP_DUMMY(w)->options = NULL;

    return 0;
}
