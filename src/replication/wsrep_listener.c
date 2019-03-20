/* Copyright (C) 2012 Codership Oy <info@codersihp.com>

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

/*! @file Example of wsrep event listener. Outputs description of received
 *        events to stdout. To get a general picture you should start with
 *        main() function. */

#include <wsrep_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

/*! This is global application context, it will be used by wsrep callbacks */
struct application_context
{};

static struct application_context global_ctx;

/*! This is receiving thread context, it will be used by wsrep callbacks */
struct receiver_context
{
    char msg[4096];
};

/*! This is a logger callback which library will be using to log events. */
static void
logger_cb (wsrep_log_level_t level __attribute__((unused)), const char* msg)
{
    fprintf (stderr, "WSREP: %s\n", msg);
}

/*! This will be called on cluster view change (nodes joining, leaving, etc.).
 *  Each view change is the point where application may be pronounced out of
 *  sync with the current cluster view and need state transfer.
 *  It is guaranteed that no other callbacks are called concurrently with it. */
static wsrep_cb_status_t
view_cb (void*                    app_ctx   __attribute__((unused)),
         void*                    recv_ctx  __attribute__((unused)),
         const wsrep_view_info_t* view,
         const char*              state     __attribute__((unused)),
         size_t                   state_len __attribute__((unused)),
         void**                   sst_req,
         size_t*                  sst_req_len)
{
    printf ("New cluster membership view: %d nodes, my index is %d, "
            "global seqno: %lld\n",
            view->memb_num, view->my_idx, (long long)view->state_id.seqno);

    if (view->state_gap) /* we need to receive new state from the cluster */
    {
        /* For simplicity we're skipping state transfer by using magic string
         * as a state transfer request.
         * This node will not be considered JOINED (having full state)
         * by other cluster members. */
        *sst_req = strdup(WSREP_STATE_TRANSFER_NONE);

        if (*sst_req)
            *sst_req_len = strlen(*sst_req) + 1;
        else
            *sst_req_len = -ENOMEM;
    }

    return WSREP_CB_SUCCESS;
}

/*! This is called to "apply" writeset.
 *  If writesets don't conflict on keys, it may be called concurrently to
 *  utilize several CPU cores. */
static wsrep_cb_status_t
apply_cb (void*                   recv_ctx,
          const void*             ws_data __attribute__((unused)),
          size_t                  ws_size,
          uint32_t                flags __attribute__((unused)),
          const wsrep_trx_meta_t* meta)
{
    struct receiver_context* ctx = (struct receiver_context*)recv_ctx;

    snprintf (ctx->msg, sizeof(ctx->msg),
              "Got writeset %lld, size %zu", (long long)meta->gtid.seqno,
              ws_size);

    return WSREP_CB_SUCCESS;
}

/*! This is called to "commit" or "rollback" previously applied writeset,
 *  depending on commit parameter.
 *  By default this callback is called synchronously in the order determined
 *  by seqno. */
static wsrep_cb_status_t
commit_cb (void*                   recv_ctx,
           uint32_t                flags __attribute((unused)),
           const wsrep_trx_meta_t* meta __attribute__((unused)),
           wsrep_bool_t*           exit __attribute__((unused)),
           wsrep_bool_t            commit)
{
    struct receiver_context* ctx = (struct receiver_context*)recv_ctx;

    /* Here we just print it to stdout. Since this callback is synchronous
     * we don't need to worry about exclusive access to stdout. */
    if (commit) puts(ctx->msg);

    return WSREP_CB_SUCCESS;
}

/* The following callbacks are stubs and not used in this example. */
static wsrep_cb_status_t
sst_donate_cb (void*               app_ctx   __attribute__((unused)),
               void*               recv_ctx  __attribute__((unused)),
               const void*         msg       __attribute__((unused)),
               size_t              msg_len   __attribute__((unused)),
               const wsrep_gtid_t* state_id  __attribute__((unused)),
               const char*         state     __attribute__((unused)),
               size_t              state_len __attribute__((unused)),
               wsrep_bool_t        bypass    __attribute__((unused)))
{
    return WSREP_CB_SUCCESS;
}

static void synced_cb (void* app_ctx __attribute__((unused))) {}

/* wsrep provider handle (global for simplicty) */
static wsrep_t* wsrep = NULL;

/* This is the listening thread. It blocks in wsrep::recv() call until
 * disconnect from cluster. It will apply and commit writesets through the
 * callbacks defined avbove. */
static void*
recv_thread (void* arg)
{
    struct receiver_context* ctx = (struct receiver_context*)arg;

    wsrep_status_t rc = wsrep->recv(wsrep, ctx);

    fprintf (stderr, "Receiver exited with code %d", rc);

    return NULL;
}

/* This is a signal handler to demonstrate graceful cluster leave. */
static void
graceful_leave (int signum)
{
    printf ("Got signal %d, exiting...\n", signum);
    wsrep->disconnect(wsrep);
}

int main (int argc, char* argv[])
{
    if (argc != 4)
    {
        fprintf (stderr, "Usage: %s </path/to/wsrep/provider> <wsrep URI> "
                 "<cluster name>\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    const char* const wsrep_provider = argv[1];
    const char* const wsrep_uri      = argv[2];
    const char* const cluster_name   = argv[3];

    /* Now let's load and initialize provider */
    wsrep_status_t rc = wsrep_load (wsrep_provider, &wsrep, logger_cb);
    if (WSREP_OK != rc)
    {
        fprintf (stderr, "Failed to load wsrep provider '%s'\n",wsrep_provider);
        exit (EXIT_FAILURE);
    }

    wsrep_gtid_t state_id = { WSREP_UUID_UNDEFINED, WSREP_SEQNO_UNDEFINED };

    /* wsrep provider initialization arguments */
    struct wsrep_init_args wsrep_args =
    {
        .app_ctx       = &global_ctx,

        .node_name     = "example listener",
        .node_address  = "",
        .node_incoming = "",
        .data_dir      = ".", // working directory
        .options       = "",
        .proto_ver     = 127, // maximum supported application event protocol

        .state_id      = &state_id,
        .state         = NULL,
        .state_len     = 0,

        .logger_cb       = logger_cb,
        .view_handler_cb = view_cb,
        .apply_cb        = apply_cb,
        .commit_cb       = commit_cb,
        .sst_donate_cb   = sst_donate_cb,
        .synced_cb       = synced_cb
    };

    rc = wsrep->init(wsrep, &wsrep_args);
    if (WSREP_OK != rc)
    {
        fprintf (stderr, "wsrep::init() failed: %d\n", rc);
        exit (EXIT_FAILURE);
    }

    /* Connect to cluster */
    rc = wsrep->connect (wsrep, cluster_name, wsrep_uri, "", 0);
    if (0 != rc)
    {
        fprintf (stderr,
                 "wsrep::connect() failed: %d (%s)\n", rc, strerror(-rc));
        exit (EXIT_FAILURE);
    }

    /* Now let's start several listening threads*/
    int const num_threads = 4;
    struct receiver_context thread_ctx[num_threads];
    pthread_t threads[num_threads];

    int i;
    for (i = 0; i < num_threads; i++)
    {
        int err = pthread_create (
            &threads[i], NULL, recv_thread, &thread_ctx[i]);

        if (err)
        {
            fprintf (stderr, "Failed to start thread %d: %d (%s)",
                     i, err, strerror(err));
            exit (EXIT_FAILURE);
        }
    }

    signal (SIGTERM, graceful_leave);
    signal (SIGINT,  graceful_leave);

    /* Listening threads are now running and receiving writesets. Wait for them
     * to join. Threads will join after signal handler closes wsrep connection*/
    for (i = 0; i < num_threads; i++)
    {
        pthread_join (threads[i], NULL);
    }

    /* Unload provider after nobody uses it any more. */
    wsrep_unload (wsrep);

    return 0;
}
