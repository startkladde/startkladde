#!/bin/bash

indexfile=/tmp/index.html
imagefile=/tmp/radar.gif

echo "[MSG] [Radarbild wird heruntergeladen]"

# Wetterseite herunterladen
if wget -O $indexfile "http://wetter.com/v2/index.php?SID=&LANG=DE&LOC=0270"; then
	# URL der Grafik extrahieren
	imageurl=`grep 'radar_DE[^/]*gif' $indexfile |sed 's/.*src="\([^"]*\)".*/\1/'`

	if [ -n "$imageurl" ]; then
		# Grafik herunterladen
		if wget -O $imagefile "$imageurl"; then
			echo "[IMG] [$imagefile]"
		else
			echo "[MSG] [Fehler beim Abrufen der Wettergrafik]"
		fi
	else
		echo "[MSG] [Auf der Wetterseite wurde keine Wettergrafik gefunden]"
	fi
else
	echo "[MSG] [Fehler beim Abrufen der Wetterseite]"
fi

rm $indexfile



