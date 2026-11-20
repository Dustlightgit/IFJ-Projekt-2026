/*
    IFJ projekt 2026
 
    xsantar00 Radovan Santavy
    xnovacm00 Michal Novacek
 
    memory.c
 
    Tento soubor obsahuje implementaci pomocnych funkci pro praci s pameti ze souboru memory.h
*/
#include "memory.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

// edit: kvuli zmenam v kodu ast.c symtable.c memory.h a memory.c tento prompt uz nemusi byt platny pro nasledujici kod
/* ----- Zacatek casti generovane AI (Claude)  -----
 * Prompt: "Do ast.c potrebuju pomocne funkce pro praci s pameti, aby se
 * kontrola vysledku alokace nemusela opakovat v kazdem konstruktoru.
 * Napis xmalloc a xrealloc, ktere pri neuspechu ukonci program pres
 * error_exit s kodem ERR_INTERNAL. Dale xstrdup pro kopii retezce
 * a xstrdup_opt, ktera ponecha NULL jako NULL. Nakonec grow_array, ktera
 * zvetsi dynamicke pole o jedno misto a pro pole s ukazatelem NULL ho
 * vytvori."
 */

void *xmalloc(size_t size) {
  void *p = malloc(size);
  if (!p)
    error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
  return p;
}

void *xcalloc(size_t n, size_t size) {
  void *p = calloc(n, size);
  if (!p)
    error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
  return p;
}

void *xrealloc(void *ptr, size_t size) {
  void *p = realloc(ptr, size);
  if (!p)
    error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
  return p;
}

char *xstrdup(const char *s) {
  size_t len = strlen(s) + 1;
  char *copy = xmalloc(len);
  memcpy(copy, s, len);
  return copy;
}

char *xstrdup_opt(const char *str) { return str == NULL ? NULL : xstrdup(str); }

void *grow_array(void *array, int count, size_t elem_size) {
  return xrealloc(array, (size_t)(count + 1) * elem_size);
}