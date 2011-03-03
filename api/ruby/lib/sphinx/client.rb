# = client.rb - Sphinx Client API
# 
# Author::    Dmytro Shteflyuk <mailto:kpumuk@kpumuk.info>.
# Copyright:: Copyright (c) 2006 - 2008 Dmytro Shteflyuk
# License::   Distributes under the same terms as Ruby
# Version::   0.9.9-r1299
# Website::   http://kpumuk.info/projects/ror-plugins/sphinx
#
# This library is distributed under the terms of the Ruby license.
# You can freely distribute/modify this library.

# ==Sphinx Client API
# 
# The Sphinx Client API is used to communicate with <tt>searchd</tt>
# daemon and get search results from Sphinx.
# 
# ===Usage
# 
#   sphinx = Sphinx::Client.new
#   result = sphinx.Query('test')
#   ids = result['matches'].map { |match| match['id'] }.join(',')
#   posts = Post.find :all, :conditions => "id IN (#{ids})"
#   
#   docs = posts.map(&:body)
#   excerpts = sphinx.BuildExcerpts(docs, 'index', 'test')

require 'socket'

module Sphinx
  # :stopdoc:

  class SphinxError < StandardError; end
  class SphinxArgumentError < SphinxError; end
  class SphinxConnectError < SphinxError; end
  class SphinxResponseError < SphinxError; end
  class SphinxInternalError < SphinxError; end
  class SphinxTemporaryError < SphinxError; end
  class SphinxUnknownError < SphinxError; end

  # :startdoc:

  class Client
  
    # :stopdoc:
  
    # Known searchd commands
  
    # search command
    SEARCHD_COMMAND_SEARCH   = 0
    # excerpt command
    SEARCHD_COMMAND_EXCERPT  = 1
    # update command
    SEARCHD_COMMAND_UPDATE   = 2 
    # keywords command
    SEARCHD_COMMAND_KEYWORDS = 3 
  
    # Current client-side command implementation versions
    
    # search command version
    VER_COMMAND_SEARCH   = 0x118
    # excerpt command version
    VER_COMMAND_EXCERPT  = 0x102
    # update command version
    VER_COMMAND_UPDATE   = 0x102
    # keywords command version
    VER_COMMAND_KEYWORDS = 0x100
    
    # Known searchd status codes
  
    # general success, command-specific reply follows
    SEARCHD_OK      = 0
    # general failure, command-specific reply may follow
    SEARCHD_ERROR   = 1
    # temporaty failure, client should retry later
    SEARCHD_RETRY   = 2
    # general success, warning message and command-specific reply follow 
    SEARCHD_WARNING = 3    
    
    # :startdoc:
  
    # Known match modes
  
    # match all query words
    SPH_MATCH_ALL       = 0 
    # match any query word
    SPH_MATCH_ANY       = 1 
    # match this exact phrase
    SPH_MATCH_PHRASE    = 2 
    # match this boolean query
    SPH_MATCH_BOOLEAN   = 3 
    # match this extended query
    SPH_MATCH_EXTENDED  = 4 
    # match all document IDs w/o fulltext query, apply filters
    SPH_MATCH_FULLSCAN  = 5
    # extended engine V2 (TEMPORARY, WILL BE REMOVED IN 0.9.8-RELEASE)
    SPH_MATCH_EXTENDED2 = 6
    
    # Known ranking modes (ext2 only)
  
    # default mode, phrase proximity major factor and BM25 minor one
    SPH_RANK_PROXIMITY_BM25 = 0
    # statistical mode, BM25 ranking only (faster but worse quality)
    SPH_RANK_BM25           = 1
    # no ranking, all matches get a weight of 1
    SPH_RANK_NONE           = 2
    # simple word-count weighting, rank is a weighted sum of per-field keyword occurence counts
    SPH_RANK_WORDCOUNT      = 3
    # phrase proximity
    SPH_RANK_PROXIMITY      = 4
    
    # Known sort modes
  
    # sort by document relevance desc, then by date
    SPH_SORT_RELEVANCE     = 0
    # sort by document date desc, then by relevance desc
    SPH_SORT_ATTR_DESC     = 1
    # sort by document date asc, then by relevance desc
    SPH_SORT_ATTR_ASC      = 2
    # sort by time segments (hour/day/week/etc) desc, then by relevance desc
    SPH_SORT_TIME_SEGMENTS = 3
    # sort by SQL-like expression (eg. "@relevance DESC, price ASC, @id DESC")
    SPH_SORT_EXTENDED      = 4
    # sort by arithmetic expression in descending order (eg. "@id + max(@weight,1000)*boost + log(price)")
    SPH_SORT_EXPR          = 5
    
    # Known filter types
    
    # filter by integer values set
    SPH_FILTER_VALUES      = 0
    # filter by integer range
    SPH_FILTER_RANGE       = 1
    # filter by float range
    SPH_FILTER_FLOATRANGE  = 2
    
    # Known attribute types
  
    # this attr is just an integer
    SPH_ATTR_INTEGER   = 1
    # this attr is a timestamp
    SPH_ATTR_TIMESTAMP = 2
    # this attr is an ordinal string number (integer at search time, 
    # specially handled at indexing time)
    SPH_ATTR_ORDINAL   = 3
    # this attr is a boolean bit field
    SPH_ATTR_BOOL      = 4
    # this attr is a float
    SPH_ATTR_FLOAT     = 5
    # signed 64-bit integer
    SPH_ATTR_BIGINT    = 6
	# string
	SPH_ATTR_STRING		= 7
    # this attr has multiple values (0 or more)
    SPH_ATTR_MULTI     = 0x40000000
    
    # Known grouping functions
  
    # group by day
    SPH_GROUPBY_DAY      = 0
    # group by week
    SPH_GROUPBY_WEEK     = 1 
    # group by month
    SPH_GROUPBY_MONTH    = 2 
    # group by year
    SPH_GROUPBY_YEAR     = 3
    # group by attribute value
    SPH_GROUPBY_ATTR     = 4
    # group by sequential attrs pair
    SPH_GROUPBY_ATTRPAIR = 5
    
    # Constructs the <tt>Sphinx::Client</tt> object and sets options to their default values. 
    def initialize
      # per-client-object settings
      @host          = 'localhost'             # searchd host (default is "localhost")
      @port          = 9312                    # searchd port (default is 9312)
      
      # per-query settings
      @offset        = 0                       # how many records to seek from result-set start (default is 0)
      @limit         = 20                      # how many records to return from result-set starting at offset (default is 20)
      @mode          = SPH_MATCH_ALL           # query matching mode (default is SPH_MATCH_ALL)
      @weights       = []                      # per-field weights (default is 1 for all fields)
      @sort          = SPH_SORT_RELEVANCE      # match sorting mode (default is SPH_SORT_RELEVANCE)
      @sortby        = ''                      # attribute to sort by (defualt is "")
      @min_id        = 0                       # min ID to match (default is 0, which means no limit)
      @max_id        = 0                       # max ID to match (default is 0, which means no limit)
      @filters       = []                      # search filters
      @groupby       = ''                      # group-by attribute name
      @groupfunc     = SPH_GROUPBY_DAY         # function to pre-process group-by attribute value with
      @groupsort     = '@group desc'           # group-by sorting clause (to sort groups in result set with)
      @groupdistinct = ''                      # group-by count-distinct attribute
      @maxmatches    = 1000                    # max matches to retrieve
      @cutoff        = 0                       # cutoff to stop searching at (default is 0)
      @retrycount    = 0                       # distributed retries count
      @retrydelay    = 0                       # distributed retries delay
      @anchor        = []                      # geographical anchor point
      @indexweights  = []                      # per-index weights
      @ranker        = SPH_RANK_PROXIMITY_BM25 # ranking mode (default is SPH_RANK_PROXIMITY_BM25)
      @maxquerytime  = 0                       # max query time, milliseconds (default is 0, do not limit) 
      @fieldweights  = {}                      # per-field-name weights
      @overrides     = []                      # per-query attribute values overrides
      @select        = '*'                     # select-list (attributes or expressions, with optional aliases)
    
      # per-reply fields (for single-query case)
      @error         = ''                      # last error message
      @warning       = ''                      # last warning message
      
      @reqs          = []                      # requests storage (for multi-query case)
      @mbenc         = ''                      # stored mbstring encoding
    end
  
    # Get last error message.
    def GetLastError
      @error
    end
    
    # Get last warning message.
    def GetLastWarning
      @warning
    end
    
    # Set searchd host name (string) and port (integer).
    def SetServer(host, port)
      assert { host.instance_of? String }
      assert { port.instance_of? Fixnum }

      @host = host
      @port = port
    end
   
    # Set offset and count into result set,
    # and optionally set max-matches and cutoff limits.
    def SetLimits(offset, limit, max = 0, cutoff = 0)
      assert { offset.instance_of? Fixnum }
      assert { limit.instance_of? Fixnum }
      assert { max.instance_of? Fixnum }
      assert { offset >= 0 }
      assert { limit > 0 }
      assert { max >= 0 }

      @offset = offset
      @limit = limit
      @maxmatches = max if max > 0
      @cutoff = cutoff if cutoff > 0
    end
    
    # Set maximum query time, in milliseconds, per-index,
    # integer, 0 means "do not limit"
    def SetMaxQueryTime(max)
      assert { max.instance_of? Fixnum }
      assert { max >= 0 }
      @maxquerytime = max
    end
    
    # Set matching mode.
    def SetMatchMode(mode)
      assert { mode == SPH_MATCH_ALL \
            || mode == SPH_MATCH_ANY \
            || mode == SPH_MATCH_PHRASE \
            || mode == SPH_MATCH_BOOLEAN \
            || mode == SPH_MATCH_EXTENDED \
            || mode == SPH_MATCH_FULLSCAN \
            || mode == SPH_MATCH_EXTENDED2 }

      @mode = mode
    end
    
    # Set ranking mode.
    def SetRankingMode(ranker)
      assert { ranker == SPH_RANK_PROXIMITY_BM25 \
            || ranker == SPH_RANK_BM25 \
            || ranker == SPH_RANK_NONE \
            || ranker == SPH_RANK_WORDCOUNT \
            || ranker == SPH_RANK_PROXIMITY }

      @ranker = ranker
    end
    
    # Set matches sorting mode.
    def SetSortMode(mode, sortby = '')
      assert { mode == SPH_SORT_RELEVANCE \
            || mode == SPH_SORT_ATTR_DESC \
            || mode == SPH_SORT_ATTR_ASC \
            || mode == SPH_SORT_TIME_SEGMENTS \
            || mode == SPH_SORT_EXTENDED \
            || mode == SPH_SORT_EXPR }
      assert { sortby.instance_of? String }
      assert { mode == SPH_SORT_RELEVANCE || !sortby.empty? }

      @sort = mode
      @sortby = sortby
    end
    
    # Bind per-field weights by order.
    #
    # DEPRECATED; use SetFieldWeights() instead.
    def SetWeights(weights)
      assert { weights.instance_of? Array }
      weights.each do |weight|
        assert { weight.instance_of? Fixnum }
      end

      @weights = weights
    end

    # Bind per-field weights by name.
    #
    # Takes string (field name) to integer name (field weight) hash as an argument.
    # * Takes precedence over SetWeights().
    # * Unknown names will be silently ignored.
    # * Unbound fields will be silently given a weight of 1.
    def SetFieldWeights(weights)
      assert { weights.instance_of? Hash }
      weights.each do |name, weight|
        assert { name.instance_of? String }
        assert { weight.instance_of? Fixnum }
      end

      @fieldweights = weights
    end
    
    # Bind per-index weights by name.
    def SetIndexWeights(weights)
      assert { weights.instance_of? Hash }
      weights.each do |index, weight|
        assert { index.instance_of? String }
        assert { weight.instance_of? Fixnum }
      end
      
      @indexweights = weights
    end
    
    # Set IDs range to match.
    # 
    # Only match records if document ID is beetwen <tt>min_id</tt> and <tt>max_id</tt> (inclusive). 
    def SetIDRange(min, max)
      assert { min.instance_of?(Fixnum) or min.instance_of?(Bignum) }
      assert { max.instance_of?(Fixnum) or max.instance_of?(Bignum) }
      assert { min <= max }

      @min_id = min
      @max_id = max
    end
    
    # Set values filter.
    # 
    # Only match those records where <tt>attribute</tt> column values
    # are in specified set.
    def SetFilter(attribute, values, exclude = false)
      assert { attribute.instance_of? String }
      assert { values.instance_of? Array }
      assert { !values.empty? }

      if values.instance_of?(Array) && values.size > 0
        values.each do |value|
          assert { value.instance_of? Fixnum }
        end
      
        @filters << { 'type' => SPH_FILTER_VALUES, 'attr' => attribute, 'exclude' => exclude, 'values' => values }
      end
    end
    
    # Set range filter.
    # 
    # Only match those records where <tt>attribute</tt> column value
    # is beetwen <tt>min</tt> and <tt>max</tt> (including <tt>min</tt> and <tt>max</tt>).
    def SetFilterRange(attribute, min, max, exclude = false)
      assert { attribute.instance_of? String }
      assert { min.instance_of? Fixnum or min.instance_of? Bignum }
      assert { max.instance_of? Fixnum or max.instance_of? Bignum }
      assert { min <= max }
    
      @filters << { 'type' => SPH_FILTER_RANGE, 'attr' => attribute, 'exclude' => exclude, 'min' => min, 'max' => max }
    end
    
    # Set float range filter.
    #
    # Only match those records where <tt>attribute</tt> column value
    # is beetwen <tt>min</tt> and <tt>max</tt> (including <tt>min</tt> and <tt>max</tt>).
    def SetFilterFloatRange(attribute, min, max, exclude = false)
      assert { attribute.instance_of? String }
      assert { min.instance_of? Float }
      assert { max.instance_of? Float }
      assert { min <= max }
    
      @filters << { 'type' => SPH_FILTER_FLOATRANGE, 'attr' => attribute, 'exclude' => exclude, 'min' => min, 'max' => max }
    end
    
    # Setup anchor point for geosphere distance calculations.
    #
    # Required to use <tt>@geodist</tt> in filters and sorting
    # distance will be computed to this point. Latitude and longitude 
    # must be in radians.
    #
    # * <tt>attrlat</tt> -- is the name of latitude attribute
    # * <tt>attrlong</tt> -- is the name of longitude attribute
    # * <tt>lat</tt> -- is anchor point latitude, in radians
    # * <tt>long</tt> -- is anchor point longitude, in radians
    def SetGeoAnchor(attrlat, attrlong, lat, long)
      assert { attrlat.instance_of? String }
      assert { attrlong.instance_of? String }
      assert { lat.instance_of? Float }
      assert { long.instance_of? Float }

      @anchor = { 'attrlat' => attrlat, 'attrlong' => attrlong, 'lat' => lat, 'long' => long }
    end
    
    # Set grouping attribute and function.
    #
    # In grouping mode, all matches are assigned to different groups
    # based on grouping function value.
    #
    # Each group keeps track of the total match count, and the best match
    # (in this group) according to current sorting function.
    #
    # The final result set contains one best match per group, with
    # grouping function value and matches count attached.
    #
    # Groups in result set could be sorted by any sorting clause,
    # including both document attributes and the following special
    # internal Sphinx attributes:
    #
    # * @id - match document ID;
    # * @weight, @rank, @relevance -  match weight;
    # * @group - groupby function value;
    # * @count - amount of matches in group.
    #
    # the default mode is to sort by groupby value in descending order,
    # ie. by '@group desc'.
    #
    # 'total_found' would contain total amount of matching groups over
    # the whole index.
    #
    # WARNING: grouping is done in fixed memory and thus its results
    # are only approximate; so there might be more groups reported
    # in total_found than actually present. @count might also
    # be underestimated. 
    #
    # For example, if sorting by relevance and grouping by "published"
    # attribute with SPH_GROUPBY_DAY function, then the result set will
    # contain one most relevant match per each day when there were any
    # matches published, with day number and per-day match count attached,
    # and sorted by day number in descending order (ie. recent days first).
    def SetGroupBy(attribute, func, groupsort = '@group desc')
      assert { attribute.instance_of? String }
      assert { groupsort.instance_of? String }
      assert { func == SPH_GROUPBY_DAY \
            || func == SPH_GROUPBY_WEEK \
            || func == SPH_GROUPBY_MONTH \
            || func == SPH_GROUPBY_YEAR \
            || func == SPH_GROUPBY_ATTR \
            || func == SPH_GROUPBY_ATTRPAIR }

      @groupby = attribute
      @groupfunc = func
      @groupsort = groupsort
    end
    
    # Set count-distinct attribute for group-by queries.
    def SetGroupDistinct(attribute)
      assert { attribute.instance_of? String }
      @groupdistinct = attribute
    end
    
    # Set distributed retries count and delay.
    def SetRetries(count, delay = 0)
      assert { count.instance_of? Fixnum }
      assert { delay.instance_of? Fixnum }
      
      @retrycount = count
      @retrydelay = delay
    end
    
    # Set attribute values override
    #
	  # There can be only one override per attribute.
	  # +values+ must be a hash that maps document IDs to attribute values.
	  def SetOverride(attrname, attrtype, values)
      assert { attrname.instance_of? String }
      assert { [SPH_ATTR_INTEGER, SPH_ATTR_TIMESTAMP, SPH_ATTR_BOOL, SPH_ATTR_FLOAT, SPH_ATTR_BIGINT].include?(attrtype) }
      assert { values.instance_of? Hash }

      @overrides << { 'attr' => attrname, 'type' => attrtype, 'values' => values }
	  end

    # Set select-list (attributes or expressions), SQL-like syntax.
    def SetSelect(select)
		  assert { select.instance_of? String }
		  @select = select
		end
    
    # Clear all filters (for multi-queries).
    def ResetFilters
      @filters = []
      @anchor = []
    end
    
    # Clear groupby settings (for multi-queries).
    def ResetGroupBy
      @groupby       = ''
      @groupfunc     = SPH_GROUPBY_DAY
      @groupsort     = '@group desc'
      @groupdistinct = ''
    end
    
    # Clear all attribute value overrides (for multi-queries).
    def ResetOverrides
      @overrides = []
    end
    
    # Connect to searchd server and run given search query.
    #
    # <tt>query</tt> is query string

    # <tt>index</tt> is index name (or names) to query. default value is "*" which means
    # to query all indexes. Accepted characters for index names are letters, numbers,
    # dash, and underscore; everything else is considered a separator. Therefore,
    # all the following calls are valid and will search two indexes:
    #
    #   sphinx.Query('test query', 'main delta')
    #   sphinx.Query('test query', 'main;delta')
    #   sphinx.Query('test query', 'main, delta')
    #
    # Index order matters. If identical IDs are found in two or more indexes,
    # weight and attribute values from the very last matching index will be used
    # for sorting and returning to client. Therefore, in the example above,
    # matches from "delta" index will always "win" over matches from "main".
    #
    # Returns false on failure.
    # Returns hash which has the following keys on success:
    # 
    # * <tt>'matches'</tt> -- array of hashes {'weight', 'group', 'id'}, where 'id' is document_id.
    # * <tt>'total'</tt> -- total amount of matches retrieved (upto SPH_MAX_MATCHES, see sphinx.h)
    # * <tt>'total_found'</tt> -- total amount of matching documents in index
    # * <tt>'time'</tt> -- search time
    # * <tt>'words'</tt> -- hash which maps query terms (stemmed!) to ('docs', 'hits') hash
    def Query(query, index = '*', comment = '')
      assert { @reqs.empty? }
      @reqs = []
      
      self.AddQuery(query, index, comment)
      results = self.RunQueries
      
      # probably network error; error message should be already filled
      return false unless results.instance_of?(Array)
      
      @error = results[0]['error']
      @warning = results[0]['warning']
      
      return false if results[0]['status'] == SEARCHD_ERROR
      return results[0]
    end
    
    # Add query to batch.
    #
    # Batch queries enable searchd to perform internal optimizations,
    # if possible; and reduce network connection overheads in all cases.
    #
    # For instance, running exactly the same query with different
    # groupby settings will enable searched to perform expensive
    # full-text search and ranking operation only once, but compute
    # multiple groupby results from its output.
    #
    # Parameters are exactly the same as in <tt>Query</tt> call.
    # Returns index to results array returned by <tt>RunQueries</tt> call.
    def AddQuery(query, index = '*', comment = '')
      # build request
  
      # mode and limits
      request = Request.new
      request.put_int @offset, @limit, @mode, @ranker, @sort
      request.put_string @sortby
      # query itself
      request.put_string query
      # weights
      request.put_int_array @weights
      # indexes
      request.put_string index
      # id64 range marker
      request.put_int 1
      # id64 range
      request.put_int64 @min_id.to_i, @max_id.to_i 
      
      # filters
      request.put_int @filters.length
      @filters.each do |filter|
        request.put_string filter['attr']
        request.put_int filter['type']

        case filter['type']
          when SPH_FILTER_VALUES
            request.put_int64_array filter['values']
          when SPH_FILTER_RANGE
            request.put_int64 filter['min'], filter['max']
          when SPH_FILTER_FLOATRANGE
            request.put_float filter['min'], filter['max']
          else
            raise SphinxInternalError, 'Internal error: unhandled filter type'
        end
        request.put_int filter['exclude'] ? 1 : 0
      end
      
      # group-by clause, max-matches count, group-sort clause, cutoff count
      request.put_int @groupfunc
      request.put_string @groupby
      request.put_int @maxmatches
      request.put_string @groupsort
      request.put_int @cutoff, @retrycount, @retrydelay
      request.put_string @groupdistinct
      
      # anchor point
      if @anchor.empty?
        request.put_int 0
      else
        request.put_int 1
        request.put_string @anchor['attrlat'], @anchor['attrlong']
        request.put_float @anchor['lat'], @anchor['long']
      end
      
      # per-index weights
      request.put_int @indexweights.length
      @indexweights.each do |idx, weight|
        request.put_string idx
        request.put_int weight
      end
      
      # max query time
      request.put_int @maxquerytime
      
      # per-field weights
      request.put_int @fieldweights.length
      @fieldweights.each do |field, weight|
        request.put_string field
        request.put_int weight
      end
      
      # comment
      request.put_string comment
      
      # attribute overrides
      request.put_int @overrides.length
      for entry in @overrides do
        request.put_string entry['attr']
        request.put_int entry['type'], entry['values'].size
        entry['values'].each do |id, val|
          assert { id.instance_of?(Fixnum) || id.instance_of?(Bignum) }
          assert { val.instance_of?(Fixnum) || val.instance_of?(Bignum) || val.instance_of?(Float) }
          
          request.put_int64 id
          case entry['type']
            when SPH_ATTR_FLOAT
              request.put_float val
            when SPH_ATTR_BIGINT
              request.put_int64 val
            else
              request.put_int val
          end
        end
      end
      
      # select-list
      request.put_string @select
      
      # store request to requests array
      @reqs << request.to_s;
      return @reqs.length - 1
    end
    
    # Run queries batch.
    #
    # Returns an array of result sets on success.
    # Returns false on network IO failure.
    #
    # Each result set in returned array is a hash which containts
    # the same keys as the hash returned by <tt>Query</tt>, plus:
    #
    # * <tt>'error'</tt> -- search error for this query
    # * <tt>'words'</tt> -- hash which maps query terms (stemmed!) to ( "docs", "hits" ) hash
    def RunQueries
      if @reqs.empty?
        @error = 'No queries defined, issue AddQuery() first'
        return false
      end

      req = @reqs.join('')
      nreqs = @reqs.length
      @reqs = []
      response = PerformRequest(:search, req, nreqs)
     
      # parse response
      begin
        results = []
        ires = 0
        while ires < nreqs
          ires += 1
          result = {}
          
          result['error'] = ''
          result['warning'] = ''
          
          # extract status
          status = result['status'] = response.get_int
          if status != SEARCHD_OK
            message = response.get_string
            if status == SEARCHD_WARNING
              result['warning'] = message
            else
              result['error'] = message
              results << result
              next
            end
          end
      
          # read schema
          fields = []
          attrs = {}
          attrs_names_in_order = []
          
          nfields = response.get_int
          while nfields > 0
            nfields -= 1
            fields << response.get_string
          end
          result['fields'] = fields
      
          nattrs = response.get_int
          while nattrs > 0
            nattrs -= 1
            attr = response.get_string
            type = response.get_int
            attrs[attr] = type
            attrs_names_in_order << attr
          end
          result['attrs'] = attrs
          
          # read match count
          count = response.get_int
          id64 = response.get_int
          
          # read matches
          result['matches'] = []
          while count > 0
            count -= 1
            
            if id64 != 0
              doc = response.get_int64
              weight = response.get_int
            else
              doc, weight = response.get_ints(2)
            end
      
            r = {} # This is a single result put in the result['matches'] array
            r['id'] = doc
            r['weight'] = weight
            attrs_names_in_order.each do |a|
              r['attrs'] ||= {}
  
              case attrs[a]
                when SPH_ATTR_BIGINT
                  # handle 64-bit ints
                  r['attrs'][a] = response.get_int64
                when SPH_ATTR_FLOAT
                  # handle floats
                  r['attrs'][a] = response.get_float
				when SPH_ATTR_STRING
				  # handle string
				  r['attrs'][a] = response.get_string
                else
                  # handle everything else as unsigned ints
                  val = response.get_int
                  if (attrs[a] & SPH_ATTR_MULTI) != 0
                    r['attrs'][a] = []
                    1.upto(val) do
                      r['attrs'][a] << response.get_int
                    end
                  else
                    r['attrs'][a] = val
                  end
              end
            end
            result['matches'] << r
          end
          result['total'], result['total_found'], msecs, words = response.get_ints(4)
          result['time'] = '%.3f' % (msecs / 1000.0)
  
          result['words'] = {}
          while words > 0
            words -= 1
            word = response.get_string
            docs, hits = response.get_ints(2)
            result['words'][word] = { 'docs' => docs, 'hits' => hits }
          end
          
          results << result
        end
      #rescue EOFError
      #  @error = 'incomplete reply'
      #  raise SphinxResponseError, @error
      end
      
      return results
    end
  
    # Connect to searchd server and generate exceprts from given documents.
    #
    # * <tt>docs</tt> -- an array of strings which represent the documents' contents
    # * <tt>index</tt> -- a string specifiying the index which settings will be used
    # for stemming, lexing and case folding
    # * <tt>words</tt> -- a string which contains the words to highlight
    # * <tt>opts</tt> is a hash which contains additional optional highlighting parameters.
    # 
    # You can use following parameters:
    # * <tt>'before_match'</tt> -- a string to insert before a set of matching words, default is "<b>"
    # * <tt>'after_match'</tt> -- a string to insert after a set of matching words, default is "<b>"
    # * <tt>'chunk_separator'</tt> -- a string to insert between excerpts chunks, default is " ... "
    # * <tt>'limit'</tt> -- max excerpt size in symbols (codepoints), default is 256
    # * <tt>'around'</tt> -- how much words to highlight around each match, default is 5
    # * <tt>'exact_phrase'</tt> -- whether to highlight exact phrase matches only, default is <tt>false</tt>
    # * <tt>'single_passage'</tt> -- whether to extract single best passage only, default is false
    # * <tt>'use_boundaries'</tt> -- whether to extract passages by phrase boundaries setup in tokenizer
    # * <tt>'weight_order'</tt> -- whether to order best passages in document (default) or weight order
    #
    # Returns false on failure.
    # Returns an array of string excerpts on success.
    def BuildExcerpts(docs, index, words, opts = {})
      assert { docs.instance_of? Array }
      assert { index.instance_of? String }
      assert { words.instance_of? String }
      assert { opts.instance_of? Hash }

      # fixup options
      opts['before_match'] ||= '<b>';
      opts['after_match'] ||= '</b>';
      opts['chunk_separator'] ||= ' ... ';
	  opts['html_strip_mode'] ||= 'index';
      opts['limit'] ||= 256;
	  opts['limit_passages'] ||= 0;
	  opts['limit_words'] ||= 0;
      opts['around'] ||= 5;
	  opts['start_passage_id'] ||= 1;
      opts['exact_phrase'] ||= false
      opts['single_passage'] ||= false
      opts['use_boundaries'] ||= false
      opts['weight_order'] ||= false
	  opts['load_files'] ||= false
	  opts['allow_empty'] ||= false
      
      # build request
      
      # v.1.0 req
      flags = 1
      flags |= 2  if opts['exact_phrase']
      flags |= 4  if opts['single_passage']
      flags |= 8  if opts['use_boundaries']
      flags |= 16 if opts['weight_order']
	  flags |= 32 if opts['query_mode']
	  flags |= 64 if opts['force_all_words']
	  flags |= 128 if opts['load_files']
	  flags |= 256 if opts['allow_empty']
      
      request = Request.new
      request.put_int 0, flags # mode=0, flags=1 (remove spaces)
      # req index
      request.put_string index
      # req words
      request.put_string words
  
      # options
      request.put_string opts['before_match']
      request.put_string opts['after_match']
      request.put_string opts['chunk_separator']
      request.put_int opts['limit'].to_i, opts['around'].to_i
	  
	  # options v1.2
	  request.put_int opts['limit_passages'].to_i
	  request.put_int opts['limit_words'].to_i
	  request.put_int opts['start_passage_id'].to_i
	  request.put_string opts['html_strip_mode']
      
      # documents
      request.put_int docs.size
      docs.each do |doc|
        assert { doc.instance_of? String }

        request.put_string doc
      end
      
      response = PerformRequest(:excerpt, request)
      
      # parse response
      begin
        res = []
        docs.each do |doc|
          res << response.get_string
        end
      rescue EOFError
        @error = 'incomplete reply'
        raise SphinxResponseError, @error
      end
      return res
    end
    
    # Connect to searchd server, and generate keyword list for a given query.
    #
    # Returns an array of words on success.
    def BuildKeywords(query, index, hits)
      assert { query.instance_of? String }
      assert { index.instance_of? String }
      assert { hits.instance_of?(TrueClass) || hits.instance_of?(FalseClass) }
      
      # build request
      request = Request.new
      # v.1.0 req
      request.put_string query # req query
      request.put_string index # req index
      request.put_int hits ? 1 : 0

      response = PerformRequest(:keywords, request)
      
      # parse response
      begin
        res = []
        nwords = response.get_int
        0.upto(nwords - 1) do |i|
          tokenized = response.get_string
          normalized = response.get_string
          
          entry = { 'tokenized' => tokenized, 'normalized' => normalized }
          entry['docs'], entry['hits'] = response.get_ints(2) if hits
          
          res << entry
        end
      rescue EOFError
        @error = 'incomplete reply'
        raise SphinxResponseError, @error
      end
      
      return res
    end

    # Batch update given attributes in given rows in given indexes.
    #
    # * +index+ is a name of the index to be updated
    # * +attrs+ is an array of attribute name strings.
    # * +values+ is a hash where key is document id, and value is an array of
    # * +mva+ identifies whether update MVA
    # new attribute values
    #
    # Returns number of actually updated documents (0 or more) on success.
    # Returns -1 on failure.
    #
    # Usage example:
    #    sphinx.UpdateAttributes('test1', ['group_id'], { 1 => [456] })
    def UpdateAttributes(index, attrs, values, mva = false)
      # verify everything
      assert { index.instance_of? String }
      assert { mva.instance_of?(TrueClass) || mva.instance_of?(FalseClass) }
      
      assert { attrs.instance_of? Array }
      attrs.each do |attr|
        assert { attr.instance_of? String }
      end
      
      assert { values.instance_of? Hash }
      values.each do |id, entry|
        assert { id.instance_of? Fixnum }
        assert { entry.instance_of? Array }
        assert { entry.length == attrs.length }
        entry.each do |v|
          if mva
            assert { v.instance_of? Array }
            v.each { |vv| assert { vv.instance_of? Fixnum } }
          else
            assert { v.instance_of? Fixnum }
          end
        end
      end
      
      # build request
      request = Request.new
      request.put_string index
      
      request.put_int attrs.length
      for attr in attrs
        request.put_string attr
        request.put_int mva ? 1 : 0
      end
      
      request.put_int values.length
      values.each do |id, entry|
        request.put_int64 id
        if mva
          entry.each { |v| request.put_int_array v }
        else
          request.put_int(*entry)
        end
      end
      
      response = PerformRequest(:update, request)
      
      # parse response
      begin
        return response.get_int
      rescue EOFError
        @error = 'incomplete reply'
        raise SphinxResponseError, @error
      end
    end
  
    protected
    
      # Connect to searchd server.
      def Connect
        begin
          if @host[0,1]=='/'
            sock = UNIXSocket.new(@host)
          else
            sock = TCPSocket.new(@host, @port)
          end
        rescue => err
          @error = "connection to #{@host}:#{@port} failed (error=#{err})"
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
      
      # Get and check response packet from searchd server.
      def GetResponse(sock, client_version)
        response = ''
        len = 0
        
        header = sock.recv(8)
        if header.length == 8
          status, ver, len = header.unpack('n2N')
          left = len.to_i
          while left > 0 do
            begin
              chunk = sock.recv(left)
              if chunk
                response << chunk
                left -= chunk.length
              end
            rescue EOFError
              break
            end
          end
        end
        sock.close
    
        # check response
        read = response.length
        if response.empty? or read != len.to_i
          @error = response.empty? \
            ? 'received zero-sized searchd response' \
            : "failed to read searchd response (status=#{status}, ver=#{ver}, len=#{len}, read=#{read})"
          raise SphinxResponseError, @error
        end
        
        # check status
        if (status == SEARCHD_WARNING)
          wlen = response[0, 4].unpack('N*').first
          @warning = response[4, wlen]
          return response[4 + wlen, response.length - 4 - wlen]
        end

        if status == SEARCHD_ERROR
          @error = 'searchd error: ' + response[4, response.length - 4]
          raise SphinxInternalError, @error
        end
    
        if status == SEARCHD_RETRY
          @error = 'temporary searchd error: ' + response[4, response.length - 4]
          raise SphinxTemporaryError, @error
        end
    
        unless status == SEARCHD_OK
          @error = "unknown status code: '#{status}'"
          raise SphinxUnknownError, @error
        end
        
        # check version
        if ver < client_version
          @warning = "searchd command v.#{ver >> 8}.#{ver & 0xff} older than client's " +
            "v.#{client_version >> 8}.#{client_version & 0xff}, some options might not work"
        end
        
        return response
      end
      
      # Connect, send query, get response.
      def PerformRequest(command, request, additional = nil)
        cmd = command.to_s.upcase
        command_id = Sphinx::Client.const_get('SEARCHD_COMMAND_' + cmd)
        command_ver = Sphinx::Client.const_get('VER_COMMAND_' + cmd)
        
        sock = self.Connect
        len = request.to_s.length + (additional != nil ? 8 : 0)
        header = [command_id, command_ver, len].pack('nnN')
        header << [0, additional].pack('NN') if additional != nil
        sock.send(header + request.to_s, 0)
        response = self.GetResponse(sock, command_ver)
        return Response.new(response)
      end
      
      # :stopdoc:
      def assert
        raise 'Assertion failed!' unless yield if $DEBUG
      end
      # :startdoc:
  end
end
