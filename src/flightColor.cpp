#include "src/flightColor.h"


// Ganz tolle Präprozessor-Macros für das Ankreuzfeld-System, siehe
// flightColor ().
#define ist_true(x) ((x)==true)
#define ist_false(x) ((x)==false)
#define ist_egal(x) (true)
#define ist_lokal(x) ((x)==Flight::modeLocal)
#define ist_kommt(x) ((x)==Flight::modeComing)
#define ist_geht(x) ((x)==Flight::modeLeaving)
#define farbe(mod, sch, sta, lan, feh, far)	\
	if (mod(modus) && sch(schlepp) && sta(gestartet) && lan(gelandet) && feh(fehler)) return QColor (far);
#define fargb(mod, sch, sta, lan, feh, rot, gru, bla)	\
	if (mod(modus) && sch(schlepp) && sta(gestartet) && lan(gelandet) && feh(fehler)) return QColor (rot, gru, bla);
#define t ist_true
#define f ist_false
#define x ist_egal
#define lo ist_lokal
#define ko ist_kommt
#define ge ist_geht
#define xx ist_egal

#define rot 255,0,0
#define hellrot 255,127,127
#define blau 127,127,255
#define hellblau 191,191,255
#define schleppblau 162,162,216
#define gruen 0,255,0
#define hellgruen 191,255,191
#define gelb 255,255,0
#define hellgelb 255,255,191
#define pink 255,255,0
#define white 255,255,255

// TODO move to flight
QColor flightColor (Flight::Mode modus, bool fehler, bool schlepp, bool gestartet, bool gelandet)
	/*
	 * Finds out which color to use for a given flight.
	 * Parameters:
	 *   - mode: the mode of the flight.
	 *   - fehler: whether the flight is erroneous.
	 *   - schlepp: whether the flight is a towflight
	 *   - gestarted: whether the flight has departed
	 *   - landed: whether the flight has landed.
	 * Return value:
	 *   - the color for the flight.
	 */
{
	// Ganz tolles Farbauswahl-Ankreuzfeld-C-Präprozessor-System, gebaut von
	// Martin, dem Helden der Softwareentwicklung für die Luftfahrt.
	// Zweck der Aktion ist, dass man erkennen kann, dass jeder mögliche Fall
	// behandelt ist.
	// Man trage in die entsprechende Spalte ein, wofür diese Farbe gelten
	// soll. t: die entsprechende Variable muss true sein, f: die
	// entsprechende Variable muss false sein, x: der Wert der Variablen ist
	// egal. Der erste Treffer gewinnt.
	// Bei Modus: "ge": Modus muss "geht" sein; "ko": ..."kommt"...; "lo":
	// ..."lokal"...; "xx": Modus ist egal
	//         S
	//         c        F
	//     M   h  S     e
	//     o   l  t  L  h
	//     d   e  a  a  l
	//     u   p  r  n  e
	//     s   p  t  d  r

	// Fehlerhafte Flüge
	farbe (xx, t, x, x, t, hellrot);	// Schleppflug Fehler
	farbe (xx, f, x, x, t, hellrot);	// Fehler
	farbe (ge, x, f, t, x, pink);		// }Gelandet, aber nicht gestartet: Programmfehler
	farbe (lo, x, f, t, x, pink);		// }muss vorher gecheckt werden und Fehler auslösen

	// Flüge von extern
	farbe (ko, f, x, f, f, hellblau);	// Angekündigt==Fliegt
	farbe (ko, f, x, t, f, hellgruen);	// Gelandet
	farbe (ko, t, x, x, f, pink);		// Programmfehler: es gibt keine kommenden Schlepps

	// Flüge nach extern
	farbe (ge, f, f, x, f, hellgelb);	// Vorbereitet
	farbe (ge, f, t, x, f, hellgruen);	// Landung wird nicht aufgezeichnet
	farbe (ge, t, f, x, f, gelb);		// Schlepp, Vorbereitet, kommt nicht vor
	farbe (ge, t, t, x, f, hellgruen);	// Schlepp, Landung wird nicht aufgezeichnet

	// Lokale Flüge
	farbe (lo, f, f, x, f, hellgelb);	// nicht gestartet
	farbe (lo, f, t, f, f, hellblau);	// Flug in der Luft
	farbe (lo, f, t, t, f, hellgruen);	// gelandeter Flug
	farbe (lo, t, f, x, f, gelb);		// nicht gestarteter Schleppflug (gibt es nicht)
	farbe (lo, t, t, f, f, schleppblau);// Schleppflug in der Luft
	farbe (lo, t, t, t, f, hellgruen);	// gelandeter Schleppflug

//	Template
//	farbe (ll, x, x, x, x, white);
//	fargb (ll, x, x, x, x, 000, 000, 000);

	return QColor (255, 255, 255);	// Default: weiß
}
