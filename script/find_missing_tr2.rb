#!/usr/bin/env ruby

# TODO:
#   * print tokens only for bad lines
#   * allow qnotrCount
#   * make sure that variables starting with ntr_ are passed through a
#     translation function, and that on assignment, the value is wrapped in
#     QT_TRANSLATE_NOOP3.

$: << File.dirname($0)

require 'ostruct'
require 'rubygems'

require 'rainbow'

require 'cpp_scanner'
require 'token'


################
## Recognizer ##
################

# An FSM that inputs tokens and determines whether a string is preceded by a
# given sequence of tokens
# On each token input, if the token matches the pattern for the current state,
# the state is advanced. The next token will then be checked against the next
# pattern. If the token does not match the pattern, the state is reset.
# A string token is accepted whenever it matches the pattern at the time when
# it is input.
class Recognizer
	# Parameters:
	#   name: only used for display to the user if verbose is true
	#   verbose: report state changes to the user
	#   patterns: an array of patterns to check the tokens against. A pattern
	#             is an array containing the parameters passed to Token.match?.
	def initialize(name, verbose, patterns)
		@name=name
		@verbose=verbose
		@patterns=patterns

		@state=0
		@accept=false
	end

	def input_token(token)
		print "Recognizer #{@name} input token #{token.type.inspect} #{token.code.inspect}, " if @verbose

		# Fetch the pattern for the current state from the list
		current_pattern=@patterns[@state]

		# Try to match the token against the pattern
		if token.match?(*current_pattern)
			# The token matched tha pattern. Advance the state and accept the
			# token.
			puts "accepted" if @verbose
			@state+=1
			@accept=true
			@state=0 if @state>=@patterns.size
		else
			# The token did not match the pattern. Reset the state and reject
			# the token.
			puts "reset" if @verbose
			@state=0
			@accept=false
		end
	end

	# Returns whether the last token was accepted
	def accept?
		@accept
	end
end

# The recognizer list
@@recognizers=[
	Recognizer.new("Empty strings", false, [
		[:string_literal, /"\s*"/ ]]),
	Recognizer.new("Single-argument functions", false, [
		[:identifier    , /^(notr|qnotr|qnotrUtf8|QDir)$/],
		[:default       , "(" ],
		[:string_literal, nil ]]),
	Recognizer.new("Two-argument functions", false, [
		[:identifier    , "tr"],
		[:default       , "(" ],
		[:string_literal, nil ],
		[:default       , "," ],
		[:string_literal, nil ]]),
	Recognizer.new("Three-argument functions", false, [
		[:identifier    , /^(QT_TRANSLATE_NOOP|QT_TRANSLATE_NOOP3)$/],
		[:default       , "(" ],
		[:string_literal, nil ],
		[:default       , "," ],
		[:string_literal, nil ],
		[:default       , "," ],
		[:string_literal, nil ]]),
	# Also a three-argument function, but not invoked with a single token
	Recognizer.new("qApp->translate", false, [
		[:identifier    , "qApp"],
		[:default       , "->" ],
		[:identifier    , "translate"],
		[:default       , "(" ],
		[:string_literal, nil ],
		[:default       , "," ],
		[:string_literal, nil ],
		[:default       , "," ],
		[:string_literal, nil ]]),
	Recognizer.new("include", false, [
		[:default       , /\#$/     ],
		[:identifier    , "include"],
		[:string_literal, nil      ]]),
	Recognizer.new("extern \"C\"", false, [
		[:identifier    , "extern"],
		[:string_literal, nil     ]])]


###############
## Test data ##
###############

# Returns an array of test lines.
# Each line is an array consting of:
#  * a string of code to test
#  * an array, indicating, for each string in the code, whether it is good (the
#    ground truth)
#
def test_data
	# [] is mapped to an empty line, that is, a zero-length string and an empty
	# array.
	test_lines=[
		# Simple cases
		['foo ();'                        , []      ], # No quoted text
		['a="moo"'                        , [false] ], # Quoted text
		[],
		# With comments
		['foo ();  // A "commented" string'     , []      ], # Quoted text in comment
		['a="moo"; // A comment'                , [false] ], # Quoted text and comment
		['a="moo"; // A "commented" string'     , [false] ], # Quoted text and quoted text in comment
		['a="moo", 2/3; // A "commented" string', [false] ], # Quoted text and quoted text in comment
		[],
		# With include
		['#include "foo.h"'               , [true]  ], # Regular include
		['    #include "foo.h"'           , [true]  ], # Indented include
		['foo (); // #include "foo.h"'    , []      ], # No quote, include in comment
		['a="moo"; // #include "foo.h"'   , [false] ], # Quoted text, #include in comment
		[],
		# With tr
		['foo=tr ("moo");'                , [true ] ],
		['foo=tr("moo");'                 , [true ] ],
		['foo=notr("moo");'               , [true ] ],
		['foo=qnotr("moo");'              , [true ] ],
		['foo=fauxtr ("moo");'            , [false] ],
		['foo=fauxtr("moo");'             , [false] ],
		['foo=t/*.*/r("moo");'            , [false] ], # Not tr
		['foo=faux/*.*/tr("moo");'        , [true ] ], # Not fauxtr
		[],
		# With string concatenation
		['foo=tr("moo" "bert");'          , [true, true] ],
		['foo=tr("moo" /* ... */ "bert");', [true, true] ],
		['foo=tr("moo" // ...'            , [true] ],
		['    "bert");'                   , [true] ],
		[],
		# With two-argument form of tr
		['foo=tr ("moo", "disambiguation")'       , [true, true] ],
		['foo=tr ("moo" "bert", "disambiguation")', [true, true, true] ],
		['foo=tr ("moo", "dis" "ambiguation")'    , [true, true, true] ],
		[],
		# With escaped quotes
		['foo=tr ("foo\\"bar")',           [true ]       ], # Single escaped quote
		['foo=tr ("foo\\"bar\\"baz")',     [true ]       ], # Multiple escaped quotes
		['foo=tr ("foo\\\\"bar\\\\"baz")', [true, false] ], # Now they are not escaped and actually end the string
		[],
		# With more complex tr
		['foo=qApp->translate("context","moo")'                        , [true, true]       ], # No whitespace
		[' foo  =  qApp  ->  translate  (  " context "  ,  " moo "  ) ', [true, true]       ], # Lots of whitespace
		['foo=qApp->translate("context","moo","comment")'              , [true, true, true] ], # 3-argument form
		[],
		# extern "C"
		['extern"C"{}'        , [true] ],
		['extern  "C"  {  }  ', [true] ],
		[],
		# Multi-line strings
		['foo="moo'    , [false] ], # Multi-line string
		['bert";'      , [false] ], # Multi-line string, continued
		['foo=tr ("moo', [true ] ], # Multi-line string
		['bert");'     , [true ] ], # Multi-line string, continued
		['foo=tr('     , []      ], # String on next line
		['"bar");'     , [true ] ], # String on next line
		[],
		# Empty strings
		['a=""'    , [true] ], # Empty string
		['a=" "'   , [true] ], # Empty string
		['a="   "' , [true] ], # Empty string
		[],
		# String concatenation
		['foo=tr ("moo" "bert")'        , [true, true]        ], # String concatenation
		['foo=tr ("moo" "bert"), "bar"' , [true, true, false] ], # Missing after string concatenation
		[],
		# With multiple strings
		['foo (tr ("moo"), tr ("bert"));' , [true , true ] ],
		['foo (("moo"), tr ("bert"));'    , [false, true ] ],
		['foo (tr ("moo"), ("bert"));'    , [true , false] ],
		['foo (tr (42), ("bert"));'       , [false]        ], # This tr may not be associated with that string
	].map { |x| if x==[] then ['', []] else x; end }
end

# A string construccted by concatenating the text parts of the test data
def test_data_text
	test_data.map { |l| l[0] }.join("\n")
end


############
## Output ##
############

# Format a line, given the tokens: strings will be colored according to whether
# they are good, everything else will be plain.
#
# Paramters:
#   * tokens: the tokens for the line
#   * string_good: a hash from string tokens to true if the string is good
#   * chomp: whether to chomp the token values. Note that the string may be
#     wrapped in color codes, so chomping the returned value may not work.
#
def format_single_line(tokens, string_good, chomp)
	tokens.map { |token|
		code=token.code
		type=token.type

		code=code.chomp if chomp

		if type==:string_literal
			good=string_good[token];
			if    good==true  then color=:green
			else                   color=:red
			end
			code.color(color)
		else
			code
		end
	}.join
end

# Prints lines, given the tokens
#
# Parameters:
#   * lines_tokens: an array containing an array of tokens for each line
#   * filename: the name of the file the data was read from, for display
#   * string_good: a hash from string tokens to true if the string is good
#   * display_all: whether to display all lines, as opposed to displaying only
#     lines containing at least one bad string
def print_lines(lines_tokens, filename, string_good, display_all)

	# Iterate over the lines
	lines_tokens.each_with_index { |tokens, index|

		# Determine whether this line will be displayed
		display=if display_all
			# All line are displayed
			true
		else
			# Only lines with a bad string are displayed. Find out whether this
			# line contains a bad string.
			tokens.find { |token|
				token.type==:string_literal && !string_good[token]
			}
		end
			
		if display
			# Display the line
			formatted_line=format_single_line(tokens, string_good, true)
			puts "#{filename}:#{index+1}:#{formatted_line}"
		end
	}
end


########################
## Main functionality ##
########################

# Creates a hash from string tokens to true if the token is good
#
# This function implements the core checking functionality.
#
def check_string_literals(lines_tokens)

	# Initialize the hash. A string not explicitly set to true (good) will
	# return false (bad)
	string_good={}
	string_good.default=false

	# Adjacent string literal tokens will be treated lke a single token. This
	# is useful for handling C string concatenation.
	last_token_was_string=false
	last_token_was_good  =false

	# Iterate over the lines
	lines_tokens.each { |tokens|

		# Iterate over the tokens of this line
		tokens.each { |token|

			# Whitespace and comments will be ignored. All other tokens will be
			# passed to the recognizers.
			if token.type==:whitespace || token.type==:line_comment || token.type==:block_comment
				# Ignore
			else
				if token.type==:string_literal && last_token_was_string
					# This token is a string literal, as was the last token.
					# Give it the same status as the last one.
					string_good[token]=true if last_token_was_good
				else
					# Only accept the token if at least recognizer accepts it
					accept=false

					# Pass the token to all recognizers. Note that even when
					# one recognizer has accepted the token (i. e., marked it
					# as good), we still have to pass it to the other
					# recognizers so their state will be tracked correctly.
					@@recognizers.each { |recognizer|
						recognizer.input_token token
						accept=true if recognizer.accept?
					}

					# If any recognizer has accepted the token, add it to the
					# list of good tokens.
					string_good[token]=true if accept

					# Store whether this token was a string for string
					# concatenation handling
					last_token_was_string=(token.type==:string_literal)
					last_token_was_good  =accept
				end
			end
		}
	}

	return string_good
end


# Perform the unit tests
def do_unit_tests
	data=test_data

	# Construct the raw text
	text=data.map { |data_line| data_line[0] }.join("\n")

	# Scan the text
	scanner=CppScanner.new
	lines_tokens=scanner.scan_lines_token_list(text)

	# Make sure that we have as many token lines as text lines
	# The text is assembled from the test data lines and then split for
	# tokenization, so this may go wrong.
	raise "Test data has #{data.size} lines, lines_tokens has #{lines_tokens.size} lines" if lines_tokens.size!=data.size

	# Do the actual checking
	string_good=check_string_literals(lines_tokens)

	# For each line, extract the string validity list and compare it with the
	# ground truth
	data.each_with_index { |data_line, index|
		ground_truth=data_line[1]

		tokens=lines_tokens[index]

		check_result=tokens.select { |token|
			token.type==:string_literal
		}.map { |token|
			string_good[token]
		}

		if check_result==ground_truth
			puts "OK      - #{format_single_line(tokens, string_good, true)}"
		else
			puts "Failure - #{format_single_line(tokens, string_good, true)}       - expected #{ground_truth.inspect}, got #{check_result.inspect}"
		end
	}

end

# Check a text and print it
def do_check_text(text, filename, print_tokens, print_all)
	# Scan the text, produce tokens
	scanner=CppScanner.new
	lines_tokens=scanner.scan_lines_token_list(text)

	# Print the tokens if requested
	CppScanner.pretty_print_lines_tokens lines_tokens if print_tokens

	# Check the strings
	string_good=check_string_literals(lines_tokens)

	# Print lines (all or bad only)
	print_lines(lines_tokens, filename, string_good, print_all)

end


#############
## Options ##
#############

# List the files to process
# Returns an array of paths relative to the current directory.
def list_automatic_files
	filenames=Dir.glob('**/*.cpp')+Dir.glob('**/*.h')
	filenames.reject! { |filename| filename =~ /^src\/db\/migrations\/Migration_/ }
	filenames.reject! { |filename| filename =~ /^test\// }

	return filenames
end

# Print a help message
def help_message
	puts "Usage:"
	puts "#{$0} [options] files...    - process specified files"
	puts "#{$0} [options] -a          - process automatically found files"
	puts "#{$0} [options] --test-data - process integrated test data"
	puts "#{$0} -t                    - run integrated unit tests"
	puts
	puts "Options:"
	puts "  -v             - verbose"
	puts "  --print-tokens - print tokenization of the lines"
	puts "  --print-all    - print all lines, not just lines containing errors"
end

# Parse the options. On error, output a usage message and exit.
# Returns an OpenStruct containing the option values.
def parse_options
	options=OpenStruct.new
	options.unit_tests   =false
	options.use_test_data=false
	options.print_tokens =false
	options.print_all    =false
	options.auto         =false
	options.verbose      =false
	options.filenames    =[]

	while arg=ARGV.shift do
		if    arg=="--test-data"    then options.use_test_data=true
		elsif arg=="--print-all"    then options.print_all    =true
		elsif arg=="--print-tokens" then options.print_tokens =true
		elsif arg=="-t"             then options.unit_tests   =true
		elsif arg=="-a"             then options.auto         =true
		elsif arg=="-v"             then options.verbose      =true
		else  filenames << arg
		end
	end

	options.filenames+=list_automatic_files if options.auto

	if options.filenames.empty? && !options.use_test_data && !options.unit_tests
		help_message
		exit 1
	end

	return options
end


##########
## Main ##
##########

# Parse the options. On error, output a usage message and exit.
options=parse_options

# Perform the requested operation
if options.unit_tests
	# Perform unit tests. Disregard any files that may have been specified.
	do_unit_tests
else
	# Perform the actual string checks .

	if options.use_test_data
		# Check the test data. Disregard any files that may have been
		# specified in addition.

		text=test_data_text
		do_check_text text, "test-data", options.print_tokens, options.print_all

	else
		# Check the specified files, including automatically found files.

		options.filenames.each { |filename|
			puts filename if options.verbose
			text=File.read(filename)
			do_check_text text, filename, options.print_tokens, options.print_all
		}

	end

end

