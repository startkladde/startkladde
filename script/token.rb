# A token, consisting of
#   * a type (a symbol)
#   * the code (a string)
class Token
	attr_accessor :type
	attr_accessor :code

	def initialize(type=nil, code="")
		@type=type
		@code=code
	end

	# Determines whether a token matches a pattern.
	# 
	# Parameters:
	#   * type: must match the token's type
	#   * code: can be a string (must be identical to the token's code), a
	#           regexp (must match the token's code) or nil (the token's code
	#           is ignored)
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


