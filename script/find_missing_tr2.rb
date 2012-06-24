#!/usr/bin/env ruby

$: << File.dirname($0)

require 'rubygems'
require 'rainbow'

require 'Scanner'

class CodeSegment
	attr_accessor :type # :default, :string_literal, :block_comment, :line_comment
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
		:line_comment   => :blue
	}

	max_line_num=lines.size+1
	padlength=max_line_num.to_s.length

	lines.each_with_index { |segments, index|
		line_number=index+1
		print "#{line_number.to_s.rjust(padlength)}:"
		segments.each { |segment|
			color=colors[segment.type]
			raise "Unhandled segment type #{segment.type.inspect}" if !color
			print segment.code.chomp.color(color)
		}
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
  FUNCTION (); FUNCTION (); // COMMENT
  FUNCTION (); /* COMMENT */ FUNCTION ();
  FUNCTION ("STRING");
  /* Empty comment at end of line */ //
  multiple (); function_calls ();
  without (); any ();
  strings_or_comments ();

  // Adjacent segments
  "STRING""STRING"/* COMMENT *//* COMMENT */"STRING"// COMMENT

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
	@scanner=Scanner.new(:default) { |states|
		# Default
		states.add_state(:default) { |state|
			state.on '"', :string_literal, :string_literal
			state.on '/', :default_slash , nil
			state.default :default       , :default
		}
		states.add_state(:default_slash) { |state|
			state.on '/' , :line_comment , :line_comment
			state.on '*' , :block_comment, :block_comment
			state.on "\n", :default, :default
			state.default  :default, :default
		}

		# String literals
		states.add_state(:string_literal) { |state|
			state.on '"' , :default                 , :string_literal
			state.on '\\', :string_literal_backslash, :string_literal
			state.default  :string_literal          , :string_literal
		}
		states.add_state(:string_literal_backslash) { |state|
			state.default :string_literal          , :string_literal
		}

		# Line comment
		states.add_state(:line_comment) { |state|
			state.on "\n", :default                 , nil          
			state.on '\\', :line_comment_backslash  , :line_comment
			state.default  :line_comment            , :line_comment
		}
		states.add_state(:line_comment_backslash) { |state|
			state.default :line_comment, :line_comment
		}

		# Block comment
		states.add_state(:block_comment) { |state|
			state.on '*', :block_comment_asterisk  , :block_comment
			state.default :block_comment           , :block_comment
		}
		states.add_state(:block_comment_asterisk) { |state|
			state.on '/', :default                 , :block_comment
			state.default :block_comment           , :block_comment
		}
	}


	lines_tokens=text.lines.map { |line|
		line_tokens=[]

		@scanner.scan(line) { |token_type, code|
			line_tokens << CodeSegment.new(token_type, code)
		}

		line_tokens
	}
#ensure
	pretty_print lines_tokens if print
end

# FIXME handle unterminated things
#scan_cpp "// Unterminated line comment", true
#scan_cpp "/* Unterminated block comment", true
#scan_cpp "\"Unterminated string", true



