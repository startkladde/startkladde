#!/usr/bin/env ruby

# FIXME for strings and block comments, the token should always end at the
# closing character, even if a token of the same type starts immediately
# after. For example, /* foo *//* bar */ should be two tokens. Currently, we
# start a new token when the type changes. We either have to force a new token
# at the closing character or use a different criterion.

$: << File.dirname($0)

require 'rubygems'
require 'rainbow'

require 'cpp_scanner'
require 'token'

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

# For unit testing:
#def t(*args)
#	Token.new(*args)
#end
#lines=[]
#lines << ["foo ();", [t(:identifier, "foo"), t(:whitespace, " "), t(:default, "();"), t(:whitespace, "\n")]]
#text=lines.map { |line| line[0] }.join

else
	text=File.read(filename)
end

begin
	@scanner=CppScanner.new


	lines_tokens=text.lines.map { |line|
		line_tokens=[]

		@scanner.scan(line) { |token_type, code|
			#puts "Add token #{token_type.inspect}"
			line_tokens << Token.new(token_type, code)
		}

		line_tokens
	}
#ensure
	pretty_print lines_tokens if print
end

