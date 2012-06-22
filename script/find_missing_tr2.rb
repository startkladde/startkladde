#!/usr/bin/env ruby

$: << File.dirname($0)

require 'CppSegmenter1'
require 'CppSegmenter2'
require 'CppSegmenter3'
require 'CppSegmenter4'


require 'rubygems'
require 'rainbow'


#class Location
#	attr_accessor :line, :column
#
#	def initialize(line, column)
#		@line=line
#		@column=column
#	end
#
#	def to_s
#		"#{line}.#{column}"
#	end
#end

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
			print segment.code.color(color)
		}
		puts
	}
end

use_test=false
print=false
segmenter_class=nil
filename=nil

while arg=ARGV.shift do
	if    arg=="-t" then use_test=true
	elsif arg=="-p" then print=true
	elsif arg=="-1" then segmenter_class=CppSegmenter1
	elsif arg=="-2" then segmenter_class=CppSegmenter2
	elsif arg=="-3" then segmenter_class=CppSegmenter3
	elsif arg=="-4" then segmenter_class=CppSegmenter4
	else  filename=arg
	end
end

error=false

if filename.nil? && !use_test
	puts "Specify a file name or -t for test text"
	error=true
end

if segmenter_class.nil?
	puts "Sepcify -1, -2, -3 or -4 for a segmenter implementation to use"
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
}
// Comment at the beginning of a line
EOF
else
	text=File.read(filename)
end

begin
	segmenter=segmenter_class.new
	segments=segmenter.process_text text, true
#ensure
	pretty_print segments if print
end

# FIXME handle unterminated things
#scan_cpp "// Unterminated line comment", true
#scan_cpp "/* Unterminated block comment", true
#scan_cpp "\"Unterminated string", true



