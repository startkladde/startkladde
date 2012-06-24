# This one does not copy the characters one by one
class CppSegmenter4
	attr_reader :state

	def initialize
		reset
	end

	def reset
		@state=:default
	end


	def process_line(line)
		result=[]

		# Clean up the line breaks (we should do this before)
		line=line.chomp+"\n"

		segment_start=0  # The start position of the current segment
		segment_type=nil # The type of the current segment
		unknown_length=0 # The number of unknown characters in the current unknown part

		# FIXME speedup: can we "compile" it?
		transitions={
			# Default
			:default                  => { '"'  => [:string_literal          , :string_literal ],
			                               '/'  => [:default_slash           , nil             ],
			                               nil  => [:default                 , :default        ]},
			:default_slash            => { '/'  => [:line_comment            , :line_comment   ],
			                               '*'  => [:block_comment           , :block_comment  ],
			                               nil  => [:default                 , :default        ],
			                               "\n" => [:default                 , :default        ]},
			# String literals
			:string_literal           => { '"'  => [:default                 , :string_literal ],
			                               '\\' => [:string_literal_backslash, :string_literal ],
			                               nil  => [:string_literal          , :string_literal ]},
			:string_literal_backslash => { nil  => [:string_literal          , :string_literal ]},
			# Line comment
			:line_comment             => { "\n" => [:default                 , nil             ],
			                               '\\' => [:line_comment_backslash  , :line_comment   ],
			                               nil  => [:line_comment            , :line_comment   ]},
			:line_comment_backslash   => { nil  => [:line_comment            , :line_comment   ]},
			# Block comment
			:block_comment            => { '*'  => [:block_comment_asterisk  , :block_comment  ],
			                               nil  => [:block_comment           , :block_comment  ]},
			:block_comment_asterisk   => { '/'  => [:default                 , :block_comment  ],
			                               nil  => [:block_comment           , :block_comment  ]}
		}

		#  function ("foo");
		# ?..........sssss..
		#  !         !    !
		#  <         <    <

		#  function () /* test */;
		# ?............?bbbbbbbbb.
		#  !           !!        !
		#  <           <

		#  function () / 2;
		# ?............?...
		#  !           !!
		#  <

		#  // foo
		# ??lllll
		#   !
		#  <

		#  /
		# ?? => at EOL, we must set the segment type to :default

		#  "foo"
		# ?sssss => at EOL, we have to leave the segment type at
		#           :string_literal, OR we have to write the segment
		#           when we see the closing quote (this would mean that we
		#           couldn't write the segment when the type changes, we'd
		#           probably have to use the state - need a state :nothing?
		#           Can we always return to a :nothing state?
		#           Can we fix this by explicitly assigning type :whitespace to
		#           EOL?
		#           FIXME


		# segment type | type      | meaning                                    | action
		# unknown      | unknown   | line starts with unknown character         |
		# known        | unknown   | unknown character in the middle of segment |

		# unknown      | known     | line starts with known character           |
		# known        | same      | segment continued                          |
		# known        | different | new segment started                        |

		len=line.length
		len.times { |pos|
			# Fetch the current character from the string
			c=line[pos].chr

			# Determine the next state and the type of the current character
			# (can be nil for "unknown")
			transition=transitions[@state]
			next_state, character_type = (transition[c] || transition[nil])
			#puts "State #{@state.inspect.color(:yellow)}, input #{c.inspect.color(:magenta)}, next state #{next_state.inspect.color(:yellow)}, type #{character_type.inspect}"

			if character_type
				# The type of the current character is known. If the character
				# type is the same as the segment type, we just continue
				# reading.
				if character_type!=segment_type
					if segment_type
						# The segment type changed. Add the previous
						# (completed) segment to the list and start the new
						# segment.

						# If there have been unknown characters, they are part
						# of the new segment.
						# FIXME can the length ever be 0?
						next_segment_start=pos-unknown_length

						code=line[segment_start...next_segment_start]
						#raise "empty" if code.empty?
						#puts "Add code segment, type #{segment_type.inspect}: #{code.inspect.color(:red)}"
						result << CodeSegment.new(segment_type, code)
						segment_start=next_segment_start
					end

					# Either this character starts a new segment (segment_type
					# is valid) or the segment type is unknown (segment_type is
					# nil). In both cases, set the segment type to the
					# character type.
					segment_type=character_type

				else
					#puts "Segment continued"
				end

				unknown_length=0
			else
				# The type of the current character is unknown. There's not a
				# lot we can do now, so we'll just count the unknown characters
				# so we can consider them when we get the next known character.
				unknown_length+=1
				#puts "Unknown character (#{unknown_length} so far)"
			end
			
			@state=next_state
		}

		if segment_start<len
			code=line[segment_start..-1]
			#puts "Add code segment at EOL, #{segment_type.inspect}: #{code.inspect.color(:red)}"
			result << CodeSegment.new(segment_type, code)
		end

		return result
	end

	def process_text(text, print)
		text.lines.map { |line|
			process_line(line)
		}
	end
end


