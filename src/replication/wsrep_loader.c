/* Copyright (C) 2009-2011 Codership Oy <info@codersihp.com>

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

/*! @file wsrep implementation loader */

#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "wsrep_api.h"

// Logging stuff for the loader
static const char* log_levels[] = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG"};

static void default_logger (wsrep_log_level_t lvl, const char* msg)
{
    fprintf (stderr, "wsrep loader: [%s] %s\n", log_levels[lvl], msg);
}

static wsrep_log_cb_t logger = default_logger;

/**************************************************************************
 * Library loader
 **************************************************************************/

static int wsrep_check_iface_version(const char* found, const char* iface_ver)
{
    const size_t msg_len = 128;
    char msg[128];

    if (strcmp(found, iface_ver)) {
        snprintf (msg, msg_len,
                  "provider interface version mismatch: need '%s', found '%s'",
                  iface_ver, found);
        logger (WSREP_LOG_ERROR, msg);
        return EINVAL;
    }

    return 0;
}

static int verify(const wsrep_t *wh, const char *iface_ver)
{
    char msg[128];
    const size_t msg_len = sizeof(msg);

#define VERIFY(_p) if (!(_p)) {                                       \
        snprintf(msg, msg_len, "wsrep_load(): verify(): %s\n", # _p); \
        logger (WSREP_LOG_ERROR, msg);                                \
        return EINVAL;                                                \
    }

    VERIFY(wh);
    VERIFY(wh->version);

    if (wsrep_check_iface_version(wh->version, iface_ver))
        return EINVAL;

    VERIFY(wh->init);
    VERIFY(wh->options_set);
    VERIFY(wh->options_get);
    VERIFY(wh->connect);
    VERIFY(wh->disconnect);
    VERIFY(wh->recv);
    VERIFY(wh->pre_commit);
    VERIFY(wh->post_commit);
    VERIFY(wh->post_rollback);
    VERIFY(wh->replay_trx);
    VERIFY(wh->abort_pre_commit);
    VERIFY(wh->append_key);
    VERIFY(wh->append_data);
    VERIFY(wh->free_connection);
    VERIFY(wh->to_execute_start);
    VERIFY(wh->to_execute_end);
    VERIFY(wh->preordered_collect);
    VERIFY(wh->preordered_commit);
    VERIFY(wh->sst_sent);
    VERIFY(wh->sst_received);
    VERIFY(wh->stats_get);
    VERIFY(wh->stats_free);
    VERIFY(wh->stats_reset);
    VERIFY(wh->pause);
    VERIFY(wh->resume);
    VERIFY(wh->desync);
    VERIFY(wh->resync);
    VERIFY(wh->lock);
    VERIFY(wh->unlock);
    VERIFY(wh->is_locked);
    VERIFY(wh->provider_name);
    VERIFY(wh->provider_version);
    VERIFY(wh->provider_vendor);
    VERIFY(wh->free);
    return 0;
}

typedef int (*wsrep_loader_fun)(wsrep_t*);

static wsrep_loader_fun wsrep_dlf(void *dlh, const char *sym)
{
    union {
        wsrep_loader_fun dlfun;
        void *obj;
    } alias;
    alias.obj = dlsym(dlh, sym);
    return alias.dlfun;
}

static int wsrep_check_version_symbol(void *dlh)
{
    char** dlversion = NULL;
    dlversion = (char**) dlsym(dlh, "wsrep_interface_version");
    if (dlversion == NULL)
        return 0;
    return wsrep_check_iface_version(*dlversion, WSREP_INTERFACE_VERSION);
}

extern int wsrep_dummy_loader(wsrep_t *w);

int wsrep_load(const char *spec, wsrep_t **hptr, wsrep_log_cb_t log_cb)
{
    int ret = 0;
    void *dlh = NULL;
    wsrep_loader_fun dlfun;
    char msg[1024];
    const size_t msg_len = sizeof(msg) - 1;
    msg[msg_len] = 0;

    if (NULL != log_cb)
        logger = log_cb;

    if (!(spec && hptr))
        return EINVAL;

    snprintf (msg, msg_len,
              "wsrep_load(): loading provider library '%s'", spec);
    logger (WSREP_LOG_INFO, msg);

    if (!(*hptr = malloc(sizeof(wsrep_t)))) {
        logger (WSREP_LOG_FATAL, "wsrep_load(): out of memory");
        return ENOMEM;
    }

    if (!spec || strcmp(spec, WSREP_NONE) == 0) {
        if ((ret = wsrep_dummy_loader(*hptr)) != 0) {
            free (*hptr);
            *hptr = NULL;
        }
        return ret;
    }

    if (!(dlh = dlopen(spec, RTLD_NOW | RTLD_LOCAL))) {
        snprintf(msg, msg_len, "wsrep_load(): dlopen(): %s", dlerror());
        logger (WSREP_LOG_ERROR, msg);
        ret = EINVAL;
        goto out;
    }

    if (!(dlfun = wsrep_dlf(dlh, "wsrep_loader"))) {
        ret = EINVAL;
        goto out;
    }

    if (wsrep_check_version_symbol(dlh) != 0) {
        ret = EINVAL;
        goto out;
    }

    if ((ret = (*dlfun)(*hptr)) != 0) {
        snprintf(msg, msg_len, "wsrep_load(): loader failed: %s",
                 strerror(ret));
        logger (WSREP_LOG_ERROR, msg);
        goto out;
    }

    if ((ret = verify(*hptr, WSREP_INTERFACE_VERSION)) != 0) {
        snprintf (msg, msg_len,
                  "wsrep_load(): interface version mismatch: my version %s, "
                  "provider version %s", WSREP_INTERFACE_VERSION,
                  (*hptr)->version);
        logger (WSREP_LOG_ERROR, msg);
        goto out;
    }

    (*hptr)->dlh = dlh;

out:
    if (ret != 0) {
        if (dlh) dlclose(dlh);
        free(*hptr);
        *hptr = NULL;
    } else {
        snprintf (msg, msg_len,
                  "wsrep_load(): %s %s by %s loaded successfully.",
                  (*hptr)->provider_name, (*hptr)->provider_version,
                  (*hptr)->provider_vendor);
        logger (WSREP_LOG_INFO, msg);
    }

    return ret;
}

void wsrep_unload(wsrep_t *hptr)
{
    if (!hptr) {
        logger (WSREP_LOG_WARN, "wsrep_unload(): null pointer.");
    } else {
        if (hptr->free)
            hptr->free(hptr);
        if (hptr->dlh)
            dlclose(hptr->dlh);
        free(hptr);
    }
}

