#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>


int fprintfr(char *, char, void *);
int fscanfr(char*, char, void*);
int clean();
