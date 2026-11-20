/*
    IFJ projekt 2026
 
    xsantar00 Radovan Santavy

    memory.h
 
    Tento soubor obsahuje deklarace pomocnych funkci pro praci s pameti v souboru memory.c
*/


#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

/**
 * @brief Alokuje blok pameti o zadane velikosti.
 *
 * Pri selhani alokace ukonci program volanim error_exit s kodem ERR_INTERNAL.
 *
 * @param size Velikost alokovane pameti v bajtech.
 * @return Ukazatel na alokovanou pamet.
 */
void *xmalloc(size_t size);

/**
 * @brief Alokuje pole prvku a inicializuje vsechny bajty na nulu.
 *
 * Pri selhani alokace ukonci program volanim error_exit s kodem ERR_INTERNAL.
 *
 * @param n Pocet prvku k alokaci.
 * @param size Velikost jednoho prvku v bajtech.
 * @return Ukazatel na vynulovanou alokovanou pamet.
 */
void *xcalloc(size_t n, size_t size);

/**
 * @brief Zmeni velikost drive alokovaneho bloku pameti.
 *
 * Pri selhani realokace ukonci program volanim error_exit s kodem ERR_INTERNAL.
 *
 * @param ptr Ukazatel na puvodni blok pameti (nebo NULL pro novou alokaci).
 * @param size Nova pozadovana velikost v bajtech.
 * @return Ukazatel na realokovany blok pameti.
 */
void *xrealloc(void *ptr, size_t size);

/**
 * @brief Vytvori dynamicky alokovanou kopii retezce zakoncenou nulovym znakem.
 *
 * Pri selhani alokace ukonci program volanim error_exit s kodem ERR_INTERNAL.
 *
 * @param s Zdrojovy retezec (nesmi byt NULL).
 * @return Ukazatel na nove alokovanou kopii retezce.
 */
char *xstrdup(const char *s);

/**
 * @brief Vytvori dynamicky alokovanou kopii volitelneho retezce.
 *
 * Pokud je vstupni ukazatel NULL, funkce nealokuje pamet a vrati NULL.
 *
 * @param str Zdrojovy retezec nebo NULL.
 * @return Ukazatel na kopii retezce, nebo NULL pokud byl vstup NULL.
 */
char *xstrdup_opt(const char *str);

/**
 * @brief Zvetsi dynamicke pole o jeden prvek.
 *
 * Pokud je ukazatel na pole NULL (prazdne pole), alokuje pole pro jeden prvek.
 * Pri selhani alokace ukonci program volanim error_exit s kodem ERR_INTERNAL.
 *
 * @param array Ukazatel na stavajici pole nebo NULL.
 * @param count Aktualni pocet prvku v poli pred zvetsenim.
 * @param elem_size Velikost jednoho prvku pole v bajtech.
 * @return Ukazatel na zvetsene pole.
 */
void *grow_array(void *array, int count, size_t elem_size);

#endif /* MEMORY_H */