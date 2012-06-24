class StateActions
	attr_accessor :next_state, :character_type, :reread

	def initialize
		@next_state={}
		@character_type={}
		@reread={}
	end

	def on(input, next_state, character_type, flags=nil)
		if input.is_a?(Range)
			input.each { |char| on char, next_state, character_type, flags }
		elsif input.length>1
			input.each_char { |char| on char, next_state, character_type, flags }
		else
			@next_state    [input]=next_state
			@character_type[input]=character_type
			@reread        [input]=(flags==:reread)
		end
	end

	def default(next_state, character_type, flags=nil)
		@next_state    .default=next_state
		@character_type.default=character_type
		@reread        .default=(flags==:reread)
	end

end

class Scanner
	def initialize(default_state, &proc)
		@state=nil

		@default_state=default_state
		@state_actions={}
		instance_eval(&proc)
		compile
		reset
	end

	def add_state(state, &block)
		state_actions=StateActions.new
		state_actions.instance_eval(&block)
		@state_actions[state]=state_actions
	end

	def compile
		@states=@state_actions.keys

		@next_state=@states.map { |state|
			(0..255).map { |char|
				next_state=@state_actions[state].next_state[char.chr]
				raise "Compile error: no next state for state #{state.inspect}, input #{char}" if !next_state

				next_state_index=@states.index(next_state)
				raise "Compile error: unknown state #{next_state.inspect}" if !next_state_index

				next_state_index
			}
		}

		@character_type=@states.map { |state|
			(0..255).map { |char|
				@state_actions[state].character_type[char.chr]
			}
		}

		@reread=@states.map { |state|
			(0..255).map { |char|
				@state_actions[state].reread[char.chr]
			}
		}
	end

	def reset
		@state=@states.index(@default_state)
	end

	def dump
		p @next_state
		p @character_type
	end

	def scan(string)
		token_start=0    # The start position of the current token
		token_type=nil   # The type of the current token
		unknown_length=0 # The number of unknown characters in the current unknown part

		len=string.length
		len.times { |pos|
			# Fetch the current character from the string
			c=string[pos]

			# Determine the next state and the type of the current character
			# (can be nil for "unknown")
			next_state    =@next_state     [@state][c]
			character_type=@character_type [@state][c]
			reread        =@reread         [@state][c]
			#puts "State #{@states[@state].inspect.color(:yellow)}, input #{c.chr.inspect.color(:magenta)}, next state #{@states[next_state].inspect.color(:yellow)}, character type #{character_type.inspect}"

			if character_type==:_ignore
				# Ignore the current character
			elsif character_type==:_unknown
				# The type of the current character is unknown. There's not a
				# lot we can do now, so we'll just count the unknown characters
				# so we can consider them when we get the next known character.
				unknown_length+=1
				#puts "Unknown character (#{unknown_length} so far)"
			else
				# The type of the current character is known. If the character
				# type is the same as the token type, we just continue reading.
				if character_type!=token_type
					if token_type
						# The token type changed. Add the previous (completed)
						# token to the list and start the new token.

						# If there have been unknown characters, they are part
						# of the new token.
						# FIXME can the length ever be 0?
						next_token_start=pos-unknown_length

						code=string[token_start...next_token_start]
						#raise "empty" if code.empty?
						#puts "Add code segment, type #{segment_type.inspect}: #{code.inspect.color(:red)}"
						yield token_type, code
						token_start=next_token_start
					end

					# Either this character starts a new token (token_type
					# is valid) or the segment type is unknown (segment_type is
					# nil). In both cases, set the segment type to the
					# character type.
					token_type=character_type

				else
					#puts "Segment continued"
				end

				unknown_length=0
			end

			@state=next_state
			redo if reread
		}

		if token_start<len
			code=string[token_start..-1]
			#puts "Add code segment at EOL, #{segment_type.inspect}: #{code.inspect.color(:red)}"
			yield token_type, code
		end
	end

end

