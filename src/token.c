// xnovacm00
// token.c - implementace funkci pro pro spravu a reprezentaci tokenu lexikalniho analyzatoru


#include "token.h"
 
#include <stdlib.h>
 
// Vrati citelny nazev typu tokenu
const char *token_type_name(TokenType type)
{
    switch (type) {
    case TOKEN_EOF: return "EOF";
    case TOKEN_EOL: return "EOL";
    case TOKEN_INDENT: return "INDENT";
    case TOKEN_DEDENT: return "DEDENT";
    case TOKEN_ID: return "ID";
    case TOKEN_INT_LIT: return "INT_LIT";
    case TOKEN_DOUBLE_LIT: return "DOUBLE_LIT";
    case TOKEN_STRING_LIT: return "STRING_LIT";
    case TOKEN_KW_DEF: return "def";
    case TOKEN_KW_DO: return "do";
    case TOKEN_KW_ELSE: return "else";
    case TOKEN_KW_IF: return "if";
    case TOKEN_KW_IMPORT: return "import";
    case TOKEN_KW_RETURN: return "return";
    case TOKEN_KW_THEN: return "then";
    case TOKEN_KW_VAL: return "val";
    case TOKEN_KW_VAR: return "var";
    case TOKEN_KW_WHILE: return "while";
    case TOKEN_KW_FOR: return "for";
    case TOKEN_KW_TRUE: return "true";
    case TOKEN_KW_FALSE: return "false";
    case TOKEN_KW_INT: return "Int";
    case TOKEN_KW_DOUBLE: return "Double";
    case TOKEN_KW_STRING: return "String";
    case TOKEN_KW_UNIT: return "Unit";
    case TOKEN_KW_BOOLEAN: return "Boolean";
    case TOKEN_KW_LIST: return "List";
    case TOKEN_ANNOT_MAIN: return "@main";
    case TOKEN_PLUS: return "+";
    case TOKEN_MINUS: return "-";
    case TOKEN_STAR: return "*";
    case TOKEN_SLASH: return "/";
    case TOKEN_PLUSPLUS: return "++";
    case TOKEN_LT: return "<";
    case TOKEN_LE: return "<=";
    case TOKEN_GT: return ">";
    case TOKEN_GE: return ">=";
    case TOKEN_EQ: return "==";
    case TOKEN_NE: return "!=";
    case TOKEN_NOT: return "!";
    case TOKEN_AND: return "&&";
    case TOKEN_OR: return "||";
    case TOKEN_ASSIGN: return "=";
    case TOKEN_LARROW: return "<-";
    case TOKEN_LPAREN: return "(";
    case TOKEN_RPAREN: return ")";
    case TOKEN_LBRACKET: return "[";
    case TOKEN_RBRACKET: return "]";
    case TOKEN_COMMA: return ",";
    case TOKEN_COLON: return ":";
    case TOKEN_SEMICOLON: return ";";
    case TOKEN_DOT: return ".";
    }
    return "?";
}

// Uvolni text vlastneny tokenem (jen u ID a retezcu) a token zneplatni
void token_free(Token *t)
{
    if (t->type == TOKEN_ID || t->type == TOKEN_STRING_LIT)
        free(t->value.text);
    t->value.text = NULL;
}