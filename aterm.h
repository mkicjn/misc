#ifndef ATERM_H
#define ATERM_H

#define CSI "\033["

#define CUU "A"
#define CUD "B"
#define CUF "C"
#define CUB "D"
#define CNL "E"
#define CPL "F"
#define CHA "G"
#define CUP "H"

#define ED  "J"
#define EL  "K"
#define SU  "S"
#define SD  "T"

#define CUS "?25h" 
#define CUH "?25l"
#define DSR "6n"
#define SCP "s"
#define RCP "u"

#define CLB  "1" ED
#define CLS  "2" ED
#define CLSB "3" ED

#define SEP ";"
#define AT_XY(X,Y) CSI #Y SEP #X CUP

#define SGR "m"

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
#define FONT(N)      "1" #N
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
#define CUSTOM   "8" SEP
#define DEFAULT  "9"

#define FG_COLR(S)   "3" S
#define BG_COLR(S)   "4" S
#define FG_BCOLR(S)  "9" S
#define BG_BCOLR(S) "10" S

#define RGB(R,G,B)   "2;" #R SEP #G SEP #B
#define COLR_8BIT(N) "5;" #N

#endif
