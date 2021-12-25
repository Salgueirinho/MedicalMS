#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

int	putString(const char *s, int fd);
int	putNumber(int n, int fd);
bool	serviceDeskIsRunning(const int pid); 

#endif
