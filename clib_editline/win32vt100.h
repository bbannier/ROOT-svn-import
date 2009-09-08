#define ERR 0
typedef int (*PutcFunc_t)(int);

char* tigetstr(const char*);
int tputs(const char* what, int, PutcFunc_t putc);
char* tparm(const char* termstr, ...);
int setupterm(const char* /*term*/, int fd, int* errcode);
