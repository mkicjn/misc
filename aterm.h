#ifndef ATERM_H
#define ATERM_H

#define CSI "\033["
#define CUP "H"
#define ED  "J"
#define SGR "m"
#define SCP "s"
#define RCP "u"

#define CUU "A"
#define CUD "B"
#define CUF "C"
#define CUB "D"

#define CUS "?25h" 
#define CUH "?25l"

#define CLS "2" ED

#define AT_XY(X,Y) CSI #X ";" #Y CUP

#endif
