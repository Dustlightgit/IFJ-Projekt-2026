// xnovacm00
// scanner.c - lexikalni analyzator implementace


#include "scanner.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "error.h"
#include "dstring.h"


// Vracene znaky (pouziva se jako zasobnik)
#define PUSHBACK_MAX 16
static int pushback[PUSHBACK_MAX];
static int pushback_count = 0;

// Cislo aktualniho radku vstupu
static int line = 1;

// Stavy konecneho automatu
typedef enum {
    STATE_START, // Pocatecni stav
    STATE_ASSIGN, // Precteno '=', cekame na dalsi znak
    STATE_NOT,    // Precteno '!', cekame na dalsi znak
    STATE_GREATER, // Precteno '>', cekame na dalsi znak
    STATE_PLUS,    // Precteno '+', cekame na dalsi znak
    STATE_AMPERSAND, // Precteno '&', musi nasledovat dalsi '&'
    STATE_PIPE,      // Precteno '|', musi nasledovat dalsi '|'
    STATE_LESS,      // Precteno '<', cekame na dalsi znak
    STATE_SLASH,        // Precteno '/', cekame na dalsi znak
    STATE_LINE_COMMENT, // Jsme uvnitr radkoveho komentare
    STATE_BLOCK_COMMENT, // Jsme uvnitr blokoveho komentare
    STATE_BLOCK_SLASH,   // V komentari precteno '/', mozny vnoreny komentar
    STATE_BLOCK_STAR,    // V komentari precteno '*', mozny konec komentare
    STATE_ID, // Cteme identifikator nebo klicove slovo
    STATE_INT,            // Cteme cele cislo
    STATE_INT_UNDERSCORE, // V cisle precteno '_', musi nasledovat cislice
    STATE_DOT,             // Tecka na zacatku tokenu
    STATE_INT_DOT,         // Za celym cislem precteno '.'
    STATE_FRAC,            // Cteme desetinnou cast
    STATE_FRAC_UNDERSCORE, // V desetinne casti precteno '_', musi nasledovat cislice
    STATE_EXP_START,      // Precteno 'e' nebo 'E', cekame znamenko nebo cislici
    STATE_EXP_SIGN,       // Precteno znamenko exponentu, musi nasledovat cislice
    STATE_EXP,            // Cteme cislice exponentu
    STATE_EXP_UNDERSCORE, // V exponentu precteno '_', musi nasledovat cislice
    STATE_STRING,        // Jsme uvnitr retezce
    STATE_STRING_ESCAPE, // V retezci precteno '\', cekame na znak escape sekvence
    STATE_QUOTE,  // Precteno prvni '"'
    STATE_QUOTE2, // Precteny dve '""', dalsi '"' znamena viceradkovy retezec
    STATE_MULTI,  // Jsme uvnitr viceradkoveho retezce
    STATE_AT, // Precteno '@', cteme nazev anotace
} State;


// Pomocne testy znaku (jen ASCII)
static bool is_alpha(int character)
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z');
}

static bool is_digit(int character)
{
    return character >= '0' && character <= '9';
}

// Precte dalsi znak (nejdriv z vracenych, pak ze vstupu) a pocita radky
static int next_char(void)
{
    int character;
    if (pushback_count > 0)
        character = pushback[--pushback_count];
    else
        character = getchar();

    if (character == '\n')
        line++;
    return character;
}

// Vrati znak zpet na vstup
static void put_back(int character)
{
    if (pushback_count >= PUSHBACK_MAX)
        error_exit(ERR_INTERNAL, line, "Preteceni bufferu vracenych znaku");

    if (character == '\n')
        line--;
    pushback[pushback_count++] = character;
}


// Zjisti, jestli je znak samostatny token z jednoho znaku
// Pokud ano, vrati true a nastavi typ tokenu
static bool single_char_type(int character, TokenType *type)
{
    switch (character) {
    case '(': *type = TOKEN_LPAREN; return true;
    case ')': *type = TOKEN_RPAREN; return true;
    case '[': *type = TOKEN_LBRACKET; return true;
    case ']': *type = TOKEN_RBRACKET; return true;
    case ',': *type = TOKEN_COMMA; return true;
    case ':': *type = TOKEN_COLON; return true;
    case ';': *type = TOKEN_SEMICOLON; return true;
    case '*': *type = TOKEN_STAR; return true;
    case '-': *type = TOKEN_MINUS; return true;
    default: return false;
    }
}

// Tabulka klicovych slov
// Slova "to", "until", "by" a "main" tu nejsou, jsou to identifikatory
typedef struct {
    const char *name;
    TokenType type;
} Keyword;

static const Keyword keywords[] = {
    { "def", TOKEN_KW_DEF },
    { "do", TOKEN_KW_DO },
    { "else", TOKEN_KW_ELSE },
    { "if", TOKEN_KW_IF },
    { "import", TOKEN_KW_IMPORT },
    { "return", TOKEN_KW_RETURN },
    { "then", TOKEN_KW_THEN },
    { "val", TOKEN_KW_VAL },
    { "var", TOKEN_KW_VAR },
    { "while", TOKEN_KW_WHILE },
    { "for", TOKEN_KW_FOR },
    { "true", TOKEN_KW_TRUE },
    { "false", TOKEN_KW_FALSE },
    { "Int", TOKEN_KW_INT },
    { "Double", TOKEN_KW_DOUBLE },
    { "String", TOKEN_KW_STRING },
    { "Unit", TOKEN_KW_UNIT },
    { "Boolean", TOKEN_KW_BOOLEAN },
    { "List", TOKEN_KW_LIST },
};

// Vrati typ klicoveho slova, nebo TOKEN_ID pokud slovo klicove neni
static TokenType lookup_keyword(const char *word)
{
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(keywords[i].name, word) == 0)
            return keywords[i].type;
    }
    return TOKEN_ID;
}

// Dokonci celociselny literal: zkontroluje nulu na zacatku a prevede text na cislo
static void finish_int(Token *token, DString *buffer)
{
    // Nenulove cislo nesmi zacinat nulou
    if (buffer->len > 1 && buffer->data[0] == '0')
        error_exit(ERR_LEXICAL, token->line, "Nenulove cislo nesmi zacinat nulou");

    token->type = TOKEN_INT_LIT;
    token->value.int_val = strtoll(buffer->data, NULL, 10);
    dstr_free(buffer);
}

// Dokonci desetinny literal: prevede text na cislo
static void finish_double(Token *token, DString *buffer)
{
    token->type = TOKEN_DOUBLE_LIT;
    token->value.double_val = strtod(buffer->data, NULL);
    dstr_free(buffer);
}


// Precte dalsi token
void get_token(Token *token) {
    memset(token, 0, sizeof(*token));
    State state = STATE_START;
    int comment_depth = 0; // Hloubka vnoreni blokoveho komentare
    DString buffer; // Inicializuje se az pri vstupu do stavu ID
    int quote_count = 0; // Pocet uvozovek za sebou ve viceradkovem retezci
    

    for (;;) {
        int character = next_char();

        switch (state) {
        case STATE_START:

            // Bile znaky preskakujeme, zustavame ve stavu START
            if (character == ' ' || character == '\t' || character == '\r' || character == '\n')
                continue;

            token->line = line;

            // Konec vstupu
            if (character == EOF) {
                token->type = TOKEN_EOF;
                return;
            }
            
            // Token z jednoho znaku
            TokenType type;
            if (single_char_type(character, &type)) {
                token->type = type;
                return;
            }

            // Zacatek '=' nebo '=='
            if (character == '=') {
                state = STATE_ASSIGN;
                break;
            }

            // Zacatek '!' nebo '!='
            if (character == '!') {
                state = STATE_NOT;
                break;
            }

            // Zacatek '>' nebo '>='
            if (character == '>') {
                state = STATE_GREATER;
                break;
            }

            // Zacatek '+' nebo '++'
            if (character == '+') {
                state = STATE_PLUS;
                break;
            }

            // Zacatek '&&'
            if (character == '&') {
                state = STATE_AMPERSAND;
                break;
            }

            // Zacatek '||'
            if (character == '|') {
                state = STATE_PIPE;
                break;
            }

            // Zacatek '<', '<=' nebo '<-'
            if (character == '<') {
                state = STATE_LESS;
                break;
            }

            // Zacatek '/' nebo '//'
            if (character == '/') {
                state = STATE_SLASH;
                break;
            }
            
            // Zacatek identifikatoru
            if (is_alpha(character) || character == '_') {
                dstr_init(&buffer);
                dstr_append_char(&buffer, (char)character);
                state = STATE_ID;
                break;
            }

            // Zacatek cisla
            if (is_digit(character)) {
                dstr_init(&buffer);
                dstr_append_char(&buffer, (char)character);
                state = STATE_INT;
                break;
            }

            // Zacatek '.' (tecka nebo desetinne cislo bez cele casti)
            if (character == '.') {
                state = STATE_DOT;
                break;
            }

            // Zacatek retezce
            if (character == '"') {
                state = STATE_QUOTE;
                break;
            }

            // Zacatek anotace
            if (character == '@') {
                dstr_init(&buffer);
                state = STATE_AT;
                break;
            }

            // nic jineho zatim neumim
            error_exit(ERR_LEXICAL, token->line, "Zatim neumim tento znak");
            break;
        
        case STATE_ASSIGN:
            if (character == '=') {
                token->type = TOKEN_EQ;
                return;
            }

            // Dalsi znak k tokenu nepatri
            put_back(character);
            token->type = TOKEN_ASSIGN;
            return;
        
        case STATE_NOT:
            if (character == '=') {
                token->type = TOKEN_NE;
                return;
            }

            // Dalsi znak k tokenu nepatri
            put_back(character);
            token->type = TOKEN_NOT;
            return;

        case STATE_GREATER:
            if (character == '=') {
                token->type = TOKEN_GE;
                return;
            }

            // Dalsi znak k tokenu nepatri
            put_back(character);
            token->type = TOKEN_GT;
            return;
        
        case STATE_PLUS:
            if (character == '+') {
                token->type = TOKEN_PLUSPLUS;
                return;
            }

            // Dalsi znak k tokenu nepatri
            put_back(character);
            token->type = TOKEN_PLUS;
            return;

        case STATE_AMPERSAND:
            if (character == '&') {
                token->type = TOKEN_AND;
                return;
            }

            // Samotne '&' neni platny token
            error_exit(ERR_LEXICAL, token->line, "Za '&' musi nasledovat dalsi '&'");
            break;

        case STATE_PIPE:
            if (character == '|') {
                token->type = TOKEN_OR;
                return;
            }

            // Samotne '|' neni platny token
            error_exit(ERR_LEXICAL, token->line, "Za '|' musi nasledovat dalsi '|'");
            break;
        
        case STATE_LESS:
            if (character == '=') {
                token->type = TOKEN_LE;
                return;
            }

            if (character == '-') {
                token->type = TOKEN_LARROW;
                return;
            }

            // Dalsi znak k tokenu nepatri
            put_back(character);
            token->type = TOKEN_LT;
            return;
        
        case STATE_SLASH:
            if (character == '/') {
                state = STATE_LINE_COMMENT;
                break;
            }

            if (character == '*') {
                comment_depth = 1;
                state = STATE_BLOCK_COMMENT;
                break;
            }


            // Dalsi znak k tokenu nepatri
            put_back(character);
            token->type = TOKEN_SLASH;
            return;

        case STATE_LINE_COMMENT:
            // Komentar konci koncem radku, ten ale nechame na vstupu
            if (character == '\n' || character == EOF) {
                put_back(character);
                state = STATE_START;
            }
            break;    
        
        case STATE_BLOCK_COMMENT:
            if (character == EOF)
                error_exit(ERR_LEXICAL, token->line, "Neukonceny blokovy komentar");

            if (character == '/')
                state = STATE_BLOCK_SLASH;
            else if (character == '*')
                state = STATE_BLOCK_STAR;
            break;

        case STATE_BLOCK_SLASH:
            if (character == '*')
                comment_depth++;
            else
                put_back(character);
            state = STATE_BLOCK_COMMENT;
            break;
        
        case STATE_BLOCK_STAR:
            if (character == '/') {
                comment_depth--;
                state = (comment_depth == 0) ? STATE_START : STATE_BLOCK_COMMENT;
            } else {
                put_back(character);
                state = STATE_BLOCK_COMMENT;
            }
            break;
        
        case STATE_ID:
            // Pokracovani identifikatoru
            if (is_alpha(character) || is_digit(character) || character == '_') {
                dstr_append_char(&buffer, (char)character);
                break;
            }

            // Konec identifikatoru, znak do nej nepatri
            put_back(character);

            // Samotne podtrzitko neni identifikator
            if (buffer.len == 1 && buffer.data[0] == '_')
                error_exit(ERR_LEXICAL, token->line, "Samotne podtrzitko neni identifikator");

            token->type = lookup_keyword(buffer.data);
            if (token->type == TOKEN_ID)
                token->value.text = dstr_copy(&buffer);
            dstr_free(&buffer);
            return;

        case STATE_INT:
            if (is_digit(character)) {
                dstr_append_char(&buffer, (char)character);
                break;
            }

            if (character == '_') {
                state = STATE_INT_UNDERSCORE;
                break;
            }

            if (character == 'e' || character == 'E') {
                dstr_append_char(&buffer, (char)character);
                state = STATE_EXP_START;
                break;
            }

            if (character == '.') {
                state = STATE_INT_DOT;
                break;
            }

            // Konec cisla, znak do nej nepatri
            put_back(character);
            finish_int(token, &buffer);
            return;

        case STATE_INT_UNDERSCORE:
            if (is_digit(character)) {
                dstr_append_char(&buffer, (char)character);
                state = STATE_INT;
                break;
            }

            error_exit(ERR_LEXICAL, token->line, "Podtrzitko v cisle musi byt mezi dvema cislicemi");
            break;
        
        case STATE_DOT:
            if (is_digit(character)) {
                dstr_init(&buffer);
                dstr_append_char(&buffer, '.');
                dstr_append_char(&buffer, (char)character);
                state = STATE_FRAC;
                break;
            }

            // Samotna tecka
            put_back(character);
            token->type = TOKEN_DOT;
            return;
        
        case STATE_INT_DOT:
            if (is_digit(character)) {
                dstr_append_char(&buffer, '.');
                dstr_append_char(&buffer, (char)character);
                state = STATE_FRAC;
                break;
            }

            // Tecka k cislu nepatri (napr. 5.toString), vratime znak i tecku
            put_back(character);
            put_back('.');
            finish_int(token, &buffer);
            return;
        
        case STATE_FRAC:
            if (is_digit(character)) {
                dstr_append_char(&buffer, (char)character);
                break;
            }

            if (character == '_') {
                state = STATE_FRAC_UNDERSCORE;
                break;
            }

            if (character == 'e' || character == 'E') {
                dstr_append_char(&buffer, (char)character);
                state = STATE_EXP_START;
                break;
            }

            // Pripona 'd' nebo 'D' patri k cislu
            if (character == 'd' || character == 'D') {
                finish_double(token, &buffer);
                return;
            }

            // Konec cisla, znak do nej nepatri
            put_back(character);
            finish_double(token, &buffer);
            return;
    
        case STATE_FRAC_UNDERSCORE:
            if (is_digit(character)) {
                dstr_append_char(&buffer, (char)character);
                state = STATE_FRAC;
                break;
            }

            error_exit(ERR_LEXICAL, token->line, "Podtrzitko v cisle musi byt mezi dvema cislicemi");
            break;
        
        case STATE_EXP_START:
            if (is_digit(character)) {
                dstr_append_char(&buffer, (char)character);
                state = STATE_EXP;
                break;
            }

            if (character == '+' || character == '-') {
                dstr_append_char(&buffer, (char)character);
                state = STATE_EXP_SIGN;
                break;
            }

            error_exit(ERR_LEXICAL, token->line, "Exponent musi obsahovat cislici");
            break;
         
        case STATE_EXP_SIGN:
            if (is_digit(character)) {
                dstr_append_char(&buffer, (char)character);
                state = STATE_EXP;
                break;
            }

            error_exit(ERR_LEXICAL, token->line, "Exponent musi obsahovat cislici");
            break;
    
        case STATE_EXP:
            if (is_digit(character)) {
                dstr_append_char(&buffer, (char)character);
                break;
            }

            if (character == '_') {
                state = STATE_EXP_UNDERSCORE;
                break;
            }

            // Pripona 'd' nebo 'D' patri k cislu
            if (character == 'd' || character == 'D') {
                finish_double(token, &buffer);
                return;
            }

            // Konec cisla, znak do nej nepatri
            put_back(character);
            finish_double(token, &buffer);
            return;

        case STATE_EXP_UNDERSCORE:
            if (is_digit(character)) {
                dstr_append_char(&buffer, (char)character);
                state = STATE_EXP;
                break;
            }

            error_exit(ERR_LEXICAL, token->line, "Podtrzitko v cisle musi byt mezi dvema cislicemi");
            break;   
        
        case STATE_STRING:
            // Konec retezce
            if (character == '"') {
                token->type = TOKEN_STRING_LIT;
                token->value.text = dstr_copy(&buffer);
                dstr_free(&buffer);
                return;
            }

            // Zacatek escape sekvence
            if (character == '\\') {
                state = STATE_STRING_ESCAPE;
                break;
            }

            // Retezec nesmi obsahovat skutecny konec radku a nesmi byt neukonceny
            if (character == '\n' || character == EOF)
                error_exit(ERR_LEXICAL, token->line, "Neukonceny retezec");

            dstr_append_char(&buffer, (char)character);
            break;
    
        case STATE_STRING_ESCAPE:
            switch (character) {
            case 'n': dstr_append_char(&buffer, '\n'); break;
            case 't': dstr_append_char(&buffer, '\t'); break;
            case 'r': dstr_append_char(&buffer, '\r'); break;
            case '"': dstr_append_char(&buffer, '"'); break;
            case '\\': dstr_append_char(&buffer, '\\'); break;
            default:
                error_exit(ERR_LEXICAL, token->line, "Neplatna escape sekvence");
            }
            state = STATE_STRING;
            break;

        case STATE_QUOTE:
            if (character == '"') {
                state = STATE_QUOTE2;
                break;
            }

            // Obycejny retezec, znak patri k jeho obsahu
            put_back(character);
            dstr_init(&buffer);
            state = STATE_STRING;
            break;

        case STATE_QUOTE2:
            if (character == '"') {
                dstr_init(&buffer);
                state = STATE_MULTI;
                break;
            }

            // Prazdny retezec ""
            put_back(character);
            dstr_init(&buffer);
            token->type = TOKEN_STRING_LIT;
            token->value.text = dstr_copy(&buffer);
            dstr_free(&buffer);
            return;

        case STATE_MULTI:
            if (character == '"') {
                quote_count++;
                break;
            }

            // Konec retezce: posledni tri uvozovky jsou zakonceni, ostatni patri k obsahu
            if (quote_count >= 3) {
                for (int i = 0; i < quote_count - 3; i++)
                    dstr_append_char(&buffer, '"');
                put_back(character);
                token->type = TOKEN_STRING_LIT;
                token->value.text = dstr_copy(&buffer);
                dstr_free(&buffer);
                return;
            }

            // Uvozovky, ktere nebyly zakonceni, patri k obsahu
            for (int i = 0; i < quote_count; i++)
                dstr_append_char(&buffer, '"');
            quote_count = 0;

            if (character == EOF)
                error_exit(ERR_LEXICAL, token->line, "Neukonceny viceradkovy retezec");

            dstr_append_char(&buffer, (char)character);
            break;
        
        case STATE_AT:
            // Pokracovani nazvu anotace
            if (is_alpha(character) || is_digit(character) || character == '_') {
                dstr_append_char(&buffer, (char)character);
                break;
            }

            // Konec nazvu, znak do nej nepatri
            put_back(character);

            if (strcmp(buffer.data, "main") != 0)
                error_exit(ERR_LEXICAL, token->line, "Neznama anotace, ocekava se @main");

            token->type = TOKEN_ANNOT_MAIN;
            dstr_free(&buffer);
            return;    
        }
    }

}

// TODO: pokud blokovy komentar obsahoval '\n', ma se chovat jako konec radku
// (vratit TOKEN_EOL, pak zkontrolovat, ze do konce radku nejsou
// dalsi tokeny). Dodelat spolu s EOL a odsazenim.