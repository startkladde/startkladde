require 'scanner'
require 'token'

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
end

