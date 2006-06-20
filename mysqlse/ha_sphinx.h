/* Copyright (C) 2003 MySQL AB

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/*
  Please read ha_exmple.cc before reading this file.
  Please keep in mind that the sphinx storage engine implements all methods
  that are required to be implemented. handler.h has a full list of methods
  that you can implement.
*/

#ifdef USE_PRAGMA_INTERFACE
#pragma interface			/* gcc class implementation */
#endif


#define SPHINX_QUERY_BUFFER_SIZE 2048 /* 2K is enough */

#if MYSQL_VERSION_ID < 50100
#define TABLE_ARG	st_table
#else
#define TABLE_ARG	st_table_share
#endif


/*
  SPHINX_SHARE is a structure that will be shared amoung all open handlers
  The sphinx implements the minimum of what you will probably need.
*/
typedef struct st_sphinx_share {
  char *table_name;
  char *scheme;
  char *hostname;
  char *socket;
  char *sport;
  char *indexname;
  uint parse_error;
  ushort port;
  uint table_name_length,use_count;
  pthread_mutex_t mutex;
  THR_LOCK lock;
} SPHINX_SHARE;


typedef struct st_query_req {
  uint offset;
  uint limit;
  uint mode;
  uint sort;
  uint min_id;
  uint max_id;
  uint min_tid;
  uint max_tid;
  uint min_gid;
  uint max_gid;
  char *query;
  char *index;
  uint query_len;
  uint index_len;
  uint query_len_code;
  uint index_len_code;
  uint weights_count;
  uint weights_count_code;
  int  *weights;
  uint groups_count;
  uint groups_count_code;
  int  *groups;
} query_req;



typedef struct st_sphinx_repinfo {
  byte *response;
} sphinx_repinfo;

class THD;
  
/*
  Class definition for the storage engine
*/
class ha_sphinx: public handler
{
  THR_LOCK_DATA lock;      /* MySQL lock */
  SPHINX_SHARE *share;    /* Shared lock info */
  String buffer;
  byte byte_buffer[1024];
  int start_of_scan;
  int fd_socket;
  uint count_of_found_recs;
  uint current_pos;
  const byte *current_key;
  uint current_key_len;   
  char *response;
public:
  ha_sphinx(TABLE_ARG *table_arg);
  ~ha_sphinx()
  {
  }
  /* The name that will be used for display purposes */
  const char *table_type() const { return "SPHINX"; }
  /*
    The name of the index type that will be used for display
    don't implement this method unless you really have indexes
   */
  const char *index_type(uint inx) { return "HASH"; }
  const char **bas_ext() const;
  /*
    This is a list of flags that says what the storage engine
    implements. The current table flags are documented in
    handler.h
  */
#if MYSQL_VERSION_ID < 50100
  ulong table_flags() const
#else
  ulonglong table_flags() const
#endif
  {
    return 0;
  }
  /*
    This is a bitmap of flags that says how the storage engine
    implements indexes. The current index flags are documented in
    handler.h. If you do not implement indexes, just return zero
    here.

    part is the key part to check. First key part is 0
    If all_parts it's set, MySQL want to know the flags for the combined
    index up to and including 'part'.
  */
  ulong index_flags(uint inx, uint part, bool all_parts) const
  {
    return 0;
  }
  /*
    unireg.cc will call the following to make sure that the storage engine can
    handle the data it is about to send.

    Return *real* limits of your storage engine here. MySQL will do
    min(your_limits, MySQL_limits) automatically

    There is no need to implement ..._key_... methods if you don't suport
    indexes.
  */
  uint max_supported_record_length() const { return HA_MAX_REC_LENGTH; }
  uint max_supported_keys()          const { return 1; }
  uint max_supported_key_parts()     const { return 1; }
  uint max_supported_key_length()    const { return 1024; }
  /*
    Called in test_quick_select to determine if indexes should be used.
  */
#if MYSQL_VERSION_ID > 50100
  virtual double scan_time() { return (double) (stats.records + stats.deleted) / 20.0+10; }
#else
  virtual double scan_time() { return (double) (records + deleted) / 20.0+10; }
#endif

  /*
    The next method will never be called if you do not implement indexes.
  */
  virtual double read_time(ha_rows rows) { return (double) rows /  20.0+1; }

  /*
    Everything below are methods that we implment in ha_sphinx.cc.

    Most of these methods are not obligatory, skip them and
    MySQL will treat them as not implemented
  */
  int open(const char *name, int mode, uint test_if_locked);    // required
  int close(void);                                              // required

  int do_open_connection();
  int parse_query(char *query, char *query_req);
  int write_row(byte * buf);
  int update_row(const byte * old_data, byte * new_data);
  int delete_row(const byte * buf);
  int index_init(uint    keynr, bool sorted); 
  int index_end(); 
  int index_read(byte *buf, const byte *key,
		  uint key_len, enum ha_rkey_function find_flag);
  int index_read_idx(byte * buf, uint idx, const byte * key,
                     uint key_len, enum ha_rkey_function find_flag);
  int index_next(byte * buf);
  int index_next_same(byte * buf, const byte *key, uint keylen);
  int index_prev(byte * buf);
  int index_first(byte * buf);
  int index_last(byte * buf);
  int get_rec(byte * buf, const byte *key, uint keylen);
  /*
    unlike index_init(), rnd_init() can be called two times
    without rnd_end() in between (it only makes sense if scan=1).
    then the second call should prepare for the new table scan
    (e.g if rnd_init allocates the cursor, second call should
    position it to the start of the table, no need to deallocate
    and allocate it again
  */
  int rnd_init(bool scan);                                      //required
  int rnd_end();
  int rnd_next(byte *buf);                                      //required
  int rnd_pos(byte * buf, byte *pos);                           //required
  void position(const byte *record);                            //required
  void info(uint);                                              //required

  int extra(enum ha_extra_function operation);
  int reset(void);
  int external_lock(THD *thd, int lock_type);                   //required
  int delete_all_rows(void);
  ha_rows records_in_range(uint inx, key_range *min_key,
                           key_range *max_key);
  int delete_table(const char *from);
  int rename_table(const char * from, const char * to);
  int create(const char *name, TABLE *form,
             HA_CREATE_INFO *create_info);                      //required

  THR_LOCK_DATA **store_lock(THD *thd, THR_LOCK_DATA **to,
                             enum thr_lock_type lock_type);     //required
};

#if MYSQL_VERSION_ID < 50100
bool sphinx_show_status(THD* thd);
#endif
