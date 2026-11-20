/*
    IFJ projekt 2026

    xnovacm00 Michal Novacek

    error.c

    Tento soubor obsahuje implementaci funkce pro vypis chyb a ukonceni programu.
*/

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