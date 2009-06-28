#include "src/color.h"


// Ganz tolle Pr�prozessor-Macros f�r das Ankreuzfeld-System, siehe
// flug_farbe ().
#define ist_true(x) ((x)==true)
#define ist_false(x) ((x)==false)
#define ist_egal(x) (true)
#define ist_lokal(x) ((x)==fmLocal)
#define ist_kommt(x) ((x)==fmComing)
#define ist_geht(x) ((x)==fmLeaving)
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
#define gruen 0,255,0
#define hellgruen 191,255,191
#define gelb 255,255,0
#define hellgelb 255,255,191
#define pink 255,255,0
#define white 255,255,255


QColor flug_farbe (FlightMode modus, bool fehler, bool schlepp, bool gestartet, bool gelandet)
	/*
	 * Finds out which color to use for a given flight.
	 * Parameters:
	 *   - modus: the mode of the flight.
	 *   - fehler: whether the flight is erroneous.
	 *   - schlepp: whether the flight is a towflight
	 *   - gestarted: whether the flight has started.
	 *   - gelandet: whether the flight has landed.
	 * Return value:
	 *   - the color for the flight.
	 */
{
	// Ganz tolles Farbauswahl-Ankreuzfeld-C-Pr�prozessor-System, gebaut von
	// Martin, dem Helden der Softwareentwicklung f�r die Luftfahrt.
	// Man trage in die entsprechende Spalte ein, wof�r diese Farbe gelten
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

	// Fehlerhafte Fl�ge
	farbe (xx, t, x, x, t, rot);		// Schleppflug Fehler
	farbe (xx, f, x, x, t, hellrot);	// Fehler
	farbe (ge, x, f, t, x, pink);		// }Gelandet, aber nicht gestartet: Programmfehler
	farbe (lo, x, f, t, x, pink);		// }muss vorher gecheckt werden und Fehler ausl�sen

	// Fl�ge von extern
	farbe (ko, f, x, f, f, hellblau);	// Angek�ndigt==Fliegt
	farbe (ko, f, x, t, f, hellgruen);	// Gelandet
	farbe (ko, t, x, x, f, pink);		// Programmfehler: es gibt keine kommenden Schlepps

	// Fl�ge nach extern
	farbe (ge, f, f, x, f, hellgelb);	// Vorbereitet
	farbe (ge, f, t, x, f, hellgruen);	// Landung wird nicht aufgezeichnet
	farbe (ge, t, f, x, f, gelb);		// Schlepp, Vorbereitet, kommt nicht vor
	farbe (ge, t, t, x, f, gruen);		// Schlepp, Landung wird nicht aufgezeichnet

	// Lokale Fl�ge
	farbe (lo, f, f, x, f, hellgelb);	// nicht gestartet
	farbe (lo, f, t, f, f, hellblau);	// Flug in der Luft
	farbe (lo, f, t, t, f, hellgruen);	// gelandeter Flug
	farbe (lo, t, f, x, f, gelb);		// nicht gestarteter Schleppflug
	farbe (lo, t, t, f, f, blau);		// Schleppflug in der Luft
	farbe (lo, t, t, t, f, gruen);		// gelandeter Schleppflug: gr�n

//	Template
//	farbe (ll, x, x, x, x, white);
//	fargb (ll, x, x, x, x, 000, 000, 000);

	return QColor (255, 255, 255);	// Default: wei�
}







