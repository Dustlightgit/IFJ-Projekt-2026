/*
    IFJ projekt 2026

    production.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - pridana makra GRAMMAR_TERMINAL a GRAMMAR_NONTERMINAL.
        - xbalaka00 - pridany deklarace statickych produkci z gramatiky
          (definice jsou v production.c).
        - xbalaka00 - aktualizace deklaraci dle nove verze gramatiky
          (odstranena pravidla 33, 45 a 46).
        - xbalaka00 - odstranena deklarace testovaci produkce <expr> => ID.

    Tento soubor obsahuje datove struktury a pomocne funkce pro reprezentaci
    prvku na zasobniku parseru a pro reprezentaci produkci bez semanticke akce.
    Produkce je prakticky prava strana daneho pravidla. Obsahuje take
    deklarace konkretnich statickych produkci pro LL tabulku.

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
 * Staticke produkce predavaji semanticky ukazatel z neterminalu na leve strane pravidla do vsech symbolu produkce.
 */
typedef struct {
  size_t length; ///< Pocet symbolu
  /// Symboly, vyuzito je prvnich length
  GrammarSymbol production[STATIC_PRODUCTION_MAX_LENGTH];
} StaticProduction;

/*
    Prompt pro deklarace statickych produkci a jejich definice v production.c
    je v souboru production.c, stejne jako prompty pro jejich aktualizaci a pro upravu
    komentaru na ceskou terminologii. Prompt pro odstraneni testovaciho
    pravidla <expr> => ID je v souboru LL_table.c.
*/

// Staticke produkce gramatiky, definice jsou v production.c. Cisla pravidel
// odpovidaji tabulce gramatiky (LL_gramatika.xlsx), (ptr) znaci pravidlo,
// ktere predava semanticky ukazatel.

/// Pravidlo 1 (Seznam deklaraci) (ptr): <defList> => <def> <defList>
extern const StaticProduction PRODUCTION_DEF_LIST_NEXT;

/// Pravidlo 2 (Konec deklaraci): <defList> => epsilon
extern const StaticProduction PRODUCTION_DEF_LIST_END;

/// Pravidlo 5 (Anotace neni): <main> => epsilon
extern const StaticProduction PRODUCTION_MAIN_ABSENT;

/// Pravidlo 6 (EOL je): <optionalEOL> => EOL
extern const StaticProduction PRODUCTION_OPTIONAL_EOL_PRESENT;

/// Pravidlo 7 (EOL neni): <optionalEOL> => epsilon
extern const StaticProduction PRODUCTION_OPTIONAL_EOL_ABSENT;

/// Pravidlo 9 (Pridani carky) (ptr): <paramComma> => , <paramAdd>
extern const StaticProduction PRODUCTION_PARAM_LIST_TAIL_NEXT;

/// Pravidlo 10 (Konec parametru A): <paramAdd> => epsilon
extern const StaticProduction PRODUCTION_PARAM_LIST_END;

/// Pravidlo 11 (Konec parametru B): <paramComma> => epsilon
extern const StaticProduction PRODUCTION_PARAM_LIST_TAIL_END;

/// Pravidlo 12 (Navratovy typ je) (ptr): <optionalType> => : <type>
extern const StaticProduction PRODUCTION_OPTIONAL_TYPE_PRESENT;

/// Pravidlo 13 (Navratovy typ neni): <optionalType> => epsilon
extern const StaticProduction PRODUCTION_OPTIONAL_TYPE_ABSENT;

/// Pravidlo 20 (Jednoradkovy vyraz misto bloku) (ptr): <blockOrExpr> => <expr> EOL
extern const StaticProduction PRODUCTION_BLOCK_OR_EXPR_EXPR;

/// Pravidlo 21 (Pouziti bloku) (ptr): <blockOrExpr> => <block>
extern const StaticProduction PRODUCTION_BLOCK_OR_EXPR_BLOCK;

/// Pravidlo 23 (Dalsi prikaz) (ptr): <commandList> => <command> <commandList>
extern const StaticProduction PRODUCTION_COMMAND_LIST_NEXT;

/// Pravidlo 24 (Konec seznamu prikazu): <commandList> => epsilon
extern const StaticProduction PRODUCTION_COMMAND_LIST_END;

/// Pravidlo 25 (Konec prikazu - strednik): <commandEnd> => ;
extern const StaticProduction PRODUCTION_COMMAND_END_SEMICOLON;

/// Pravidlo 26 (Konec prikazu - EOL): <commandEnd> => EOL
extern const StaticProduction PRODUCTION_COMMAND_END_EOL;

/// Pravidlo 29 (Deklarace modifikovatelne promenne): <valVar> => var
extern const StaticProduction PRODUCTION_VAR;

/// Pravidlo 30 (Prikaz vyrazu) (ptr): <command> => <exprNoID> <commandEnd>
extern const StaticProduction PRODUCTION_COMMAND_EXPR;

/// Pravidlo 35 (Return vyrazu) (ptr): <returnValue> => <expr>
extern const StaticProduction PRODUCTION_RETURN_VALUE_EXPR;

/// Pravidlo 36 (Prazdny return): <returnValue> => epsilon
extern const StaticProduction PRODUCTION_RETURN_VALUE_EMPTY;

/// Pravidlo 42 (Konec strazi): <guardList> => epsilon
extern const StaticProduction PRODUCTION_GUARD_LIST_END;

/// Pravidlo 43 (Nasleduje blok) (ptr): <blockOrCommand> => <block>
extern const StaticProduction PRODUCTION_BLOCK_OR_COMMAND_BLOCK;

/// Pravidlo 44 (Nasleduje prikaz) (ptr): <blockOrCommand> => <command>
extern const StaticProduction PRODUCTION_BLOCK_OR_COMMAND_COMMAND;

/// Pravidlo 49 (Vynechana vetev else): <optionalElse> => epsilon
extern const StaticProduction PRODUCTION_OPTIONAL_ELSE_ABSENT;

/// Pravidlo 51 (Odsazeni vyrazu v ternarni podmince) (ptr): <exprOrIndentExpr> => EOL INDENT <expr> EOL DEDENT
extern const StaticProduction PRODUCTION_EXPR_OR_INDENT_EXPR_INDENTED;

/// Pravidlo 52 (Neodsazeny vyraz v ternarni podmince) (ptr): <exprOrIndentExpr> => <expr>
extern const StaticProduction PRODUCTION_EXPR_OR_INDENT_EXPR_PLAIN;

/// Pravidlo 54 (Nedokoncena vetev else): <elseTernaryIf> => epsilon
extern const StaticProduction PRODUCTION_ELSE_TERNARY_IF_ABSENT;

// Testovaci pravidlo mimo gramatiku, v hotovem parseru je <expr> prechod
// do vyrazoveho parseru

/// Testovaci pravidlo mimo gramatiku: <expr> => *
extern const StaticProduction PRODUCTION_TEST_EXPR_STAR;

#endif // PRODUCTION_H
