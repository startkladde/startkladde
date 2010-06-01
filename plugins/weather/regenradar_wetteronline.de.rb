#!/usr/bin/env ruby

$: << File.dirname($0) # Load files from the script directory

require 'weather_common'



begin
	index_url="http://www.wetteronline.de/daten/radarhtml/de/dwddg/radarf.htm"

	output_message "Radarbild wird heruntergeladen (1)..."
	index=download(index_url)
	raise "Fehler beim Abrufen der Wetterseite" if !index

	# Extract the image URL
	if index =~ /(daten\/radar[^"]*)"/
		image_url="http://www.wetteronline.de/#{$1}"
		puts image_url

		output_message "Radarbild wird heruntergeladen (2)..."
		image=download(image_url)
		raise "Fehler beim Abrufen der Wettergrafik" if !image

		image_file_name=temp_file_name("radar", "gif")
		File.open(image_file_name, "wb") { |file| file.print image }
		output_image image_file_name
	else
		raise "Auf der Wetterseite wurde keine Wettergrafik gefunden"
	end
rescue RuntimeError => ex
	output_message ex.to_s
end

