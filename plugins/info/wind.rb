#!/usr/bin/env ruby

host="localhost"
port=8905

command="netcat #{host} #{port}"

class NilClass
	def round
		nil
	end
end

def extract(line, key)
	result=($1 if line =~ /#{key}([^,]*),/)
	result=result.to_f if result
	result
end


puts "Verbindung zur Wetterstation wird aufgebaut..."

IO.popen(command) { |io|
	while !io.eof?
		line=io.readline

		# Format:
		# 17:48:36, 13.02.08, TE5.32, DR1018.96, WR45.33, FE78.68, WG1.61, WS2.85, WD2.16, WC5.32, WV45.45,

		# Geschwindigkeiten in m/s
		wind_direction=extract(line, "WR")
		wind_velocity =extract(line, "WG")
		#wind_gust     =extract(line, "WS")

		# Geschwindigkeiten in Knoten umrechnen
		wind_velocity_kt=(wind_velocity*2 if wind_velocity)
		#wind_gust_kt    =(wind_gust    *2 if wind_gust    )

		# Ausgabe formatieren
		display_direction=(  "#{wind_direction  .round}°"  if wind_direction) || "?"
		display_velocity =(  "#{wind_velocity_kt.round}kt" if wind_velocity ) || "?"
		#display_gust     =(" G#{wind_gust_kt    .round}kt" if wind_gust     ) || ""

		#puts "#{display_direction}/#{display_velocity}#{display_gust}"
		puts "#{display_direction}, #{display_velocity}"
	end
}

