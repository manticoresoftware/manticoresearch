module Sphinx
  # Pack ints, floats, strings, and arrays to internal representation
  # needed by Sphinx search engine.
  class Request
    # Initialize new request.
    def initialize
      @request = ''
    end
    
    # Put int(s) to request.
    def put_int(*ints)
      ints.each { |i| @request << [i].pack('N') }
    end

    # Put 64-bit int(s) to request.
    def put_int64(*ints)
      ints.each { |i| @request << [i >> 32, i & ((1 << 32) - 1)].pack('NN') }
    end

    # Put string(s) to request (first length, then the string itself).
    def put_string(*strings)
      strings.each { |s| @request << [s.length].pack('N') + s }
    end
    
    # Put float(s) to request.
    def put_float(*floats)
      floats.each do |f|
        t1 = [f].pack('f') # machine order
        t2 = t1.unpack('L*').first # int in machine order
        @request << [t2].pack('N')
      end
    end
    
    # Put array of ints to request (first length, then the array itself)
    def put_int_array(arr)
      put_int arr.length, *arr
    end
    
    # Returns the entire message
    def to_s
      @request
    end
  end
end