//
// $Id$
//

#ifdef USE_PRAGMA_INTERFACE
#pragma interface // gcc class implementation
#endif


#define SPHINX_QUERY_BUFFER_SIZE	2048		// 2K should be enough


#if MYSQL_VERSION_ID>50100
#define TABLE_ARG	st_table_share
#else
#define TABLE_ARG	st_table
#endif


/// SPHINX_SHARE is a structure that will be shared
/// among all open Sphinx SE handlers
typedef struct st_sphinx_share
{
	char *			table_name;
	char *			scheme;
	char *			hostname;
	char *			socket;
	char *			sport;
	char *			indexname;
	uint			parse_error;
	ushort			port;
	uint			table_name_length,use_count;
	pthread_mutex_t	mutex;
	THR_LOCK		lock;
} SPHINX_SHARE;


/// Sphinx network request
typedef struct st_query_req
{
	uint	offset;
	uint	limit;
	uint	mode;
	uint	sort;
	uint	min_id;
	uint	max_id;
	uint	min_tid;
	uint	max_tid;
	uint	min_gid;
	uint	max_gid;
	char *	query;
	char *	index;
	uint	query_len;
	uint	index_len;
	uint	query_len_code;
	uint	index_len_code;
	uint	weights_count;
	uint	weights_count_code;
	int *	weights;
	uint	groups_count;
	uint	groups_count_code;
	int *	groups;
} query_req;


/// forward decl
class THD;


/// Sphinx SE handler class
class ha_sphinx : public handler
{
protected:
	THR_LOCK_DATA	lock;				///< MySQL lock
	SPHINX_SHARE *	share;				///< shared lock info
	String			buffer;
	byte			byte_buffer[1024];
	int				start_of_scan;
	int				fd_socket;
	uint			count_of_found_recs;
	uint			current_pos;
	const byte *	current_key;
	uint			current_key_len;
	char *			response;

public:
					ha_sphinx ( TABLE_ARG * table_arg );
					~ha_sphinx () {}

	const char *	table_type () const		{ return "SPHINX"; }	///< SE name for display purposes
	const char *	index_type ( uint )		{ return "HASH"; }		///< index type name for display purposes
	const char **	bas_ext () const;								///< my file extensions

	#if MYSQL_VERSION_ID>50100
	ulonglong		table_flags () const	{ return 0; }			///< bitmap of implemented flags (see handler.h for more info)
	#else
	ulong			table_flags () const	{ return 0; }			///< bitmap of implemented flags (see handler.h for more info)
	#endif

	ulong			index_flags ( uint, uint, bool ) const	{ return 0; }	///< bitmap of flags that says how SE implements indexes
	uint			max_supported_record_length () const	{ return HA_MAX_REC_LENGTH; }
	uint			max_supported_keys () const				{ return 1; }
	uint			max_supported_key_parts () const		{ return 1; }
	uint			max_supported_key_length () const		{ return 1024; }

	#if MYSQL_VERSION_ID>50100
	virtual double	scan_time ()	{ return (double)( stats.records+stats.deleted )/20.0 + 10; }	///< called in test_quick_select to determine if indexes should be used
	#else
	virtual double	scan_time ()	{ return (double)( records+deleted )/20.0 + 10; }				///< called in test_quick_select to determine if indexes should be used
	#endif

	virtual double	read_time ( ha_rows rows )	{ return (double)rows/20.0 + 1; }					///< index read time estimate

public:
	int				open ( const char * name, int mode, uint test_if_locked );
	int				close ();

	int				write_row ( byte * buf );
	int				update_row ( const byte * old_data, byte * new_data );
	int				delete_row ( const byte * buf );

	int				index_init ( uint keynr, bool sorted ); 
	int				index_end (); 
	int				index_read ( byte * buf, const byte * key, uint key_len, enum ha_rkey_function find_flag );
	int				index_read_idx ( byte * buf, uint idx, const byte * key, uint key_len, enum ha_rkey_function find_flag );
	int				index_next ( byte * buf );
	int				index_next_same ( byte * buf, const byte * key, uint keylen );
	int				index_prev ( byte * buf );
	int				index_first ( byte * buf );
	int				index_last ( byte * buf );

	int				get_rec ( byte * buf, const byte * key, uint keylen );

	int				rnd_init ( bool scan );
	int				rnd_end ();
	int				rnd_next ( byte * buf );
	int				rnd_pos ( byte * buf, byte * pos );
	void			position ( const byte * record );
#if MYSQL_VERSION_ID>=50114
	int			info ( uint );
#else
	void			info ( uint );
#endif

	int				extra ( enum ha_extra_function operation );
	int				reset ();
	int				external_lock ( THD * thd, int lock_type );
	int				delete_all_rows ();
	ha_rows			records_in_range ( uint inx, key_range * min_key, key_range * max_key );

	int				delete_table ( const char * from );
	int				rename_table ( const char * from, const char * to );
	int				create ( const char * name, TABLE * form, HA_CREATE_INFO * create_info );

	THR_LOCK_DATA **store_lock ( THD * thd, THR_LOCK_DATA ** to, enum thr_lock_type lock_type );

protected:
	int				do_open_connection ();
	int				parse_query ( char * query, char * query_req );
};


#if MYSQL_VERSION_ID < 50100
bool sphinx_show_status ( THD * thd );
#endif

//
// $Id$
//
