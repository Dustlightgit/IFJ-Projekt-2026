// xnovacm00
// stack.h - obecny zasobnik prvku pevne velikosti

#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  void *data;       // Souvisle pole prvku
  size_t elem_size; // velikost jednoho prvku v bajtech (> 0)
  size_t count;     // Pocet ulozenych prvku
  size_t cap;       // Kapacita v prvcich
} Stack;

// Inicializuje prazdny zasobnik prvku o velikosti elem_size (musi byt > 0)
void stack_init(Stack *s, size_t elem_size);

// Vlozi kopii prvku *elem na vrchol. Pri nedostatku pameti ukonci program s
// kodem 99
void stack_push(Stack *s, const void *elem);

// Odebere prvek z vrcholu; je-li out != NULL
// Vrací false, pokud je zásobník prázdný.
bool stack_pop(Stack *s, void *out);

// Ukazatel na prvek na vrcholu, nebo NULL, je-li zasobnik prazdny
void *stack_top(const Stack *s);

// Ukazatel na prvek v hloubce depth (0 = vrchol, 1 = tesne pod nim) nebo NULL,
// pokud takovy prvek neni
void *stack_peek(const Stack *s, size_t depth);

// Pocet prvku na zásobníku
size_t stack_size(const Stack *s);

// True, je-li zasobnik prazdny
bool stack_is_empty(const Stack *s);

// Vyprazdni zasobnik, alokovanou pamet ponecha pro dalsi pouziti
void stack_clear(Stack *s);

// Uvolni pamet zasobniku;
void stack_free(Stack *s);

#endif