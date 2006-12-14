# = sphinx.rb - Sphinx Client Library
# 
# Author::    Dmytro Shteflyuk <mailto:kpumuk@kpumuk.info>.
# Copyright:: Copyright (c) 2006 Wildbit, LLC
# License::   Distributes under the same terms as Ruby
# Version::   0.1.0
# Website::   http://kpumuk.info/projects/ror-plugins/using-sphinx-search-engine-in-ruby-on-rails
#
# This library is distributed under the terms of the Ruby license.
# You can freely distribute/modify this library.

# ==Sphinx Client Library 
# 
# The Sphinx Client Library is used to communicate with <tt>searchd</tt>
# daemon and get search results from Sphinx.
# 
# ===Usage
# 
#   sphinx = Sphinx.new
#   result = sphinx.query('test')
#   ids = result[:matches].map { |id, value| id }.join(',')
#   posts = Post.find :all, :conditions => "id IN (#{ids})"
#   
#   docs = posts.map { |post| post.body }
#   excerpts = sphinx.build_excerpts(docs, 'index', 'test')
# 
class Sphinx

  # :stopdoc:
  class SphinxError < StandardError; end
  class SphinxConnectError < SphinxError; end
  class SphinxResponseError < SphinxError; end
  class SphinxInternalError < SphinxError; end
  class SphinxTemporaryError < SphinxError; end
  class SphinxUnknownError < SphinxError; end

  # Known searchd commands

  # search command
  SEARCHD_COMMAND_SEARCH  = 0
  # excerpt command
  SEARCHD_COMMAND_EXCERPT = 1

  # Current client-side command implementation versions
  
  # search command version
  VER_COMMAND_SEARCH  = 0x104
  # excerpt command version
  VER_COMMAND_EXCERPT = 0x100
  
  # Known searchd status codes

  # match all query words
  SEARCHD_OK    = 0
  # match all query words
  SEARCHD_ERROR = 1
  # match all query words
  SEARCHD_RETRY = 2
  
  # :startdoc:

  # Known match modes

  # match all query words
  SPH_MATCH_ALL     = 0 
  # match any query word
  SPH_MATCH_ANY     = 1 
  # match this exact phrase
  SPH_MATCH_PHRASE  = 2 
  # match this boolean query
  SPH_MATCH_BOOLEAN = 3 
  # match this extended query
  SPH_MATCH_EXTENDED= 4
  
  # Known sort modes

  # sort by document relevance desc, then by date
  SPH_SORT_RELEVANCE     = 0
  # sort by document date desc, then by relevance desc
  SPH_SORT_ATTR_DESC     = 1
  # sort by document date asc, then by relevance desc
  SPH_SORT_ATTR_ASC      = 2
  # sort by time segments (hour/day/week/etc) desc, then by relevance desc
  SPH_SORT_TIME_SEGMENTS = 3
  # sort by SQL-like expression (eg. "@weight DESC my_attr ASC")
  SPH_SORT_EXTENDED      = 4

  # Known attribute types

  # this attr is just an integer
  SPH_ATTR_INTEGER   = 1
  # this attr is a timestamp
  SPH_ATTR_TIMESTAMP = 2 
  
  # Known grouping functions

  # group by day
  SPH_GROUPBY_DAY   = 0
  # group by week
  SPH_GROUPBY_WEEK  = 1 
  # group by month
  SPH_GROUPBY_MONTH = 2 
  # group by year
  SPH_GROUPBY_YEAR  = 3
  # group by attribute value
  SPH_GROUPBY_ATTR  = 4
  
  # Constructs the Sphinx object and sets options to their default values. 
  def initialize
    @host       = 'localhost'         # searchd host (default is "localhost")
    @port       = 3312                # searchd port (default is 3312)
    @offset     = 0                   # how much records to seek from result-set start (default is 0)
    @limit      = 20                  # how much records to return from result-set starting at offset (default is 20)
    @mode       = SPH_MATCH_ALL       # query matching mode (default is SPH_MATCH_ALL)
    @weights    = []                  # per-field weights (default is 1 for all fields)
    @sort       = SPH_SORT_RELEVANCE  # match sorting mode (default is SPH_SORT_RELEVANCE)
    @sortby     = ''                  # attribute to sort by (defualt is "")
    @min_id     = 0                   # min ID to match (default is 0)
    @max_id     = 0xFFFFFFFF          # max ID to match (default is UINT_MAX)
    @min        = {}                  # attribute name to min-value hash (for range filters)
    @max        = {}                  # attribute name to max-value hash (for range filters)
    @filter     = {}                  # attribute name to values set hash (for values-set filters)
    @groupby    = ''                  # group-by attribute name
    @groupfunc  = SPH_GROUPBY_DAY     # function to pre-process group-by attribute value with
    @maxmatches = 1000                # max matches to retrieve
  
    @error      = ''                  # last error message
    @warning    = ''                  # last warning message
  end

  # Get last error message.
  def last_error
    @error
  end
  
  # Get last warning message.
  def last_warning
    @warning
  end
  
  # Set searchd server.
  def set_server(host, port)
    @host = host
    @port = port
  end
 
  # Set match offset, count, and max number to retrieve.
  def set_limits(offset, limit, max = 0)
    @offset = offset
    @limit = limit
    @maxmatches = max if max > 0
  end
  
  # Set match mode.
  def set_match_mode(mode)
    @mode = mode
  end
  
  # Set sort mode.
  def set_sort_mode(mode, sortby = '')
    @sort = mode
    @sortby = sortby
  end
  
  # Set per-field weights.
  def set_weights(weights)
    @weights = weights
  end
  
  # Set IDs range to match.
  # 
  # Only match those records where document ID is beetwen <tt>min_id</tt> and <tt>max_id</tt> 
  # (including <tt>min_id</tt> and <tt>max_id</tt>).
  def set_id_range(min_id, max_id)
    @min_id = min_id
    @max_id = max_id
  end
  
  # Set values filter.
  # 
  # Only match those records where <tt>attr</tt> column values
  # are in specified set.
  def set_filter(attr, values)
    @filter[attr] = values
  end
  
  # Set range filter.
  # 
  # Only match those records where <tt>attr</tt> column value
  # is beetwen <tt>min</tt> and <tt>max</tt> (including <tt>min</tt> and <tt>max</tt>).
  def set_filter_range(attr, min, max)
    @min[attr] = min
    @max[attr] = max
  end
  
  # Set grouping.
  # 
  # if grouping
  def set_group_by(attr, func)
    @groupby = attr
    @groupfunc = func
  end
  
  # Connect to searchd server and run given search query.
  #
  # * <tt>query</tt> -- query string
  # * <tt>index</tt> -- index name to query, default is "*" which means to query all indexes
  #
  # returns hash which has the following keys on success:
  # 
  # * <tt>:matches</tt> -- hash which maps found document_id to ( "weight", "group" ) hash
  # * <tt>:total</tt> -- total amount of matches retrieved (upto SPH_MAX_MATCHES, see sphinx.h)
  # * <tt>:total_found</tt> -- total amount of matching documents in index
  # * <tt>:time</tt> -- search time
  # * <tt>:words</tt> -- hash which maps query terms (stemmed!) to ( :docs, :hits ) hash
  def query(query, index = '*')
    sock = connect
    
    # build request

    # mode and limits
    req = [@offset, @limit, @mode, @sort].pack('NNNN')
    req << [@sortby.length].pack('N')
    req << @sortby
    # query itself
    req << [query.length].pack('N')
    req << query
    # weights
    req << [@weights.length].pack('N')
    req << @weights.pack('N' * @weights.length)
    # indexes
    req << [index.length].pack('N')
    req << index
    # id range
    req << [@min_id.to_i, @max_id.to_i].pack('NN')
    
    # filters
    req << [@min.length + @filter.length].pack('N')
    @min.each do |attribute, min|
      req << [attribute.length].pack('N')
      req << attribute
      req << [0, min, @max[attribute]].pack('NNN')
    end
    
    @filter.each do |attribute, values|
      req << [attribute.length].pack('N')
      req << attribute
      req << [values.length].pack('N')
      req << values.pack('N' * values.length)
    end
    
    # group-by
    req << [@groupfunc, @groupby.length].pack('NN')
    req << @groupby
    
    # max matches to retrieve
    req << [@maxmatches].pack('N')
    
    # send query, get response
    len = req.length
    # add header
    req = [SEARCHD_COMMAND_SEARCH, VER_COMMAND_SEARCH, len].pack('nnN') + req
    sock.send(req, 0)
    
    response = get_response(sock, VER_COMMAND_SEARCH)
    
    # parse response
    result = {}
    max = response.length # protection from broken response

    #read schema
    p = 0
    fields = []
    attrs = {}
    
    nfields = response[p, 4].unpack('N*').first
    p += 4
    while nfields > 0 and p < max
      nfields -= 1
      len = response[p, 4].unpack('N*').first
      p += 4
      fields << response[p, len]
      p += len
    end
    result[:fields] = fields

    nattrs = response[p, 4].unpack('N*').first
    p += 4
    while nattrs > 0 && p < max
      nattrs -= 1
      len = response[p, 4].unpack('N*').first
      p += 4
      attr = response[p, len]
      p += len
      type = response[p, 4].unpack('N*').first
      p += 4
      attrs[attr.to_sym] = type;
    end
    result[:attrs] = attrs
    
    # read match count
    count = response[p, 4].unpack('N*').first
    p += 4
    
    # read matches
    result[:matches] = {}
    while count > 0 and p < max
      count -= 1
      doc, weight = response[p, 8].unpack('N*N*')
      p += 8

      result[:matches][doc] ||= {}
      result[:matches][doc][:weight] = weight
      attrs.each do |attr, type|
        val = response[p, 4].unpack('N*').first
        p += 4
        result[:matches][doc][:attrs] ||= {}
        result[:matches][doc][:attrs][attr] = val
      end
    end
    result[:total], result[:total_found], result[:time], words = \
      response[p, 16].unpack('N*N*N*N*')
    result[:time] = '%.3f' % (result[:time] / 1000)
    p += 16
    
    result[:words] = {}
    while words > 0 and p < max
      words -= 1
      len = response[p, 4].unpack('N*').first
      p += 4
      word = response[p, len]
      p += len
      docs, hits = response[p, 8].unpack('N*N*')
      p += 8
      result[:words][word] = {:docs => docs, :hits => hits}
    end
    
    result
  end

  # Connect to searchd server and generate exceprts from given documents.
  #
  # * <tt>index</tt> -- a string specifiying the index which settings will be used
  # for stemming, lexing and case folding
  # * <tt>docs</tt> -- an array of strings which represent the documents' contents
  # * <tt>words</tt> -- a string which contains the words to highlight
  # * <tt>opts</tt> is a hash which contains additional optional highlighting parameters.
  # 
  # You can use following parameters:
  # * <tt>:before_match</tt> -- a string to insert before a set of matching words, default is "<b>"
  # * <tt>:after_match</tt> -- a string to insert after a set of matching words, default is "<b>"
  # * <tt>:chunk_separator</tt> -- a string to insert between excerpts chunks, default is " ... "
  # * <tt>:limit</tt> -- max excerpt size in symbols (codepoints), default is 256
  # * <tt>:around</tt> -- how much words to highlight around each match, default is 5
  #
  # Returns an array of string excerpts on success.
  def build_excerpts(docs, index, words, opts = {})
    sock = connect

    # fixup options
    opts[:before_match] ||= '<b>';
    opts[:after_match] ||= '</b>';
    opts[:chunk_separator] ||= ' ... ';
    opts[:limit] ||= 256;
    opts[:around] ||= 5;
    
    # build request
    
    # v.1.0 req
    req = [0, 1].pack('N2'); # mode=0, flags=1 (remove spaces)
    # req index
    req << [index.length].pack('N')
    req << index
    # req words
    req << [words.length].pack('N')
    req << words

    # options
    req << [opts[:before_match].length].pack('N')
    req << opts[:before_match]
    req << [opts[:after_match].length].pack('N')
    req << opts[:after_match]
    req << [opts[:chunk_separator].length].pack('N')
    req << opts[:chunk_separator]
    req << [opts[:limit].to_i, opts[:around].to_i].pack('NN')
    
    # documents
    req << [docs.size].pack('N');
    docs.each do |doc|
      req << [doc.length].pack('N')
      req << doc
    end
    
    # send query, get response
    len = req.length
    # add header
    req = [SEARCHD_COMMAND_EXCERPT, VER_COMMAND_EXCERPT, len].pack('nnN') + req
    sock.send(req, 0)
    
    response = get_response(sock, VER_COMMAND_EXCERPT)
    
    # parse response
    p = 0
    res = []
    rlen = response.length
    docs.each do |doc|
      len = response[p, 4].unpack('N*').first;
      p += 4
      if p + len > rlen
        @error = 'incomplete reply'
        raise SphinxResponseError, @error
      end
      res << response[p, len]
      p += len
    end
    return res;
  end

  # Connect to searchd server.
  def connect
    begin
      sock = TCPSocket.new(@host, @port)
    rescue
      @error = "connection to #{@host}:#{@port} failed"
      raise SphinxConnectError, @error
    end
    
    v = sock.recv(4).unpack('N*').first
    if v < 1
      sock.close
      @error = "expected searchd protocol version 1+, got version '#{v}'"
      raise SphinxConnectError, @error
    end
    
    sock.send([1].pack('N'), 0)
    sock
  end
  private :connect
  
  # get and check response packet from searchd server
  def get_response(sock, client_version)
    header = sock.recv(8)
    status, ver, len = header.unpack('n2N')
    response = ''
    left = len
    while left > 0 do
      begin
        chunk = sock.recv(left)
        if chunk
          response << chunk
          left -= chunk.length
        end
      rescue EOFError
      end
    end
    sock.close

    # check response
    read = response.length
    if not response or read != len
      @error = len \
        ? "failed to read searchd response (status=#{status}, ver=#{ver}, len=#{len}, read=#{read})" \
        : "received zero-sized searchd response"
      raise SphinxResponseError, @error
    end
    
    # check status
    if status == SEARCHD_ERROR
      @error = "searchd error: " + response[4,].to_s
      raise SphinxInternalError, @error
    end

    if status == SEARCHD_RETRY
      @error = "temporary searchd error: " + response[4,]
      raise SphinxTemporaryError, @error
    end

    unless status == SEARCHD_OK
      @error = "unknown status code '#{status}'"
      raise SphinxUnknownError, @error
    end
    
    # check version
    if ver < client_version
      @warning = "searchd command v.%d.%d older than client's v.%d.%d, some options might not work" % \
        ver >> 8, ver & 0xff, client_ver >> 8, client_ver & 0xff
    end
    
    return response
  end
  private :get_response
 
end
