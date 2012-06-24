#!/usr/bin/env ruby


$: << File.dirname($0)

require 'rubygems'
require 'rainbow'

require 'cpp_scanner'
require 'token'

# For unit testing:
#def t(*args)
#	Token.new(*args)
#end
#lines=[]
#lines << ["foo ();", [t(:identifier, "foo"), t(:whitespace, " "), t(:default, "();"), t(:whitespace, "\n")]]
#text=lines.map { |line| line[0] }.join
def test_data
	<<EOF
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
end

use_test_data=false
print=false
filename=nil

while arg=ARGV.shift do
	if    arg=="-t" then use_test_data=true
	elsif arg=="-p" then print=true
	else  filename=arg
	end
end

error=false

if filename.nil? && !use_test_data
	puts "Specify a file name or -t for a test text"
	error=true
end

exit 1 if error

if use_test_data
	text=test_data
else
	text=File.read(filename)
end

begin
	scanner=CppScanner.new

	lines_tokens=scanner.scan_lines_token_list(text)

	if print
		CppScanner.pretty_print_lines_tokens lines_tokens
	else
		puts "#{lines_tokens.size} lines processed (-p to print)"
	end
end

