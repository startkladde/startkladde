#!/usr/bin/env ruby

puts "Winddaten sind nicht verfügbar"


#echo "Verbindung zur Wetterstation wird hergestellt..."
#message=""
#file=/tmp/wind.$$
#
#netcat -w 5 adam 8905 |while read a; do
#	# Format:
#	# 17:48:36, 13.02.08, TE5.32, DR1018.96, WR45.33, FE78.68, WG1.61, WS2.85, WD2.16, WC5.32, WV45.45,
#
#	# Rohdaten extrahieren
#	ws_time=`echo "$a" |sed 's/^\([^,]*\),.*/\1/'`
#	wind_dir=`echo "$a" |sed 's/.*WR\([^,]*\),.*/\1/'`
#	wind_vel=`echo "$a" |sed 's/.*WG\([^,]*\),.*/\1/'`
#	wind_gust=`echo "$a" |sed 's/.*WS\([^,]*\),.*/\1/'`
#	#echo "$ws_time - $wind_dir/${wind_vel}G${wind_gust}"
#
#	# Geschwindigkeiten in Knoten umrechnen
#	# $((...)) kann nur Ganzzahlen
#	wind_vel_kt=`echo "2*$wind_vel" |bc -l`
#	wind_gust_kt=`echo "2*$wind_gust" |bc -l`
#
#	# Geschwindigkeiten runden
#	wind_dir_display=`echo "scale=0; (${wind_dir}+0.5)/1" |bc -l`
#	wind_vel_kt_display=`echo "scale=0; (${wind_vel_kt}+0.5)/1" |bc -l`
#	wind_gust_kt_display=`echo "scale=0; (${wind_gust_kt}+0.5)/1" |bc -l`
#
#	# Ausgeben in der Form (dir)/(vel)G(gust)
#	message="$wind_dir_display/${wind_vel_kt_display}KT G${wind_gust_kt_display}KT"
#	echo "$message"
#	echo "$message" >$file
#done
#
#lastmessage=`head -n 1 $file`
#if [ -n "$lastmessage" ]; then
#	echo "($lastmessage)"
#fi
#
#rm -f $file


