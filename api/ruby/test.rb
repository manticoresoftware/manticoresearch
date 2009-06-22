require File.dirname(__FILE__) + "/lib/sphinx"

sphinx = Sphinx::Client.new
result = sphinx.Query("test", "test1")

print "Found ", result["total_found"], " matches.\n\n"

n = 1
result['matches'].each do |m|
	print "#{n}. id=#{m['id']}, weight=#{m['weight']}"
	m['attrs'].each { |a| print ", #{a[0]}=#{a[1]}" }
	print "\n"
	n = n+1
end
