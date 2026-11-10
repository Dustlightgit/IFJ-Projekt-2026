/*
    IFJ projekt 2026

    nonterminal.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - pridan nazev pro NONTERM_ELSE_TERNARY_IF.

    Tento soubor obsahuje pomocne funkce pro praci s neterminaly.

*/

#include "nonterminal.h"

/*
    Prompt pro Claude:
        Teď prosím po vzoru následujícího úryvku k tokenům vygeneruj funkci,
        která každému neterminálu přidá lidsky čitelný název. U tokenů byl
        využit jejich zápis, jak by jej scanner přijmul; zde neterminál nemá
        podobný čitelný protějšek, proto jim přiřaď spíše lidsky čitelný název v
        češtině (aby byly chybové výstupy konzistentní se scannerem). Hlavičku k
        ní jsem už napsal:
        const char *nonterm_type_name(NonterminalType type);
        ale můžeš vygenerovat doxygen komentář k ní.

        const char *token_type_name(TokenType type)
        {
            switch (type) {
                case TOKEN_EOF: return "EOF";
                case TOKEN_EOL: return "EOL";
                case TOKEN_INDENT: return "INDENT";
                case TOKEN_DEDENT: return "DEDENT";
                ...
                case TOKEN_DOT: return ".";
            }
            return "?";
        }
*/

const char *nonterm_type_name(NonterminalType type) {
  switch (type) {
  // Struktura programu
  case NONTERM_PROGRAM:
    return "program";
  case NONTERM_DEF_LIST:
    return "seznam deklaraci";
  case NONTERM_DEF:
    return "deklarace funkce";
  case NONTERM_MAIN_ANNOTATION:
    return "anotace @main";
  case NONTERM_OPTIONAL_EOL:
    return "nepovinny konec radku";

  // Parametry a typy
  case NONTERM_PARAM_LIST:
    return "seznam parametru";
  case NONTERM_PARAM_LIST_TAIL:
    return "dalsi parametry";
  case NONTERM_OPTIONAL_TYPE:
    return "nepovinny typ";
  case NONTERM_TYPE:
    return "datovy typ";

  // Bloky a prikazy
  case NONTERM_BLOCK_OR_EXPR:
    return "blok nebo vyraz";
  case NONTERM_BLOCK:
    return "blok";
  case NONTERM_COMMAND_LIST:
    return "seznam prikazu";
  case NONTERM_COMMAND_END:
    return "konec prikazu";
  case NONTERM_COMMAND:
    return "prikaz";
  case NONTERM_VAL_VAR:
    return "val nebo var";
  case NONTERM_ID_COMMAND:
    return "prikaz zacinajici identifikatorem";
  case NONTERM_RETURN_VALUE:
    return "navratova hodnota";
  case NONTERM_BLOCK_OR_COMMAND:
    return "blok nebo prikaz";
  case NONTERM_OPTIONAL_SEMICOLON:
    return "nepovinny strednik";

  // Cyklus for
  case NONTERM_FOR_HEADER:
    return "hlavicka cyklu for";
  case NONTERM_GUARD_LIST:
    return "seznam guardu";

  // Podminky
  case NONTERM_OPTIONAL_ELSE:
    return "nepovinna vetev else";
  case NONTERM_TERNARY_IF:
    return "ternarni if";
  case NONTERM_EXPR_OR_INDENT_EXPR:
    return "vyraz nebo odsazeny vyraz";
  case NONTERM_ELSE_TERNARY_IF:
    return "else vetev ternarniho ifu";

  // Pseudoneterminaly
  case NONTERM_EXPR:
    return "vyraz";
  case NONTERM_EXPR_NO_ID:
    return "vyraz nezacinajici identifikatorem";

  // Neni skutecny neterminal
  case NONTERM_COUNT:
    break;
  }
  return "?";
}
