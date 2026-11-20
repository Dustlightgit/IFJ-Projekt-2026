/*
    IFJ projekt 2026

    xnovacm00 Michal Novacek
    xbalaka00 Artur Balak

    token.c

    Puvodni autor: xnovacm00
    Seznam zmen:
        - xbalaka00 - pridana explicitni vetev default ve funkci
          token_type_name kvuli novym tokenum TOKEN_LL_TERM_COUNT
          a TOKEN_EXPRESSION_BEGINNING_DELIMITER.
        - xbalaka00 - pridane nazvy tokenu TOKEN_ID_OPERATOR a TOKEN_DOLLAR,
          pridana funkce token_type_validate (prompt je v nonterminal.h).
        - xbalaka00 - token_free uvolnuje text i u TOKEN_ID_OPERATOR (prompt
          je v precedence_helpers.h).
        - xbalaka00 - pridane nazvy tokenu TOKEN_PLUS_UNARY
          a TOKEN_MINUS_UNARY (prompt je v token.h).
        - xbalaka00 - pridan nazev tokenu TOKEN_TIF (prompt je v souboru token.h).

    Implementace funkci pro spravu a reprezentaci tokenu lexikalniho
    analyzatoru.

*/

#include "token.h"

#include "error.h"

#include <stdlib.h>

// Vrati citelny nazev typu tokenu
const char *token_type_name(TokenType type) {
  switch (type) {
  case TOKEN_EOF:
    return "EOF";
  case TOKEN_EOL:
    return "EOL";
  case TOKEN_INDENT:
    return "INDENT";
  case TOKEN_DEDENT:
    return "DEDENT";
  case TOKEN_ID:
    return "ID";
  case TOKEN_INT_LIT:
    return "INT_LIT";
  case TOKEN_DOUBLE_LIT:
    return "DOUBLE_LIT";
  case TOKEN_STRING_LIT:
    return "STRING_LIT";
  case TOKEN_KW_DEF:
    return "def";
  case TOKEN_KW_DO:
    return "do";
  case TOKEN_KW_ELSE:
    return "else";
  case TOKEN_KW_IF:
    return "if";
  case TOKEN_KW_IMPORT:
    return "import";
  case TOKEN_KW_RETURN:
    return "return";
  case TOKEN_KW_THEN:
    return "then";
  case TOKEN_KW_VAL:
    return "val";
  case TOKEN_KW_VAR:
    return "var";
  case TOKEN_KW_WHILE:
    return "while";
  case TOKEN_KW_FOR:
    return "for";
  case TOKEN_KW_TRUE:
    return "true";
  case TOKEN_KW_FALSE:
    return "false";
  case TOKEN_KW_INT:
    return "Int";
  case TOKEN_KW_DOUBLE:
    return "Double";
  case TOKEN_KW_STRING:
    return "String";
  case TOKEN_KW_UNIT:
    return "Unit";
  case TOKEN_KW_BOOLEAN:
    return "Boolean";
  case TOKEN_KW_LIST:
    return "List";
  case TOKEN_ANNOT_MAIN:
    return "@main";
  case TOKEN_PLUS:
    return "+";
  case TOKEN_MINUS:
    return "-";
  case TOKEN_STAR:
    return "*";
  case TOKEN_SLASH:
    return "/";
  case TOKEN_PLUSPLUS:
    return "++";
  case TOKEN_LT:
    return "<";
  case TOKEN_LE:
    return "<=";
  case TOKEN_GT:
    return ">";
  case TOKEN_GE:
    return ">=";
  case TOKEN_EQ:
    return "==";
  case TOKEN_NE:
    return "!=";
  case TOKEN_NOT:
    return "!";
  case TOKEN_AND:
    return "&&";
  case TOKEN_OR:
    return "||";
  case TOKEN_ASSIGN:
    return "=";
  case TOKEN_LARROW:
    return "<-";
  case TOKEN_LPAREN:
    return "(";
  case TOKEN_RPAREN:
    return ")";
  case TOKEN_LBRACKET:
    return "[";
  case TOKEN_RBRACKET:
    return "]";
  case TOKEN_COMMA:
    return ",";
  case TOKEN_COLON:
    return ":";
  case TOKEN_SEMICOLON:
    return ";";
  case TOKEN_DOT:
    return ".";
  case TOKEN_PLUS_UNARY:
    return "+ (unarni)";
  case TOKEN_MINUS_UNARY:
    return "- (unarni)";
  case TOKEN_ID_OPERATOR:
    return "ID_OPERATOR";
  case TOKEN_DOLLAR:
    return "$";
  case TOKEN_TIF:
    return "TIF";
  default:
    return "?";
  }
}

// Uvolni text vlastneny tokenem (jen u ID, ID_OPERATOR a retezcu) a token zneplatni
void token_free(Token *t) {
  if (t->type == TOKEN_ID || t->type == TOKEN_ID_OPERATOR ||
      t->type == TOKEN_STRING_LIT)
    free(t->value.text);
  t->value.text = NULL;
}
// Overi, ze typ tokenu je skutecny token (ne specialni hodnota enumu)
void token_type_validate(TokenType type, int line) {
  // Pretypovani zachyti i zaporne hodnoty mimo enum
  if ((unsigned)type >= (unsigned)TOKEN_COUNT || type == TOKEN_LL_TERM_COUNT ||
      type == TOKEN_EXPRESSION_BEGINNING_DELIMITER ||
      type == TOKEN_EXPR_TERM_DELIMITER)
    error_exit(ERR_INTERNAL, line, "Neplatny typ tokenu.");
}
