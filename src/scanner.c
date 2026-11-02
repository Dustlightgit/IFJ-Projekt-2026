// xnovacm00
// scanner.c - lexikalni analyzator implementace


#include "scanner.h"
#include "stack.h"

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

// Byl na aktualnim logickem radku uz nejaky token?
static bool line_has_token = false;

// Hloubka zavorek: uvnitr zavorek konec radku neni EOL
static int paren_depth = 0;

// Byl pred dalsim tokenem uvnitr zavorek konec radku?
static bool pending_nl = false;

// Skoncil blokovy komentar s koncem radku? Pak uz na radku nesmi byt token
static bool after_newline_comment = false;

// Pocitani odsazeni na zacatku radku
static bool at_line_start = true;   // Jsme jeste v uvodnich bilych znacich radku?
static int indent_count = 0;        // Pocet uvodnich mezer aktualniho radku
static bool indent_has_tab = false; // Byl v uvodnich bilych znacich tabulator?

// Zasobnik urovni odsazeni
static Stack indent_stack;
static bool indent_ready = false;

// Podrzeny token (INDENT a DEDENT se vraci pred nim) a pocet cekajicich DEDENTu
static Token held_token;
static bool has_held = false;
static int pending_dedents = 0;

// Kontrola odsazeni pokracovacich radku uvnitr zavorek
static int logical_indent = 0;      // Odsazeni zacatku aktualniho logickeho radku
static bool pending_check = false;  // Dalsi token je prvni na novem fyzickem radku v zavorkach

// Pracovni buffer je globalni, aby ho slo uvolnit i pri chybe
static DString buffer;
static bool buffer_active = false;

static void buf_start(void)
{
    dstr_init(&buffer);
    buffer_active = true;
}

static void buf_release(void)
{
    if (buffer_active) {
        dstr_free(&buffer);
        buffer_active = false;
    }
}

// Uvolni text tokenu (jen u typu, ktere ho vlastni)
static void free_token_text(Token *token)
{
    if ((token->type == TOKEN_ID || token->type == TOKEN_STRING_LIT)
        && token->value.text != NULL) {
        free(token->value.text);
        token->value.text = NULL;
    }
}

// Uvolni vse, co scanner drzi
void scanner_cleanup(void)
{
    buf_release();

    if (has_held) {
        free_token_text(&held_token);
        has_held = false;
    }

    if (indent_ready) {
        stack_free(&indent_stack);
        indent_ready = false;
    }
}


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
    buf_release();
}

// Dokonci desetinny literal: prevede text na cislo
static void finish_double(Token *token, DString *buffer)
{
    token->type = TOKEN_DOUBLE_LIT;
    token->value.double_val = strtod(buffer->data, NULL);
    buf_release();
}


// Precte dalsi token
static void read_token(Token *token) {
    memset(token, 0, sizeof(*token));
    State state = STATE_START;
    int comment_depth = 0; // Hloubka vnoreni blokoveho komentare
    int quote_count = 0; // Pocet uvozovek za sebou ve viceradkovem retezci
    bool comment_has_newline = false; // Obsahuje blokovy komentar konec radku?
    

    for (;;) {
        int character = next_char();

        switch (state) {
        case STATE_START:

            // Konec radku
            if (character == '\n') {
                after_newline_comment = false;
                at_line_start = true;
                indent_count = 0;
                indent_has_tab = false;

                // Uvnitr zavorek je to jen bily znak, ale zapamatujeme si ho
                if (paren_depth > 0) {
                    pending_nl = true;
                    pending_check = true;
                    continue;
                }

                // Mimo zavorky: EOL, ale jen pokud na radku byl nejaky token
                if (line_has_token) {
                    line_has_token = false;
                    token->type = TOKEN_EOL;
                    token->line = line - 1; // Konec radku uz zvysil citac radku
                    return;
                }
                continue;
            }

            // Bile znaky preskakujeme, na zacatku radku je pocitame jako odsazeni
            if (character == ' ') {
                if (at_line_start)
                    indent_count++;
                continue;
            }
            if (character == '\t') {
                if (at_line_start)
                    indent_has_tab = true;
                continue;
            }
            if (character == '\r')
                continue;

            at_line_start = false;

            token->line = line;

            // Konec vstupu: pokud radek mel token, nejdriv EOL
            if (character == EOF) {
                after_newline_comment = false;

                if (line_has_token) {
                    line_has_token = false;
                    put_back(EOF);
                    token->type = TOKEN_EOL;
                    return;
                }
                token->type = TOKEN_EOF;
                return;
            }

            // Za blokovym komentarem s koncem radku uz nesmi byt token
            if (after_newline_comment)
                error_exit(ERR_LEXICAL, token->line, "Za blokovym komentarem s koncem radku nesmi nasledovat token");
            
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
                buf_start();
                dstr_append_char(&buffer, (char)character);
                state = STATE_ID;
                break;
            }

            // Zacatek cisla
            if (is_digit(character)) {
                buf_start();
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
                buf_start();
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
                comment_has_newline = false;
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

            if (character == '\n')
                comment_has_newline = true;

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
                if (comment_depth > 0) {
                    state = STATE_BLOCK_COMMENT;
                    break;
                }

                // Komentar skoncil, pokud obsahoval konec radku, chova se jako konec radku
                if (comment_has_newline) {
                    if (paren_depth > 0) {
                        pending_nl = true;
                    } else {
                        after_newline_comment = true;
                        if (line_has_token) {
                            line_has_token = false;
                            token->type = TOKEN_EOL;
                            return;
                        }
                    }
                }
                state = STATE_START;
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
            buf_release();
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
                buf_start();
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
                buf_release();
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
            buf_start();
            state = STATE_STRING;
            break;

        case STATE_QUOTE2:
            if (character == '"') {
                buf_start();
                state = STATE_MULTI;
                break;
            }

            // Prazdny retezec ""
            put_back(character);
            buf_start();
            token->type = TOKEN_STRING_LIT;
            token->value.text = dstr_copy(&buffer);
            buf_release();
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
                buf_release();
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
            buf_release();
            return;    
        }
    }

}

// Vytvori token INDENT nebo DEDENT
static void make_token(Token *token, TokenType type, int token_line)
{
    memset(token, 0, sizeof(*token));
    token->type = type;
    token->line = token_line;
    token->nl_before = true;
}


// Prvni token logickeho radku: porovna odsazeni se zasobnikem
// Podrzi token a misto nej vrati INDENT nebo DEDENT
static void apply_indent(Token *token)
{   
    // Odsazeni obsahuje tabulator, nesmi
    if (indent_has_tab) {
        free_token_text(token);
        error_exit(ERR_LEXICAL, token->line, "Odsazeni nesmi obsahovat tabulator");
    }

    // Predchozi odsazeni
    int top = *(int *)stack_top(&indent_stack); 

    // Aktualni odsazeni vetsi nez predchozi odsazeni
    if (indent_count > top) {
        stack_push(&indent_stack, &indent_count); // Ulozi nove odsazeni na stack
        held_token = *token; // Puvodni token si schovam, nejdriv vratim INDENT
        has_held = true;

        // Prepisu aktualni token na INDENT token
        //(skutecny si drzim v held_token)
        make_token(token, TOKEN_INDENT, held_token.line);
    
    // Aktualni mensi odsazeni nez predtim, to je konec bloku kodu
    } else if (indent_count < top) {
        int count = 0;

        // Odebiram ze zasobniku tak dlouho dokud se nedostanu na uroven aktualniho odsazeni
        // a pocita se kolik bloku se uzavrelo (count)
        while (indent_count < top) {
            stack_pop(&indent_stack, NULL);
            count++;
            top = *(int *)stack_top(&indent_stack);
        }

        // Kontrola, zda nove odsazeni presne odpovida nejake predchozi urovni
        if (indent_count != top) {
            free_token_text(token);
            error_exit(ERR_LEXICAL, token->line, "Nesedici odsazeni");
        }

        // Ulozeni puvodniho tokenu pro pozdejsi vraceni
        held_token = *token;
        has_held = true;

        // Pocet skoncenych bloku = 2, count-1=1
        // Do pending_dedents si ulozim ze dluzim jeste jeden TOKEN_DEDENT navic
        // Protoze ted hned muzu vratit jeden DEDENT
        pending_dedents = count - 1;
        make_token(token, TOKEN_DEDENT, held_token.line);
    }
}

// Konec souboru: uzavre vsechny zbyvajici urovne odsazeni
static void flush_indent(Token *token)
{
    // Spocitani poctu otevrenych bloku (odecteni zakladni urovne na dne zasobniku)
    int count = (int)stack_size(&indent_stack) - 1;

    // Pokud neni otevreny zadny blok, neni co zavirat a funkce skonci
    if (count <= 0)
        return;

    // Smeteni vsech dodatecnych urovni odsazeni ze zasobniku (zbyde jen zakladni dno)
    while (stack_size(&indent_stack) > 1)
        stack_pop(&indent_stack, NULL);

    held_token = *token;
    has_held = true;

    // Nastaveni poctu zbyvajicich dedentu pro dalsi volani lexeru
    pending_dedents = count - 1;

    // Vraceni prvniho tokenu DEDENT misto konce souboru
    make_token(token, TOKEN_DEDENT, held_token.line);
}




// Precte dalsi token a udrzuje stav scanneru mezi tokeny
void get_token(Token *token)
{
    if (!indent_ready) {
        int zero = 0;
        stack_init(&indent_stack, sizeof(int));

        // Vlozeni vychozi nulove urovne na dno zasobniku
        stack_push(&indent_stack, &zero);
        
        // Sledovani odsazeni je pripraveno
        indent_ready = true;
        atexit(scanner_cleanup);
    }

    // Nejdriv cekajici DEDENTy, potom podrzeny token
    if (pending_dedents > 0) {
        pending_dedents--;
        make_token(token, TOKEN_DEDENT, held_token.line);
        return;
    }
    if (has_held) {
        *token = held_token;
        has_held = false;
        return;
    }

    read_token(token);

    // Preneseni informace o konci radku predchazejiciho tokenu a vymazani priznaku
    token->nl_before = pending_nl;
    pending_nl = false;
    bool check_continuation = pending_check;
    pending_check = false;

    // Prvni token neprazdneho logickeho radku mimo zavorky rozhoduje o odsazeni
    bool first_on_line = !line_has_token && paren_depth == 0
        && token->type != TOKEN_EOL && token->type != TOKEN_EOF;

    // Pokracovaci radek uvnitr zavorek musi byt odsazen vic nez zacatek logickeho radku
    if (check_continuation && paren_depth > 0
        && token->type != TOKEN_EOL && token->type != TOKEN_EOF) {
        // Zkontroluje zda, odsazeni neobsahuje tabulator    
        if (indent_has_tab) {
            free_token_text(token);
            error_exit(ERR_LEXICAL, token->line, "Odsazeni nesmi obsahovat tabulator");
        }
        // Zkontroluje zda odsazeni je vetsi nez u zacatku logickeho radku
        if (indent_count <= logical_indent) {
            free_token_text(token);
            error_exit(ERR_LEXICAL, token->line, "Pokracovaci radek v zavorkach musi byt odsazen vic");
        }
    }

    if (first_on_line)
        logical_indent = indent_count;

    // Hloubka zavorek
    if (token->type == TOKEN_LPAREN || token->type == TOKEN_LBRACKET)
        paren_depth++;
    else if ((token->type == TOKEN_RPAREN || token->type == TOKEN_RBRACKET) && paren_depth > 0)
        paren_depth--;

    // Radek ma aspon jeden token
    if (token->type != TOKEN_EOL && token->type != TOKEN_EOF)
        line_has_token = true;

    // Pokud je to prvni token na radku, aplikuje se logika odsazeni
    if (first_on_line)
        apply_indent(token);
    // Pokud se jedna o konec souboru, uzavrou se vsechny zbyvajici bloky    
    else if (token->type == TOKEN_EOF)
        flush_indent(token);
}