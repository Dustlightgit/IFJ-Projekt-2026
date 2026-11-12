/*
    IFJ projekt 2026

    stack.h

    Puvodni autor: xnovacm00
    Seznam zmen:
        - xbalaka00 - doplneni doxygen dokumentace a sjednoceni komentaru
          za asistence Claude.

    Obecny zasobnik prvku pevne velikosti.

*/

#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Obecny zasobnik prvku pevne velikosti.
 */
typedef struct {
  void *data;       ///< Souvisle pole prvku
  size_t elem_size; ///< Velikost jednoho prvku v bajtech (> 0)
  size_t count;     ///< Pocet ulozenych prvku
  size_t cap;       ///< Kapacita v prvcich
} Stack;

/**
 * @brief Inicializuje prazdny zasobnik.
 *
 * @param s Zasobnik k inicializaci.
 * @param elem_size Velikost jednoho prvku v bajtech, musi byt > 0.
 */
void stack_init(Stack *s, size_t elem_size);

/**
 * @brief Vlozi kopii prvku na vrchol zasobniku.
 *
 * Pri nedostatku pameti ukonci program s kodem 99.
 *
 * @param s Zasobnik.
 * @param elem Ukazatel na prvek, ktery se zkopiruje.
 */
void stack_push(Stack *s, const void *elem);

/**
 * @brief Odebere prvek z vrcholu zasobniku.
 *
 * @param s Zasobnik.
 * @param out Kam se odebrany prvek zkopiruje, pokud neni NULL.
 * @return False, pokud je zasobnik prazdny, jinak true.
 */
bool stack_pop(Stack *s, void *out);

/**
 * @brief Vrati ukazatel na prvek na vrcholu zasobniku.
 *
 * @param s Zasobnik.
 * @return Ukazatel na prvek na vrcholu, nebo NULL, je-li zasobnik prazdny.
 */
void *stack_top(const Stack *s);

/**
 * @brief Vrati ukazatel na prvek v dane hloubce.
 *
 * @param s Zasobnik.
 * @param depth Hloubka, 0 je vrchol, 1 je prvek tesne pod nim.
 * @return Ukazatel na prvek, nebo NULL, pokud takovy prvek neni.
 */
void *stack_peek(const Stack *s, size_t depth);

/**
 * @brief Vrati pocet prvku na zasobniku.
 */
size_t stack_size(const Stack *s);

/**
 * @brief Vrati true, je-li zasobnik prazdny.
 */
bool stack_is_empty(const Stack *s);

/**
 * @brief Vyprazdni zasobnik, alokovanou pamet ponecha pro dalsi pouziti.
 */
void stack_clear(Stack *s);

/**
 * @brief Uvolni pamet zasobniku.
 */
void stack_free(Stack *s);

#endif // STACK_H
