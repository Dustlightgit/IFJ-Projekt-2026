/*
    IFJ projekt 2026

    production.h

    Puvodni autor: xbakaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        -

    Tento soubor obsahuje datove struktury a pomocne funkce pro reprezentaci
   prvku na zasobniku parseru a pro reprezentaci produkci bez semanticke akce.
   Produkce je prakticky prava strana daneho pravidla.

*/

/*
    Prompt pro vychozi kostru souboru s GrammarSymbol a StaticProduction

    Prompt pro Claude:
        Napsal jsem si některé deklarace struktur, které budu potřebovat. Vždy
   ti pošlu zapsané struktury (bez přílišného uhlazování), a ty je prosím přetav
   v použitelné hlavičkové soubory. Dodržuj předešlé konvence, vygeneruj i
   doxygen dokumentaci (k jednotlivým strukturám a souborům napíši i popis).
   Pokud by něco působilo nedostatečně okomentovaně, můžeš komentář přidat,
   pokud víš, co daná věc dělá třeba z mého popisu. Kontroluj prosím i to, aby
   nenastaly cyklické reference mezi hlavičkovými soubory a případně dej vědět.
   Ošetři i guardy #IFNDED a spol. a kdybych zapomněl na nějakou knihovnu ze
   stardardních knihoven, tak přidej import. Pokud by jsi chtěl více informací k
   některé struktuře, řekni si.

        Začneme souborem production.h, kde bude především definice GrammarSymbol
   (terminál + neterminál) a také StaticProduction. Parser bude prediktivní a
   pravidla budou dvou typů - se sémantickým callbackem a bez něj. Ty bez něj
   budou obvykle jednoduchá pravidla, třeba epsilon pravidla nebo pravidla,
   která jen dosadí token typu ; bez zvláštního významu. Případné pomocné funkce
   pro práci s nimi doplním později, ale pravděpodobně ani nebudou potřeba. Buď
   přidružený .c, nebo dvojice .h/.c s LL tabulkou pak budou obsahovat konkrétní
   deklarace statických produkcí (nejspíše na globální úrovni) pro vložení do LL
   tabulky referencí.

        #include "nonterminal.h"
        #include "token.h"

        #define STATIC_PRODUCTION_MAX_LENGTH 9
        // Hodnota bude upravena pro nejlepsi vyuziti pameti,
        // prozatim dle nejdelsiho pravidla ktere ale neni staticke.

        typedef struct {
            bool is_terminal;           // Odliseni terminalu od neterminalu
            union {                     // Samotny symbol gramatiky
                TokenType terminal;
                NonterminalType nonterminal;
            };
        } GrammarSymbol;


        typedef struct {
            size_t length;         // Pocet symbolu GrammarSymbol
            production[STATIC_PRODUCTION_MAX_LENGTH];     // Symboly
        } StaticProduction;

*/

#ifndef PRODUCTION_H
#define PRODUCTION_H

#include <stdbool.h>
#include <stddef.h>

#include "nonterminal.h"
#include "token.h"

/**
 * @brief Maximalni pocet symbolu v jedne staticke produkci.
 *
 * Hodnota bude upravena pro nejlepsi vyuziti pameti, prozatim je dle
 * nejdelsiho pravidla, ktere ale neni staticke.
 */
#define STATIC_PRODUCTION_MAX_LENGTH 9

/**
 * @brief Symbol gramatiky, terminal nebo neterminal.
 */
typedef struct {
  bool is_terminal; ///< Odliseni terminalu od neterminalu
  union {
    TokenType terminal;          ///< Platne, pokud je is_terminal true
    NonterminalType nonterminal; ///< Platne, pokud je is_terminal false
  };
} GrammarSymbol;

/**
 * @brief Staticka produkce, prava strana pravidla ulozena primo ve strukture.
 */
typedef struct {
  size_t length;                                          ///< Pocet symbolu
  GrammarSymbol production[STATIC_PRODUCTION_MAX_LENGTH]; ///< Symboly, vyuzito
                                                          ///< je prvnich length
} StaticProduction;

#endif // PRODUCTION_H
