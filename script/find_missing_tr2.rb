#!/usr/bin/env ruby

$: << File.dirname($0)

require 'rubygems'
require 'rainbow'

require 'Scanner'

class Token
	attr_accessor :type
	attr_accessor :code

	def initialize(type=nil, code="")
		@type=type
		@code=code
	end
end

def pretty_print(lines)
	colors = {
		:default        => :cyan,
		:string_literal => :yellow,
		:block_comment  => :green,
		:line_comment   => :blue,
		:identifier     => :red,
		:whitespace     => :magenta
	}

	max_line_num=lines.size+1
	padlength=max_line_num.to_s.length

	lines.each_with_index { |tokens, index|
		line_number=index+1
		print "#{line_number.to_s.rjust(padlength)}:"
		tokens.each { |token|
			type=token.type
			code=token.code

			color=colors[token.type]
			raise "Unhandled token type #{token.type.inspect}" if !color

			if type==:whitespace
				code=code.gsub(' ' , '.')
				code=code.gsub("\t", '>>>>')
			end

			code=code.gsub("\n", '<')

			print "|"
			print code.color(color)
		}
		print "|"
		puts
	}
end

use_test=false
print=false
filename=nil

while arg=ARGV.shift do
	if    arg=="-t" then use_test=true
	elsif arg=="-p" then print=true
	else  filename=arg
	end
end

error=false

if filename.nil? && !use_test
	puts "Specify a file name or -t for test text"
	error=true
end

exit 1 if error

if use_test
	# TODO test cases:
	#  single / at SOL
	#  single / at EOL
	text=<<EOF
// Comment at the beginning

void foo (const char *x="STRING")
{
  // Simple cases
  FUNCTION (); FUNCTION(); // COMMENT
  FUNCTION (); /* COMMENT */ FUNCTION ();
  FUNCTION ("STRING");
  /* Empty comment at end of line */ //
  multiple (); function_calls ();
  without (); any ();
  strings_or_comments ();

  // Adjacent tokens
  "STRING" "STRING"
  /* COMMENT */ /* COMMENT */
  "STRING""STRING"/* COMMENT *//* COMMENT */"STRING"// COMMENT
  qux=foo+bar-baz;

  // String with comment delimiters
  FUNCTION ("before // not a comment, a STRING");
  FUNCTION ("before /* not a comment, a STRING */ after");

  // Comments with string delimiters
  FUNCTION (); // before "not a string, a COMMENT"
  FUNCTION (); /* before "not a string, a COMMENT" after */ FUNCTION ();

  // Comments with other comment delimiters
  FUNCTION (); // /* block comment */ still a comment
  FUNCTION (); /* block comment // still a comment */
  FUNCTION (); /* block comment /* still a comment */

  // Strings with escaped characters
  "STRING\\nSTRING";
  "STRING\\\\STRING";
  "STRING\\"STRING";
  "STRING\\\\";
  "STRING";
  // line comment with \\n in the middle
  "multi_
line_
string";
  "string_with_\\
line continuation";

  // comment_with_\\
line_continuation

  single / line / division;
  / single_line_division /
  multiline
  /
  division;

  // Single things on a line
"STRING"
// line comment
/* block comment */
"
STRING
"
//
division
/
operator
}
// Comment at the beginning of a line
EOF
else
	text=File.read(filename)
end

begin
	@scanner=Scanner.new(:default) {
		# Default
		add_state(:default) {
			on '"'     , :string_literal       , :string_literal
			on '/'     , :default_slash        , :_unknown
			on ' '     , :default              , :whitespace
			on "\t"    , :default              , :whitespace
			on "\n"    , :default              , :whitespace
			on 'a'..'z', :identifier           , :identifier
			on 'A'..'Z', :identifier           , :identifier
			default :default                   , :default
		}
		add_state(:default_slash) {
			on '/' , :line_comment             , :line_comment
			on '*' , :block_comment            , :block_comment
#			on "\n", :default                  , :whitespace
#			on ' ' , :default                  , :whitespace
#			on "\t", :default                  , :whitespace
			default  :default                  , :default, :reread
		}

		# Identifier
		add_state(:identifier) {
			on 'a'..'z', :identifier, :identifier
			on 'A'..'Z', :identifier, :identifier
			on '_'     , :identifier, :identifier
			default      :default   , :_ignore   , :reread
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


	lines_tokens=text.lines.map { |line|
		line_tokens=[]

		@scanner.scan(line) { |token_type, code|
			puts "Add token #{token_type.inspect}"
			line_tokens << Token.new(token_type, code)
		}

		line_tokens
	}
#ensure
	pretty_print lines_tokens if print
end

