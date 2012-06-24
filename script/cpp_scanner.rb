require 'scanner'
require 'token'

# FIXME for strings and block comments, the token should always end at the
# closing character, even if a token of the same type starts immediately
# after. For example, /* foo *//* bar */ should be two tokens. Currently, we
# start a new token when the type changes. We either have to force a new token
# at the closing character or use a different criterion.

class CppScanner <Scanner
	def initialize
		super(:default) {
			# Default
			add_state(:default) {
				on '"'     , :string_literal       , :string_literal
				on '/'     , :default_slash        , :_unknown
				on " \t\n" , :default              , :whitespace
				on 'a'..'z', :identifier           , :identifier
				on 'A'..'Z', :identifier           , :identifier
				default :default                   , :default
			}
			add_state(:default_slash) {
				on '/' , :line_comment             , :line_comment
				on '*' , :block_comment            , :block_comment
				default  :default                  , :default, :reread
			}

			# Identifier
			add_state(:identifier) {
				on 'a'..'z', :identifier           , :identifier
				on 'A'..'Z', :identifier           , :identifier
				on '0'..'9', :identifier           , :identifier
				on '_'     , :identifier           , :identifier
				default      :default              , :_ignore   , :reread
			}

			# String literals
			add_state(:string_literal) {
				on '"' , :default                  , :string_literal
				on '\\', :string_literal_backslash , :_unknown
				default  :string_literal           , :string_literal
			}
			add_state(:string_literal_backslash) {
				on "\n", :string_literal           , :whitespace
				default  :string_literal           , :string_literal
			}

			# Line comment
			add_state(:line_comment) {
				on "\n", :default                  , :whitespace
				on '\\', :line_comment_backslash   , :_unknown
				default  :line_comment             , :line_comment
			}
			add_state(:line_comment_backslash) {
				on "\n", :line_comment             , :whitespace
				default  :line_comment             , :line_comment
			}

			# Block comment
			add_state(:block_comment) {
				on '*' , :block_comment_asterisk   , :block_comment
				default  :block_comment            , :block_comment
			}
			add_state(:block_comment_asterisk) {
				on '/' , :default                  , :block_comment
				default  :block_comment            , :block_comment
			}
		}
	end

	def self.pretty_print_token(token)
		colors = {
			:default        => :cyan,
			:string_literal => :yellow,
			:block_comment  => :green,
			:line_comment   => :blue,
			:identifier     => :red,
			:whitespace     => :magenta
		}

		type=token.type
		code=token.code

		color=colors[type]
		raise "Unhandled token type #{type.inspect}" if !color

		if type==:whitespace
			code=code.gsub(' ' , '.')
			code=code.gsub("\t", '>>>>')
		end
		code=code.gsub("\n", '<')

		code.color(color)
	end

	def self.pretty_print_tokens(tokens)
		tokens.each { |token|
			print "|"
			print pretty_print_token(token)
		}
		print "|"
	end

	def self.pretty_print_lines_tokens(lines_tokens)
		max_line_num=lines_tokens.size+1
		padlength=max_line_num.to_s.length

		lines_tokens.each_with_index { |tokens, index|
			line_number=index+1
			print "#{line_number.to_s.rjust(padlength)}:"
			pretty_print_tokens(tokens)
			puts
		}
	end
end

