#include <stdio.h>
int main() { const char *s = "#include <stdio.h>%cint main() { const char *s = %c%s%c; printf(s, 10, 34, s, 34, 10); return 0; }%c"; printf(s, 10, 34, s, 34, 10); return 0; }
