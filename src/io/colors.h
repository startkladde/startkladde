#ifndef COLORS_H_
#define COLORS_H_

// TODO remove


#define c_1 "\033["
#define c_0 "m"
#define	clr_default "0"
#define at_bold "1"
#define at_underline "4"
#define at_blink "5"
#define at_inverse "7"
#define at_nobold "22"
#define at_nounderline "24"
#define at_noblink "25"
#define at_noinverse "27"

#define fg_black "30"
#define fg_red "31"
#define fg_greed "32"
#define fg_yellow "33"
#define fg_blue "34"
#define fg_magenta "35"
#define fg_cyan "36"
#define fg_white "37"
#define fg_default "39"

#define bg_black "40"
#define bg_red "41"
#define bg_greed "42"
#define bg_yellow "43"
#define bg_blue "44"
#define bg_magenta "45"
#define bg_cyan "46"
#define bg_white "47"
#define bg_default "49"

#define c_default c_1 clr_default c_0
#define c_error c_1 fg_red ";" at_bold c_0
#define c_message c_1 fg_yellow c_0
#define c_funclog c_1 fg_black ";" at_bold c_0
#define c_dbase c_1 fg_blue c_0


#endif

