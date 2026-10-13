require File.join(__dir__, 'init')

def check_request(client, flags, ranker)
  client.AddQuery('query')
  request = client.instance_variable_get(:@reqs).first
  got_flags, got_ranker = request.unpack('@0N@16N')
  raise "unexpected request: flags=#{got_flags} ranker=#{got_ranker}" unless got_flags == flags && got_ranker == ranker
  client.instance_variable_set(:@reqs, [])
end

client = Sphinx::Client.new
check_request(client, 1 << 6, Sphinx::Client::SPH_RANK_PROXIMITY_BM25)
client.SetRankingMode(Sphinx::Client::SPH_RANK_PROXIMITY_BM25)
check_request(client, (1 << 6) | (1 << 15), Sphinx::Client::SPH_RANK_PROXIMITY_BM25)
client.SetRankingMode(Sphinx::Client::SPH_RANK_BM25)
check_request(client, (1 << 6) | (1 << 15), Sphinx::Client::SPH_RANK_BM25)
client.ResetQueryFlag
check_request(client, (1 << 6) | (1 << 15), Sphinx::Client::SPH_RANK_BM25)
raise 'unexpected search protocol version' unless Sphinx::Client::VER_COMMAND_SEARCH == 0x11b
