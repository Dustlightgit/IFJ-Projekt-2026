// xnovacm00
// dstring.h - dynamicky retezec rozhrani

#ifndef DSTRING_H
#define DSTRING_H

#include <stddef.h>

typedef struct {
    char *data;  // Retezec ukonceny znakem '\0'
    size_t len;  // Pocet znaku bez ukoncovaciho '\0'
    size_t cap;  // Alokovana kapacita v bajtech
} DString;


// Pripravi prazdny retezec
void dstr_init(DString *s);

// Prida znak na konec retezce
void dstr_append_char(DString *s, char c);

// Vyprazdni retezec, pamet ponecha
void dstr_clear(DString *s);

// Vrati novou kopii retezce (volajici ji musi uvolnit pres free)
char *dstr_copy(const DString *s);

// Uvolni pamet retezce
void dstr_free(DString *s);

#endif