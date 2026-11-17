#include "error.h"

#include <stdio.h>
#include <stdlib.h>

// Vypise chybu na stderr a ukonci program
void error_exit(int code, int line, const char *message) {
  if (line > 0)
    fprintf(stderr, "Error %d (line %d): %s\n", code, line, message);
  else
    fprintf(stderr, "Error %d: %s\n", code, message);
  exit(code);
}