/*
    IFJ projekt 2026

    xnovacm00 Michal Novacek

    dstring.h

    Tento soubor obsahuje datovy typ dynamickeho retezce a deklarace funkci
    pro praci s nim.
*/

#ifndef DSTRING_H
#define DSTRING_H

#include <stddef.h>

/**
 * @brief Dynamicky retezec.
 */
typedef struct {
  char *data; ///< Retezec ukonceny znakem '\0'.
  size_t len; ///< Pocet znaku bez ukoncovaciho '\0'.
  size_t cap; ///< Alokovana kapacita v bajtech.
} DString;

/**
 * @brief Pripravi prazdny retezec.
 *
 * @param s Retezec k inicializaci.
 */
void dstr_init(DString *s);

/**
 * @brief Prida znak na konec retezce.
 *
 * @param s Retezec, do ktereho se pridava.
 * @param c Pridavany znak.
 */
void dstr_append_char(DString *s, char c);

/**
 * @brief Vyprazdni retezec, pamet ponecha.
 *
 * @param s Retezec k vyprazdneni.
 */
void dstr_clear(DString *s);

/**
 * @brief Vytvori novou kopii retezce.
 *
 * @param s Retezec ke zkopirovani.
 * @return Nova kopie, kterou musi volajici uvolnit pres free.
 */
char *dstr_copy(const DString *s);

/**
 * @brief Uvolni pamet retezce.
 *
 * @param s Retezec k uvolneni.
 */
void dstr_free(DString *s);

#endif