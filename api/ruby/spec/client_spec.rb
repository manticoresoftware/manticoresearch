require File.dirname(__FILE__) + '/../init'

module SphinxFixtureHelper
  def sphinx_fixture(name)
    `php #{File.dirname(__FILE__)}/fixtures/#{name}.php`
  end
end

describe 'The Connect method of SphinxApi' do
  before(:each) do
    @sphinx = Sphinx::Client.new
    @sock = mock('TCPSocket')
  end

  it 'should establish TCP connection to the server and initialize session' do
    TCPSocket.should_receive(:new).with('localhost', 3312).and_return(@sock)
    @sock.should_receive(:recv).with(4).and_return([1].pack('N'))
    @sock.should_receive(:send).with([1].pack('N'), 0)
    @sphinx.send(:Connect).should be(@sock)
  end

  it 'should raise exception when searchd protocol is not 1+' do
    TCPSocket.should_receive(:new).with('localhost', 3312).and_return(@sock)
    @sock.should_receive(:recv).with(4).and_return([0].pack('N'))
    @sock.should_receive(:close)
    lambda { @sphinx.send(:Connect) }.should raise_error(Sphinx::SphinxConnectError)
    @sphinx.GetLastError.should == 'expected searchd protocol version 1+, got version \'0\''
  end

  it 'should raise exception on connection error' do
    TCPSocket.should_receive(:new).with('localhost', 3312).and_raise(Errno::EBADF)
    lambda { @sphinx.send(:Connect) }.should raise_error(Sphinx::SphinxConnectError)
    @sphinx.GetLastError.should == 'connection to localhost:3312 failed'
  end

  it 'should use custom host and port' do
    @sphinx.SetServer('anotherhost', 55555)
    TCPSocket.should_receive(:new).with('anotherhost', 55555).and_raise(Errno::EBADF)
    lambda { @sphinx.send(:Connect) }.should raise_error(Sphinx::SphinxConnectError)
  end
end

describe 'The GetResponse method of SphinxApi' do
  before(:each) do
    @sphinx = Sphinx::Client.new
    @sock = mock('TCPSocket')
    @sock.should_receive(:close)
  end
  
  it 'should receive response' do
    @sock.should_receive(:recv).with(8).and_return([Sphinx::Client::SEARCHD_OK, 1, 4].pack('n2N'))
    @sock.should_receive(:recv).with(4).and_return([0].pack('N'))
    @sphinx.send(:GetResponse, @sock, 1)
  end

  it 'should raise exception on zero-sized response' do
    @sock.should_receive(:recv).with(8).and_return([Sphinx::Client::SEARCHD_OK, 1, 0].pack('n2N'))
    lambda { @sphinx.send(:GetResponse, @sock, 1) }.should raise_error(Sphinx::SphinxResponseError)
  end

  it 'should raise exception when response is incomplete' do
    @sock.should_receive(:recv).with(8).and_return([Sphinx::Client::SEARCHD_OK, 1, 4].pack('n2N'))
    @sock.should_receive(:recv).with(4).and_raise(EOFError)
    lambda { @sphinx.send(:GetResponse, @sock, 1) }.should raise_error(Sphinx::SphinxResponseError)
  end

  it 'should set warning message when SEARCHD_WARNING received' do
    @sock.should_receive(:recv).with(8).and_return([Sphinx::Client::SEARCHD_WARNING, 1, 14].pack('n2N'))
    @sock.should_receive(:recv).with(14).and_return([5].pack('N') + 'helloworld')
    @sphinx.send(:GetResponse, @sock, 1).should == 'world'
    @sphinx.GetLastWarning.should == 'hello'
  end

  it 'should raise exception when SEARCHD_ERROR received' do
    @sock.should_receive(:recv).with(8).and_return([Sphinx::Client::SEARCHD_ERROR, 1, 9].pack('n2N'))
    @sock.should_receive(:recv).with(9).and_return([1].pack('N') + 'hello')
    lambda { @sphinx.send(:GetResponse, @sock, 1) }.should raise_error(Sphinx::SphinxInternalError)
    @sphinx.GetLastError.should == 'searchd error: hello'
  end

  it 'should raise exception when SEARCHD_RETRY received' do
    @sock.should_receive(:recv).with(8).and_return([Sphinx::Client::SEARCHD_RETRY, 1, 9].pack('n2N'))
    @sock.should_receive(:recv).with(9).and_return([1].pack('N') + 'hello')
    lambda { @sphinx.send(:GetResponse, @sock, 1) }.should raise_error(Sphinx::SphinxTemporaryError)
    @sphinx.GetLastError.should == 'temporary searchd error: hello'
  end

  it 'should raise exception when unknown status received' do
    @sock.should_receive(:recv).with(8).and_return([65535, 1, 9].pack('n2N'))
    @sock.should_receive(:recv).with(9).and_return([1].pack('N') + 'hello')
    lambda { @sphinx.send(:GetResponse, @sock, 1) }.should raise_error(Sphinx::SphinxUnknownError)
    @sphinx.GetLastError.should == 'unknown status code: \'65535\''
  end

  it 'should set warning when server is older than client' do
    @sock.should_receive(:recv).with(8).and_return([Sphinx::Client::SEARCHD_OK, 1, 9].pack('n2N'))
    @sock.should_receive(:recv).with(9).and_return([1].pack('N') + 'hello')
    @sphinx.send(:GetResponse, @sock, 5)
    @sphinx.GetLastWarning.should == 'searchd command v.0.1 older than client\'s v.0.5, some options might not work'
  end
end

describe 'The Query method of SphinxApi' do
  include SphinxFixtureHelper

  before(:each) do
    @sphinx = Sphinx::Client.new
    @sock = mock('TCPSocket')
    @sphinx.stub!(:Connect).and_return(@sock)
    @sphinx.stub!(:GetResponse).and_raise(Sphinx::SphinxError)
  end

  it 'should generate valid request with default parameters' do
    expected = sphinx_fixture('default_search')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with default parameters and index' do
    expected = sphinx_fixture('default_search_index')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.Query('query', 'index') rescue nil?
  end
  
  it 'should generate valid request with limits' do
    expected = sphinx_fixture('limits')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetLimits(10, 20)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with limits and max number to retrieve' do
    expected = sphinx_fixture('limits_max')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetLimits(10, 20, 30)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with limits and cutoff to retrieve' do
    expected = sphinx_fixture('limits_cutoff')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetLimits(10, 20, 30, 40)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with max query time specified' do
    expected = sphinx_fixture('max_query_time')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetMaxQueryTime(1000)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with match SPH_MATCH_ALL' do
    expected = sphinx_fixture('match_all')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetMatchMode(Sphinx::Client::SPH_MATCH_ALL)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with match SPH_MATCH_ANY' do
    expected = sphinx_fixture('match_any')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetMatchMode(Sphinx::Client::SPH_MATCH_ANY)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with match SPH_MATCH_PHRASE' do
    expected = sphinx_fixture('match_phrase')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetMatchMode(Sphinx::Client::SPH_MATCH_PHRASE)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with match SPH_MATCH_BOOLEAN' do
    expected = sphinx_fixture('match_boolean')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetMatchMode(Sphinx::Client::SPH_MATCH_BOOLEAN)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with match SPH_MATCH_EXTENDED' do
    expected = sphinx_fixture('match_extended')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetMatchMode(Sphinx::Client::SPH_MATCH_EXTENDED)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with match SPH_MATCH_FULLSCAN' do
    expected = sphinx_fixture('match_fullscan')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetMatchMode(Sphinx::Client::SPH_MATCH_FULLSCAN)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with match SPH_MATCH_EXTENDED2' do
    expected = sphinx_fixture('match_extended2')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetMatchMode(Sphinx::Client::SPH_MATCH_EXTENDED2)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with ranking mode SPH_RANK_PROXIMITY_BM25' do
    expected = sphinx_fixture('ranking_proximity_bm25')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetRankingMode(Sphinx::Client::SPH_RANK_PROXIMITY_BM25)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with ranking mode SPH_RANK_BM25' do
    expected = sphinx_fixture('ranking_bm25')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetRankingMode(Sphinx::Client::SPH_RANK_BM25)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with ranking mode SPH_RANK_NONE' do
    expected = sphinx_fixture('ranking_none')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetRankingMode(Sphinx::Client::SPH_RANK_NONE)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with ranking mode SPH_RANK_WORDCOUNT' do
    expected = sphinx_fixture('ranking_wordcount')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetRankingMode(Sphinx::Client::SPH_RANK_WORDCOUNT)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with sort mode SPH_SORT_RELEVANCE' do
    expected = sphinx_fixture('sort_relevance')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetSortMode(Sphinx::Client::SPH_SORT_RELEVANCE)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with sort mode SPH_SORT_ATTR_DESC' do
    expected = sphinx_fixture('sort_attr_desc')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetSortMode(Sphinx::Client::SPH_SORT_ATTR_DESC, 'sortby')
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with sort mode SPH_SORT_ATTR_ASC' do
    expected = sphinx_fixture('sort_attr_asc')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetSortMode(Sphinx::Client::SPH_SORT_ATTR_ASC, 'sortby')
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with sort mode SPH_SORT_TIME_SEGMENTS' do
    expected = sphinx_fixture('sort_time_segments')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetSortMode(Sphinx::Client::SPH_SORT_TIME_SEGMENTS, 'sortby')
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with sort mode SPH_SORT_EXTENDED' do
    expected = sphinx_fixture('sort_extended')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetSortMode(Sphinx::Client::SPH_SORT_EXTENDED, 'sortby')
    @sphinx.Query('query') rescue nil?
  end


  it 'should generate valid request with sort mode SPH_SORT_EXPR' do
    expected = sphinx_fixture('sort_EXPR')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetSortMode(Sphinx::Client::SPH_SORT_EXPR, 'sortby')
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with weights' do
    expected = sphinx_fixture('weights')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetWeights([10, 20, 30, 40])
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with field weights' do
    expected = sphinx_fixture('field_weights')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetFieldWeights({'field1' => 10, 'field2' => 20})
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with index weights' do
    expected = sphinx_fixture('index_weights')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetIndexWeights({'index1' => 10, 'index2' => 20})
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with ID range' do
    expected = sphinx_fixture('id_range')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetIDRange(10, 20)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with ID range and 64-bit ints' do
    expected = sphinx_fixture('id_range64')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetIDRange(8589934591, 17179869183)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with values filter' do
    expected = sphinx_fixture('filter')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetFilter('attr', [10, 20, 30])
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with two values filters' do
    expected = sphinx_fixture('filters')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetFilter('attr2', [40, 50])
    @sphinx.SetFilter('attr1', [10, 20, 30])
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with values filter excluded' do
    expected = sphinx_fixture('filter_exclude')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetFilter('attr', [10, 20, 30], true)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with values filter range' do
    expected = sphinx_fixture('filter_range')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetFilterRange('attr', 10, 20)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with two filter ranges' do
    expected = sphinx_fixture('filter_ranges')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetFilterRange('attr2', 30, 40)
    @sphinx.SetFilterRange('attr1', 10, 20)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with filter range excluded' do
    expected = sphinx_fixture('filter_range_exclude')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetFilterRange('attr', 10, 20, true)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with float filter range' do
    expected = sphinx_fixture('filter_float_range')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetFilterFloatRange('attr', 10.5, 20.3)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with float filter excluded' do
    expected = sphinx_fixture('filter_float_range_exclude')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetFilterFloatRange('attr', 10.5, 20.3, true)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with different filters' do
    expected = sphinx_fixture('filters_different')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetFilterRange('attr1', 10, 20, true)
    @sphinx.SetFilter('attr3', [30, 40, 50])
    @sphinx.SetFilterRange('attr1', 60, 70)
    @sphinx.SetFilter('attr2', [80, 90, 100], true)
    @sphinx.SetFilterFloatRange('attr1', 60.8, 70.5)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with geographical anchor point' do
    expected = sphinx_fixture('geo_anchor')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetGeoAnchor('attrlat', 'attrlong', 20.3, 40.7)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with group by SPH_GROUPBY_DAY' do
    expected = sphinx_fixture('group_by_day')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetGroupBy('attr', Sphinx::Client::SPH_GROUPBY_DAY)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with group by SPH_GROUPBY_WEEK' do
    expected = sphinx_fixture('group_by_week')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetGroupBy('attr', Sphinx::Client::SPH_GROUPBY_WEEK)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with group by SPH_GROUPBY_MONTH' do
    expected = sphinx_fixture('group_by_month')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetGroupBy('attr', Sphinx::Client::SPH_GROUPBY_MONTH)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with group by SPH_GROUPBY_YEAR' do
    expected = sphinx_fixture('group_by_year')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetGroupBy('attr', Sphinx::Client::SPH_GROUPBY_YEAR)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with group by SPH_GROUPBY_ATTR' do
    expected = sphinx_fixture('group_by_attr')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetGroupBy('attr', Sphinx::Client::SPH_GROUPBY_ATTR)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with group by SPH_GROUPBY_ATTRPAIR' do
    expected = sphinx_fixture('group_by_attrpair')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetGroupBy('attr', Sphinx::Client::SPH_GROUPBY_ATTRPAIR)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with group by SPH_GROUPBY_DAY with sort' do
    expected = sphinx_fixture('group_by_day_sort')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetGroupBy('attr', Sphinx::Client::SPH_GROUPBY_DAY, 'somesort')
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with count-distinct attribute' do
    expected = sphinx_fixture('group_distinct')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetGroupBy('attr', Sphinx::Client::SPH_GROUPBY_DAY)
    @sphinx.SetGroupDistinct('attr')
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with retries count specified' do
    expected = sphinx_fixture('retries')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetRetries(10)
    @sphinx.Query('query') rescue nil?
  end

  it 'should generate valid request with retries count and delay specified' do
    expected = sphinx_fixture('retries_delay')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.SetRetries(10, 20)
    @sphinx.Query('query') rescue nil?
  end
end

describe 'The RunQueries method of SphinxApi' do
  include SphinxFixtureHelper

  before(:each) do
    @sphinx = Sphinx::Client.new
    @sock = mock('TCPSocket')
    @sphinx.stub!(:Connect).and_return(@sock)
    @sphinx.stub!(:GetResponse).and_raise(Sphinx::SphinxError)
  end

  it 'should generate valid request for multiple queries' do
    expected = sphinx_fixture('miltiple_queries')
    @sock.should_receive(:send).with(expected, 0)
    
    @sphinx.SetRetries(10, 20)
    @sphinx.AddQuery('test1')
    @sphinx.SetGroupBy('attr', Sphinx::Client::SPH_GROUPBY_DAY)
    @sphinx.AddQuery('test2') rescue nil?
    
    @sphinx.RunQueries rescue nil?
  end
end

describe 'The BuildExcerpts method of SphinxApi' do
  include SphinxFixtureHelper

  before(:each) do
    @sphinx = Sphinx::Client.new
    @sock = mock('TCPSocket')
    @sphinx.stub!(:Connect).and_return(@sock)
    @sphinx.stub!(:GetResponse).and_raise(Sphinx::SphinxError)
  end
  
  it 'should generate valid request with default parameters' do
    expected = sphinx_fixture('excerpt_default')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.BuildExcerpts(['10', '20'], 'index', 'word1 word2') rescue nil?
  end

  it 'should generate valid request with custom parameters' do
    expected = sphinx_fixture('excerpt_custom')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.BuildExcerpts(['10', '20'], 'index', 'word1 word2', { 'before_match' => 'before',
                                                                  'after_match' => 'after',
                                                                  'chunk_separator' => 'separator',
                                                                  'limit' => 10 }) rescue nil?
  end
  
  it 'should generate valid request with flags' do
    expected = sphinx_fixture('excerpt_flags')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.BuildExcerpts(['10', '20'], 'index', 'word1 word2', { 'exact_phrase' => true,
                                                                  'single_passage' => true,
                                                                  'use_boundaries' => true,
                                                                  'weight_order' => true }) rescue nil?
  end
end

describe 'The BuildKeywords method of SphinxApi' do
  include SphinxFixtureHelper

  before(:each) do
    @sphinx = Sphinx::Client.new
    @sock = mock('TCPSocket')
    @sphinx.stub!(:Connect).and_return(@sock)
    @sphinx.stub!(:GetResponse).and_raise(Sphinx::SphinxError)
  end
  
  it 'should generate valid request' do
    expected = sphinx_fixture('keywords')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.BuildKeywords('test', 'index', true) rescue nil?
  end
end

describe 'The UpdateAttributes method of SphinxApi' do
  include SphinxFixtureHelper

  before(:each) do
    @sphinx = Sphinx::Client.new
    @sock = mock('TCPSocket')
    @sphinx.stub!(:Connect).and_return(@sock)
    @sphinx.stub!(:GetResponse).and_raise(Sphinx::SphinxError)
  end
  
  it 'should generate valid request' do
    expected = sphinx_fixture('update_attributes')
    @sock.should_receive(:send).with(expected, 0)
    @sphinx.UpdateAttributes('index', ['group'], { 123 => [456] }) rescue nil?
  end
end
