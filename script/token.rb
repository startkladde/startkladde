class Token
	attr_accessor :type
	attr_accessor :code

	def initialize(type=nil, code="")
		@type=type
		@code=code
	end
end


