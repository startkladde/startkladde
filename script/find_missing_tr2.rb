#!/usr/bin/env ruby

require 'rubygems'
require 'rainbow'




def scan_cpp(text, print)
	colors = {
		:default        => :cyan,
		:string_literal => :yellow,
		:block_comment  => :green,
		:line_comment   => :blue
	}

	colored=""

#
#                |   |  |            |     |                   |         |   |   index of separator element
#                1   1  2            1     2                   2         1   1
# <-------------><---><><-----------><----><--------------------><------><--->x  segment boundaries
# >               v   v   v           v      v                   v        v   >  start position for next search
# void foo (); a="bar"; // assignment|b=2; /* more assignments */ return "baz";
# ---------------sssss--lllllllllllll------bbbbbbbbbbbbbbbbbbbbbb--------sssss-


	# :default :string_literal :block_comment :line_comment
	state=:default
	segment_start=0

	begin
		# segment_start must be a valid position at this point.

		while state
			case state
			when :default
				pos=text.index(/ (") | (\/\/) | (\/\*) /x, segment_start)
				if    $1 then next_state=:string_literal; next_segment_start=pos
				elsif $2 then next_state=:line_comment  ; next_segment_start=pos
				elsif $3 then next_state=:block_comment ; next_segment_start=pos
				else          next_state=nil
				end
			when :string_literal
				pos=segment_start+1
				begin
					pos=text.index(/ (") | (\\\" | \\\\) /x, pos)
					if    $1 then next_state=:default; next_segment_start=pos+1
					elsif $2 then pos+=2
					else          next_state=nil
					end
				end until next_state!=:string_literal
			when :block_comment
				pos=text.index(/ (\*\/) /x, segment_start+2)
				if $1 then next_state=:default; next_segment_start=pos+2
				else       next_state=nil
				end
			when :line_comment
				pos=text.index(/ ($) /x, segment_start)
				if $1 then next_state=:default; next_segment_start=pos
				else       next_state=nil
				end
			else raise "Unhandled state #{state.inspect}"
			end

			if (next_segment_start!=segment_start)
				next_segment_start=0 if !next_state

				segment=text[segment_start..next_segment_start-1]
				puts "#{segment_start}-#{next_segment_start-1} - #{state.inspect} - #{segment.inspect}" if print
				colored += segment.color(colors[state])
			end

			segment_start=next_segment_start
			state=next_state
		end
	ensure
		if print
			puts
			puts colored
			puts
		end
	end

end

if ARGV[0]
	text=File.read(ARGV[0])
else
	text=<<EOF
// Comment at the beginning

void foo (const QString &x="STRING")
{
	// Simple cases
	FUNCTION (); FUNCTION (); // COMMENT
	FUNCTION (); /* COMMENT */ FUNCTION ();
	FUNCTION ("STRING");

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
}
// Comment at the beginning of a line
EOF
	puts text
	puts
end


scan_cpp text, true

scan_cpp "// Unterminated line comment", true
scan_cpp "/* Unterminated block comment", true
scan_cpp "\"Unterminated string", true



