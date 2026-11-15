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
        - xbalaka00 - komentare pravidel prepsany s ceskou terminologii (straz,
          visici else apod.).
        - xbalaka00 - odstranena deklarace testovaci produkce <expr> => ID.
        - xbalaka00 - zdokumentovano kopirovani semantickeho ukazatele
          a kontextove anotace ve statickych produkcich.
        - xbalaka00 - poznamka k pravidlu 30 (NONTERM_EXPR_NO_ID byl
          odstranen, nahrazuje ho NONTERM_EXPR).
        - xbalaka00 - odkazy na excel s gramatikou zmeneny na
          parser_formalisms.xlsx.
        - xbalaka00 - GrammarSymbol a makra GRAMMAR_TERMINAL
          a GRAMMAR_NONTERMINAL presunuty do grammar_symbol.h,
          STATIC_PRODUCTION_MAX_LENGTH snizena na 5 (prompt je v
          reduction_table.h).

    Tento soubor obsahuje datove struktury a pomocne funkce pro reprezentaci
    prvku na zasobniku parseru a pro reprezentaci produkci bez semanticke akce.
    Produkce je prakticky prava strana daneho pravidla. Obsahuje take
    deklarace konkretnich statickych produkci pro LL tabulku.

*/

#ifndef PRODUCTION_H
#define PRODUCTION_H

#include <stddef.h>

#include "grammar_symbol.h"

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
 * Hodnota odpovida nejdelsi staticke produkci (pravidlo 51).
 */
#define STATIC_PRODUCTION_MAX_LENGTH 5

/*
    GrammarSymbol a makra GRAMMAR_TERMINAL a GRAMMAR_NONTERMINAL jsou v souboru
    grammar_symbol.h. Prompt pro jejich presun je v souboru reduction_table.h,
    prompt pro makra je v souboru LL_table.c.
*/

/**
 * @brief Staticka produkce, prava strana pravidla ulozena primo ve strukture.
 *
 * Pri vlozeni staticke produkce na zasobnik parser vzdy zkopiruje semanticky
 * ukazatel a kontextovou anotaci neterminalu na leve strane pravidla do
 * vsech symbolu produkce. Staticka pravidla tak sirí tyto informace dal,
 * aniz by musela byt callbacky.
 */
typedef struct {
  size_t length; ///< Pocet symbolu
  /// Symboly, vyuzito je prvnich length
  GrammarSymbol production[STATIC_PRODUCTION_MAX_LENGTH];
} StaticProduction;

/*
    Prompt pro deklarace statickych produkci a jejich definice v production.c
    je v souboru production.c, stejne jako prompty pro jejich aktualizaci
    a pro upravu komentaru na ceskou terminologii. Prompt pro odstraneni
    testovaciho pravidla <expr> => ID je v souboru LL_table.c. Prompt pro
    kopirovani kontextove anotace ve statickych produkcich je v souboru
    semantic_types.h. Prompt pro odstraneni NONTERM_EXPR_NO_ID je v souboru
    nonterminal.h. Prompt pro prejmenovani odkazu na excel je v souboru
    production.c.
*/

// Staticke produkce gramatiky, definice jsou v production.c. Cisla pravidel
// odpovidaji tabulce gramatiky (parser_formalisms.xlsx), (ptr) znaci pravidlo,
// ktere predava semanticky ukazatel (parser ukazatel i kontextovou anotaci
// kopiruje u vsech statickych produkci, znacka je jen informativni).

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
/// Pozn.: <exprNoID> z tabulky gramatiky je v implementaci nahrazen <expr>.
extern const StaticProduction PRODUCTION_COMMAND_EXPR;


/// Pravidlo 30 (Prikaz vyrazu) (ptr): <commandSimple> => <exprNoID>
/// Pozn.: nahradi PRODUCTION_COMMAND_EXPR, ktere se odstrani v LL tabulce.
extern const StaticProduction PRODUCTION_COMMAND_SIMPLE_EXPR;

/// Pravidlo 45 (Jednoduchy prikaz) (ptr): <command> => <commandSimple> <commandEnd>
extern const StaticProduction PRODUCTION_COMMAND_SIMPLE;

/// Pravidlo 46 (Ridici prikaz) (ptr): <command> => <commandControl>
extern const StaticProduction PRODUCTION_COMMAND_CONTROL;

/// Pravidlo 55 (Then vetev s blokem) (ptr): <thenBranch> => <block> <optionalElse>
extern const StaticProduction PRODUCTION_THEN_BRANCH_BLOCK;

/// Pravidlo 56 (Then vetev s prikazem) (ptr): <thenBranch> => <commandSimple> <elseOrCommandEnd>
extern const StaticProduction PRODUCTION_THEN_BRANCH_SIMPLE;

/// Pravidlo 57 (Then vetev s ridicim prikazem, bez else) (ptr): <thenBranch> => <commandControl>
extern const StaticProduction PRODUCTION_THEN_BRANCH_CONTROL;

/// Pravidlo 58 (Konec prikazu v then vetvi) (ptr): <elseOrCommandEnd> => <commandEnd>
extern const StaticProduction PRODUCTION_ELSE_OR_COMMAND_END_COMMAND_END;


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
