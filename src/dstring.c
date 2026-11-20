/*
    IFJ projekt 2026

    xnovacm00 Michal Novacek

    dstring.c

    Tento soubor obsahuje implementaci dynamickeho retezce.
*/

#include "dstring.h"

#include <stdlib.h>
#include <string.h>

#include "error.h"

#define INITIAL_CAP 16

// Pripravi prazdny retezec
void dstr_init(DString *s) {
  s->data = malloc(INITIAL_CAP);
  if (!s->data)
    error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
  s->data[0] = '\0';
  s->len = 0;
  s->cap = INITIAL_CAP;
}

// Prida znak na konec retezce
void dstr_append_char(DString *s, char c) {
  // Potrebuji misto pro novy znak a pro ukoncovaci '\0'
  if (s->len + 2 > s->cap) {
    size_t new_cap = s->cap * 2;
    char *p = realloc(s->data, new_cap);
    if (!p)
      error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
    s->data = p;
    s->cap = new_cap;
  }
  s->data[s->len] = c;
  s->len++;
  s->data[s->len] = '\0';
}

// Vyprazdni retezec, pamet ponecha
void dstr_clear(DString *s) {
  s->len = 0;
  s->data[0] = '\0';
}

// Vrati novou kopii retezce
char *dstr_copy(const DString *s) {
  char *copy = malloc(s->len + 1);
  if (!copy)
    error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
  memcpy(copy, s->data, s->len + 1);
  return copy;
}

// Uvolni pamet retezce
void dstr_free(DString *s) {
  free(s->data);
  s->data = NULL;
  s->len = 0;
  s->cap = 0;
}