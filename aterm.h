#ifndef ATERM_H
#define ATERM_H

#define ESC "\033"
#define AND ";"
#define CSI ESC "["				// Control sequence introducer

#define CUU(n) CSI n "A"			// Cursor up
#define CUD(n) CSI n "B"			// Cursor down
#define CUF(n) CSI n "C"			// Cursor forward
#define CUB(n) CSI n "D"			// Cursor backward
#define CNL(n) CSI n "E"			// Cursor next line
#define CPL(n) CSI n "F"			// Cursor previous line
#define CHA(n) CSI n "G"			// Cursor horizontal absolute
#define CUP(y,x) CSI y AND x "H"		// Cursor position

#define ED(n) CSI n  "J"			// Erase display
#define EL(n) CSI n  "K"			// Erase line
#define SU(n) CSI n  "S"			// Scroll up
#define SD(n) CSI n  "T"			// Scroll down

#define CUS CSI "?25h" 				// Cursor show
#define CUH CSI "?25l"				// Cursor hide
#define DSR CSI "6n"				// Device status report
#define SCP CSI "s"				// Save cursor position
#define RCP CSI "u"				// Restore cursor position

#define CLS ED("2")

#define SGR(xs) CSI xs "m"			// Set graphics rendition

#define RESET         "0"
#define BOLD          "1"
#define FAINT         "2"
#define ITALIC        "3"
#define UNDERLINE     "4"
#define SLOWBLINK     "5"
#define FASTBLINK     "6"
#define REVERSE       "7"
#define CONCEAL       "8"
#define STRIKETHRU    "9"
#define DEF_FONT     "10"
#define FONT(N)      "1" N
#define FRAKTUR      "20"
#define DUB_UNDERLN  "21"
#define NORMAL_COLR  "22"
#define NOITALIC     "23"
#define NOUNDERLN    "24"
#define NOBLINK      "25"
#define NOINVERSE    "27"
#define REVEAL       "28"
#define NOSTRKTHRU   "29"

#define BLACK    "0"
#define RED      "1"
#define GREEN    "2"
#define YELLOW   "3"
#define BLUE     "4"
#define MAGENTA  "5"
#define CYAN     "6"
#define WHITE    "7"
#define DEFAULT  "9"

#define FG_COLR(S)   "3" S
#define BG_COLR(S)   "4" S
#define FG_BCOLR(S)  "9" S
#define BG_BCOLR(S) "10" S

#define CUSTOM       "8" AND
#define RGB(R,G,B)   "2;" R AND G AND B
#define COLR_8BIT(N) "5;" N

#endif
