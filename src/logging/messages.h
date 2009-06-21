#ifndef messages_h
#define messages_h

#include <cstdio>
#include <ctime>
#include <string>

#include "src/io/colors.h"

using namespace std;

void log_message (const char *message);
void log_error (const char *message);
void log_error (string message);




#endif

