/*
    IFJ projekt 2026

    xbalaka00 Artur Balak

    precedence_table.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - vstupni token, ktery neindexuje precedencni tabulku,
          je v precedence_table_lookup syntakticka chyba (ERR_SYNTAX) misto
          interni (prompt je v precedence_parser.h).
        - xbalaka00 - indexace podporuje tokeny TOKEN_PLUS_UNARY
          a TOKEN_MINUS_UNARY, ktere jsou (stejne jako TOKEN_PLUS
          a TOKEN_MINUS) ve souvislem rozsahu enumu, vypocet indexu tedy
          zustava, zvetsil se jen PRECEDENCE_RANGE_COUNT (prompt je v token.h).
        - xbalaka00 - vyplnena precedencni tabulka podle rucne navrzene
          tabulky, vypocet indexu je sdileny makry PRECEDENCE_RANGE_INDEX
          a PIDX_* (prompt je zde).
        - xbalaka00 - TOKEN_STAR je nyni v souvislem rozsahu enumu, odstranen
          ze switch v precedence_table_try_index, PRECEDENCE_SWITCH_COUNT
          snizen na 5. Pridan radek a sloupec pro TOKEN_TIF (zatim same
          prazdne polozky), tabulka ma 30 radku a sloupcu (prompt je v token.h).

    Tento soubor obsahuje precedencni tabulku precedencniho parseru vyrazu
    a funkce a makra pro prevod terminalu na jeji index a funkce pro vyhledani direktivy.
    Prompt pro vznik souboru je v souboru precedence_table.h.
*/

/*
    Prompt pro vyplneni precedencni tabulky.

    Prompt pro Claude:
        Dělám parser založený na hybridu prediktivní a precedenční analýzy. Přikládám relevantní soubory. Pokud by ti některý soubor chyběl, vyžádej si jej, než začneš pracovat. Kontext návrhu a význam kódu si přečti v komentářích a deklarovaných promptech. Dodržuj jmenné konvence (včetně komentářů v češtině bez diakritiky - používej české názvy všech věcí i přesto, že v promptech někdy věci pojmenuji anglicky; prompty ale cituj doslovně po vzoru souborů), formát hlaviček souborů s popisem souboru, dle vzoru přiložených souborů také deklaruj prompty a aktualizuj změnové seznamy. Kód můžeš zkompilovat, ale podle mě není nutné ho testovat na vlastní implementaci.

        Vytvořil jsem precedenční tabulku (ručně navrhl, claude ji zkontroloval a našel tucet chyb). Mohl by jsi mi ji přetvořit do kódu? Všechny potřebné věci by jsi měl mít už v souborech, které přiložím.

        Pozor - indexy do tabulky neodpovídají pořadí terminálů v tabulce, kterou ti dám. Terminály se na prvky tabulky převádí indexační funkcí. Při indexaci ji asi použij, pokud to jde. Smíš si napsat makra pro zjednodušení zápisu.

        Precedenční tabulku mám v excelu, zde ti ji překopíruju v textové podobě (tu nemusíš zahrnovat do deklarovaného promptu):
        (tabulka vynechana, jak je v promptu uvedeno; radky jsou terminal na stacku, sloupce vstupni terminal)
*/

#include "precedence_table.h"

#include <stdio.h>

#include "error.h"
#include "token.h"

// Index terminalu t z souvisleho rozsahu enumu TokenType mezi
// TOKEN_LL_TERM_COUNT a TOKEN_EXPR_TERM_DELIMITER (oddelovac
// TOKEN_EXPRESSION_BEGINNING_DELIMITER neni terminal a index se za nim snizi).
// Je to celociselny konstantni vyraz, takze jej lze pouzit i v inicializaci
// tabulky.
#define PRECEDENCE_RANGE_INDEX(t)                                              \
  ((size_t)(t) - (size_t)TOKEN_LL_TERM_COUNT - 1 -                             \
   ((t) > TOKEN_EXPRESSION_BEGINNING_DELIMITER ? 1 : 0))

// Pocet terminalu v souvislem rozsahu enumu TokenType mezi TOKEN_LL_TERM_COUNT
// a TOKEN_EXPR_TERM_DELIMITER bez oddelovace TOKEN_EXPRESSION_BEGINNING_DELIMITER
// (zahrnuje i TOKEN_PLUS_UNARY, TOKEN_MINUS_UNARY, TOKEN_PLUS a TOKEN_MINUS)
#define PRECEDENCE_RANGE_COUNT                                                 \
  ((size_t)TOKEN_EXPR_TERM_DELIMITER - (size_t)TOKEN_LL_TERM_COUNT - 2)

// Pocet terminalu precedencni tabulky, ktere jsou v enumu TokenType mezi
// indexatory LL tabulky a v precedence_table_try_index je resi switch
// (KW_LIST, (, ), carka a ID).
#define PRECEDENCE_SWITCH_COUNT 5

// Kontrola, ze pocet indexatoru v souvislem rozsahu a ve switch odpovida
// velikosti tabulky.
_Static_assert(PRECEDENCE_RANGE_COUNT + PRECEDENCE_SWITCH_COUNT ==
                   PRECEDENCE_TABLE_SIZE,
               "Pocet indexatoru precedencni tabulky nesedi s "
               "PRECEDENCE_TABLE_SIZE.");

// Indexy terminalu zamichanych mezi indexatory LL tabulky (za souvislym
// rozsahem). Pouziva je switch v precedence_table_try_index i inicializace
// tabulky.
#define PIDX_SWITCH_KW_LIST (PRECEDENCE_RANGE_COUNT + 0)
#define PIDX_SWITCH_LPAREN (PRECEDENCE_RANGE_COUNT + 1)
#define PIDX_SWITCH_RPAREN (PRECEDENCE_RANGE_COUNT + 2)
#define PIDX_SWITCH_COMMA (PRECEDENCE_RANGE_COUNT + 3)
#define PIDX_SWITCH_ID (PRECEDENCE_RANGE_COUNT + 4)

// Indexy terminalu pro zapis tabulky (pojmenovani odpovida radkum a sloupcum
// tabulky navrzene rucne).
#define PIDX_LIST PIDX_SWITCH_KW_LIST
#define PIDX_LPAREN PIDX_SWITCH_LPAREN
#define PIDX_RPAREN PIDX_SWITCH_RPAREN
#define PIDX_COMMA PIDX_SWITCH_COMMA
#define PIDX_DOT PRECEDENCE_RANGE_INDEX(TOKEN_DOT)
#define PIDX_ID PIDX_SWITCH_ID
#define PIDX_INT PRECEDENCE_RANGE_INDEX(TOKEN_INT_LIT)
#define PIDX_DBL PRECEDENCE_RANGE_INDEX(TOKEN_DOUBLE_LIT)
#define PIDX_STR PRECEDENCE_RANGE_INDEX(TOKEN_STRING_LIT)
#define PIDX_TRUE PRECEDENCE_RANGE_INDEX(TOKEN_KW_TRUE)
#define PIDX_FALSE PRECEDENCE_RANGE_INDEX(TOKEN_KW_FALSE)
#define PIDX_PPLUS PRECEDENCE_RANGE_INDEX(TOKEN_PLUS_UNARY)
#define PIDX_PMINUS PRECEDENCE_RANGE_INDEX(TOKEN_MINUS_UNARY)
#define PIDX_NOT PRECEDENCE_RANGE_INDEX(TOKEN_NOT)
#define PIDX_STAR PRECEDENCE_RANGE_INDEX(TOKEN_STAR)
#define PIDX_SLASH PRECEDENCE_RANGE_INDEX(TOKEN_SLASH)
#define PIDX_PLUS PRECEDENCE_RANGE_INDEX(TOKEN_PLUS)
#define PIDX_MINUS PRECEDENCE_RANGE_INDEX(TOKEN_MINUS)
#define PIDX_CONCAT PRECEDENCE_RANGE_INDEX(TOKEN_PLUSPLUS)
#define PIDX_LT PRECEDENCE_RANGE_INDEX(TOKEN_LT)
#define PIDX_LE PRECEDENCE_RANGE_INDEX(TOKEN_LE)
#define PIDX_GT PRECEDENCE_RANGE_INDEX(TOKEN_GT)
#define PIDX_GE PRECEDENCE_RANGE_INDEX(TOKEN_GE)
#define PIDX_EQ PRECEDENCE_RANGE_INDEX(TOKEN_EQ)
#define PIDX_NE PRECEDENCE_RANGE_INDEX(TOKEN_NE)
#define PIDX_AND PRECEDENCE_RANGE_INDEX(TOKEN_AND)
#define PIDX_OR PRECEDENCE_RANGE_INDEX(TOKEN_OR)
#define PIDX_IDOP PRECEDENCE_RANGE_INDEX(TOKEN_ID_OPERATOR)
#define PIDX_DOLLAR PRECEDENCE_RANGE_INDEX(TOKEN_DOLLAR)
#define PIDX_TIF PRECEDENCE_RANGE_INDEX(TOKEN_TIF)

// Zkratky polozek tabulky.
#define NO DIRECTIVE_ERROR             // - (prazdne pole)
#define MK DIRECTIVE_REDUCTION_MARKER  // <
#define EX DIRECTIVE_REDUCTION_EXECUTE // >
#define SH DIRECTIVE_SHIFT_SYMBOL      // =

// Jeden radek tabulky. Polozky se zapisuji v poradi sloupcu z rucne navrzene
// tabulky, makro je rozmisti na spravne indexy (sloupce):
// List ( ) , . ID INT DBL STR true false +u -u ! * / + - ++ < <= > >= == !=
// && || ID_OPERATOR $ TIF
#define PRECEDENCE_ROW(c01, c02, c03, c04, c05, c06, c07, c08, c09, c10, c11,  \
                       c12, c13, c14, c15, c16, c17, c18, c19, c20, c21, c22,  \
                       c23, c24, c25, c26, c27, c28, c29, c30)                 \
  {                                                                            \
    [PIDX_LIST] = c01, [PIDX_LPAREN] = c02, [PIDX_RPAREN] = c03,               \
    [PIDX_COMMA] = c04, [PIDX_DOT] = c05, [PIDX_ID] = c06, [PIDX_INT] = c07,   \
    [PIDX_DBL] = c08, [PIDX_STR] = c09, [PIDX_TRUE] = c10, [PIDX_FALSE] = c11, \
    [PIDX_PPLUS] = c12, [PIDX_PMINUS] = c13, [PIDX_NOT] = c14,                 \
    [PIDX_STAR] = c15, [PIDX_SLASH] = c16, [PIDX_PLUS] = c17,                  \
    [PIDX_MINUS] = c18, [PIDX_CONCAT] = c19, [PIDX_LT] = c20, [PIDX_LE] = c21, \
    [PIDX_GT] = c22, [PIDX_GE] = c23, [PIDX_EQ] = c24, [PIDX_NE] = c25,        \
    [PIDX_AND] = c26, [PIDX_OR] = c27, [PIDX_IDOP] = c28, [PIDX_DOLLAR] = c29, \
    [PIDX_TIF] = c30                                                           \
  }

// Precedencni tabulka, radky jsou terminal na zasobniku, sloupce terminal na
// vstupu. Radky jsou zapsany v poradi rucne navrzene tabulky, na spravny index
// je umistuje makro PIDX_*. Kazdy radek ma 30 polozek (sloupce: List az *,
// / az $, TIF). Radek a sloupec TIF jsou zatim same prazdne polozky.
const PrecedenceDirective
    PRECEDENCE_TABLE[PRECEDENCE_TABLE_SIZE][PRECEDENCE_TABLE_SIZE] = {
        [PIDX_LIST] = PRECEDENCE_ROW(NO, SH, NO, NO, SH, NO, NO, NO, NO, NO, NO,
                                     NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO,
                                     NO, NO, NO, NO, NO, NO, NO, NO),
        [PIDX_LPAREN] = PRECEDENCE_ROW(MK, MK, SH, MK, MK, MK, MK, MK, MK, MK,
                                       MK, MK, MK, MK, MK, MK, MK, MK, MK, MK,
                                       MK, MK, MK, MK, MK, MK, MK, MK, NO, NO),
        [PIDX_RPAREN] = PRECEDENCE_ROW(NO, NO, EX, EX, EX, NO, NO, NO, NO, NO,
                                       NO, NO, NO, NO, EX, EX, EX, EX, EX, EX,
                                       EX, EX, EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_COMMA] = PRECEDENCE_ROW(MK, MK, EX, EX, EX, MK, MK, MK, MK, MK,
                                      MK, MK, MK, MK, MK, MK, MK, MK, MK, MK,
                                      MK, MK, MK, MK, MK, MK, MK, MK, NO, NO),
        [PIDX_DOT] = PRECEDENCE_ROW(NO, NO, NO, NO, EX, SH, NO, NO, NO, NO, NO,
                                    NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO,
                                    NO, NO, NO, NO, NO, NO, NO, NO),
        [PIDX_ID] = PRECEDENCE_ROW(NO, SH, EX, EX, EX, NO, NO, NO, NO, NO, NO,
                                   NO, NO, NO, EX, EX, EX, EX, EX, EX, EX, EX,
                                   EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_INT] = PRECEDENCE_ROW(NO, NO, EX, EX, EX, NO, NO, NO, NO, NO, NO,
                                    NO, NO, NO, EX, EX, EX, EX, EX, EX, EX, EX,
                                    EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_DBL] = PRECEDENCE_ROW(NO, NO, EX, EX, EX, NO, NO, NO, NO, NO, NO,
                                    NO, NO, NO, EX, EX, EX, EX, EX, EX, EX, EX,
                                    EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_STR] = PRECEDENCE_ROW(NO, NO, EX, EX, EX, NO, NO, NO, NO, NO, NO,
                                    NO, NO, NO, EX, EX, EX, EX, EX, EX, EX, EX,
                                    EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_TRUE] = PRECEDENCE_ROW(NO, NO, EX, EX, EX, NO, NO, NO, NO, NO, NO,
                                     NO, NO, NO, EX, EX, EX, EX, EX, EX, EX, EX,
                                     EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_FALSE] = PRECEDENCE_ROW(NO, NO, EX, EX, EX, NO, NO, NO, NO, NO,
                                      NO, NO, NO, NO, EX, EX, EX, EX, EX, EX,
                                      EX, EX, EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_PPLUS] = PRECEDENCE_ROW(NO, MK, EX, EX, MK, MK, MK, MK, MK, MK,
                                      MK, MK, MK, MK, EX, EX, EX, EX, EX, EX,
                                      EX, EX, EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_PMINUS] = PRECEDENCE_ROW(NO, MK, EX, EX, MK, MK, MK, MK, MK, MK,
                                       MK, MK, MK, MK, EX, EX, EX, EX, EX, EX,
                                       EX, EX, EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_NOT] = PRECEDENCE_ROW(NO, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                    MK, MK, MK, EX, EX, EX, EX, EX, EX, EX, EX,
                                    EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_STAR] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                     MK, MK, MK, EX, EX, EX, EX, EX, EX, EX, EX,
                                     EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_SLASH] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK,
                                      MK, MK, MK, MK, EX, EX, EX, EX, EX, EX,
                                      EX, EX, EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_PLUS] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                     MK, MK, MK, MK, MK, EX, EX, EX, EX, EX, EX,
                                     EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_MINUS] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK,
                                      MK, MK, MK, MK, MK, MK, EX, EX, EX, EX,
                                      EX, EX, EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_CONCAT] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK,
                                       MK, MK, MK, MK, MK, MK, EX, EX, EX, EX,
                                       EX, EX, EX, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_LT] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                   MK, MK, MK, MK, MK, MK, MK, MK, NO, NO, NO,
                                   NO, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_LE] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                   MK, MK, MK, MK, MK, MK, MK, MK, NO, NO, NO,
                                   NO, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_GT] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                   MK, MK, MK, MK, MK, MK, MK, MK, NO, NO, NO,
                                   NO, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_GE] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                   MK, MK, MK, MK, MK, MK, MK, MK, NO, NO, NO,
                                   NO, EX, EX, EX, EX, EX, EX, NO),
        [PIDX_EQ] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                   MK, MK, MK, MK, MK, MK, MK, MK, MK, MK, MK,
                                   MK, NO, NO, EX, EX, EX, EX, NO),
        [PIDX_NE] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                   MK, MK, MK, MK, MK, MK, MK, MK, MK, MK, MK,
                                   MK, NO, NO, EX, EX, EX, EX, NO),
        [PIDX_AND] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                    MK, MK, MK, MK, MK, MK, MK, MK, MK, MK, MK,
                                    MK, MK, MK, EX, EX, EX, EX, NO),
        [PIDX_OR] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                   MK, MK, MK, MK, MK, MK, MK, MK, MK, MK, MK,
                                   MK, MK, MK, MK, EX, EX, EX, NO),
        [PIDX_IDOP] = PRECEDENCE_ROW(MK, MK, EX, EX, MK, MK, MK, MK, MK, MK, MK,
                                     MK, MK, MK, MK, MK, MK, MK, MK, MK, MK, MK,
                                     MK, MK, MK, MK, MK, EX, EX, NO),
        [PIDX_DOLLAR] = PRECEDENCE_ROW(MK, MK, NO, NO, MK, MK, MK, MK, MK, MK,
                                       MK, MK, MK, MK, MK, MK, MK, MK, MK, MK,
                                       MK, MK, MK, MK, MK, MK, MK, MK, NO, NO),
        [PIDX_TIF] = PRECEDENCE_ROW(NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO,
                                    NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO,
                                    NO, NO, NO, NO, NO, NO, NO, NO),
};

#undef NO
#undef MK
#undef EX
#undef SH

// Zjisti index terminalu do precedencni tabulky. Vraci false, pokud terminal
// precedencni tabulku neindexuje. Token musi byt uz overeny.
static bool precedence_table_try_index(TokenType terminal, size_t *index) {

  // Terminaly v souvislem rozsahu mezi TOKEN_LL_TERM_COUNT
  // a TOKEN_EXPR_TERM_DELIMITER, indexy zacinaji od 0. Oddelovac
  // TOKEN_EXPRESSION_BEGINNING_DELIMITER neni terminal.
  if (terminal > TOKEN_LL_TERM_COUNT && terminal < TOKEN_EXPR_TERM_DELIMITER &&
      terminal != TOKEN_EXPRESSION_BEGINNING_DELIMITER) {
    *index = PRECEDENCE_RANGE_INDEX(terminal);
    return true;
  }

  // Terminaly zamichane mezi indexatory LL tabulky, indexy za souvislym
  // rozsahem
  switch (terminal) {
  case TOKEN_KW_LIST:
    *index = PIDX_SWITCH_KW_LIST;
    return true;
  case TOKEN_LPAREN:
    *index = PIDX_SWITCH_LPAREN;
    return true;
  case TOKEN_RPAREN:
    *index = PIDX_SWITCH_RPAREN;
    return true;
  case TOKEN_COMMA:
    *index = PIDX_SWITCH_COMMA;
    return true;
  case TOKEN_ID:
    *index = PIDX_SWITCH_ID;
    return true;
  default:
    return false;
  }
}

size_t precedence_table_index(TokenType terminal, int line) {

  // Overeni, ze se jedna o skutecny token
  token_type_validate(terminal, line);

  size_t index;
  if (!precedence_table_try_index(terminal, &index))
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: Spatne pouziti precedence_table_index(): "
               "Token neni indexem precedencni tabulky.");

  return index;
}

PrecedenceDirective precedence_table_lookup(TokenType stack_terminal,
                                            TokenType input_terminal,
                                            int line) {

  // Terminal na zasobniku je vzdy indexem tabulky, jinak jde o internich chybu
  size_t row = precedence_table_index(stack_terminal, line);

  // Vstupni token muze byt cokoliv, token, ktery precedencni tabulku
  // neindexuje, je chyba ve vstupnim programu
  token_type_validate(input_terminal, line);
  size_t column;
  if (!precedence_table_try_index(input_terminal, &column)) {
    fprintf(stderr, "Precedencni parser: Neocekavany token na vstupu: %s\n",
            token_type_name(input_terminal));
    error_exit(ERR_SYNTAX, line,
               "Precedencni parser: Token se nesmi vyskytovat ve vyrazu.");
  }

  PrecedenceDirective directive = PRECEDENCE_TABLE[row][column];

  if (directive == DIRECTIVE_ERROR) {
    fprintf(stderr,
            "Precedencni parser: Terminal na zasobniku: %s\nTerminal na "
            "vstupu: %s\n",
            token_type_name(stack_terminal), token_type_name(input_terminal));

    char message[128];
    snprintf(message, sizeof message,
             "Precedencni parser: Kombinace terminalu neni platna (index "
             "zasobniku %zu, index vstupu %zu).",
             row, column);
    error_exit(ERR_SYNTAX, line, message);
  }

  // Kontrola platnosti zaznamu
  if (directive != DIRECTIVE_REDUCTION_MARKER &&
      directive != DIRECTIVE_REDUCTION_EXECUTE &&
      directive != DIRECTIVE_SHIFT_SYMBOL)
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: Neznama direktiva v precedencni tabulce.");

  return directive;
}
