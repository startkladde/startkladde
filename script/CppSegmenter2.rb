# This one is based on characters instead of regular expressions
class CppSegmenter2
	attr_reader :state

	def initialize
		reset
	end

	def reset
		@state=:default
	end


	def process_line(line)
		result=[]

		# We don't want to include the line breaks in the segments
		line=line.chomp

		code=""

		segment_finished=lambda { |type|
			result << CodeSegment.new(type, code)
			code=""
		}

		line.each_char { |c|
			case @state
			when :default
				case c
				when '"'  then @state=:string_literal          ;          segment_finished.call(:default); code+=c
				when '/'  then @state=:default_slash
				else                                             code+=c
				end
			when :default_slash
				case c
				when '/' then @state=:line_comment             ;           segment_finished.call(:default); code+='//'
				when '*' then @state=:block_comment            ;           segment_finished.call(:default); code+='/*'
				else          @state=:default                  ; code+='/'+c
				end
			when :line_comment
				case c
				when '\\' then @state=:line_comment_backslash
				else                                             code+=c
				end
			when :line_comment_backslash
				@state=:line_comment; code+='\\'+c
			when :block_comment
				case c
				when '*'  then @state=:block_comment_asterisk
				else                                             code+=c
				end
			when :block_comment_asterisk
				case c
				when '/' then @state=:default                  ; code+='*/'; segment_finished.call(:block_comment)
				else          @state=:block_comment            ; code+=c
				end
			when :string_literal
				case c
				when '"'  then @state=:default                 ; code+=c   ; segment_finished.call(:string_literal)
				when '\\' then @state=:string_literal_backslash
				else                                             code+=c
				end
			when :string_literal_backslash
				case c
				when '\\' then @state=:string_literal          ; code+='\\'+c
				when '"'  then @state=:string_literal          ; code+='\\'+c
				else           @state=:string_literal          ; code+='\\'+c
				end
			else
				raise "Unhandled state #{@state.inspect}"
			end
		}

		# EOL
		# FIXME call segment_finished
		case @state
		when :line_comment
			@state=:default; result << CodeSegment.new(:line_comment, code); code="";
		when :string_literal_backslash
			@state=:string_literal; code+='\\'; result << CodeSegment.new(:string_literal, code); code="";
		when :line_comment_backslash
			result << CodeSegment.new(:line_comment, code); code="";
		else
			# FIXME this stinks => have all cases been handled?
			# FIXME comment with line continuation => backslash in wrong line?
			result << CodeSegment.new(@state, code); code="";
		end

		return result
	end

	def process_text(text, print)
		text.lines.map { |line|
			process_line(line)
		}
	end
end


