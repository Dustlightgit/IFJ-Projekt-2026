/*
    IFJ projekt 2026

    xnovacm00 Michal Novacek

    error.h

    Chybove navratove kody podle specifikace a funkce pro ukonceni programu
    s chybou.

*/

#ifndef ERROR_H
#define ERROR_H

// Chybove navratove kody podle specifikace
#define ERR_LEXICAL 1   // Lexikalni chyba
#define ERR_SYNTAX 2    // Syntakticka chyba
#define ERR_UNDEFINED 3 // Nedefinovana funkce, promenna nebo typ
#define ERR_REDEFINED 4 // Redefinice
#define ERR_ARGS 5      // Spatny pocet nebo typ argumentu
#define ERR_TYPE 6      // Typova kompatibilita
#define ERR_IMMUTABLE 7 // Zapis do nemodifikovatelne promenne
#define ERR_OTHER 10    // Ostatni semanticke chyby
#define ERR_INTERNAL 99 // Interni chyba prekladace

// Vypise chybu na stderr (kod, radek, zprava) a ukonci program s danym kodem
// Radek <= 0 znamena, ze radek neni znam
void error_exit(int code, int line, const char *message);

#endif // ERROR_H
