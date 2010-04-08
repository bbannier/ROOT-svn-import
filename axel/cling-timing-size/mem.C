#include <stdlib.h>
void mem() {
   system("ps -orss,vsz,args -C cling|grep -v RSS > mem.txt;ps -orss,vsz,args -C cint|grep -v RSS >> mem.txt");
}
