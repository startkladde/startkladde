# Takes a string of C++ code and splits it into lines and segments
# For example, the following code:
#   /* const */ int foo=42;
#   const char *bar="fred"; // A string
# Will be represented by:
#   * line 0:
#      * default        '  '
#      * block_comment  '/* const */'
#      * default        ' int foo=42;'
#   * line 1:
#      * default        '  const char *bar='
#      * string_literal '"fred"
#      * default        '; '
#      * line_comment   '// A string'
# This simple task is complicated by the fact that the meaning of the separator
# characters depends on their context. For example, double quotes inside a
# comment do not delimit a string.
class CppSegmenter1
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

		# Start at the beginning of the line
		segment_start=0
		skip=0

		# FIXME: can we simplify this by
		#   * having more states and assigning the type explicitly, like for 3?
		#     => create new segment when type changes
		#     => maybe this gets rid of next_segment_start and skip
		#   * taking away the processed part of the line?
		# Note that all input characters should be output again, no
		# transformation takes place, we only assign the segment type

		# As long as there is a segment, we have to process it
		while segment_start
			# We may have to skip some characters
			pos=segment_start+skip

			# By default, we stay in the current state and don't skip anything.
			next_state=@state; next_segment_start=nil; skip=0

			# Find the next separator. Possible separators depend on the state
			# we're currently in. For example, inside a string literal, "//" is
			# not a separator.
			# We set the following variables:
			#   * next_state: the state associated with the next segment
			#   * next_segment_start: the starting position of the next
			#     segment, or nil if the current segment extends to the end of
			#     the line (and possibly beyond)
			#   * skip: the number of characters to skip, starting at
			#     next_segment_start. This is the length of the separator if
			#     the separator belongs to the next part, or 0 if the separator
			#     belongs to the current part.
			case @state
			when :default
				# Look for the first string that can start a new segment
				# (separator): a double quote starts a string literal, two
				# slashes start a line comment and a slash followed by an
				# asterisk starts a block comment. Go to the appropriate state
				# and skip the separator.
				# If no separator is found, stay in the current state and
				# next_segment_start will be left at nil to indicate that the
				# segment extends to the end of the line.
				pos=line.index(/ (") | (\/\/) | (\/\*) /x, pos)
				if    $1 then next_state=:string_literal; next_segment_start=pos; skip=1
				elsif $2 then next_state=:line_comment  ; next_segment_start=pos; skip=2
				elsif $3 then next_state=:block_comment ; next_segment_start=pos; skip=2
				end
			when :string_literal
				# Look for the end of the string literal (a double quote), an
				# escaped backslash or an escaped quote. If an escaped
				# backslash or an escaped quote is encountered, it is skipped
				# and the process is repeated.
				# If no end of the string literal is found, stay in the current
				# state and next_segment_start will be left at nil to indicate
				# that the segment extends to the end of the line.
				begin
					pos=line.index(/ (") | (\\\") | (\\\\) /x, pos)
					if    $1 then next_state=:default; next_segment_start=pos+1; skip=0
					elsif $2 then pos+=2 # Skip over the escaped quote
					elsif $3 then pos+=2 # Skip over the escaped backslash
					end
				end until next_state!=@state || !pos
			when :block_comment
				# Look for the end of the block comment (an asterisk followed
				# by a slash).
				# If no end of the block comment is found, stay in the current
				# state and next_segment_start will be left at nil to indicate
				# that the segment extends to the end of the line.
				pos=line.index(/ (\*\/) /x, pos)
				if $1 then next_state=:default; next_segment_start=pos+2; skip=0
				end
			when :line_comment
				# Look for the end of the line comment (the end of the line).
				# If a backslash preceds the end of the line, this is not
				# counted as the end of the line comment. In this case, the
				# state is not changed.
				# In any case, next_segment_start is left at nil to indicate
				# that the segment extends to the end of the line.
				pos=line.index(/ (\\$) | ($) /x, pos)
				if    $1 then 
				elsif $2 then next_state=:default
				end
			else raise "Unhandled state #{@state.inspect}"
			end

			# It is possible that the segment has a length of zero. For
			# example, in the line
			#   "foo""bar"
			# there is a zero-length "default" segment between the two strings.
			# In this case, ignore it.
			if (next_segment_start!=segment_start)
				# If we found another segment, take the text up to that
				# segment. Otherwise, take the text up to the end of the line.
				if next_segment_start
					code=line[segment_start..next_segment_start-1]
				else
					code=line[segment_start..-1]
				end

				# Add the segment to the list
				result << CodeSegment.new(@state, code)
			end

			segment_start=next_segment_start
			@state=next_state
		end

		return result
	end

	def process_text(text, print)
		text.lines.map { |line|
			process_line(line)
		}
	end
end


