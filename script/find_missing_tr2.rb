#!/usr/bin/env ruby

require 'rubygems'
require 'rainbow'


class Location
	attr_accessor :line, :column

	def initialize(line, column)
		@line=line
		@column=column
	end

	def to_s
		"#{line}.#{column}"
	end
end

class CodeSegment
	attr_accessor :type # :default, :string_literal, :block_comment, :line_comment
	attr_accessor :start, :end
	attr_accessor :code

	def initialize
		@code=""
	end
end

class CppSegmenter
	attr_reader :segments, :state

	def initialize
		@state=:default
		@segments=[]
		@current_segment=nil
	end


	def process_line(line_number, line)
		# We split the line into parts corresponding to segments, except that
		# the segment corresponding to the first part may have been started in
		# a previous line, and that the segment corresponding to the last part
		# may continue to the following line(s).

		# The column pointer is the position of the part we're currently
		# looking at. We'll set it to nil when there are no more parts in the
		# line. We'll start at the beginning of the line.
		part_start=0

		skip=0

		# Keep analyzing parts until there are no parts left in this line
		while part_start
			#puts "Analyze part at line #{line_number}, column #{part_start}"

			# By default, we'll stay in the current state
			next_state=@state

			# Find the next separator. Possible separators depend on the state
			# we're currently in.
			# We set the following variables:
			#   * next_state: the state associated with the next part
			#   * next_part_start: the starting position of the next part, or
			#     nil if there is no next part in this line (this probably
			#     means that the part is continued in the next line)
			#   * skip: the number of characters to skip, starting at
			#     next_part_start. This is the length of the separator if the
			#     separator belongs to the next part, or 0 if the separator
			#     belongs to the current part.
			case @state
			when :default
				pos=line.index(/ (") | (\/\/) | (\/\*) /x, part_start+skip)
				if    $1 then next_state=:string_literal; next_part_start=pos; skip=1
				elsif $2 then next_state=:line_comment  ; next_part_start=pos; skip=2
				elsif $3 then next_state=:block_comment ; next_part_start=pos; skip=2
				else                                      next_part_start=nil
				end
			when :string_literal
				pos=part_start+skip
				begin
					pos=line.index(/ (") | (\\\" | \\\\) /x, pos)
					if    $1 then next_state=:default; next_part_start=pos+1; skip=0
					elsif $2 then pos+=2 # Skip over the escaped backslash or quote
					else                               next_part_start=nil
					end
				end until next_state!=@state || !pos
			when :block_comment
				pos=line.index(/ (\*\/) /x, part_start+skip)
				if $1 then next_state=:default; next_part_start=pos+2; skip=0
				else                            next_part_start=nil
				end
			when :line_comment
				pos=part_start+skip
				begin
					pos=line.index(/ (\\$) | ($) /x, pos)
					if    $1 then pos+=2 # Skip over the escaped newline
					elsif $2 then next_state=:default; next_part_start=nil
					else                               next_part_start=nil
					end
				end until next_state!=@state
			else raise "Unhandled state #{@state.inspect}"
			end

			# It is possible that the part has a length of zero. In this case,
			# ignore it.
			if (next_part_start!=part_start)
				# We need to append the part to the current segment. If there
				# is no current segment, create one.
				if !@current_segment
					@current_segment=CodeSegment.new
					@current_segment.type=@state
					@current_segment.start=Location.new(line_number, part_start)
					@current_segment.end=nil

					@segments << @current_segment
				end

				# Determine the end of the current part. If another part starts in
				# this line, the current part ends right before the start of the
				# next part. Otherwise, the current part ends at the end of the
				# line.
				if next_part_start
					part_end=next_part_start-1
				else
					part_end=line.length-1
				end

				# Append the part to the segment. Also set the end of the part
				# in case there is nothing after it.
				@current_segment.code+=line[part_start..part_end]
				@current_segment.end=Location.new(line_number, part_end)

				# Now, if the state changed, this means that we are done with
				# the last segment.
				@current_segment=nil
			end

			#puts "State: #{@state.inspect} -> #{next_state.inspect}, at column #{part_start}"

			part_start=next_part_start
			@state=next_state
		end
	end

	def process_text(text, print)
		text.lines.each_with_index { |line_number, line|
			process_line line, line_number
		}
	end

#	def segment(text, print)
#		# :default :string_literal :block_comment :line_comment
#		state=:default
#		segment_start=0
#
#		begin
#			# segment_start must be a valid position at this point.
#
#			end
#		end
#	end

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

def pretty_print(segments)
	colors = {
		:default        => :cyan,
		:string_literal => :yellow,
		:block_comment  => :green,
		:line_comment   => :blue
	}
	colored=""
	segments.each { |segment|
		puts "#{segment.start}-#{segment.end} - #{segment.type.inspect} - #{segment.code.inspect}"
		colored += segment.code.color(colors[segment.type])
	}
	puts colored
end

begin
	segmenter=CppSegmenter.new
	segmenter.process_text text, true
#ensure
	pretty_print segmenter.segments
end


#scan_cpp text, true

#scan_cpp "// Unterminated line comment", true
#scan_cpp "/* Unterminated block comment", true
#scan_cpp "\"Unterminated string", true



