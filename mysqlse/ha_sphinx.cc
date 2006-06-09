/* 
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
  ha_sphinx is a  storage engine.  You can enable it
  in your buld by doing the following during your build process:
  ./configure --with-sphinx-storage-engine

*/

#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation        // gcc: Class implementation
#endif

#ifndef __WIN__
#include "mysql_priv.h"
#else
#include "../mysql_priv.h"
#endif

#include <mysys_err.h>
#include <my_sys.h>

#ifndef __WIN__
#include <my_net.h>
#include <netdb.h>
#else
#include <io.h>
#endif

#include "ha_sphinx.h"

#if MYSQL_VERSION_ID > 50100
#include <mysql/plugin.h>
#endif

#define SPHINX_DEBUG 0

#ifndef __WIN__
#if SPHINX_DEBUG > 0
#define DGPRINT(format, args...) fprintf (stderr, format, args)
#else
#define DGPRINT(format, ...)
#endif
#else
inline void DGPRINT (...) {}
#endif


#define DEF_SPHINX_HOST "127.0.0.1"
#define DEF_SPHINX_PORT 3312
#define DEF_INDEX_NAME "*"

#if MYSQL_VERSION_ID > 50100
static handler* sphinx_create_handler(TABLE_SHARE *table, MEM_ROOT *mem_root);
#endif
static int sphinx_init_func();
static bool sphinx_init_func_for_handlerton();
static int sphinx_close_connection(THD *thd);
static int sphinx_panic(enum ha_panic_function flag);
#if MYSQL_VERSION_ID > 50100
static bool sphinx_show_status(THD *thd, stat_print_fn *stat_print, enum ha_stat_type stat_type);
#else
bool sphinx_show_status(THD* thd);
#endif

static const char sphinx_hton_name[]= "SPHINX";
static const char sphinx_hton_comment[]= "Sphinx storage engine";

#if MYSQL_VERSION_ID < 50100
handlerton sphinx_hton= {
#ifdef MYSQL_HANDLERTON_INTERFACE_VERSION
  MYSQL_HANDLERTON_INTERFACE_VERSION,
#endif
  "SPHINX",
  SHOW_OPTION_YES,
  "Sphinx storage engine", 
    DB_TYPE_SPHINX_DB,
  sphinx_init_func_for_handlerton,
  0,       /* slot */
  0,       /* savepoint size. */
  sphinx_close_connection,    /* close_connection */
  NULL,    /* savepoint */
  NULL,    /* rollback to savepoint */
  NULL,    /* release savepoint */
  NULL,    /* commit */
  NULL,    /* rollback */
  NULL,    /* prepare */
  NULL,    /* recover */
  NULL,    /* commit_by_xid */
  NULL,    /* rollback_by_xid */
  NULL,    /* create_cursor_read_view */
  NULL,    /* set_cursor_read_view */
  NULL,    /* close_cursor_read_view */
  HTON_CAN_RECREATE
};
#else
handlerton sphinx_hton;
#endif


/* Variables for sphinx share methods */
static HASH sphinx_open_tables; // Hash used to track open tables
pthread_mutex_t sphinx_mutex;   // This is the mutex we use to init the hash
static int sphinx_init= 0;      // Variable for checking the init state of hash


/*
  Function we use in the creation of our hash to get key.
*/
static byte* sphinx_get_key(SPHINX_SHARE *share,uint *length,
                             my_bool not_used __attribute__((unused)))
{
  *length=share->table_name_length;
  return (byte*) share->table_name;
}

static int sphinx_init_func()
{
  DBUG_ENTER("sphinx_init_func");
  if (!sphinx_init)
  {
    sphinx_init= 1;
    VOID(pthread_mutex_init(&sphinx_mutex,MY_MUTEX_INIT_FAST));
    (void) hash_init(&sphinx_open_tables,system_charset_info,32,0,0,
                     (hash_get_key) sphinx_get_key,0,0);
#if MYSQL_VERSION_ID > 50100
    sphinx_hton.state=   SHOW_OPTION_YES;
    sphinx_hton.db_type= DB_TYPE_DEFAULT;
    sphinx_hton.create=  sphinx_create_handler;
    sphinx_hton.close_connection= sphinx_close_connection;
    sphinx_hton.show_status= sphinx_show_status;
    sphinx_hton.panic=   sphinx_panic;
    sphinx_hton.flags=   HTON_CAN_RECREATE;
#endif
  }
  DBUG_RETURN(0);
}

static int sphinx_done_func()
{
  int error= 0;
  DBUG_ENTER("sphinx_done_func");

  if (sphinx_init)
  {
    sphinx_init= 0;
    if (sphinx_open_tables.records)
      error= 1;
    hash_free(&sphinx_open_tables);
    pthread_mutex_destroy(&sphinx_mutex);
  }
  DBUG_RETURN(0);
}

static bool sphinx_init_func_for_handlerton()
{
  return sphinx_init_func();
}

static int sphinx_close_connection(THD *thd)
{
  /* deallocate common handler data */ 
#if MYSQL_VERSION_ID < 50100
  my_free((gptr) thd->ha_data[sphinx_hton.slot], MYF(MY_ALLOW_ZERO_PTR));
#else  
  void **tmp= thd_ha_data(thd, &sphinx_hton);
  my_free((gptr)*tmp, MYF(MY_ALLOW_ZERO_PTR));
#endif  
  DGPRINT("%s\n","sphinx_close_connection");
  return 0;
}

static int sphinx_panic(enum ha_panic_function flag)
{
  return sphinx_done_func();
}


/* In SHOW STATUS we have 5.0 / 5.1 madness ... */

#if MYSQL_VERSION_ID > 50100
static bool sphinx_show_status(THD *thd, stat_print_fn *stat_print, enum ha_stat_type stat_type)
#else  
bool sphinx_show_status(THD *thd)
#endif  
{
#if MYSQL_VERSION_ID < 50100
  Protocol*		protocol = thd->protocol;
  List<Item> field_list;
#endif  

  char buf1[IO_SIZE];
  uint buf1len;
  char buf2[IO_SIZE];
  uint buf2len= 0;
  uint d1,d2,d3,num_words;
  byte *response;
  uint word_length;
  uint word_docs;
  uint word_hits;
  uint i,current_word_offset= 0;
  String words;
  
  DBUG_ENTER("sphinx_show_status");
#if MYSQL_VERSION_ID < 50100
  if (have_sphinx_db != SHOW_OPTION_YES) {
    my_message(ER_NOT_SUPPORTED_YET,
               "Cannot call SHOW sphinx STATUS because skip-sphinx is defined",
               MYF(0));
    DBUG_RETURN(TRUE);
  }
  response= (byte *) thd->ha_data[sphinx_hton.slot];
#else
  response= (byte *) (*thd_ha_data(thd, &sphinx_hton));
#endif

  DGPRINT("%s\n","show_sphinx_status\n");

  if (response) 
  {
    DGPRINT("response %p\n",response);
    memcpy(&d1, response, 4);
    memcpy(&d2, response + 4, 4);
    memcpy(&d3, response + 8, 4);
    memcpy(&num_words, response + 12, 4);

    d1= ntohl(d1);
    d2= ntohl(d2);
    d3= ntohl(d3);
    num_words= ntohl(num_words);

    buf1len= my_snprintf(buf1, sizeof(buf1), "total: %d, total found: %d, time: %d, words: %d", 
                         d1, d2, d3, num_words);


#if MYSQL_VERSION_ID < 50100
    field_list.push_back(new Item_empty_string("Type",10));
    field_list.push_back(new Item_empty_string("Name",FN_REFLEN));
    field_list.push_back(new Item_empty_string("Status",10));

	if (protocol->send_fields(&field_list, Protocol::SEND_NUM_ROWS |
                                               Protocol::SEND_EOF)) {
		DBUG_RETURN(TRUE);
	}
        protocol->prepare_for_resend();
        protocol->store(STRING_WITH_LEN("SPHINX"), system_charset_info);
        protocol->store(STRING_WITH_LEN("stats"), system_charset_info);
        protocol->store(buf1, buf1len, system_charset_info);
        if (protocol->write()) {

        	DBUG_RETURN(TRUE);
	}
#else    
    stat_print(thd, sphinx_hton_name, strlen(sphinx_hton_name),
               STRING_WITH_LEN("stats"), buf1, buf1len);
#endif    
    if (num_words > 0)
    {
      memset(buf2, 0, IO_SIZE);
      for (i=0; i< num_words; i++)
      {
        memcpy(&word_length, response + 16 + current_word_offset, sizeof(uint));      
        word_length= ntohl(word_length);
        words.set_quick((char *) response + 16 + current_word_offset + sizeof(uint), 
                        word_length, &my_charset_bin);
        DGPRINT("readed word: %s\n", words.ptr());

        memcpy(&word_docs, response + 16 + current_word_offset + sizeof(uint) + word_length, sizeof(uint));      
        memcpy(&word_hits, response + 16 + current_word_offset + 2 * sizeof(uint) + word_length, sizeof(uint));      

        word_docs= ntohl(word_docs);
        word_hits= ntohl(word_hits);

        DGPRINT("readed stats : %d, %d\n", word_docs, word_hits);
        current_word_offset += 3 * sizeof(uint) + word_length; 
        buf2len = my_snprintf(buf2, sizeof(buf2), "%s%s:%d:%d ", buf2, words.ptr(), word_docs, word_hits);
      }
#if MYSQL_VERSION_ID < 50100
        protocol->prepare_for_resend();
        protocol->store(STRING_WITH_LEN("SPHINX"), system_charset_info);
        protocol->store(STRING_WITH_LEN("words"), system_charset_info);
        protocol->store(buf2, buf2len, system_charset_info);
        if (protocol->write()) {

        	DBUG_RETURN(TRUE);
	}
#else
      stat_print(thd, sphinx_hton_name, strlen(sphinx_hton_name),
                 STRING_WITH_LEN("words"), buf2, buf2len);
#endif      
    }
#if MYSQL_VERSION_ID < 50100
    send_eof(thd);
#endif        
  }
  else
  {
    DGPRINT("empty response %p\n",response);
#if MYSQL_VERSION_ID < 50100
    field_list.push_back(new Item_empty_string("Type",10));
    field_list.push_back(new Item_empty_string("Name",FN_REFLEN));
    field_list.push_back(new Item_empty_string("Status",10));

	if (protocol->send_fields(&field_list, Protocol::SEND_NUM_ROWS |
                                               Protocol::SEND_EOF)) {
		DBUG_RETURN(TRUE);
	}
        protocol->prepare_for_resend();
        protocol->store(STRING_WITH_LEN("SPHINX"), system_charset_info);
        protocol->store(STRING_WITH_LEN("stats"), system_charset_info);
        protocol->store(STRING_WITH_LEN("no query has been executed yet"), system_charset_info);
        if (protocol->write()) {

        	DBUG_RETURN(TRUE);
	}
    send_eof(thd);
#endif    

  }
  DBUG_RETURN(FALSE);
}

/* Support next scheme: sphinx://host:port */

static int parse_url(SPHINX_SHARE *share, TABLE *table,  uint table_create_flag)
{ 
  uint error_num= ER_FOREIGN_DATA_STRING_INVALID;
  char buf[SPHINX_QUERY_BUFFER_SIZE];
  int buf_len;
  

  DBUG_ENTER("ha_sphinx::parse_url"); 
  share->port= 0;

#if MYSQL_VERSION_ID < 50100 
#define my_strndup my_strdup_with_length
#endif

  share->hostname= DEF_SPHINX_HOST;
  share->port    = DEF_SPHINX_PORT;
  share->indexname  = DEF_INDEX_NAME;

  if (table->s->connect_string.length == 0)
    goto ok;    

  share->scheme = my_strndup((const byte*)table->s->connect_string.str,
			     table->s->connect_string.length, MYF(0));

  share->scheme[table->s->connect_string.length]= 0;

  DGPRINT("scheme: %s\n", share->scheme);
  if (!(share->hostname= strstr(share->scheme, "://")))
    goto error;

  share->scheme[share->hostname - share->scheme]= '\0';
  
  if (strcmp(share->scheme, "sphinx") != 0)
    goto error;
  
  share->hostname+= 3; 

  if ((share->sport= strchr(share->hostname, ':')))
  {
    share->hostname[share->sport - share->hostname]= '\0';
    share->sport++;
    if (share->sport[0] == '\0')
      share->sport= NULL;
    else 
    {
      if ((share->indexname= strchr(share->sport, '/')))
      {
        share->sport[share->indexname - share->sport]= '\0'; 
        share->indexname++;
      }
      else
        share->indexname= DEF_INDEX_NAME;
      share->port= atoi(share->sport);
    }
  }

ok:
  
  DGPRINT("host:port/index %s:%d/%s\n", share->hostname, share->port, share->indexname);
 
  DBUG_RETURN(0);

error:

  share->parse_error= 1;
  DBUG_RETURN(0);
  
  my_free((gptr) share->scheme, MYF(MY_ALLOW_ZERO_PTR));
  buf_len= min(table->s->connect_string.length,
               SPHINX_QUERY_BUFFER_SIZE-1);
  strmake(buf, table->s->connect_string.str, buf_len);
  my_error(error_num, MYF(0), buf);
  DBUG_RETURN(error_num);
  
}


/*
  Example of simple lock controls. The "share" it creates is structure we will
  pass to each sphinx handler. Do you have to have one of these? Well, you have
  pieces that are used for locking, and they are needed to function.
*/
static SPHINX_SHARE *get_share(const char *table_name, TABLE *table)
{
  SPHINX_SHARE *share;
  uint length;
  char *tmp_name;

  pthread_mutex_lock(&sphinx_mutex);
  length=(uint) strlen(table_name);

  if (!(share=(SPHINX_SHARE*) hash_search(&sphinx_open_tables,
                                           (byte*) table_name,
                                           length)))
  {
    if (!(share=(SPHINX_SHARE *)
          my_multi_malloc(MYF(MY_WME | MY_ZEROFILL),
                          &share, sizeof(*share),
                          &tmp_name, length+1,
                          NullS)))
    {
      pthread_mutex_unlock(&sphinx_mutex);
      return NULL;
    }
    
    share->parse_error= 0;
    parse_url(share, table, 0);
/*      goto error;

    if (parse_url(share, table, 0))
      goto error;
  */
    share->use_count=0;
    share->table_name_length=length;
    share->table_name=tmp_name;
    strmov(share->table_name,table_name);
    if (my_hash_insert(&sphinx_open_tables, (byte*) share))
      goto error;
    thr_lock_init(&share->lock);
    pthread_mutex_init(&share->mutex,MY_MUTEX_INIT_FAST);
  }
  share->use_count++;
  pthread_mutex_unlock(&sphinx_mutex);

  return share;

error:
  pthread_mutex_unlock(&sphinx_mutex);
  my_free((gptr) share, MYF(0));

  return NULL;
}


/*
  Free lock controls. We call this whenever we close a table. If the table had
  the last reference to the share then we free memory associated with it.
*/
static int free_share(SPHINX_SHARE *share)
{
  pthread_mutex_lock(&sphinx_mutex);
  if (!--share->use_count)
  {
    hash_delete(&sphinx_open_tables, (byte*) share);

    my_free((gptr) share->scheme, MYF(MY_ALLOW_ZERO_PTR));
    share->scheme= 0;

    thr_lock_delete(&share->lock);
    pthread_mutex_destroy(&share->mutex);
    my_free((gptr) share, MYF(0));
  }
  pthread_mutex_unlock(&sphinx_mutex);

  return 0;
}

#if MYSQL_VERSION_ID > 50100
static handler* sphinx_create_handler(TABLE_SHARE *table, MEM_ROOT *mem_root)
{
  return new (mem_root) ha_sphinx(table);
}
#endif

ha_sphinx::ha_sphinx(TABLE_ARG *table_arg)
  :handler(&sphinx_hton, table_arg)
{

  buffer.set((char*)byte_buffer, 1024, system_charset_info);

}




/*
  If frm_error() is called then we will use this to to find out what file extentions
  exist for the storage engine. This is also used by the default rename_table and
  delete_table method in handler.cc.
*/
static const char *ha_sphinx_exts[] = {
  NullS
};

const char **ha_sphinx::bas_ext() const
{
  return ha_sphinx_exts;
}


/*
  Used for opening tables. The name will be the name of the file.
  A table is opened when it needs to be opened. For instance
  when a request comes in for a select on the table (tables are not
  open and closed for each request, they are cached).

  Called from handler.cc by handler::ha_open(). The server opens all tables by
  calling ha_open() which then calls the handler specific open().
*/
int ha_sphinx::open(const char *name, int mode, uint test_if_locked)
{
  DBUG_ENTER("ha_sphinx::open");
  if (!(share = get_share(name, table)))
    DBUG_RETURN(1);
  thr_lock_data_init(&share->lock,&lock,NULL);
#if MYSQL_VERSION_ID < 50100
  table->in_use->ha_data[sphinx_hton.slot]= NULL;
#else  
  *thd_ha_data(table->in_use, &sphinx_hton) = NULL;
#endif  
  DBUG_RETURN(0);
}

int ha_sphinx::do_open_connection()
{
  DBUG_ENTER("ha_sphinx::do_open_connection");

  struct sockaddr_in sa;
  in_addr_t     ip_addr;
  int version;
  int my_version = htonl(1);
  /* connect to remore server */
  //struct hostent * hp = gethostbyname("127.0.0.1");
  DGPRINT("%s\n","open connection");
  memset ( &sa, 0, sizeof(sa) );
  sa.sin_family      = AF_INET;


  /* prepare host address */
  
  if ((int) (ip_addr = inet_addr(share->hostname)) != (int) INADDR_NONE)
  { 
	  memcpy_fixed(&sa.sin_addr,&ip_addr,sizeof(ip_addr));
  }
  else
  {
	  int tmp_errno;
	  struct hostent tmp_hostent,*hp;
	  char buff2[GETHOSTBYNAME_BUFF_SIZE];
	  hp = my_gethostbyname_r(share->hostname, &tmp_hostent, buff2, sizeof(buff2),
			          &tmp_errno);
	  if (!hp)
	  { 
		  my_gethostbyname_r_free();
		  push_warning(table->in_use, MYSQL_ERROR::WARN_LEVEL_WARN,
				  ER_CONNECT_TO_FOREIGN_DATA_SOURCE, "Can't resolve hostname");
		  DBUG_RETURN(1);
	  }
	  memcpy(&sa.sin_addr, hp->h_addr,
			  min(sizeof(sa.sin_addr), (size_t) hp->h_length));
	  my_gethostbyname_r_free();
  }

  sa.sin_port        = htons(share->port);

  if ( (fd_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
	  DGPRINT("%s", "client: can't open stream socket\n");
          push_warning(table->in_use, MYSQL_ERROR::WARN_LEVEL_WARN,
                       ER_CONNECT_TO_FOREIGN_DATA_SOURCE, "Can't open client socket");
          DBUG_RETURN(1);
  }

  if (connect(fd_socket, (struct sockaddr *) &sa, sizeof(sa)) < 0)
  {
	  DGPRINT("%s","client: can't connect to sphinx\n");
          push_warning(table->in_use, MYSQL_ERROR::WARN_LEVEL_WARN,
                       ER_CONNECT_TO_FOREIGN_DATA_SOURCE, "Can't connect to sphinx server");
	  DBUG_RETURN(1);
  }

  if (::recv(fd_socket, (char *)&version, sizeof(version), 0) != sizeof(version))
  {
	  DGPRINT("%s","client: can't get version\n");
          push_warning(table->in_use, MYSQL_ERROR::WARN_LEVEL_WARN,
                       ER_CONNECT_TO_FOREIGN_DATA_SOURCE, "Can't get server version");
	  DBUG_RETURN(1);
  }  

  DGPRINT("version %d\n", version);
  if (::send(fd_socket, (char *)&my_version, sizeof(my_version), 0) != sizeof(my_version))
  {
	  DGPRINT("%s","client: can't send version\n");
          push_warning(table->in_use, MYSQL_ERROR::WARN_LEVEL_WARN,
                       ER_CONNECT_TO_FOREIGN_DATA_SOURCE, "Can't send client version");
	  DBUG_RETURN(1);
  }

  DBUG_RETURN(0);
}

/*
  Closes a table. We call the free_share() function to free any resources
  that we have allocated in the "shared" structure.

  Called from sql_base.cc, sql_select.cc, and table.cc.
  In sql_select.cc it is only used to close up temporary tables or during
  the process where a temporary table is converted over to being a
  myisam table.
  For sql_base.cc look at close_data_tables().
*/

int ha_sphinx::close(void)
{
  DBUG_ENTER("ha_sphinx::close");
  DGPRINT("%s","close\n");
  DBUG_RETURN(free_share(share));
}


int ha_sphinx::write_row(byte * buf)
{
  DBUG_ENTER("ha_sphinx::write_row");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


int ha_sphinx::update_row(const byte * old_data, byte * new_data)
{
  DBUG_ENTER("ha_sphinx::update_row");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


int ha_sphinx::delete_row(const byte * buf)
{
  DBUG_ENTER("ha_sphinx::delete_row");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}

int ha_sphinx::index_init(uint    keynr,  /* in: key (index) number */
                          bool sorted)    /* in: 1 if result MUST be sorted according to index */
{
  int     error   = 0;
  DBUG_ENTER("index_init");
  DGPRINT("index_init, index number = %d\n",keynr);
  start_of_scan = 1;
  active_index=keynr;
  DBUG_RETURN(error);
}

int ha_sphinx::index_end()
{
  int     error   = 0;
  DBUG_ENTER("index_end");
  DGPRINT("%s","index_end\n");
  DBUG_RETURN(error);
}

/*
  Positions an index cursor to the index specified in the handle. Fetches the
  row if available. If the key value is null, begin at the first key of the
  index.
*/

static int fill_array(int **weights, char* str)
{
  uint n;
  char *fs;
  DBUG_ENTER("ha_sphinx::fill_array");

  *weights= (int *)my_malloc(strlen(str) * sizeof(int), MYF(0));
  
  DGPRINT("fill array str= %s\n", str);
  n= 0; 
  while(str[0] != '\0')
  {
      if ((fs= strchr(str, ',')))
      {
        str[fs-str]=  '\0';
        (*weights)[n++]= atoi(str);
        DGPRINT("found member: %d\n", (*weights)[n-1]);
        str= fs+1;
      }
      else
      {
        (*weights)[n++]= atoi(str);
        DGPRINT("found member: %d\n", (*weights)[n-1]);
        break;
      }  
       
  }

  DGPRINT("fill array total = %d\n", n);
  DBUG_RETURN(n);  
}

static int fill_temp_query_req(char *field, query_req *req)
{
  char *fn, *fs;
  uint flen;
  DBUG_ENTER("ha_sphinx::fill_temp_query_req");
 
  if ((fs= strchr(field, '='))) 
  { 
     char *str="test status";
     flen= fs - field;
     if (strncmp(field, "query", flen)==0)
     {
       req->query= fs + 1;
       req->query_len= strlen(req->query);
     }
     else if (strncmp(field, "index", flen)==0)
     {
       req->index= fs + 1;
       req->index_len= strlen(req->index);
     }
     else if (strncmp(field, "offset", flen)==0)
     {
       req->offset= atoi(fs + 1);
     }
     else if (strncmp(field, "limit", flen)==0)
     {
       req->limit= atoi(fs + 1);
     }
     else if (strncmp(field, "weights", flen)==0)
     {
       req->weights_count= fill_array(&req->weights, fs + 1);
     }
     else if (strncmp(field, "groups", flen)==0)
     {
       req->groups_count= fill_array(&req->groups, fs + 1);
     }
     else if (strncmp(field, "mode", flen)==0)
     {
       req->mode= atoi(fs + 1);
     }
     else if (strncmp(field, "sort", flen)==0)
     {
       req->sort= atoi(fs + 1);
     }
     else if (strncmp(field, "minid", flen)==0)
     {
       req->min_id= atoi(fs + 1);
     }
     else if (strncmp(field, "maxid", flen)==0)
     {
       req->max_id= atoi(fs + 1);
     }
     else if (strncmp(field, "min_ts", flen)==0)
     {
       req->min_tid= atoi(fs + 1);
     }
     else if (strncmp(field, "max_ts", flen)==0)
     {
       req->max_tid= atoi(fs + 1);
     }
     else if (strncmp(field, "min_gid", flen)==0)
     {
       req->min_gid= atoi(fs + 1);
     }
     else if (strncmp(field, "max_gid", flen)==0)
     {
       req->max_gid= atoi(fs + 1);
     }
  }
  else
  {  // by default let's assume it is query
     if (field[0] != '\0')
     {
       req->query= field;
       req->query_len= strlen(req->query);
     }
  }
     

  DBUG_RETURN(0);  
}

#define COPYBUF(a) memcpy(buffer_req + buf_len, &query_fields.a, sizeof(query_fields.a)); \
  buf_len += sizeof(query_fields.a);  

#define CONVINT(a) query_fields.a= htonl(query_fields.a);

int ha_sphinx::parse_query(char *query, char *buffer_req)
{
  uint n, buf_len;
  char *ap;
  char *current_string= query;
  query_req query_fields;

  DBUG_ENTER("ha_sphinx::parse_query");
  
  DGPRINT("got query: %s\n", query);
 
  memset(&query_fields, 0, sizeof(query_fields)); 
  query_fields.index= share->indexname; // default value
  query_fields.index_len= strlen(share->indexname); // default value
  query_fields.limit= 20; 
  query_fields.max_id= 0xFFFFFFFF;
  query_fields.max_tid= 0xFFFFFFFF;
  query_fields.max_gid= 0xFFFFFFFF;
 
  while (ap= strchr(current_string, ';'))
  {
        current_string[ap - current_string]= '\0';
        fill_temp_query_req(current_string, &query_fields);
	DGPRINT("found: %s\n",  current_string);
        current_string= ap + 1;
  }

  DGPRINT("req->query: %s\n",query_fields.query);
  DGPRINT("req->query_len: %d\n",query_fields.query_len);
  DGPRINT("req->index: %s\n",query_fields.index);
  DGPRINT("req->index_len: %d\n",query_fields.index_len);
  DGPRINT("req->offset: %d\n",query_fields.offset);
  DGPRINT("req->limit: %d\n",query_fields.limit);
  DGPRINT("req->mode: %d\n",query_fields.mode);
  DGPRINT("req->sort: %d\n",query_fields.sort);
  DGPRINT("req->min_id: %d\n",query_fields.min_id);
  DGPRINT("req->max_id: %d\n",query_fields.max_id);


  query_fields.offset= htonl(query_fields.offset);
  query_fields.limit=  htonl(query_fields.limit);
  query_fields.mode=   htonl(query_fields.mode);
  query_fields.sort=   htonl(query_fields.sort);
  query_fields.min_id= htonl(query_fields.min_id);
  query_fields.max_id= htonl(query_fields.max_id);
  query_fields.min_gid= htonl(query_fields.min_gid);
  query_fields.max_gid= htonl(query_fields.max_gid);
  query_fields.min_tid= htonl(query_fields.min_tid);
  query_fields.max_tid= htonl(query_fields.max_tid);
  query_fields.query_len_code= htonl(query_fields.query_len);
  query_fields.index_len_code= htonl(query_fields.index_len);
  query_fields.groups_count_code= htonl(query_fields.groups_count);
  query_fields.weights_count_code= htonl(query_fields.weights_count);
  
  buf_len= 0 ; // fill buffer starting with offset 
  
  COPYBUF(offset);
  COPYBUF(limit);
  COPYBUF(mode);
  COPYBUF(sort);

  
  COPYBUF(groups_count_code);
  DGPRINT("req->groups_count: %d\n",query_fields.groups_count);
  for (n= 0; n < query_fields.groups_count; n++)
  {
    DGPRINT("req->groups: n=%d, val=%d\n", n, query_fields.groups[n]);
    query_fields.groups[n]= htonl(query_fields.groups[n]); 
    memcpy(buffer_req + buf_len, query_fields.groups + n, sizeof(int)); 
    buf_len += sizeof(int);
  }
 
  COPYBUF(query_len_code);
  memcpy(buffer_req + buf_len, query_fields.query, query_fields.query_len); buf_len += query_fields.query_len; // query 

  COPYBUF(weights_count_code);
  DGPRINT("req->weights_count: %d\n",query_fields.weights_count);
  for (n= 0; n < query_fields.weights_count; n++)
  {
    DGPRINT("req->weights: n=%d, val=%d\n", n, query_fields.weights[n]);
    query_fields.weights[n]= htonl(query_fields.weights[n]); 
    memcpy(buffer_req + buf_len, query_fields.weights + n, sizeof(int)); 
    buf_len += sizeof(int);
  }

  COPYBUF(index_len_code);
  memcpy(buffer_req + buf_len, query_fields.index, query_fields.index_len); buf_len += query_fields.index_len; // index 
  COPYBUF(min_id);
  COPYBUF(max_id);
  COPYBUF(min_tid);
  COPYBUF(max_tid);
  COPYBUF(min_gid);
  COPYBUF(max_gid);

  
  my_free( (gptr) query_fields.groups, MYF(MY_ALLOW_ZERO_PTR));
  my_free( (gptr) query_fields.weights, MYF(MY_ALLOW_ZERO_PTR));

  DBUG_RETURN(buf_len);  
 
}

int ha_sphinx::index_read(byte *buf, const byte *key,
                             uint key_len, enum ha_rkey_function find_flag)
{

  char buffer[SPHINX_QUERY_BUFFER_SIZE];
  char *index    ="*";
  uint index_len =strlen(index);
  uint index_len_code =htonl(index_len);
  char *query;
  uint buf_len   =0;
  uint length_of_req   =0;
  uint my_version   =htonl(1);
  short int resp_status;
  short int resp_ver;
  int resp_length=0;
  uint i,count=0;
  short command_search = htons(0x101);
  String varchar;
  uint var_length= uint2korr(key);
  void **tmp_ha_data;

  DBUG_ENTER("ha_sphinx::index_read");
  DGPRINT("%s","index_read\n");

  varchar.set_quick((char*) key + HA_KEY_BLOB_LENGTH,
		  var_length, &my_charset_bin);

 
  current_key = key;
  current_key_len = key_len;
 
  start_of_scan = 1;
//  index_inited = 1;
  if (do_open_connection())
     DBUG_RETURN(HA_ERR_END_OF_FILE);
       
#define START_OFFSET 8

  memset(buffer, 0, SPHINX_QUERY_BUFFER_SIZE);
  memcpy(buffer + 2, &command_search, 2);

  query = my_malloc(var_length + 2, MYF(0));
  memcpy(query, (char*) key + HA_KEY_BLOB_LENGTH, var_length);
  query[var_length]= ';';
  query[var_length+1]= '\0';
  buf_len= parse_query(query, buffer + START_OFFSET);
  my_free(query, MYF(0));

  //buf_len = buf_len - 4;
  DGPRINT("req length: %d\n", buf_len);
  length_of_req = htonl(buf_len);
    
  memcpy(buffer + 4, &length_of_req, sizeof(uint));
  
  length_of_req = ntohl(length_of_req);
  DGPRINT("req length1: %d\n", length_of_req);

  ::send(fd_socket,buffer, buf_len + 8, 0);

  ::recv(fd_socket,(char *)&resp_status,2,0);
  resp_status= ntohs(resp_status);
  
  if (resp_status)
  {

	  ::recv(fd_socket,(char *)&resp_length,4,0);
	  resp_length= ntohl(resp_length);
	  response =(char *)my_malloc(resp_length + 1, MYF(0));
          memset(response, 0, resp_length + 1);
	  ::recv(fd_socket,response,resp_length,0);
	  DGPRINT("res length: %d\n",resp_length); 

          fprintf(stderr, "got error: %s\n", response); 
	  DBUG_RETURN(HA_ERR_END_OF_FILE);

  }  

  ::recv(fd_socket, (char *)&resp_ver, 2, 0);
  ::recv(fd_socket, (char *)&resp_length, 4, 0);
 
  DGPRINT("res status: %d\n",resp_status); 
  DGPRINT("res ver: %d\n",resp_ver); 
  resp_length= ntohl(resp_length);
  DGPRINT("res length: %d\n",resp_length); 
   
  response =(char *)my_malloc(resp_length, MYF(0));

  if (!response)
  {
  	  push_warning(table->in_use, MYSQL_ERROR::WARN_LEVEL_WARN,
				  ER_CONNECT_TO_FOREIGN_DATA_SOURCE, "Wrong response from SPHINX");

	  DBUG_RETURN(HA_ERR_END_OF_FILE);
  }	  

  ::recv(fd_socket, response, resp_length, 0);
  
  memcpy(&count, response, 4);
  count_of_found_recs= ntohl(count);
  DGPRINT("count records: %d\n",count_of_found_recs); 
  
  current_pos= 0;
  
  // set new data for thd->ha_data, it is used in show_status
#if MYSQL_VERSION_ID < 50100
#define TARGET current_thd->ha_data[sphinx_hton.slot]  
#else  
  tmp_ha_data= thd_ha_data(table->in_use, &sphinx_hton);
#define TARGET *tmp_ha_data  
#endif  
  my_free((gptr) TARGET, MYF(MY_ALLOW_ZERO_PTR));
  TARGET= my_malloc(resp_length - count_of_found_recs * 4 * 4 - 4, MYF(0));
  memcpy(TARGET, response + count_of_found_recs * 4 * 4 + 4, resp_length - count_of_found_recs * 4 * 4 - 4);
   
  ::close(fd_socket);
  DBUG_RETURN(get_rec(buf, key, key_len));  
}


/*
  Positions an index cursor to the index specified in key. Fetches the
  row if any.  This is only used to read whole keys.
*/
int ha_sphinx::index_read_idx(byte * buf, uint index, const byte * key,
                               uint key_len __attribute__((unused)),
                               enum ha_rkey_function find_flag
                               __attribute__((unused)))
{
  DBUG_ENTER("ha_sphinx::index_read_idx");
  DGPRINT("%s","index_read_idx\n");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


/*
  Used to read forward through the index.
*/
int ha_sphinx::index_next(byte * buf)
{
  DBUG_ENTER("ha_sphinx::index_next");
  DGPRINT("%s","index_next\n");
  DBUG_RETURN(get_rec(buf,current_key,current_key_len));
}


int ha_sphinx::index_next_same(byte * buf, const byte *key, uint keylen)
{
  DBUG_ENTER("ha_sphinx::index_next");
  DGPRINT("%s","index_next_same\n");
  DBUG_RETURN(get_rec(buf,key,keylen));  
}

int  ha_sphinx::get_rec(byte * buf, const byte *key, uint keylen)
{
  uint fn=0;
  int tmp;
  byte *tmp1;
  TIME tmptime;

#if MYSQL_VERSION_ID > 50100
  my_bitmap_map *org_bitmap;
#endif

  DBUG_ENTER("ha_sphinx::get_rec");


  if (current_pos < count_of_found_recs)
  {
#if MYSQL_VERSION_ID > 50100
    org_bitmap= dbug_tmp_use_all_columns(table, table->write_set);
#endif
    Field **field=table->field;

    /* read and store ID */    

    memcpy(&tmp, response + (1 + current_pos * 4) * 4, sizeof(int));  
    
    tmp1=(byte *)&tmp;
    tmp= ntohl(tmp);
    field[0]->store(tmp, 1);


    /* read and store GID */
    memcpy(&tmp, response + (2 + current_pos * 4) * 4, sizeof(int));  

    tmp1=(byte *)&tmp;
//    DGPRINT("%02x,%02x,%02x,%02x\n",tmp1[0],tmp1[1],tmp1[2],tmp1[3]);

    tmp= ntohl(tmp);
//    DGPRINT("val 2 %d\n",tmp);
    field[1]->store(tmp, 1);

    
    /* read and store TIMESTAMP */
    memcpy(&tmp, response + (3 + current_pos * 4) * 4, sizeof(int));  


    tmp= ntohl(tmp);
//    DGPRINT("val 3 %d\n",tmp);

    longstore(field[2]->ptr,tmp); // way to store TIMESTAMP

    /* read and store WEIGHT */
    memcpy(&tmp, response + (4 + current_pos * 4) * 4, sizeof(int));  

    tmp= ntohl(tmp);
//    DGPRINT("val 4 %d\n",tmp);
    
    field[3]->store(tmp, 1);
//    DGPRINT("%s","last field\n");
   
    /* Store requested query, it's necessary, otherwise mysql goes crazy */ 
    field[4]->set_key_image((char *)key, keylen);
    
    memset(buf, 0, table->s->null_bytes);
    current_pos++;

#if MYSQL_VERSION_ID > 50100
  dbug_tmp_restore_column_map(table->write_set, org_bitmap);
#endif

  }
  else
  {    
    my_free(response, MYF(0));
    DBUG_RETURN(HA_ERR_END_OF_FILE); 
  }
      
  DBUG_RETURN(0);  


}


/*
  Used to read backwards through the index.
*/
int ha_sphinx::index_prev(byte * buf)
{
  DBUG_ENTER("ha_sphinx::index_prev");
  DGPRINT("%s","index_prev\n");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


/*
  index_first() asks for the first key in the index.

  Called from opt_range.cc, opt_sum.cc, sql_handler.cc,
  and sql_select.cc.
*/
int ha_sphinx::index_first(byte * buf)
{
  DBUG_ENTER("ha_sphinx::index_first");
  DGPRINT("%s","index_first\n");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}


/*
  index_last() asks for the last key in the index.

  Called from opt_range.cc, opt_sum.cc, sql_handler.cc,
  and sql_select.cc.
*/
int ha_sphinx::index_last(byte * buf)
{
  DBUG_ENTER("ha_sphinx::index_last");
  DGPRINT("%s","index_last\n");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


int ha_sphinx::rnd_init(bool scan)
{
  DBUG_ENTER("ha_sphinx::rnd_init");
  DGPRINT("%s","rnd_init\n");
  DBUG_RETURN(0);
}

int ha_sphinx::rnd_end()
{
  DBUG_ENTER("ha_sphinx::rnd_end");
  DBUG_RETURN(0);
}

int ha_sphinx::rnd_next(byte *buf)
{
  DBUG_ENTER("ha_sphinx::rnd_next");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}


void ha_sphinx::position(const byte *record)
{
  DBUG_ENTER("ha_sphinx::position");
  DBUG_VOID_RETURN;
}


/*
  This is like rnd_next, but you are given a position to use
  to determine the row. The position will be of the type that you stored in
  ref. You can use ha_get_ptr(pos,ref_length) to retrieve whatever key
  or position you saved when position() was called.
  Called from filesort.cc records.cc sql_insert.cc sql_select.cc sql_update.cc.
*/
int ha_sphinx::rnd_pos(byte * buf, byte *pos)
{
  DBUG_ENTER("ha_sphinx::rnd_pos");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}



void ha_sphinx::info(uint flag)
{
  DBUG_ENTER("ha_sphinx::info");
  if (table->s->keys > 0)
    table->key_info[0].rec_per_key[0]=1;

#if MYSQL_VERSION_ID > 50100
  stats.records= 20;
#else
  records= 20;
#endif

  DBUG_VOID_RETURN;
}


int ha_sphinx::extra(enum ha_extra_function operation)
{
  DBUG_ENTER("ha_sphinx::extra");
  DBUG_RETURN(0);
}


int ha_sphinx::reset(void)
{
  DBUG_ENTER("ha_sphinx::reset");
  DBUG_RETURN(0);
}


int ha_sphinx::delete_all_rows()
{
  DBUG_ENTER("ha_sphinx::delete_all_rows");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}


/*
  First you should go read the section "locking functions for mysql" in
  lock.cc to understand this.
  This create a lock on the table. If you are implementing a storage engine
  that can handle transacations look at ha_berkely.cc to see how you will
  want to goo about doing this. Otherwise you should consider calling flock()
  here.

  Called from lock.cc by lock_external() and unlock_external(). Also called
  from sql_table.cc by copy_data_between_tables().
*/
int ha_sphinx::external_lock(THD *thd, int lock_type)
{
  DBUG_ENTER("ha_sphinx::external_lock");
  DBUG_RETURN(0);
}


THR_LOCK_DATA **ha_sphinx::store_lock(THD *thd,
                                       THR_LOCK_DATA **to,
                                       enum thr_lock_type lock_type)
{
  if (lock_type != TL_IGNORE && lock.type == TL_UNLOCK)
    lock.type=lock_type;
  *to++= &lock;
  return to;
}

int ha_sphinx::delete_table(const char *name)
{
  DBUG_ENTER("ha_sphinx::delete_table");
  /* This is not implemented but we want someone to be able that it works. */
  DBUG_RETURN(0);
}

/*
  Renames a table from one name to another from alter table call.

  If you do not implement this, the default rename_table() is called from
  handler.cc and it will delete all files with the file extentions returned
  by bas_ext().

  Called from sql_table.cc by mysql_rename_table().
*/
int ha_sphinx::rename_table(const char * from, const char * to)
{
  DBUG_ENTER("ha_sphinx::rename_table ");
  DBUG_RETURN(0);
}

/*
  Given a starting key, and an ending key estimate the number of rows that
  will exist between the two. end_key may be empty which in case determine
  if start_key matches any rows.

  Called from opt_range.cc by check_quick_keys().
*/
ha_rows ha_sphinx::records_in_range(uint inx, key_range *min_key,
                                     key_range *max_key)
{
  char buf[128];
  String varchar;
  uint var_length= uint2korr(min_key->key);
  DBUG_ENTER("ha_sphinx::records_in_range");
  varchar.set_quick((char*) min_key->key + HA_KEY_BLOB_LENGTH,
		  var_length, &my_charset_bin);
  DGPRINT("records_in range = %s, %d\n",varchar.ptr(),var_length);
//  DBUG_RETURN(HA_POS_ERROR);                         // low number to force index usage
  DBUG_RETURN(3);                         // low number to force index usage
}


/*
  create() is called to create a database. The variable name will have the name
  of the table. When create() is called you do not need to worry about opening
  the table. Also, the FRM file will have already been created so adjusting
  create_info will not do you any good. You can overwrite the frm file at this
  point if you wish to change the table definition, but there are no methods
  currently provided for doing that.

  Called from handle.cc by ha_create_table().
*/
int ha_sphinx::create(const char *name, TABLE *table_arg,
                       HA_CREATE_INFO *create_info)
{
  DBUG_ENTER("ha_sphinx::create");
  DGPRINT("count of fields %d\n", table_arg->s->fields);
  if (table_arg->s->fields != 5)
  {
    DBUG_RETURN(-1);
  
  }
  DBUG_RETURN(0);
}

#if MYSQL_VERSION_ID > 50100
struct st_mysql_storage_engine sphinx_storage_engine=
{ MYSQL_HANDLERTON_INTERFACE_VERSION, &sphinx_hton };


mysql_declare_plugin(sphinx)
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &sphinx_storage_engine,
  sphinx_hton_name,
  "Sphinx developers",
  sphinx_hton_comment,
  sphinx_init_func, /* Plugin Init */
  sphinx_done_func, /* Plugin Deinit */
  0x0001 /* 0.1 */,
}
mysql_declare_plugin_end;

#endif
