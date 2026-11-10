/*
    IFJ projekt 2026

    production.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - pridana makra GRAMMAR_TERMINAL a GRAMMAR_NONTERMINAL.

    Tento soubor obsahuje datove struktury a pomocne funkce pro reprezentaci
    prvku na zasobniku parseru a pro reprezentaci produkci bez semanticke akce.
    Produkce je prakticky prava strana daneho pravidla.

*/

#ifndef PRODUCTION_H
#define PRODUCTION_H

#include <stdbool.h>
#include <stddef.h>

#include "nonterminal.h"
#include "token.h"

/*
    Prompt pro vychozi kostru souboru s GrammarSymbol a StaticProduction

    Prompt pro Claude:
        Napsal jsem si některé deklarace struktur, které budu potřebovat. Vždy
        ti pošlu zapsané struktury (bez přílišného uhlazování), a ty je prosím
        přetav v použitelné hlavičkové soubory. Dodržuj předešlé konvence,
        vygeneruj i doxygen dokumentaci (k jednotlivým strukturám a souborům
        napíši i popis). Pokud by něco působilo nedostatečně okomentovaně, můžeš
        komentář přidat, pokud víš, co daná věc dělá třeba z mého popisu.
        Kontroluj prosím i to, aby nenastaly cyklické reference mezi
        hlavičkovými soubory a případně dej vědět. Ošetři i guardy #IFNDED a
        spol. a kdybych zapomněl na nějakou knihovnu ze stardardních knihoven,
        tak přidej import. Pokud by jsi chtěl více informací k některé
        struktuře, řekni si.

        Začneme souborem production.h, kde bude především definice GrammarSymbol
        (terminál + neterminál) a také StaticProduction. Parser bude prediktivní
        a pravidla budou dvou typů - se sémantickým callbackem a bez něj. Ty bez
        něj budou obvykle jednoduchá pravidla, třeba epsilon pravidla nebo
        pravidla, která jen dosadí token typu ; bez zvláštního významu. Případné
        pomocné funkce pro práci s nimi doplním později, ale pravděpodobně ani
        nebudou potřeba. Buď přidružený .c, nebo dvojice .h/.c s LL tabulkou pak
        budou obsahovat konkrétní deklarace statických produkcí (nejspíše na
        globální úrovni) pro vložení do LL tabulky referencí.

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

/*
    Prompt pro makra GRAMMAR_TERMINAL a GRAMMAR_NONTERMINAL je v souboru
    LL_table.c.
*/

/**
 * @brief Inicializator GrammarSymbol pro terminal.
 *
 * Pouziva se pri inicializaci promenne nebo pole, napr.
 * GrammarSymbol symbol = GRAMMAR_TERMINAL(TOKEN_ID);
 */
#define GRAMMAR_TERMINAL(t)                                                    \
  { .is_terminal = true, .terminal = (t) }

/**
 * @brief Inicializator GrammarSymbol pro neterminal.
 *
 * Pouziva se pri inicializaci promenne nebo pole, napr.
 * GrammarSymbol symbol = GRAMMAR_NONTERMINAL(NONTERM_TYPE);
 */
#define GRAMMAR_NONTERMINAL(n)                                                 \
  { .is_terminal = false, .nonterminal = (n) }

/**
 * @brief Staticka produkce, prava strana pravidla ulozena primo ve strukture.
 * 
 * Staticke produkce predavaji semanticky pointer z neterminalu na leve strane pravidla do vsech symbolu produkce.
 */
typedef struct {
  size_t length; ///< Pocet symbolu
  /// Symboly, vyuzito je prvnich length
  GrammarSymbol production[STATIC_PRODUCTION_MAX_LENGTH];
} StaticProduction;

#endif // PRODUCTION_H
