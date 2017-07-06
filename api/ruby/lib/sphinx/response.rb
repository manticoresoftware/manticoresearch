module Sphinx
  # Unpack internal Sphinx representation of ints, floats, strings, and arrays.
  # needed by Sphinx search engine.
  class Response
    # Initialize new request.
    def initialize(response)
      @response = response
      @position = 0
      @size = response.length
    end
    
    # Gets current stream position.
    def position
      @position
    end
    
    # Gets response size.
    def size
      @size
    end
    
    # Returns <tt>true</tt> when response stream is out.
    def eof?
      @position >= @size
    end

    # Get int from stream.
    def get_int
      raise EOFError if @position + 4 > @size
      value = @response[@position, 4].unpack('N*').first
      @position += 4
      return value
    end

    # Get 64-bit int from stream.
    def get_int64
      raise EOFError if @position + 8 > @size
      hi, lo = @response[@position, 8].unpack('N*N*')
      @position += 8
      return (hi << 32) + lo
    end

    # Get array of <tt>count</tt> ints from stream.
    def get_ints(count)
      length = 4 * count
      raise EOFError if @position + length > @size
      values = @response[@position, length].unpack('N*' * count)
      @position += length
      return values
    end
    
    # Get string from stream.
    def get_string
      length = get_int
      raise EOFError if @position + length > @size
      value = length > 0 ? @response[@position, length] : ''
      @position += length
      return value
    end
    
    # Get float from stream.
    def get_float
      raise EOFError if @position + 4 > @size
      uval = @response[@position, 4].unpack('N*').first;
      @position += 4
      return ([uval].pack('L')).unpack('f*').first
    end
  end
end