# This one is based on characters and a generic FSM
class CppSegmenter3
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
		buffer=""
		type=nil

		transitions={
			# Default
			:default                  => { '"'  => [:string_literal          , :string_literal ],
			                               '/'  => [:default_slash           , nil             ],
			                               nil  => [:default                 , :default        ]},
			:default_slash            => { '/'  => [:line_comment            , :line_comment   ],
			                               '*'  => [:block_comment           , :block_comment  ],
			                               nil  => [:default                 , :default        ]},
			# String literals
			:string_literal           => { '"'  => [:default                 , :string_literal ],
			                               '\\' => [:string_literal_backslash, :string_literal ],
			                               nil  => [:string_literal          , :string_literal ]},
			:string_literal_backslash => { nil  => [:string_literal          , :string_literal ]},
			# Line comment
			:line_comment             => { :eol => [:default                 , nil             ],
			                               '\\' => [:line_comment_backslash  , :line_comment   ],
			                               nil  => [:line_comment            , :line_comment   ]},
			:line_comment_backslash   => { nil  => [:line_comment            , :line_comment   ]},
			# Block comment
			:block_comment            => { '*'  => [:block_comment_asterisk  , :block_comment  ],
			                               nil  => [:block_comment           , :block_comment  ]},
			:block_comment_asterisk   => { '/'  => [:default                 , :block_comment  ],
			                               nil  => [:block_comment           , :block_comment  ]}
		}

		line.each_char { |c|
			transition=transitions[@state]
			next_state, next_type = (transition[c] || transition[nil])
			#puts "State #{@state.inspect}, input #{c.inspect}, next state #{next_state.inspect}, next type #{next_type.inspect}"

			if next_type.nil?
				buffer+=c
			else
				if next_type!=type
					if !code.empty?
						result << CodeSegment.new(type, code)
					end
					code=""
					type=next_type
				end

				code+=buffer+c
				buffer=""
			end

			@state=next_state
		}

		# EOL
		transition=transitions[@state]
		next_state, next_type = transition[:eol] || transition[nil]

		if !code.empty?
			result << CodeSegment.new(type, code)
		end
		@state=next_state

		return result
	end

	def process_text(text, print)
		text.lines.map { |line|
			process_line(line)
		}
	end
end


