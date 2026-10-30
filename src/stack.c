// xnovacm00
// stack.c - implementace obecneho zasobniku

#include "stack.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_INITIAL_CAP 8

// Interni chyba prekladace - kod 99
static void stack_fatal(void) {
  fprintf(stderr, "Internal error: out of memory (stack)\n");
  exit(99);
}

// Zvetsi kapacitu zasobniku
static void stack_grow(Stack *s) {
  size_t new_cap = s->cap ? s->cap * 2 : STACK_INITIAL_CAP;
  if (new_cap < s->cap || new_cap > SIZE_MAX / s->elem_size)
    stack_fatal(); // Preteceni pri vypoctu velikosti
  void *p = realloc(s->data, new_cap * s->elem_size);
  if (!p)
    stack_fatal();
  s->data = p;
  s->cap = new_cap;
}

// Inicializuje prazdny zasobnik
void stack_init(Stack *s, size_t elem_size) {
  s->data = NULL;
  s->elem_size = elem_size ? elem_size : 1;
  s->count = 0;
  s->cap = 0;
}

void stack_push(Stack *s, const void *elem) {
  if (s->count == s->cap)
    stack_grow(s);
  // elem = ukazatel na data, ktera vkladame
  // s->elem_size (velikost jednoho prvku v bajtech)
  // kam ukladame = (char *)s->data + s->count * s->elem_size
  memcpy((char *)s->data + s->count * s->elem_size, elem, s->elem_size);
  s->count++;
}

// Odebere prvek z vrcholu a zkopiruje ho do out
bool stack_pop(Stack *s, void *out) {
  if (s->count == 0)
    return false;
  s->count--;
  if (out)
    memcpy(out, (char *)s->data + s->count * s->elem_size, s->elem_size);
  return true;
}

//  Vrati ukazatel na vrchol, nebo NULL pro prazdny zasobnik
void *stack_top(const Stack *s) { return stack_peek(s, 0); }

// Vrati ukazatel na prvek v hloubce depth (0 = vrchol), nebo NULL
void *stack_peek(const Stack *s, size_t depth) {
  if (depth >= s->count)
    return NULL;
  return (char *)s->data + (s->count - 1 - depth) * s->elem_size;
}

// Vrati pocet prvku
size_t stack_size(const Stack *s) { return s->count; }

// Vrati true pokud je zasobnik prazdny
bool stack_is_empty(const Stack *s) { return s->count == 0; }

// Vyprazdni zasobnik bez uvolneni pameti
void stack_clear(Stack *s) { s->count = 0; }

// Uvolni pamet zasobniku
void stack_free(Stack *s) {
  free(s->data);
  s->data = NULL;
  s->count = 0;
  s->cap = 0;
}