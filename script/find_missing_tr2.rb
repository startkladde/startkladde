#!/usr/bin/env ruby

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

# Takes a string of C++ code and splits it into lines and segments
# For example, the following code:
#   /* const */ int foo=42;
#   const char *bar="fred"; // A string
# Will be represented by:
#   * line 0:
#      * default        '  '
#      * block_comment  '/* const */'
#      * default        ' int foo=42;'
#   * line 1:
#      * default        '  const char *bar='
#      * string_literal '"fred"
#      * default        '; '
#      * line_comment   '// A string'
# This simple task is complicated by the fact that the meaning of the separator
# characters depends on their context. For example, double quotes inside a
# comment do not delimit a string.
class CppSegmenter
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

		# Start at the beginning of the line
		segment_start=0
		skip=0

		# As long as there is a segment, we have to process it
		while segment_start
			# We may have to skip some characters
			pos=segment_start+skip

			# By default, we stay in the current state and don't skip anything.
			next_state=@state; next_segment_start=nil; skip=0

			# Find the next separator. Possible separators depend on the state
			# we're currently in. For example, inside a string literal, "//" is
			# not a separator.
			# We set the following variables:
			#   * next_state: the state associated with the next segment
			#   * next_segment_start: the starting position of the next
			#     segment, or nil if the current segment extends to the end of
			#     the line (and possibly beyond)
			#   * skip: the number of characters to skip, starting at
			#     next_segment_start. This is the length of the separator if
			#     the separator belongs to the next part, or 0 if the separator
			#     belongs to the current part.
			case @state
			when :default
				# Look for the first string that can start a new segment
				# (separator): a double quote starts a string literal, two
				# slashes start a line comment and a slash followed by an
				# asterisk starts a block comment. Go to the appropriate state
				# and skip the separator.
				# If no separator is found, stay in the current state and
				# next_segment_start will be left at nil to indicate that the
				# segment extends to the end of the line.
				pos=line.index(/ (") | (\/\/) | (\/\*) /x, pos)
				if    $1 then next_state=:string_literal; next_segment_start=pos; skip=1
				elsif $2 then next_state=:line_comment  ; next_segment_start=pos; skip=2
				elsif $3 then next_state=:block_comment ; next_segment_start=pos; skip=2
				end
			when :string_literal
				# Look for the end of the string literal (a double quote), an
				# escaped backslash or an escaped quote. If an escaped
				# backslash or an escaped quote is encountered, it is skipped
				# and the process is repeated.
				# If no end of the string literal is found, stay in the current
				# state and next_segment_start will be left at nil to indicate
				# that the segment extends to the end of the line.
				begin
					pos=line.index(/ (") | (\\\") | (\\\\) /x, pos)
					if    $1 then next_state=:default; next_segment_start=pos+1; skip=0
					elsif $2 then pos+=2 # Skip over the escaped quote
					elsif $3 then pos+=2 # Skip over the escaped backslash
					end
				end until next_state!=@state || !pos
			when :block_comment
				# Look for the end of the block comment (an asterisk followed
				# by a slash).
				# If no end of the block comment is found, stay in the current
				# state and next_segment_start will be left at nil to indicate
				# that the segment extends to the end of the line.
				pos=line.index(/ (\*\/) /x, pos)
				if $1 then next_state=:default; next_segment_start=pos+2; skip=0
				end
			when :line_comment
				# Look for the end of the line comment (the end of the line).
				# If a backslash preceds the end of the line, this is not
				# counted as the end of the line comment. In this case, the
				# state is not changed.
				# In any case, next_segment_start is left at nil to indicate
				# that the segment extends to the end of the line.
				pos=line.index(/ (\\$) | ($) /x, pos)
				if    $1 then 
				elsif $2 then next_state=:default
				end
			else raise "Unhandled state #{@state.inspect}"
			end

			# It is possible that the segment has a length of zero. For
			# example, in the line
			#   "foo""bar"
			# there is a zero-length "default" segment between the two strings.
			# In this case, ignore it.
			if (next_segment_start!=segment_start)
				# If we found another segment, take the text up to that
				# segment. Otherwise, take the text up to the end of the line.
				if next_segment_start
					code=line[segment_start..next_segment_start-1]
				else
					code=line[segment_start..-1]
				end

				# Add the segment to the list
				result << CodeSegment.new(@state, code)
			end

			segment_start=next_segment_start
			@state=next_state
		end

		return result
	end

	def process_text(text, print)
		text.lines.map { |line|
			process_line(line)
		}
	end
end

#
#                |   |  |            |     |                   |         |   |   index of separator element
#                1   1  2            1     2                   2         1   1
# <-------------><---><><-----------><----><--------------------><------><--->x  segment boundaries
# >               v   v   v           v      v                   v        v   >  start position for next search
# void foo (); a="bar"; // assignment|b=2; /* more assignments */ return "baz";
# ---------------sssss--lllllllllllll------bbbbbbbbbbbbbbbbbbbbbb--------sssss-

if ARGV[0]
	text=File.read(ARGV[0])
else
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
  "multi_
line_
string";
  "string_with_\\
line continuation";

  // comment_with_\\
line_continuation

}
// Comment at the beginning of a line
EOF
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
			print segment.code.color(colors[segment.type])
		}
		puts
	}
end

begin
	segmenter=CppSegmenter.new
	segments=segmenter.process_text text, true
#ensure
	pretty_print segments
end


#scan_cpp text, true

#scan_cpp "// Unterminated line comment", true
#scan_cpp "/* Unterminated block comment", true
#scan_cpp "\"Unterminated string", true



