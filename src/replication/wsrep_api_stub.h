//
// Copyright (c) 2019-2023, Manticore Software LTD (https://manticoresearch.com)
//
// This is pure stub header, it does nothing and just make it possible to make build
// under windows. All other files in this folder also useless on win.
// (every necessary functions just return 'not implemented' status)


#if HAVE_WSREP
#include "wsrep_api.h"
#else

#ifndef WSREP_H
#define WSREP_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 *                                                                        *
 *                       wsrep replication API                            *
 *                                                                        *
 **************************************************************************/

#define WSREP_INTERFACE_VERSION "31"

/*! Empty backend spec */
#define WSREP_NONE "none"


/*!
 * @brief log severity levels, passed as first argument to log handler
 */
typedef enum wsrep_log_level
{
    WSREP_LOG_FATAL, //!< Unrecoverable error, application must quit.
    WSREP_LOG_ERROR, //!< Operation failed, must be repeated.
    WSREP_LOG_WARN,  //!< Unexpected condition, but no operational failure.
    WSREP_LOG_INFO,  //!< Informational message.
    WSREP_LOG_DEBUG  //!< Debug message. Shows only of compiled with debug.
} wsrep_log_level_t;

/*!
 * @brief error log handler
 *
 *        All messages from wsrep provider are directed to this
 *        handler, if present.
 *
 * @param level   log level
 * @param message log message
 */
typedef void (*wsrep_log_cb_t)(wsrep_log_level_t, const char *);


/*!
 *  Certain provider capabilities application may want to know about
 */
#define WSREP_CAP_MULTI_MASTER          ( 1ULL << 0 )
#define WSREP_CAP_CERTIFICATION         ( 1ULL << 1 )
#define WSREP_CAP_PARALLEL_APPLYING     ( 1ULL << 2 )
#define WSREP_CAP_TRX_REPLAY            ( 1ULL << 3 )
#define WSREP_CAP_ISOLATION             ( 1ULL << 4 )
#define WSREP_CAP_PAUSE                 ( 1ULL << 5 )
#define WSREP_CAP_CAUSAL_READS          ( 1ULL << 6 )
#define WSREP_CAP_CAUSAL_TRX            ( 1ULL << 7 )
#define WSREP_CAP_INCREMENTAL_WRITESET  ( 1ULL << 8 )
#define WSREP_CAP_SESSION_LOCKS         ( 1ULL << 9 )
#define WSREP_CAP_DISTRIBUTED_LOCKS     ( 1ULL << 10 )
#define WSREP_CAP_CONSISTENCY_CHECK     ( 1ULL << 11 )
#define WSREP_CAP_UNORDERED             ( 1ULL << 12 )
#define WSREP_CAP_ANNOTATION            ( 1ULL << 13 )
#define WSREP_CAP_PREORDERED            ( 1ULL << 14 )


/*!
 *  Writeset flags
 *
 * COMMIT       the writeset and all preceding writesets must be committed
 * ROLLBACK     all preceding writesets in a transaction must be rolled back
 * ISOLATION    the writeset must be applied AND committed in isolation
 * PA_UNSAFE    the writeset cannot be applied in parallel
 * COMMUTATIVE  the order in which the writeset is applied does not matter
 * NATIVE       the writeset contains another writeset in this provider format
 *
 * Note that some of the flags are mutually exclusive (e.g. COMMIT and
 * ROLLBACK).
 */
#define WSREP_FLAG_COMMIT               ( 1ULL << 0 )
#define WSREP_FLAG_ROLLBACK             ( 1ULL << 1 )
#define WSREP_FLAG_ISOLATION            ( 1ULL << 2 )
#define WSREP_FLAG_PA_UNSAFE            ( 1ULL << 3 )
#define WSREP_FLAG_COMMUTATIVE          ( 1ULL << 4 )
#define WSREP_FLAG_NATIVE               ( 1ULL << 5 )


typedef uint64_t wsrep_trx_id_t;  //!< application transaction ID
typedef uint64_t wsrep_conn_id_t; //!< application connection ID
typedef int64_t  wsrep_seqno_t;   //!< sequence number of a writeset, etc.
#ifdef __cplusplus
typedef bool     wsrep_bool_t;
#else
typedef _Bool    wsrep_bool_t;    //!< should be the same as standard (C99) bool
#endif /* __cplusplus */

/*! undefined seqno */
#define WSREP_SEQNO_UNDEFINED (-1)


/*! wsrep provider status codes */
typedef enum wsrep_status
{
    WSREP_OK = 0,          //!< success
    WSREP_WARNING,         //!< minor warning, error logged
    WSREP_TRX_MISSING,     //!< transaction is not known by wsrep
    WSREP_TRX_FAIL,        //!< transaction aborted, server can continue
    WSREP_BF_ABORT,        //!< trx was victim of brute force abort
    WSREP_SIZE_EXCEEDED,   //!< data exceeded maximum supported size
    WSREP_CONN_FAIL,       //!< error in client connection, must abort
    WSREP_NODE_FAIL,       //!< error in node state, wsrep must reinit
    WSREP_FATAL,           //!< fatal error, server must abort
    WSREP_PRECOMMIT_ABORT, //!< transaction was aborted before commencing pre-commit
    WSREP_NOT_IMPLEMENTED  //!< feature not implemented
} wsrep_status_t;


/*! wsrep callbacks status codes */
typedef enum wsrep_cb_status
{
    WSREP_CB_SUCCESS =  0, //!< success (as in "not critical failure")
    WSREP_CB_FAILURE       //!< critical failure (consistency violation)
    /* Technically, wsrep provider has no use for specific failure codes since
     * there is nothing it can do about it but abort execution. Therefore any
     * positive number shall indicate a critical failure. Optionally that value
     * may be used by provider to come to a consensus about state consistency
     * in a group of nodes. */
} wsrep_cb_status_t;


/*!
 * UUID type - for all unique IDs
 */
typedef struct wsrep_uuid {
    uint8_t data[16];
} wsrep_uuid_t;

/*! Undefined UUID */
static const wsrep_uuid_t WSREP_UUID_UNDEFINED = {{0,}};

/*! UUID string representation length, terminating '\0' not included */
#define WSREP_UUID_STR_LEN 36

static int wsrep_uuid_scan ( const char* str, size_t str_len, wsrep_uuid_t* uuid ) { return WSREP_NOT_IMPLEMENTED; }
static int wsrep_uuid_print ( const wsrep_uuid_t* uuid, char* str, size_t str_len ) { return WSREP_NOT_IMPLEMENTED; }

#define WSREP_MEMBER_NAME_LEN 32  //!< maximum logical member name length
#define WSREP_INCOMING_LEN    256 //!< max Domain Name length + 0x00


/*!
 * Global transaction identifier
 */
typedef struct wsrep_gtid
{
    wsrep_uuid_t  uuid;  /*!< History UUID */
    wsrep_seqno_t seqno; /*!< Sequence number */
} wsrep_gtid_t;

/*! Undefined GTID */
static const wsrep_gtid_t WSREP_GTID_UNDEFINED = {{{0, }}, -1};

/*! Minimum number of bytes guaranteed to store GTID string representation,
 * terminating '\0' not included (36 + 1 + 20) */
#define WSREP_GTID_STR_LEN 57

static int wsrep_gtid_scan ( const char* str, size_t str_len, wsrep_gtid_t* gtid ) { return WSREP_NOT_IMPLEMENTED; }
static int wsrep_gtid_print ( const wsrep_gtid_t* gtid, char* str, size_t str_len ) { return WSREP_NOT_IMPLEMENTED; }


/*!
 * Transaction meta data
 */
typedef struct wsrep_trx_meta
{
    wsrep_gtid_t  gtid;       /*!< Global transaction identifier */
    wsrep_seqno_t depends_on; /*!< Sequence number part of the last transaction
                                   this transaction depends on */
} wsrep_trx_meta_t;


/*!
 * member status
 */
typedef enum wsrep_member_status {
    WSREP_MEMBER_UNDEFINED, //!< undefined state
    WSREP_MEMBER_JOINER,    //!< incomplete state, requested state transfer
    WSREP_MEMBER_DONOR,     //!< complete state, donates state transfer
    WSREP_MEMBER_JOINED,    //!< complete state
    WSREP_MEMBER_SYNCED,    //!< complete state, synchronized with group
    WSREP_MEMBER_ERROR,     //!< this and above is provider-specific error code
    WSREP_MEMBER_MAX
} wsrep_member_status_t;

/*!
 * static information about a group member (some fields are tentative yet)
 */
typedef struct wsrep_member_info {
    wsrep_uuid_t id;                           //!< group-wide unique member ID
    char         name[WSREP_MEMBER_NAME_LEN];  //!< human-readable name
    char         incoming[WSREP_INCOMING_LEN]; //!< address for client requests
} wsrep_member_info_t;

/*!
 * group status
 */
typedef enum wsrep_view_status {
    WSREP_VIEW_PRIMARY,      //!< primary group configuration (quorum present)
    WSREP_VIEW_NON_PRIMARY,  //!< non-primary group configuration (quorum lost)
    WSREP_VIEW_DISCONNECTED, //!< not connected to group, retrying.
    WSREP_VIEW_MAX
} wsrep_view_status_t;

/*!
 * view of the group
 */
typedef struct wsrep_view_info {
    wsrep_gtid_t        state_id;  //!< global state ID
    wsrep_seqno_t       view;      //!< global view number
    wsrep_view_status_t status;    //!< view status
    wsrep_bool_t        state_gap; //!< gap between global and local states
    int                 my_idx;    //!< index of this member in the view
    int                 memb_num;  //!< number of members in the view
    int                 proto_ver; //!< application protocol agreed on the view
    wsrep_member_info_t members[1];//!< array of member information
} wsrep_view_info_t;

/*!
 * Magic string to tell provider to engage into trivial (empty) state transfer.
 * No data will be passed, but the node shall be considered JOINED.
 * Should be passed in sst_req parameter of wsrep_view_cb_t.
 */
#define WSREP_STATE_TRANSFER_TRIVIAL "trivial"

/*!
 * Magic string to tell provider not to engage in state transfer at all.
 * The member will stay in WSREP_MEMBER_UNDEFINED state but will keep on
 * receiving all writesets.
 * Should be passed in sst_req parameter of wsrep_view_cb_t.
 */
#define WSREP_STATE_TRANSFER_NONE "none"

/*!
 * @brief group view handler
 *
 * This handler is called in total order corresponding to the group
 * configuration change. It is to provide a vital information about
 * new group view. If view info indicates existence of discontinuity
 * between group and member states, state transfer request message
 * should be filled in by the callback implementation.
 *
 * @note Currently it is assumed that sst_req is allocated using
 *       malloc()/calloc()/realloc() and it will be freed by
 *       wsrep implementation.
 *
 * @param app_ctx     application context
 * @param recv_ctx    receiver context
 * @param view        new view on the group
 * @param state       current state
 * @param state_len   lenght of current state
 * @param sst_req     location to store SST request
 * @param sst_req_len location to store SST request length or error code,
 *                    value of 0 means no SST.
 */
typedef enum wsrep_cb_status (*wsrep_view_cb_t) (
    void*                    app_ctx,
    void*                    recv_ctx,
    const wsrep_view_info_t* view,
    const char*              state,
    size_t                   state_len,
    void**                   sst_req,
    size_t*                  sst_req_len
);


/*!
 * @brief apply callback
 *
 * This handler is called from wsrep library to apply replicated writeset
 * Must support brute force applying for multi-master operation
 *
 * @param recv_ctx receiver context pointer provided by the application
 * @param data     data buffer containing the writeset
 * @param size     data buffer size
 * @param flags    WSREP_FLAG_... flags
 * @param meta     transaction meta data of the writeset to be applied
 *
 * @return success code:
 * @retval WSREP_OK
 * @retval WSREP_NOT_IMPLEMENTED appl. does not support the writeset format
 * @retval WSREP_ERROR failed to apply the writeset
 */
typedef enum wsrep_cb_status (*wsrep_apply_cb_t) (
    void*                   recv_ctx,
    const void*             data,
    size_t                  size,
    uint32_t                flags,
    const wsrep_trx_meta_t* meta
);


/*!
 * @brief commit callback
 *
 * This handler is called to commit the changes made by apply callback.
 *
 * @param recv_ctx receiver context pointer provided by the application
 * @param flags    WSREP_FLAG_... flags
 * @param meta     transaction meta data of the writeset to be committed
 * @param exit     set to true to exit recv loop
 * @param commit   true - commit writeset, false - rollback writeset
 *
 * @return success code:
 * @retval WSREP_OK
 * @retval WSREP_ERROR call failed
 */
typedef enum wsrep_cb_status (*wsrep_commit_cb_t) (
    void*                   recv_ctx,
    const void*             trx_handle,
    uint32_t                flags,
    const wsrep_trx_meta_t* meta,
    wsrep_bool_t*           exit,
    wsrep_bool_t            commit
);


/*!
 * @brief unordered callback
 *
 * This handler is called to execute unordered actions (actions that need not
 * to be executed in any particular order) attached to writeset.
 *
 * @param recv_ctx receiver context pointer provided by the application
 * @param data     data buffer containing the writeset
 * @param size     data buffer size
 */
typedef enum wsrep_cb_status (*wsrep_unordered_cb_t) (
    void*       recv_ctx,
    const void* data,
    size_t      size
);


/*!
 * @brief a callback to donate state snapshot
 *
 * This handler is called from wsrep library when it needs this node
 * to deliver state to a new cluster member.
 * No state changes will be committed for the duration of this call.
 * Wsrep implementation may provide internal state to be transmitted
 * to new cluster member for initial state.
 *
 * @param app_ctx   application context
 * @param recv_ctx  receiver context
 * @param msg       state transfer request message
 * @param msg_len   state transfer request message length
 * @param gtid      current state ID on this node
 * @param state     current wsrep internal state buffer
 * @param state_len current wsrep internal state buffer len
 * @param bypass    bypass snapshot transfer, only transfer uuid:seqno pair
 */
typedef enum wsrep_cb_status (*wsrep_sst_donate_cb_t) (
    void*               app_ctx,
    void*               recv_ctx,
    const void*         msg,
    size_t              msg_len,
    const wsrep_gtid_t* state_id,
    const char*         state,
    size_t              state_len,
    wsrep_bool_t        bypass
);


/*!
 * @brief a callback to signal application that wsrep state is synced
 *        with cluster
 *
 * This callback is called after wsrep library has got in sync with
 * rest of the cluster.
 *
 * @param app_ctx application context
 */
typedef void (*wsrep_synced_cb_t) (void* app_ctx);

/*!
 * @brief different instruments that we want to monitor through PFS.
 */
typedef enum wsrep_pfs_instr_type
{
    WSREP_PFS_INSTR_TYPE_UNKNOWN,
    WSREP_PFS_INSTR_TYPE_MUTEX,
    WSREP_PFS_INSTR_TYPE_CONDVAR,
    WSREP_PFS_INSTR_TYPE_THREAD,
    WSREP_PFS_INSTR_TYPE_FILE
} wsrep_pfs_instr_type_t;

/*!
 * @brief type of operation on instruments to monitor.
 */
typedef enum wsrep_pfs_instr_ops
{
    WSREP_PFS_INSTR_OPS_UNKNOWN,
    WSREP_PFS_INSTR_OPS_INIT,
    WSREP_PFS_INSTR_OPS_DESTROY,
    WSREP_PFS_INSTR_OPS_LOCK,
    WSREP_PFS_INSTR_OPS_UNLOCK,
    WSREP_PFS_INSTR_OPS_WAIT,
    WSREP_PFS_INSTR_OPS_TIMEDWAIT,
    WSREP_PFS_INSTR_OPS_SIGNAL,
    WSREP_PFS_INSTR_OPS_BROADCAST,
    WSREP_PFS_INSTR_OPS_CREATE,
    WSREP_PFS_INSTR_OPS_OPEN,
    WSREP_PFS_INSTR_OPS_CLOSE,
    WSREP_PFS_INSTR_OPS_DELETE
} wsrep_pfs_instr_ops_t;

/*!
 * @brief name/tag of different instruments.
 */
typedef enum wsrep_pfs_instr_tag
{
    /* Mutex tag */
    WSREP_PFS_INSTR_TAG_CERT_MUTEX = 0,
    WSREP_PFS_INSTR_TAG_STATS_MUTEX,
    WSREP_PFS_INSTR_TAG_DUMMY_GCS_MUTEX,
    WSREP_PFS_INSTR_TAG_SERVICE_THD_MUTEX,
    WSREP_PFS_INSTR_TAG_IST_RECEIVER_MUTEX,
    WSREP_PFS_INSTR_TAG_LOCAL_MONITOR_MUTEX,
    WSREP_PFS_INSTR_TAG_APPLY_MONITOR_MUTEX,
    WSREP_PFS_INSTR_TAG_COMMIT_MONITOR_MUTEX,
    WSREP_PFS_INSTR_TAG_ASYNC_SENDER_MONITOR_MUTEX,
    WSREP_PFS_INSTR_TAG_IST_RECEIVER_MONITOR_MUTEX,
    WSREP_PFS_INSTR_TAG_SST_MUTEX,
    WSREP_PFS_INSTR_TAG_INCOMING_MUTEX,
    WSREP_PFS_INSTR_TAG_SAVED_STATE_MUTEX,
    WSREP_PFS_INSTR_TAG_TRX_HANDLE_MUTEX,
    WSREP_PFS_INSTR_TAG_WSDB_TRX_MUTEX,
    WSREP_PFS_INSTR_TAG_WSDB_CONN_MUTEX,
    WSREP_PFS_INSTR_TAG_PROFILE_MUTEX,
    WSREP_PFS_INSTR_TAG_GCACHE_MUTEX,
    WSREP_PFS_INSTR_TAG_PROTSTACK_MUTEX,
    WSREP_PFS_INSTR_TAG_PRODCONS_MUTEX,
    WSREP_PFS_INSTR_TAG_GCOMMCONN_MUTEX,
    WSREP_PFS_INSTR_TAG_RECVBUF_MUTEX,
    WSREP_PFS_INSTR_TAG_MEMPOOL_MUTEX,

    /* CondVar tag */
    WSREP_PFS_INSTR_TAG_DUMMY_GCS_CONDVAR,
    WSREP_PFS_INSTR_TAG_SERVICE_THD_CONDVAR,
    WSREP_PFS_INSTR_TAG_SERVICE_THD_FLUSH_CONDVAR,
    WSREP_PFS_INSTR_TAG_IST_RECEIVER_CONDVAR,
    WSREP_PFS_INSTR_TAG_IST_CONSUMER_CONDVAR,
    WSREP_PFS_INSTR_TAG_LOCAL_MONITOR_CONDVAR,
    WSREP_PFS_INSTR_TAG_APPLY_MONITOR_CONDVAR,
    WSREP_PFS_INSTR_TAG_COMMIT_MONITOR_CONDVAR,
    WSREP_PFS_INSTR_TAG_ASYNC_SENDER_MONITOR_CONDVAR,
    WSREP_PFS_INSTR_TAG_IST_RECEIVER_MONITOR_CONDVAR,
    WSREP_PFS_INSTR_TAG_SST_CONDVAR,
    WSREP_PFS_INSTR_TAG_PRODCONS_CONDVAR,
    WSREP_PFS_INSTR_TAG_GCACHE_CONDVAR,
    WSREP_PFS_INSTR_TAG_RECVBUF_CONDVAR,

    /* Thread tag */
    WSREP_PFS_INSTR_TAG_SERVICE_THD_THREAD,
    WSREP_PFS_INSTR_TAG_IST_RECEIVER_THREAD,
    WSREP_PFS_INSTR_TAG_IST_ASYNC_SENDER_THREAD,
    WSREP_PFS_INSTR_TAG_WRITESET_CHECKSUM_THREAD,
    WSREP_PFS_INSTR_TAG_GCACHE_REMOVEFILE_THREAD,
    WSREP_PFS_INSTR_TAG_RECEIVER_THREAD,
    WSREP_PFS_INSTR_TAG_GCOMMCONN_THREAD,

    /* File tag */
    WSREP_PFS_INSTR_TAG_RECORDSET_FILE,
    WSREP_PFS_INSTR_TAG_RINGBUFFER_FILE,
    WSREP_PFS_INSTR_TAG_GCACHE_PAGE_FILE,
    WSREP_PFS_INSTR_TAG_GRASTATE_FILE,
    WSREP_PFS_INSTR_TAG_GVWSTATE_FILE

} wsrep_pfs_instr_tag_t;

/*!
 * @brief a callback to create PFS instrumented mutex/condition variables
 * threads/file-instances
 *
 *
 * @param type          mutex/condition variable/thread/file-instance
 * @param ops           add/init or remove/destory mutex/condition variable
 * @param tag           tag/name of instrument to monitor
 * @param value         created mutex/condition variable/file-descriptor.
 * @param alliedvalue   allied value for supporting operation.
                        for example: while waiting for cond-var corresponding
                        mutex is passes through this variable.
 * @param ts            if cond-var then time to wait
                        if file then name of the file.
 */
typedef void (*wsrep_pfs_instr_cb_t) (
    wsrep_pfs_instr_type_t        type,
    wsrep_pfs_instr_ops_t         ops,
    wsrep_pfs_instr_tag_t         tag,
    void**                        value,
    void**                        alliedvalue,
    const void*                   ts);

typedef wsrep_pfs_instr_cb_t gu_pfs_instr_cb_t;

/*!
 * @brief a callback to signal application that wsrep provider was
 * terminated abnormally. In this case, the application can perform
 * the critical steps to clean its state, for example, it can terminate
 * the child processes associated with the SST.
 *
 * This callback is called after wsrep library was terminated
 * abnormally using abort() call.
 */
typedef void (*wsrep_abort_cb_t) (void);


/*!
 * Initialization parameters for wsrep provider.
 */
struct wsrep_init_args
{
    void* app_ctx;             //!< Application context for callbacks

    /* Configuration parameters */
    const char* node_name;     //!< Symbolic name of this node (e.g. hostname)
    const char* node_address;  //!< Address to be used by wsrep provider
    const char* node_incoming; //!< Address for incoming client connections
    const char* data_dir;      //!< Directory where wsrep files are kept if any
    const char* options;       //!< Provider-specific configuration string
    int         proto_ver;     //!< Max supported application protocol version

    /* Application initial state information. */
    const wsrep_gtid_t* state_id;    //!< Application state GTID
    const char*         state;       //!< Initial state for wsrep provider
    size_t              state_len;   //!< Length of state buffer

    /* Application callbacks */
    wsrep_log_cb_t        logger_cb;       //!< logging handler
    wsrep_view_cb_t       view_handler_cb; //!< group view change handler

    /* Applier callbacks */
    wsrep_apply_cb_t      apply_cb;        //!< apply  callback
    wsrep_commit_cb_t     commit_cb;       //!< commit callback
    wsrep_unordered_cb_t  unordered_cb;    //!< callback for unordered actions

    /* State Snapshot Transfer callbacks */
    wsrep_sst_donate_cb_t sst_donate_cb;   //!< starting to donate
    wsrep_synced_cb_t     synced_cb;       //!< synced with group

    /* Abnormal termination callback: */
    wsrep_abort_cb_t      abort_cb;        //!< wsrep provider terminated
                                           //!< abnormally

   /* Instrument mutex/condition variables through MySQL Performance
   Schema infrastructure. Callback help in creating these mutexes in MySQL
   space with needed infrastructure to register them. */
   wsrep_pfs_instr_cb_t   pfs_instr_cb;    //!< register for pfs instrumentation
};


/*! Type of the stats variable value in struct wsrep_status_var */
typedef enum wsrep_var_type
{
    WSREP_VAR_STRING, //!< pointer to null-terminated string
    WSREP_VAR_INT64,  //!< int64_t
    WSREP_VAR_DOUBLE  //!< double
}
wsrep_var_type_t;

#define _int64 i64

/*! Generalized stats variable representation */
struct wsrep_stats_var
{
    const char*      name;     //!< variable name
    wsrep_var_type_t type;     //!< variable value type
    union {
        int64_t     _int64;
        double      _double;
        const char* _string;
    } value;                   //!< variable value
};

/*! Structure to copy-over node information exposed through PFS. */
#define WSREP_HOSTNAME_LENGTH		64
#define WSREP_STATUS_LENGTH		64

typedef struct
{
    /* User assigned host-name */
    char host_name[WSREP_HOSTNAME_LENGTH + 1];

    /* System assigned UUID */
    char uuid[WSREP_UUID_STR_LEN + 1];

    /* Status PRIMARY/NON_PRIMARY */
    char status[WSREP_STATUS_LENGTH + 1];

    /* local index */
    uint64_t local_index;

    /* Segment of node */
    uint32_t segment;
} wsrep_node_info_t;

/*! Abstract data buffer structure */
typedef struct wsrep_buf
{
    const void* ptr; /*!< Pointer to data buffer */
    size_t      len; /*!< Length of buffer */
} wsrep_buf_t;

/*! Key struct used to pass certification keys for transaction handling calls.
 *  A key consists of zero or more key parts. */
typedef struct wsrep_key
{
    const wsrep_buf_t* key_parts;     /*!< Array of key parts  */
    size_t             key_parts_num; /*!< Number of key parts */
} wsrep_key_t;

/*! Key type:
 *  EXCLUSIVE conflicts with any key type
 *  SEMI      reserved. If not supported, should be interpeted as EXCLUSIVE
 *  SHARED    conflicts only with EXCLUSIVE keys */
typedef enum wsrep_key_type
{
    WSREP_KEY_SHARED = 0,
    WSREP_KEY_SEMI,
    WSREP_KEY_EXCLUSIVE
} wsrep_key_type_t;

/*! Data type:
 *  ORDERED    state modification event that should be applied and committed
 *             in order.
 *  UNORDERED  some action that does not modify state and execution of which is
 *             optional and does not need to happen in order.
 *  ANNOTATION (human readable) writeset annotation. */
typedef enum wsrep_data_type
{
    WSREP_DATA_ORDERED = 0,
    WSREP_DATA_UNORDERED,
    WSREP_DATA_ANNOTATION
} wsrep_data_type_t;


/*! Transaction handle struct passed for wsrep transaction handling calls */
typedef struct wsrep_ws_handle
{
    wsrep_trx_id_t trx_id; //!< transaction ID
    void*          opaque; //!< opaque provider transaction context data
} wsrep_ws_handle_t;

/*!
 * @brief Helper method to reset trx writeset handle state when trx id changes
 *
 * Instead of passing wsrep_ws_handle_t directly to wsrep calls,
 * wrapping handle with this call offloads bookkeeping from
 * application.
 */
static inline wsrep_ws_handle_t* wsrep_ws_handle_for_trx(
    wsrep_ws_handle_t* ws_handle,
    wsrep_trx_id_t     trx_id)
{
    if (ws_handle->trx_id != trx_id)
    {
        ws_handle->trx_id = trx_id;
        ws_handle->opaque = NULL;
    }
    return ws_handle;
}


/*!
 *  A handle for processing preordered actions.
 *  Must be initialized to WSREP_PO_INITIALIZER before use.
 */
typedef struct wsrep_po_handle { void* opaque; } wsrep_po_handle_t;

static const wsrep_po_handle_t WSREP_PO_INITIALIZER = { NULL };


typedef struct wsrep wsrep_t;
/*!
 * wsrep interface for dynamically loadable libraries
 */
struct wsrep {

    const char *version; //!< interface version string

  /*!
   * @brief Initializes wsrep provider
   *
   * @param wsrep provider handle
   * @param args  wsrep initialization parameters
   */
    wsrep_status_t (*init)   (wsrep_t*                      wsrep,
                              const struct wsrep_init_args* args);

  /*!
   * @brief Returns provider capabilities flag bitmap
   *
   * @param wsrep provider handle
   */
    uint64_t (*capabilities) (wsrep_t* wsrep);

  /*!
   * @brief Passes provider-specific configuration string to provider.
   *
   * @param wsrep provider handle
   * @param conf  configuration string
   *
   * @retval WSREP_OK      configuration string was parsed successfully
   * @retval WSREP_WARNING could't not parse conf string, no action taken
   */
    wsrep_status_t (*options_set) (wsrep_t* wsrep, const char* conf);

  /*!
   * @brief Returns provider-specific string with current configuration values.
   *
   * @param wsrep provider handle
   *
   * @return a dynamically allocated string with current configuration
   *         parameter values
   */
    char*          (*options_get) (wsrep_t* wsrep);

  /*!
   * @brief Opens connection to cluster
   *
   * Returns when either node is ready to operate as a part of the clsuter
   * or fails to reach operating status.
   *
   * @param wsrep        provider handle
   * @param cluster_name unique symbolic cluster name
   * @param cluster_url  URL-like cluster address (backend://address)
   * @param state_donor  name of the node to be asked for state transfer.
   * @param bootstrap    a flag to request initialization of a new wsrep
   *                     service rather then a connection to the existing one.
   *                     clister_url may still carry important initialization
   *                     parameters, like backend spec and/or listen address.
   */
    wsrep_status_t (*connect) (wsrep_t*     wsrep,
                               const char*  cluster_name,
                               const char*  cluster_url,
                               const char*  state_donor,
                               wsrep_bool_t bootstrap);

  /*!
   * @brief Closes connection to cluster.
   *
   * If state_uuid and/or state_seqno is not NULL, will store final state
   * in there.
   *
   * @param wsrep this  wsrep handler
   */
    wsrep_status_t (*disconnect)(wsrep_t* wsrep);

  /*!
   * @brief start receiving replication events
   *
   * This function never returns
   *
   * @param wsrep provider handle
   * @param recv_ctx receiver context
   */
    wsrep_status_t (*recv)(wsrep_t* wsrep, void* recv_ctx);

  /*!
   * @brief Replicates the given write-set on group channel.
   * This API will not mark start of the execution. That will be done by
   * pre_commit api below.
   *
   * Must be called before transaction commit. Returns success code, which
   * caller must check.
   * In case of WSREP_OK, starts commit critical section, transaction can
   * commit. Otherwise transaction must rollback.
   *
   * @param wsrep      provider handle
   * @param ws_handle  writeset of committing transaction
   * @param conn_id    connection ID
   * @param flags      fine tuning the replication WSREP_FLAG_*
   * @param meta       transaction meta data
   *
   * @retval WSREP_OK         cluster-wide commit succeeded
   * @retval WSREP_TRX_FAIL   must rollback transaction
   * @retval WSREP_CONN_FAIL  must close client connection
   * @retval WSREP_NODE_FAIL  must close all connections and reinit
   */
    wsrep_status_t (*replicate)(wsrep_t*                wsrep,
                                wsrep_conn_id_t         conn_id,
                                wsrep_ws_handle_t*      ws_handle,
                                uint32_t                flags,
                                wsrep_trx_meta_t*       meta);

  /*!
   * @brief Execute pre-commit hook that mark start of execution by entering
   * Apply and Commit Monitor.
   *
   * Must be called before transaction commit. Returns success code, which
   * caller must check.
   * In case of WSREP_OK, starts commit critical section, transaction can
   * commit. Otherwise transaction must rollback.
   *
   * @param wsrep      provider handle
   * @param ws_handle  writeset of committing transaction
   * @param conn_id    connection ID
   * @param flags      fine tuning the replication WSREP_FLAG_*
   * @param meta       transaction meta data
   *
   * @retval WSREP_OK         cluster-wide commit succeeded
   * @retval WSREP_TRX_FAIL   must rollback transaction
   * @retval WSREP_CONN_FAIL  must close client connection
   * @retval WSREP_NODE_FAIL  must close all connections and reinit
   */
    wsrep_status_t (*pre_commit)(wsrep_t*                wsrep,
                                 wsrep_conn_id_t         conn_id,
                                 wsrep_ws_handle_t*      ws_handle,
                                 uint32_t                flags,
                                 wsrep_trx_meta_t*       meta);

  /*!
   * @brief Replicates/logs result of transaction to other nodes and allocates
   * required resources.
   *
   * Must be called before transaction commit. Returns success code, which
   * caller must check.
   * In case of WSREP_OK, starts commit critical section, transaction can
   * commit. Otherwise transaction must rollback.
   *
   * @param wsrep      provider handle
   * @param ws_handle  writeset of committing transaction
   * @param conn_id    connection ID
   * @param flags      fine tuning the replication WSREP_FLAG_*
   * @param meta       transaction meta data
   *
   * @retval WSREP_OK         cluster-wide commit succeeded
   * @retval WSREP_TRX_FAIL   must rollback transaction
   * @retval WSREP_CONN_FAIL  must close client connection
   * @retval WSREP_NODE_FAIL  must close all connections and reinit
   */
    wsrep_status_t (*replicate_pre_commit)(wsrep_t*                wsrep,
                                           wsrep_conn_id_t         conn_id,
                                           wsrep_ws_handle_t*      ws_handle,
                                           uint32_t                flags,
                                           wsrep_trx_meta_t*       meta);

  /*!
   * @brief Exit the critical section (CommitMonitor)
   *
   * @param wsrep      provider handle
   * @param ws_handle  writeset of committing transaction
   * @retval WSREP_OK  post_commit succeeded
   */
    wsrep_status_t (*interim_commit) (wsrep_t*            wsrep,
                                      wsrep_ws_handle_t*  ws_handle);

  /*!
   * @brief Releases resources after transaction commit.
   *
   * Ends commit critical section.
   *
   * @param wsrep      provider handle
   * @param ws_handle  writeset of committing transaction
   * @retval WSREP_OK  post_commit succeeded
   */
    wsrep_status_t (*post_commit) (wsrep_t*            wsrep,
                                   wsrep_ws_handle_t*  ws_handle);

  /*!
   * @brief Enforce commit ordering for applier thread by entering
   * Commit Monitor.
   *
   * @param wsrep      provider handle
   * @param trx_handle  writeset of committing transaction
   *
   * @retval WSREP_OK         cluster-wide commit succeeded
   * @retval WSREP_TRX_FAIL   must rollback transaction
   * @retval WSREP_CONN_FAIL  must close client connection
   * @retval WSREP_NODE_FAIL  must close all connections and reinit
   */
    wsrep_status_t (*applier_pre_commit)(wsrep_t*    wsrep,
                                         void*       trx_handle);

  /*!
   * @brief Mark end of commit ordering by leaving Commit Monitor.
   *
   * @param wsrep      provider handle
   * @param trx_handle  writeset of committing transaction
   *
   * @retval WSREP_OK         cluster-wide commit succeeded
   * @retval WSREP_TRX_FAIL   must rollback transaction
   * @retval WSREP_CONN_FAIL  must close client connection
   * @retval WSREP_NODE_FAIL  must close all connections and reinit
   */
    wsrep_status_t (*applier_interim_commit)(wsrep_t*   wsrep,
                                             void*      trx_handle);

  /*!
   * @brief Mark end of commit ordering by leaving Commit Monitor.
   *
   * @param wsrep      provider handle
   * @param trx_handle  writeset of committing transaction
   *
   * @retval WSREP_OK         cluster-wide commit succeeded
   * @retval WSREP_TRX_FAIL   must rollback transaction
   * @retval WSREP_CONN_FAIL  must close client connection
   * @retval WSREP_NODE_FAIL  must close all connections and reinit
   */
    wsrep_status_t (*applier_post_commit)(wsrep_t*   wsrep,
                                          void*      trx_handle);

  /*!
   * @brief Releases resources after transaction rollback.
   *
   * @param wsrep      provider handle
   * @param ws_handle  writeset of committing transaction
   * @retval WSREP_OK  post_rollback succeeded
   */
    wsrep_status_t (*post_rollback)(wsrep_t*            wsrep,
                                    wsrep_ws_handle_t*  ws_handle);

  /*!
   * @brief Replay trx as a slave writeset
   *
   * If local trx has been aborted by brute force, and it has already
   * replicated before this abort, we must try if we can apply it as
   * slave trx. Note that slave nodes see only trx writesets and certification
   * test based on write set content can be different to DBMS lock conflicts.
   *
   * @param wsrep      provider handle
   * @param ws_handle  writeset of committing transaction
   * @param trx_ctx    transaction context
   *
   * @retval WSREP_OK         cluster commit succeeded
   * @retval WSREP_TRX_FAIL   must rollback transaction
   * @retval WSREP_BF_ABORT   brute force abort happened after trx replicated
   *                          must rollback transaction and try to replay
   * @retval WSREP_CONN_FAIL  must close client connection
   * @retval WSREP_NODE_FAIL  must close all connections and reinit
   */
    wsrep_status_t (*replay_trx)(wsrep_t*            wsrep,
                                 wsrep_ws_handle_t*  ws_handle,
                                 void*               trx_ctx);

  /*!
   * @brief Abort pre_commit() call of another thread.
   *
   * It is possible, that some high-priority transaction needs to abort
   * another transaction which is in pre_commit() call waiting for resources.
   *
   * The kill routine checks that abort is not attmpted against a transaction
   * which is front of the caller (in total order).
   *
   * @param wsrep      provider handle
   * @param bf_seqno   seqno of brute force trx, running this cancel
   * @param victim_trx transaction to be aborted, and which is committing
   *
   * @retval WSREP_OK       abort secceded
   * @retval WSREP_WARNING  abort failed
   */
    wsrep_status_t (*abort_pre_commit)(wsrep_t*       wsrep,
                                       wsrep_seqno_t  bf_seqno,
                                       wsrep_trx_id_t victim_trx);

  /*!
   * @brief Appends a row reference to transaction writeset
   *
   * Both copy flag and key_type can be ignored by provider (key type
   * interpreted as WSREP_KEY_EXCLUSIVE).
   *
   * @param wsrep      provider handle
   * @param ws_handle  writeset handle
   * @param keys       array of keys
   * @param count      length of the array of keys
   * @param type       type ot the key
   * @param copy       can be set to FALSE if keys persist through commit.
   */
    wsrep_status_t (*append_key)(wsrep_t*            wsrep,
                                 wsrep_ws_handle_t*  ws_handle,
                                 const wsrep_key_t*  keys,
                                 size_t              count,
                                 enum wsrep_key_type type,
                                 wsrep_bool_t        copy);

  /*!
   * @brief Appends data to transaction writeset
   *
   * This method can be called any time before commit and it
   * appends a number of data buffers to transaction writeset.
   *
   * Both copy and unordered flags can be ignored by provider.
   *
   * @param wsrep      provider handle
   * @param ws_handle  writeset handle
   * @param data       array of data buffers
   * @param count      buffer count
   * @param type       type of data
   * @param copy       can be set to FALSE if data persists through commit.
   */
    wsrep_status_t (*append_data)(wsrep_t*                wsrep,
                                  wsrep_ws_handle_t*      ws_handle,
                                  const struct wsrep_buf* data,
                                  size_t                  count,
                                  enum wsrep_data_type    type,
                                  wsrep_bool_t            copy);

  /*!
   * @brief Get causal ordering for read operation
   *
   * This call will block until causal ordering with all possible
   * preceding writes in the cluster is guaranteed. If pointer to
   * gtid is non-null, the call stores the global transaction ID
   * of the last transaction which is guaranteed to be ordered
   * causally before this call.
   *
   * @param wsrep provider handle
   * @param gtid  location to store GTID
   */
    wsrep_status_t (*causal_read)(wsrep_t* wsrep, wsrep_gtid_t* gtid);

  /*!
   * @brief Clears allocated connection context.
   *
   * Whenever a new connection ID is passed to wsrep provider through
   * any of the API calls, a connection context is allocated for this
   * connection. This call is to explicitly notify provider to close the
   * connection.
   *
   * @param wsrep       provider handle
   * @param conn_id     connection ID
   * @param query       the 'set database' query
   * @param query_len   length of query (does not end with 0)
   */
    wsrep_status_t (*free_connection)(wsrep_t*        wsrep,
                                      wsrep_conn_id_t conn_id);

  /*!
   * @brief Replicates a query and starts "total order isolation" section.
   *
   * Replicates the action spec and returns success code, which caller must
   * check. Total order isolation continues until to_execute_end() is called.
   *
   * @param wsrep       provider handle
   * @param conn_id     connection ID
   * @param keys        array of keys
   * @param keys_num    lenght of the array of keys
   * @param action      action buffer array to be executed
   * @param count       action buffer count
   * @param meta        transaction meta data
   *
   * @retval WSREP_OK         cluster commit succeeded
   * @retval WSREP_CONN_FAIL  must close client connection
   * @retval WSREP_NODE_FAIL  must close all connections and reinit
   */
    wsrep_status_t (*to_execute_start)(wsrep_t*                wsrep,
                                       wsrep_conn_id_t         conn_id,
                                       const wsrep_key_t*      keys,
                                       size_t                  keys_num,
                                       const struct wsrep_buf* action,
                                       size_t                  count,
                                       wsrep_trx_meta_t*       meta);

  /*!
   * @brief Ends the total order isolation section.
   *
   * Marks the end of total order isolation. TO locks are freed
   * and other transactions are free to commit from this point on.
   *
   * @param wsrep provider handle
   * @param conn_id connection ID
   *
   * @retval WSREP_OK         cluster commit succeeded
   * @retval WSREP_CONN_FAIL  must close client connection
   * @retval WSREP_NODE_FAIL  must close all connections and reinit
   */
    wsrep_status_t (*to_execute_end)(wsrep_t* wsrep, wsrep_conn_id_t conn_id);

  /*!
   * @brief Collects preordered replication events into a writeset.
   *
   * @param wsrep   wsrep provider handle
   * @param handle  a handle associated with a given writeset
   * @param data    an array of data buffers.
   * @param count   length of data buffer array.
   * @param copy    whether provider needs to make a copy of events.
   *
   * @retval WSREP_OK         cluster-wide commit succeeded
   * @retval WSREP_TRX_FAIL   operation failed (e.g. trx size exceeded limit)
   * @retval WSREP_NODE_FAIL  must close all connections and reinit
   */
    wsrep_status_t (*preordered_collect) (wsrep_t*                 wsrep,
                                          wsrep_po_handle_t*       handle,
                                          const struct wsrep_buf*  data,
                                          size_t                   count,
                                          wsrep_bool_t             copy);

  /*!
   * @brief "Commits" preordered writeset to cluster.
   *
   * The contract is that the writeset will be committed in the same (partial)
   * order this method was called. Frees resources associated with the writeset
   * handle and reinitializes the handle.
   *
   * @param wsrep     wsrep provider handle
   * @param po_handle a handle associated with a given writeset
   * @param source_id ID of the event producer, also serves as the partial order
   *                  or stream ID - events with different source_ids won't be
   *                  ordered with respect to each other.
   * @param flags     WSREP_FLAG_... flags
   * @param pa_range  the number of preceding events this event can be processed
   *                  in parallel with. A value of 0 means strict serial
   *                  processing. Note: commits always happen in wsrep order.
   * @param commit    'true'  to commit writeset to cluster (replicate) or
   *                  'false' to rollback (cancel) the writeset.
   *
   * @retval WSREP_OK         cluster-wide commit succeeded
   * @retval WSREP_TRX_FAIL   operation failed (e.g. NON-PRIMARY component)
   * @retval WSREP_NODE_FAIL  must close all connections and reinit
   */
    wsrep_status_t (*preordered_commit)  (wsrep_t*             wsrep,
                                          wsrep_po_handle_t*   handle,
                                          const wsrep_uuid_t*  source_id,
                                          uint32_t             flags,
                                          int                  pa_range,
                                          wsrep_bool_t         commit);

  /*!
   * @brief Signals to wsrep provider that state snapshot has been sent to
   *        joiner.
   *
   * @param wsrep    provider handle
   * @param state_id state ID
   * @param rcode    0 or negative error code of the operation.
   */
    wsrep_status_t (*sst_sent)(wsrep_t*            wsrep,
                               const wsrep_gtid_t* state_id,
                               int                 rcode);

  /*!
   * @brief Signals to wsrep provider that new state snapshot has been received.
   *        May deadlock if called from sst_prepare_cb.
   *
   * @param wsrep     provider handle
   * @param state_id  state ID
   * @param state     initial state provided by SST donor
   * @param state_len length of state buffer
   * @param rcode     0 or negative error code of the operation.
   */
    wsrep_status_t (*sst_received)(wsrep_t*            wsrep,
                                   const wsrep_gtid_t* state_id,
                                   const void*         state,
                                   size_t              state_len,
                                   int                 rcode);


  /*!
   * @brief Generate request for consistent snapshot.
   *
   * If successfull, this call will generate internally SST request
   * which in turn triggers calling SST donate callback on the nodes
   * specified in donor_spec. If donor_spec is null, callback is
   * called only locally. This call will block until sst_sent is called
   * from callback.
   *
   * @param wsrep      provider handle
   * @param msg        context message for SST donate callback
   * @param msg_len    length of context message
   * @param donor_spec list of snapshot donors
   */
    wsrep_status_t (*snapshot)(wsrep_t*    wsrep,
                               const void* msg,
                               size_t      msg_len,
                               const char* donor_spec);

  /*!
   * @brief Returns an array of status variables.
   *        Array is terminated by Null variable name.
   *
   * @param wsrep provider handle
   * @return array of struct wsrep_status_var.
   */
    struct wsrep_stats_var* (*stats_get) (wsrep_t* wsrep);

  /*!
   * @brief Release resources that might be associated with the array.
   *
   * @param wsrep     provider handle.
   * @param var_array array returned by stats_get().
   */
    void (*stats_free) (wsrep_t* wsrep, struct wsrep_stats_var* var_array);

  /*!
   * @brief Reset some stats variables to inital value, provider-dependent.
   *
   * @param wsrep provider handle.
   */
    void (*stats_reset) (wsrep_t* wsrep);

  /*!
   * @brief Get node information to expose through PFS
   *
   * @param wsrep     provider handle.
   * @param var_array array of node information to populate.
   * @param size      size of array.
   */
    void (*fetch_pfs_info) (wsrep_t* wsrep, wsrep_node_info_t* nodes, uint32_t size);

  /*!
   * @brief Pauses writeset applying/committing.
   *
   * @return global sequence number of the paused state or negative error code.
   */
    wsrep_seqno_t (*pause) (wsrep_t* wsrep);

  /*!
   * @brief Resumes writeset applying/committing.
   */
    wsrep_status_t (*resume) (wsrep_t* wsrep);

  /*!
   * @brief Desynchronize from cluster
   *
   * Effectively turns off flow control for this node, allowing it
   * to fall behind the cluster.
   */
    wsrep_status_t (*desync) (wsrep_t* wsrep);

  /*!
   * @brief Request to resynchronize with cluster.
   *
   * Effectively turns on flow control. Asynchronous - actual synchronization
   * event to be deliverred via sync_cb.
   */
    wsrep_status_t (*resync) (wsrep_t* wsrep);

  /*!
   * @brief Acquire global named lock
   *
   * @param wsrep  wsrep provider handle
   * @param name   lock name
   * @param shared shared or exclusive lock
   * @param owner  64-bit owner ID
   * @param tout   timeout in nanoseconds.
   *               0 - return immediately, -1 wait forever.
   * @return          wsrep status or negative error code
   * @retval -EDEADLK lock was already acquired by this thread
   * @retval -EBUSY   lock was busy
   */
    wsrep_status_t (*lock) (wsrep_t* wsrep,
                            const char* name, wsrep_bool_t shared,
                            uint64_t owner, int64_t tout);

  /*!
   * @brief Release global named lock
   *
   * @param wsrep   wsrep provider handle
   * @param name    lock name
   * @param owner   64-bit owner ID
   * @return        wsrep status or negative error code
   * @retval -EPERM lock does not belong to this owner
   */
    wsrep_status_t (*unlock) (wsrep_t* wsrep, const char* name, uint64_t owner);

  /*!
   * @brief Check if global named lock is locked
   *
   * @param wsrep wsrep provider handle
   * @param name  lock name
   * @param owner if not NULL will contain 64-bit owner ID
   * @param node  if not NULL will contain owner's node UUID
   * @return true if lock is locked
   */
    wsrep_bool_t (*is_locked) (wsrep_t* wsrep, const char* name, uint64_t* conn,
                               wsrep_uuid_t* node);

  /*!
   * wsrep provider name
   */
    const char* provider_name;

  /*!
   * wsrep provider version
   */
    const char* provider_version;

  /*!
   * wsrep provider vendor name
   */
    const char* provider_vendor;

  /*!
   * @brief Frees allocated resources before unloading the library.
   * @param wsrep provider handle
   */
    void (*free)(wsrep_t* wsrep);

    void *dlh;    //!< reserved for future use
    void *ctx;    //!< reserved for implemetation private context
};


/*!
 *
 * @brief Loads wsrep library
 *
 * @param spec   path to wsrep library. If NULL or WSREP_NONE initialises dummy
 *               pass-through implementation.
 * @param hptr   wsrep handle
 * @param log_cb callback to handle loader messages. Otherwise writes to stderr.
 *
 * @return zero on success, errno on failure
 */
int wsrep_load ( const char* spec, wsrep_t** hptr, wsrep_log_cb_t log_cb ) { return WSREP_NOT_IMPLEMENTED; }
void wsrep_unload ( wsrep_t* hptr ) {}

#ifdef __cplusplus
}
#endif

#endif /* WSREP_H */
#endif
