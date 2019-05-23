// cedar -- C++ implementation of Efficiently-updatable Double ARray trie
//  $Id: cedarpp.h 1830 2014-06-16 06:17:42Z ynaga $
// Copyright (c) 2009-2014 Naoki Yoshinaga <ynaga@tkl.iis.u-tokyo.ac.jp>
#ifndef CEDAR_H
#define CEDAR_H

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cassert>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define STATIC_ASSERT(e, msg) typedef char msg[(e) ? 1 : -1]

namespace cedar {
  // typedefs
#if LONG_BIT == 64
  typedef unsigned long       npos_t; // possibly compatible with size_t
#else
  typedef unsigned long long  npos_t;
#endif
  typedef unsigned char       uchar;
  static const npos_t TAIL_OFFSET_MASK = static_cast <npos_t> (0xffffffff);
  static const npos_t NODE_INDEX_MASK  = static_cast <npos_t> (0xffffffff) << 32;
  template <typename T> struct NaN { enum { N1 = -1, N2 = -2 }; };
  template <> struct NaN <float> { enum { N1 = 0x7f800001, N2 = 0x7f800002 }; };
  static const int MAX_ALLOC_SIZE = 1 << 16; // must be divisible by 256
  // dynamic double array
  template <typename value_type,
            const int     NO_VALUE  = NaN <value_type>::N1,
            const int     NO_PATH   = NaN <value_type>::N2,
            const bool    ORDERED   = true,
            const int     MAX_TRIAL = 1,
            const size_t  NUM_TRACKING_NODES = 0>
  class da {
  public:
    enum error_code { CEDAR_NO_VALUE = NO_VALUE, CEDAR_NO_PATH = NO_PATH };
    typedef value_type result_type;
    struct result_pair_type {
      value_type  value;
      size_t      length;  // prefix length
    };
    struct result_triple_type { // for predict ()
      value_type  value;
      size_t      length;  // suffix length
      npos_t      id;      // node id of value
    };
    struct node {
      union { int base; value_type value; }; // negative means prev empty index
      int  check;                            // negative means next empty index
      node (const int base_ = 0, const int check_ = 0)
        : base (base_), check (check_) {}
    };
    struct ninfo {  // x1.5 update speed; +.25 % memory (8n -> 10n)
      uchar  sibling;   // right sibling (= 0 if not exist)
      uchar  child;     // first child
      ninfo () : sibling (0), child (0) {}
    };
    struct block { // a block w/ 256 elements
      int   prev;   // prev block; 3 bytes
      int   next;   // next block; 3 bytes
      short num;    // # empty elements; 0 - 256
      short reject; // minimum # branching failed to locate; soft limit
      int   trial;  // # trial
      int   ehead;  // first empty item
      block () : prev (0), next (0), num (256), reject (257), trial (0), ehead (0) {}
    };
    da () : tracking_node (), _array (0), _tail (0), _tail0 (0), _ninfo (0), _block (0), _bheadF (0), _bheadC (0), _bheadO (0), _capacity (0), _size (0), _quota (0), _quota0 (0), _no_delete (false), _reject () {
      STATIC_ASSERT(sizeof (value_type) <= sizeof (int),
                    value_type_is_not_supported___maintain_a_value_array_by_yourself_and_store_its_index_to_trie
                    );
      _initialize ();
    }
    ~da () { clear (false); }
    size_t capacity   () const { return static_cast <size_t> (_capacity); }
    size_t size       () const { return static_cast <size_t> (_size); }
    size_t length     () const { return static_cast <size_t> (*_length); }
    size_t total_size () const { return sizeof (node) * _size; }
    size_t unit_size  () const { return sizeof (node); }
    size_t nonzero_size () const {
      size_t i = 0;
      for (int to = 0; to < _size; ++to)
        if (_array[to].check >= 0) ++i;
      return i;
    }
    size_t nonzero_length () const {
      size_t i (0), j (0);
      for (int to = 0; to < _size; ++to) {
        const node& n = _array[to];
        if (n.check >= 0 && _array[n.check].base != to && n.base < 0)
          { ++j; for (const char* p = &_tail[-n.base]; *p; ++p) ++i; }
      }
      return i + j * (1 + sizeof (value_type));
    }
    size_t num_keys () const {
      size_t i = 0;
      for (int to = 0; to < _size; ++to) {
        const node& n = _array[to];
        if (n.check >= 0 && (_array[n.check].base == to || n.base < 0)) ++i;
      }
      return i;
    }
    // interfance
    template <typename T>
    T exactMatchSearch (const char* key) const
    { return exactMatchSearch <T> (key, std::strlen (key)); }
    template <typename T>
    T exactMatchSearch (const char* key, size_t len, npos_t from = 0) const {
      union { int i; value_type x; } b;
      size_t pos = 0;
      b.i = _find (key, from, pos, len);
      if (b.i == CEDAR_NO_PATH) b.i = CEDAR_NO_VALUE;
      T result;
      _set_result (&result, b.x, len, from);
      return result;
    }
    template <typename T>
    size_t commonPrefixSearch (const char* key, T* result, size_t result_len) const
    { return commonPrefixSearch (key, result, result_len, std::strlen (key)); }
    template <typename T>
    size_t commonPrefixSearch (const char* key, T* result, size_t result_len, size_t len, npos_t from = 0) const {
      size_t num = 0;
      for (size_t pos = 0; pos < len; ) {
        union { int i; value_type x; } b;
        b.i = _find (key, from, pos, pos + 1);
        if (b.i == CEDAR_NO_VALUE) continue;
        if (b.i == CEDAR_NO_PATH)  return num;
        if (num < result_len) _set_result (&result[num], b.x, pos, from);
        ++num;
      }
      return num;
    }
    // predict key from double array
    template <typename T>
    size_t commonPrefixPredict (const char* key, T* result, size_t result_len)
    { return commonPrefixPredict (key, result, result_len, std::strlen (key)); }
    template <typename T>
    size_t commonPrefixPredict (const char* key, T* result, size_t result_len, size_t len, npos_t from = 0) {
      size_t num (0), pos (0), p (0);
      if (_find (key, from, pos, len) == CEDAR_NO_PATH) return 0;
      union { int i; value_type x; } b;
      const npos_t root = from;
      for (b.i = begin (from, p); b.i != CEDAR_NO_PATH; b.i = next (from, p, root)) {
        if (num < result_len)
          _set_result (&result[num], b.x, p, from);
        ++num;
      }
      return num;
    }
    void suffix (char* key, size_t len, npos_t to) const {
      key[len] = '\0';
      if (const int offset = static_cast <int> (to >> 32)) {
        to &= TAIL_OFFSET_MASK;
        size_t len_tail = std::strlen (&_tail[-_array[to].base]);
        if (len > len_tail) len -= len_tail; else len_tail = len, len = 0;
        std::memcpy (&key[len], &_tail[static_cast <size_t> (offset) - len_tail], len_tail);
      }
      while (len--) {
        const int from = _array[to].check;
        key[len] = static_cast <char> (_array[from].base ^ static_cast <int> (to));
        to = static_cast <npos_t> (from);
      }
    }
    value_type traverse (const char* key, npos_t& from, size_t& pos) const
    { return traverse (key, from, pos, std::strlen (key)); }
    value_type traverse (const char* key, npos_t& from, size_t& pos, size_t len) const {
      union { int i; value_type x; } b;
      b.i = _find (key, from, pos, len);
      return b.x;
    }
    struct empty_callback { void operator () (const int, const int) {} }; // dummy empty function
    value_type& update (const char* key)
    { return update (key, std::strlen (key)); }
    value_type& update (const char* key, size_t len, value_type val = value_type (0))
    { npos_t from (0); size_t pos (0); return update (key, from, pos, len, val); }
    value_type& update (const char* key, npos_t& from, size_t& pos, size_t len, value_type val = value_type (0))
    { empty_callback cf; return update (key, from, pos, len, val, cf); }
    template <typename T>
    value_type& update (const char* key, npos_t& from, size_t& pos, size_t len, value_type val, T& cf) {
      if (! len && ! from)
        _err (__FILE__, __LINE__, "failed to insert zero-length key\n");
#ifndef USE_FAST_LOAD
      if (! _ninfo || ! _block) restore ();
#endif
      npos_t offset = from >> 32;
      if (! offset) { // node on trie
        for (const uchar* const key_ = reinterpret_cast <const uchar*> (key);
             _array[from].base >= 0; ++pos) {
          if (pos == len)
            { const int to = _follow (from, 0, cf); return _array[to].value += val; }
          from = static_cast <size_t> (_follow (from, key_[pos], cf));
        }
        offset = static_cast <npos_t> (-_array[from].base);
      }
      if (offset >= sizeof (int)) { // go to _tail
        const size_t pos_orig = pos;
        char* const tail = &_tail[offset] - pos;
        while (pos < len && key[pos] == tail[pos]) ++pos;
        //
        if (pos == len && tail[pos] == '\0') { // found exact key
          if (const npos_t moved = pos - pos_orig) { // search end on tail
            from &= TAIL_OFFSET_MASK;
            from |= (offset + moved) << 32;
          }
          return *reinterpret_cast <value_type*> (&tail[len + 1]) += val;
        }
        // otherwise, insert the common prefix in tail if any
        if (from >> 32) {
          from &= TAIL_OFFSET_MASK; // reset to update tail offset
          for (npos_t offset_ = static_cast <npos_t> (-_array[from].base);
               offset_ < offset; ) {
            from = static_cast <size_t>
                   (_follow (from, static_cast <uchar> (_tail[offset_]), cf));
            ++offset_;
            // this shows intricacy in debugging updatable double array trie
            if (NUM_TRACKING_NODES) // keep the traversed node (on tail) updated
              for (size_t j = 0; tracking_node[j] != 0; ++j)
                if (tracking_node[j] >> 32 == offset_)
                  tracking_node[j] = static_cast <npos_t> (from);
          }
        }
        for (size_t pos_ = pos_orig; pos_ < pos; ++pos_)
          from = static_cast <size_t>
                 (_follow (from, static_cast <uchar> (key[pos_]), cf));
        npos_t moved = pos - pos_orig;
        if (tail[pos]) { // remember to move offset to existing tail
          const int to_ = _follow (from, static_cast <uchar> (tail[pos]), cf);
          _array[to_].base = - static_cast <int> (offset + ++moved);
          moved -= 1 + sizeof (value_type); // keep record
        }
        moved += offset;
        for (npos_t i = offset; i <= moved; i += 1 + sizeof (value_type)) {
          if (_quota0 == ++*_length0) {
#ifdef USE_EXACT_FIT
            _quota0 += *_length0 >= MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : *_length0;
#else
            _quota0 += _quota0;
#endif
            _realloc_array (_tail0, _quota0, *_length0);
          }
          _tail0[*_length0] = static_cast <int> (i);
        }
        if (pos == len || tail[pos] == '\0') {
          const int to = _follow (from, 0, cf);
          if (pos == len) return _array[to].value += val; // set value on tail
          _array[to].value += *reinterpret_cast <value_type*> (&tail[pos + 1]);
        }
        from = static_cast <size_t> (_follow (from, static_cast <uchar> (key[pos]), cf));
        ++pos;
      }
      const int needed = static_cast <int> (len - pos + 1 + sizeof (value_type));
      if (pos == len && *_length0) { // reuse
        const int offset0 = _tail0[*_length0];
        _tail[offset0] = '\0';
        _array[from].base = -offset0;
        --*_length0;
        return *reinterpret_cast <value_type*> (&_tail[offset0 + 1]) = val;
      }
      if (_quota < *_length + needed) {
#ifdef USE_EXACT_FIT
        _quota += needed > *_length || needed > MAX_ALLOC_SIZE ? needed :
                  (*_length >= MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : *_length);
#else
        _quota += _quota >= needed ? _quota : needed;
#endif
        _realloc_array (_tail, _quota, *_length);
      }
      _array[from].base = -*_length;
      const size_t pos_orig = pos;
      char* const tail = &_tail[*_length] - pos;
      if (pos < len) {
        do tail[pos] = key[pos]; while (++pos < len);
        from |= (static_cast <npos_t> (*_length) + (len - pos_orig)) << 32;
      }
      *_length += needed;
      return *reinterpret_cast <value_type*> (&tail[len + 1]) += val;
    }
    // easy-going erase () without compression
    int erase (const char* key) { return erase (key, std::strlen (key)); }
    int erase (const char* key, size_t len, npos_t from = 0) {
      size_t pos = 0;
      const int i = _find (key, from, pos, len);
      if (i == CEDAR_NO_PATH || i == CEDAR_NO_VALUE) return -1;
      if (from >> 32) from &= TAIL_OFFSET_MASK; // leave tail as is
      bool flag = _array[from].base < 0; // have sibling
      int e = flag ? static_cast <int> (from) : _array[from].base ^ 0;
      from  = _array[e].check;
      do {
        const node& n = _array[from];
        flag = _ninfo[n.base ^ _ninfo[from].child].sibling;
        if (flag) _pop_sibling (from, n.base, static_cast <uchar> (n.base ^ e));
        _push_enode (e);
        e = static_cast <int> (from);
        from = static_cast <size_t> (_array[from].check);
      } while (! flag);
      return 0;
    }
    int build (size_t num, const char** key, const size_t* len = 0, const value_type* val = 0) {
      for (size_t i = 0; i < num; ++i)
        update (key[i], len ? len[i] : std::strlen (key[i]), val ? val[i] : value_type (i));
      return 0;
    }
    template <typename T>
    void dump (T* result, const size_t result_len) {
      union { int i; value_type x; } b;
      size_t num (0), p (0);
      npos_t from = 0;
      for (b.i = begin (from, p); b.i != CEDAR_NO_PATH; b.i = next (from, p))
        if (num < result_len)
          _set_result (&result[num++], b.x, p, from);
        else
          _err (__FILE__, __LINE__, "dump() needs array of length = num_keys()\n");
    }
    void shrink_tail () {
      union { char* tail; int* length; } t;
      const size_t length_
        = static_cast <size_t> (*_length)
        - static_cast <size_t> (*_length0) * (1 + sizeof (value_type));
      t.tail = static_cast <char*> (std::malloc (length_));
      if (! t.tail) _err (__FILE__, __LINE__, "memory allocation failed\n");
      *t.length = static_cast <int> (sizeof (int));
      for (int to = 0; to < _size; ++to) {
        node& n = _array[to];
        if (n.check >= 0 && _array[n.check].base != to && n.base < 0) {
          char* const tail (&t.tail[*t.length]), * const tail_ (&_tail[-n.base]);
          n.base = - *t.length;
          int i = 0; do tail[i] = tail_[i]; while (tail[i++]);
          *reinterpret_cast <value_type*> (&tail[i])
            = *reinterpret_cast <const value_type*> (&tail_[i]);
          *t.length += i + static_cast <int> (sizeof (value_type));
        }
      }
      std::free (_tail);
      _tail = t.tail;
      _realloc_array (_tail,  *_length,  *_length);
      _quota  = *_length;
      _realloc_array (_tail0, 1);
      _quota0 = 1;
    }
    int save (const char* fn, const char* mode, const bool shrink) {
      if (shrink) shrink_tail ();
      return save (fn, mode);
    }
    int save (const char* fn, const char* mode = "wb") const {
      // _test ();
      FILE* fp = std::fopen (fn, mode);
      if (! fp) return -1;
      std::fwrite (_tail,  sizeof (char), static_cast <size_t> (*_length), fp);
      std::fwrite (_array, sizeof (node), static_cast <size_t> (_size), fp);
      std::fclose (fp);
#ifdef USE_FAST_LOAD
      const char* const info
        = std::strcat (std::strcpy (new char[std::strlen (fn) + 5], fn), ".sbl");
      fp = std::fopen (info, mode);
      delete [] info; // resolve memory leak
      if (! fp) return -1;
      std::fwrite (&_bheadF, sizeof (int), 1, fp);
      std::fwrite (&_bheadC, sizeof (int), 1, fp);
      std::fwrite (&_bheadO, sizeof (int), 1, fp);
      std::fwrite (_ninfo, sizeof (ninfo), static_cast <size_t> (_size), fp);
      std::fwrite (_block, sizeof (block), static_cast <size_t> (_size >> 8), fp);
      std::fclose (fp);
#endif
      return 0;
    }
    int open (const char* fn, const char* mode = "rb",
              const size_t offset = 0, size_t size_ = 0) {
      FILE* fp = std::fopen (fn, mode);
      if (! fp) return -1;
      // get size
      if (! size_) {
        if (std::fseek (fp, 0, SEEK_END) != 0) return -1;
        size_ = static_cast <size_t> (std::ftell (fp));
        if (std::fseek (fp, 0, SEEK_SET) != 0) return -1;
      }
      if (size_ <= offset) return -1;
      if (std::fseek (fp, static_cast <long> (offset), SEEK_SET) != 0) return -1;
      int len = 0;
      if (std::fread (&len, sizeof (int), 1, fp) != 1) return -1;
      const size_t length_ = static_cast <size_t> (len);
      if (size_ <= offset + length_) return -1;
      // set array
      clear (false);
      size_ = (size_ - offset - length_) / sizeof (node);
      _array = static_cast <node*>  (std::malloc (sizeof (node)  * size_));
      _tail  = static_cast <char*>  (std::malloc (length_));
      _tail0 = static_cast <int*>   (std::malloc (sizeof (int)));
#ifdef USE_FAST_LOAD
      _ninfo = static_cast <ninfo*> (std::malloc (sizeof (ninfo) * size_));
      _block = static_cast <block*> (std::malloc (sizeof (block) * size_));
      if (! _array || ! _tail || ! _tail0 || ! _ninfo || ! _block)
#else
      if (! _array || ! _tail || ! _tail0)
#endif
        _err (__FILE__, __LINE__, "memory allocation failed\n");
      if (std::fseek (fp, static_cast <long> (offset), SEEK_SET) != 0) return -1;
      if (length_ != std::fread (_tail,  sizeof (char), length_, fp) ||
          size_   != std::fread (_array, sizeof (node), size_,   fp))
        return -1;
      std::fclose (fp);
      _size = static_cast <int> (size_);
      *_length0 = 0;
#ifdef USE_FAST_LOAD
      const char* const info
        = std::strcat (std::strcpy (new char[std::strlen (fn) + 5], fn), ".sbl");
      fp = std::fopen (info, mode);
      delete [] info; // resolve memory leak
      if (! fp) return -1;
      std::fread (&_bheadF, sizeof (int), 1, fp);
      std::fread (&_bheadC, sizeof (int), 1, fp);
      std::fread (&_bheadO, sizeof (int), 1, fp);
      if (size_      != std::fread (_ninfo, sizeof (ninfo), size_, fp) ||
          size_ >> 8 != std::fread (_block, sizeof (block), size_ >> 8, fp))
        return -1;
      std::fclose (fp);
      _capacity = _size;
      _quota  = *_length;
      _quota0 = 1;
#endif
      return 0;
    }
#ifndef USE_FAST_LOAD
    void restore () { // restore information to update
      if (! _block) _restore_block ();
      if (! _ninfo) _restore_ninfo ();
      _capacity = _size;
      _quota  = *_length;
      _quota0 = 1;
    }
#endif
    void set_array (void* p, size_t size_ = 0) { // ad-hoc
      clear (false);
      if (size_)
        size_ = size_ * unit_size () - static_cast <size_t> (*static_cast <int*> (p));
      _tail  = static_cast <char*> (p);
      _array = reinterpret_cast <node*> (_tail + *_length);
      _size  = static_cast <int> (size_ / unit_size () + (size_ % unit_size () ? 1 : 0));
      _no_delete = true;
    }
    const void* array () const { return _array; }
    void clear (const bool reuse = true) {
      if (_no_delete) _array = 0, _tail = 0;
      if (_array) std::free (_array); _array = 0;
      if (_tail)  std::free (_tail);  _tail  = 0;
      if (_tail0) std::free (_tail0); _tail0 = 0;
      if (_ninfo) std::free (_ninfo); _ninfo = 0;
      if (_block) std::free (_block); _block = 0;
      _bheadF = _bheadC = _bheadO = _capacity = _size = _quota = _quota0 = 0;
      if (reuse) _initialize ();
      _no_delete = false;
    }
    // return the first child for a tree rooted by a given node
    int begin (npos_t& from, size_t& len) {
#ifndef USE_FAST_LOAD
      if (! _ninfo) _restore_ninfo ();
#endif
      int base = from >> 32 ? - static_cast <int> (from >> 32) : _array[from].base;
      if (base >= 0) { // on trie
        uchar c = _ninfo[from].child;
        if (! from && ! (c = _ninfo[base ^ c].sibling)) // bug fix
          return CEDAR_NO_PATH; // no entry
        for (; c && base >= 0; ++len) {
          from = static_cast <size_t> (base) ^ c;
          base = _array[from].base;
          c    = _ninfo[from].child;
        }
        if (base >= 0) return _array[base ^ c].base;
      }
      const size_t len_ = std::strlen (&_tail[-base]);
      from &= TAIL_OFFSET_MASK;
      from |= static_cast <npos_t> (static_cast <size_t> (-base) + len_) << 32;
      len += len_;
      return *reinterpret_cast <int*> (&_tail[-base] + len_ + 1);
    }
    // return the next child if any
    int next (npos_t& from, size_t& len, const npos_t root = 0) {
      uchar c = 0;
      if (const int offset = static_cast <int> (from >> 32)) { // on tail
        if (root >> 32) return CEDAR_NO_PATH;
        from &= TAIL_OFFSET_MASK;
        len -= static_cast <size_t> (offset - (-_array[from].base));
      } else
        c    = _ninfo[_array[from].base ^ 0].sibling;
      for (; ! c && from != root; --len) {
        c    = _ninfo[from].sibling;
        from = static_cast <size_t> (_array[from].check);
      }
      if (! c) return CEDAR_NO_PATH;
      return begin (from = static_cast <size_t> (_array[from].base) ^ c, ++len);
    }
    npos_t tracking_node[NUM_TRACKING_NODES + 1];
  private:
    // currently disabled; implement these if you need
    da (const da&);
    da& operator= (const da&);
    node*   _array;
    union { char* _tail;  int* _length;  };
    union { int*  _tail0; int* _length0; };
    ninfo*  _ninfo;
    block*  _block;
    int     _bheadF;  // first block of Full;   0
    int     _bheadC;  // first block of Closed; 0 if no Closed
    int     _bheadO;  // first block of Open;   0 if no Open
    int     _capacity;
    int     _size;
    int     _quota;
    int     _quota0;
    int     _no_delete;
    short   _reject[257];
    //
    static void _err (const char* fn, const int ln, const char* msg)
    { std::fprintf (stderr, "cedar: %s [%d]: %s", fn, ln, msg); std::exit (1); }
    template <typename T>
    static void _realloc_array (T*& p, const int size_n, const int size_p = 0) {
      void* tmp = std::realloc (p, sizeof (T) * static_cast <size_t> (size_n));
      if (! tmp)
        std::free (p), _err (__FILE__, __LINE__, "memory reallocation failed\n");
      p = static_cast <T*> (tmp);
      static const T T0 = T ();
      for (T* q (p + size_p), * const r (p + size_n); q != r; ++q) *q = T0;
    }
    void _initialize () { // initilize the first special block
      _realloc_array (_array, 256, 256);
      _realloc_array (_tail,  sizeof (int));
      _realloc_array (_tail0, 1);
      _realloc_array (_ninfo, 256);
      _realloc_array (_block, 1);
      _array[0] = node (0, -1);
      for (int i = 1; i < 256; ++i)
        _array[i] = node (i == 1 ? -255 : - (i - 1), i == 255 ? -1 : - (i + 1));
      _capacity = _size = 256;
      _block[0].ehead = 1; // bug fix for erase
      _quota  = *_length  = static_cast <int> (sizeof (int));
      _quota0 = 1;
      for (size_t i = 0 ; i <= NUM_TRACKING_NODES; ++i) tracking_node[i] = 0;
      for (short  i = 0; i <= 256; ++i) _reject[i] = i + 1;
    }
    // follow/create edge
    template <typename T>
    int _follow (npos_t& from, const uchar& ilabel, T& cf) {
      int to = 0;
      const int base = _array[from].base;
      uchar label = std::tolower(ilabel);
      if (base < 0 || _array[to = base ^ label].check < 0) {
        to = _pop_enode (base, label, static_cast <int> (from));
        _push_sibling (from, to ^ label, label, base >= 0);
      } else if (_array[to].check != static_cast <int> (from))
        to = _resolve (from, base, label, cf);
      return to;
    }
    // find key from double array
    int _find (const char* key, npos_t& from, size_t& pos, const size_t len) const {
      npos_t offset = from >> 32;
      if (! offset) { // node on trie
        for (const uchar* const key_ = reinterpret_cast <const uchar*> (key);
             _array[from].base >= 0; ) {
          if (pos == len) {
            const node& n = _array[_array[from].base ^ 0];
            if (n.check != static_cast <int> (from)) return CEDAR_NO_VALUE;
            return n.base;
          }
          size_t to = static_cast <size_t> (_array[from].base); to ^= std::tolower(key_[pos]);
          if (_array[to].check != static_cast <int> (from)) return CEDAR_NO_PATH;
          ++pos;
          from = to;
        }
        offset = static_cast <npos_t> (-_array[from].base);
      }
      // switch to _tail to match suffix
      const size_t pos_orig = pos; // start position in reading _tail
      const char* const tail = &_tail[offset] - pos;
      if (pos < len) {
        do if (key[pos] != tail[pos]) break; while (++pos < len);
        if (const npos_t moved = pos - pos_orig) {
          from &= TAIL_OFFSET_MASK;
          from |= (offset + moved) << 32;
        }
        if (pos < len) return CEDAR_NO_PATH; // input > tail, input != tail
      }
      if (tail[pos]) return CEDAR_NO_VALUE;  // input < tail
      return *reinterpret_cast <const int*> (&tail[len + 1]);
    }
#ifndef USE_FAST_LOAD
    void _restore_ninfo () {
      _realloc_array (_ninfo, _size);
      for (int to = 0; to < _size; ++to) {
        const int from = _array[to].check;
        if (from < 0) continue; // skip empty node
        const int base = _array[from].base;
        if (const uchar label = static_cast <uchar> (base ^ to)) // skip leaf
          _push_sibling (static_cast <size_t> (from), base, label,
                         ! from || _ninfo[from].child || _array[base ^ 0].check == from);
      }
    }
    void _restore_block () {
      _realloc_array (_block, _size >> 8);
      _bheadF = _bheadC = _bheadO = 0;
      for (int bi (0), e (0); e < _size; ++bi) { // register blocks to full
        block& b = _block[bi];
        b.num = 0;
        for (; e < (bi << 8) + 256; ++e)
          if (_array[e].check < 0 && ++b.num == 1) b.ehead = e;
        int& head_out = b.num == 1 ? _bheadC : (b.num == 0 ? _bheadF : _bheadO);
        _push_block (bi, head_out, ! head_out && b.num);
      }
    }
#endif
    void _set_result (result_type* x, value_type r, size_t = 0, npos_t = 0) const
    { *x = r; }
    void _set_result (result_pair_type* x, value_type r, size_t l, npos_t = 0) const
    { x->value = r; x->length = l; }
    void _set_result (result_triple_type* x, value_type r, size_t l, npos_t from) const
    { x->value = r; x->length = l; x->id = from; }
    void _pop_block (const int bi, int& head_in, const bool last) {
      if (last) { // last one poped; Closed or Open
        head_in = 0;
      } else {
        const block& b = _block[bi];
        _block[b.prev].next = b.next;
        _block[b.next].prev = b.prev;
        if (bi == head_in) head_in = b.next;
      }
    }
    void _push_block (const int bi, int& head_out, const bool empty) {
      block& b = _block[bi];
      if (empty) { // the destination is empty
        head_out = b.prev = b.next = bi;
      } else { // use most recently pushed
        int& tail_out = _block[head_out].prev;
        b.prev = tail_out;
        b.next = head_out;
        head_out = tail_out = _block[tail_out].next = bi;
      }
    }
    int _add_block () {
      if (_size == _capacity) { // allocate memory if needed
#ifdef USE_EXACT_FIT
        _capacity += _size >= MAX_ALLOC_SIZE ? MAX_ALLOC_SIZE : _size;
#else
        _capacity += _capacity;
#endif
        _realloc_array (_array, _capacity, _capacity);
        _realloc_array (_ninfo, _capacity, _size);
        _realloc_array (_block, _capacity >> 8, _size >> 8);
      }
      _block[_size >> 8].ehead = _size;
      _array[_size] = node (- (_size + 255),  - (_size + 1));
      for (int i = _size + 1; i < _size + 255; ++i)
        _array[i] = node (-(i - 1), -(i + 1));
      _array[_size + 255] = node (- (_size + 254),  -_size);
      _push_block (_size >> 8, _bheadO, ! _bheadO); // append to block Open
      _size += 256;
      return (_size >> 8) - 1;
    }
    // transfer block from one start w/ head_in to one start w/ head_out
    void _transfer_block (const int bi, int& head_in, int& head_out) {
      _pop_block  (bi, head_in, bi == _block[bi].next);
      _push_block (bi, head_out, ! head_out && _block[bi].num);
    }
    // pop empty node from block; never transfer the special block (bi = 0)
    int _pop_enode (const int base, const uchar label, const int from) {
      const int e  = base < 0 ? _find_place () : base ^ label;
      const int bi = e >> 8;
      node&  n = _array[e];
      block& b = _block[bi];
      if (--b.num == 0) {
        if (bi) _transfer_block (bi, _bheadC, _bheadF); // Closed to Full
      } else { // release empty node from empty ring
        _array[-n.base].check = n.check;
        _array[-n.check].base = n.base;
        if (e == b.ehead) b.ehead = -n.check; // set ehead
        if (bi && b.num == 1 && b.trial != MAX_TRIAL) // Open to Closed
          _transfer_block (bi, _bheadO, _bheadC);
      }
      // initialize the released node
      if (label) n.base = -1; else n.value = value_type (0);
      n.check = from;
      if (base < 0) _array[from].base = e ^ label;
      return e;
    }
    // push empty node into empty ring
    void _push_enode (const int e) {
      const int bi = e >> 8;
      block& b = _block[bi];
      if (++b.num == 1) { // Full to Closed
        b.ehead = e;
        _array[e] = node (-e, -e);
        if (bi) _transfer_block (bi, _bheadF, _bheadC); // Full to Closed
      } else {
        const int prev = b.ehead;
        const int next = -_array[prev].check;
        _array[e] = node (-prev, -next);
        _array[prev].check = _array[next].base = -e;
        if (b.num == 2 || b.trial == MAX_TRIAL) { // Closed to Open
          if (bi) _transfer_block (bi, _bheadC, _bheadO);
        }
        b.trial = 0;
      }
      if (b.reject < _reject[b.num]) b.reject = _reject[b.num];
      _ninfo[e] = ninfo (); // reset ninfo; no child, no sibling
    }
    // push label to from's child
    void _push_sibling (const npos_t from, const int base, const uchar label, const bool flag = true) {
      uchar* c = &_ninfo[from].child;
      if (flag && (ORDERED ? label > *c : ! *c))
        do c = &_ninfo[base ^ *c].sibling; while (ORDERED && *c && *c < label);
      _ninfo[base ^ label].sibling = *c, *c = label;
    }
    // pop label from from's child
    void _pop_sibling (const npos_t from, const int base, const uchar label) {
      uchar* c = &_ninfo[from].child;
      while (*c != label) c = &_ninfo[base ^ *c].sibling;
      *c = _ninfo[base ^ label].sibling;
    }
    // check whether to replace branching w/ the newly added node
    bool _consult (const int base_n, const int base_p, uchar c_n, uchar c_p) const {
      do c_n = _ninfo[base_n ^ c_n].sibling, c_p = _ninfo[base_p ^ c_p].sibling;
      while (c_n && c_p);
      return c_p;
    }
    // enumerate (equal to or more than one) child nodes
    uchar* _set_child (uchar* p, const int base, uchar c, const int label = -1) {
      --p;
      if (! c)  { *++p = c; c = _ninfo[base ^ c].sibling; } // 0: terminal
      if (ORDERED)
        while (c && c < label) { *++p = c; c = _ninfo[base ^ c].sibling; }
      if (label != -1) *++p = static_cast <uchar> (label);
      while (c) { *++p = c; c = _ninfo[base ^ c].sibling; }
      return p;
    }
    // explore new block to settle down
    int _find_place () {
      if (_bheadC) return _block[_bheadC].ehead;
      if (_bheadO) return _block[_bheadO].ehead;
      return _add_block () << 8;
    }
    int _find_place (const uchar* const first, const uchar* const last) {
      if (int bi = _bheadO) {
        const int   bz = _block[_bheadO].prev;
        const short nc = static_cast <short> (last - first + 1);
        while (1) { // set candidate block
          block& b = _block[bi];
          if (b.num >= nc && nc < b.reject) // explore configuration
            for (int e = b.ehead;;) {
              const int base = e ^ *first;
              for (const uchar* p = first; _array[base ^ *++p].check < 0; )
                if (p == last) return b.ehead = e; // no conflict
              if ((e = -_array[e].check) == b.ehead) break;
            }
          b.reject = nc;
          if (b.reject < _reject[b.num]) _reject[b.num] = b.reject;
          const int bi_ = b.next;
          if (++b.trial == MAX_TRIAL) _transfer_block (bi, _bheadO, _bheadC);
          if (bi == bz) break;
          bi = bi_;
        }
      }
      return _add_block () << 8;
    }
    // resolve conflict on base_n ^ label_n = base_p ^ label_p
    template <typename T>
    int _resolve (npos_t& from_n, const int base_n, const uchar label_n, T& cf) {
      // examine siblings of conflicted nodes
      const int to_pn  = base_n ^ label_n;
      const int from_p = _array[to_pn].check;
      const int base_p = _array[from_p].base;
      const bool flag // whether to replace siblings of newly added
        = _consult (base_n, base_p, _ninfo[from_n].child, _ninfo[from_p].child);
      uchar child[256];
      uchar* const first = &child[0];
      uchar* const last  =
        flag ? _set_child (first, base_n, _ninfo[from_n].child, label_n)
        : _set_child (first, base_p, _ninfo[from_p].child);
      const int base =
        (first == last ? _find_place () : _find_place (first, last)) ^ *first;
      // replace & modify empty list
      const int from  = flag ? static_cast <int> (from_n) : from_p;
      const int base_ = flag ? base_n : base_p;
      if (flag && *first == label_n) _ninfo[from].child = label_n; // new child
      _array[from].base = base; // new base
      for (const uchar* p = first; p <= last; ++p) { // to_ => to
        const int to  = _pop_enode (base, *p, from);
        const int to_ = base_ ^ *p;
        _ninfo[to].sibling = (p == last ? 0 : *(p + 1));
        if (flag && to_ == to_pn) continue; // skip newcomer (no child)
        cf (to_, to);
        node& n  = _array[to];
        node& n_ = _array[to_];
        if ((n.base = n_.base) > 0 && *p) { // copy base; bug fix
          uchar c = _ninfo[to].child = _ninfo[to_].child;
          do _array[n.base ^ c].check = to; // adjust grand son's check
          while ((c = _ninfo[n.base ^ c].sibling));
        }
        if (! flag && to_ == static_cast <int> (from_n)) // parent node moved
          from_n = static_cast <size_t> (to); // bug fix
        if (! flag && to_ == to_pn) { // the address is immediately used
          _push_sibling (from_n, to_pn ^ label_n, label_n);
          _ninfo[to_].child = 0; // remember to reset child
          if (label_n) n_.base = -1; else n_.value = value_type (0);
          n_.check = static_cast <int> (from_n);
        } else
          _push_enode (to_);
        if (NUM_TRACKING_NODES) // keep the traversed node updated
          for (size_t j = 0; tracking_node[j] != 0; ++j) {
            if (static_cast <int> (tracking_node[j] & TAIL_OFFSET_MASK) == to_) {
              tracking_node[j] &= NODE_INDEX_MASK;
              tracking_node[j] |= static_cast <npos_t> (to);
            }
          }
      }
      return flag ? base ^ label_n : to_pn;
    }
    // test the validity of double array for debug
    void _test (const npos_t from = 0) const {
      const int base = _array[from].base;
      if (base < 0) { // validate tail offset
        assert (*_length >= static_cast <int> (-base + 1 + sizeof (value_type)));
        return;
      }
      uchar c = _ninfo[from].child;
      do {
        if (from) assert (_array[base ^ c].check == static_cast <int> (from));
        if (c) _test (static_cast <npos_t> (base ^ c));
      } while ((c = _ninfo[base ^ c].sibling));
    }
  };
}
#endif
