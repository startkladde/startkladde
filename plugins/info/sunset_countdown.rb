#!/usr/bin/env ruby

$: << File.dirname($0) # Load files from the script directory

require 'sunset_common'

STDOUT.sync=true

while true
	begin
		# Time until sunset in seconds
		dt=truncate_seconds(determine_sunset)-truncate_seconds(Time.new)

		if dt<0
			puts "<font color=\"#FF0000\">-#{format_duration(dt)}</font>"
		else
			puts "<font color=\"#000000\">#{format_duration(dt)}</font>"
		end
	rescue RuntimeError => ex
		puts ex
	end

	wait_for_minute
end

