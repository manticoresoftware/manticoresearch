#
# $Id$
#

require 'init.rb'

q = ARGV.join(' ')
@sphinx = Sphinx::Client.new
# @sphinx.SetSortMode(Sphinx::Client::SPH_SORT_ATTR_ASC, 'created_at')
results = @sphinx.Query(q)

puts "Query '#{q}' retrieved #{results['total']} of #{results['total_found']} matches in #{results['time']} sec.";
puts "Query stats:";
results['words'].each do |word, info|
	puts "    '#{word}' found #{info['hits']} times in #{info['docs']} documents\n"
end
puts

n = 1
results['matches'].each do |doc|
	print "#{n}. doc_id=#{doc['id']}, weight=#{doc['weight']}"
	doc['attrs'].each do |attr, value|
		print ", #{attr}=#{value}"
	end
	puts
	n = n+1
end
