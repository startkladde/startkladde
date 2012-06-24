class Token
	attr_accessor :type
	attr_accessor :code

	def initialize(type=nil, code="")
		@type=type
		@code=code
	end

	def match?(type, code=nil)
		if code.nil?
			@type==type
		elsif code.is_a?(Regexp)
			@type==type && @code =~ code
		else
			@type==type && @code==code
		end
	end
end


