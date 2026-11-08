/*
    IFJ projekt 2026

    nonterminal.h

    Puvodni autor: xbakaka00 za asistence Claude
    Seznam zmen:
        - Pridana hlavicka pro funkci nonterm_type_name

    Tento soubor obsahuje vyctovy typ pro neterminaly a deklarace funkci pro
   praci s nimi.

*/

/*
    Prompty pro vychozi kostru souboru a enum neterminalu. Vysledny kod byl
   upraven.

    Prompt pro Claude:
        Ahoj, potřeboval bych pomoci s přetavením LL gramatiky do parseru. Zatím
   ještě není přechroustaná do LL tabulky, to si pak chci zkusit sám. Přikládám
   ti excel s gramatikou, možná ještě není bez much, ale to nebrání s parserem
   pokročit. Zvolil jsem přístup prediktivního parseru, takže změny by měly být
   levné. Samotné jádro parseru chci napsat převážně sám.

        Pro začátek mi jen vygeneruj enum se všemi neterminály. Zkus reflektovat
   název, který je v tabulce, ale jestli tě občas napadne lepší a výstižnější
   název pro neterminál, asi ho můžeš lehce upravit, ale vypiš mi seznam změn.
   Gramatiku neměň. Dej neterminálům prefix NONTERM. Jestli chceš, můžeš občas
   přidat komentář dle kontextu tokenu nebo neterminály rozdělit do kategorií,
   avšak nemusíš. Případně si to pak upravím.

        Tady je naše naming konvence:
        - Proměnné a funkce - snake_case
        - Konstanty a položky enum - SCREAMING_SNAKE_CASE
        - Datové typy - PascalCase
        - Komentáře česky
        - Kód, git messages anglicky

        Formátování neřeš, potom to proženu clang-formatem. Využij tyto konvence
   i v případných dalších úkolech.

    Prompt pro Claude:
        Aha, já ho zapomněl nahrát. Tady je. Píši v jazyce C.

    Prompt pro Claude:
        Mohl by jsi komentáře přepsat tak, aby byly jednoduché? Zde je úryvek
   kódu kolegy, ideálně tedy zkus držet formu s ním, ať je vše konzistentní.
   Před samotným enumem a obecně při hlavní dokumentaci nějaké funkce v
   hlavičkovém souboru však můžeš komentář udělat takový, aby byl ve tvaru
   doxygen dokumentace.

        Inspirativní úryvek ze scanneru:
        <zde jsem přepokíroval náhodný úsek z kódu scanner.c>

*/

#ifndef NONTERMINAL_H
#define NONTERMINAL_H

/**
 * @brief Neterminaly LL gramatiky.
 *
 * Poradi odpovida prvnimu vyskytu v pravidlech gramatiky.
 */
typedef enum {
  // Struktura programu
  NONTERM_PROGRAM,         // Startovni neterminal
  NONTERM_DEF_LIST,        // Seznam deklaraci funkci
  NONTERM_DEF,             // Deklarace funkce
  NONTERM_MAIN_ANNOTATION, // Anotace @main
  NONTERM_OPTIONAL_EOL,    // Nepovinny EOL

  // Parametry a typy
  NONTERM_PARAM_LIST,      // Parametr nebo konec parametru
  NONTERM_PARAM_LIST_TAIL, // Dalsi parametr za carkou nebo konec
  NONTERM_OPTIONAL_TYPE,   // Nepovinny typ
  NONTERM_TYPE,            // Datovy typ

  // Bloky a prikazy
  NONTERM_BLOCK_OR_EXPR,      // Telo funkce, vyraz nebo blok
  NONTERM_BLOCK,              // Odsazeny blok
  NONTERM_COMMAND_LIST,       // Dalsi prikazy v bloku
  NONTERM_COMMAND_END,        // Konec prikazu, strednik nebo EOL
  NONTERM_COMMAND,            // Prikaz
  NONTERM_VAL_VAR,            // val nebo var
  NONTERM_ID_COMMAND,         // Prikaz zacinajici na ID
  NONTERM_RETURN_VALUE,       // Nepovinna hodnota za return
  NONTERM_BLOCK_OR_COMMAND,   // Telo cyklu nebo podminky
  NONTERM_OPTIONAL_SEMICOLON, // Nepovinny strednik

  // Cyklus for
  NONTERM_FOR_HEADER, // Hlavicka for
  NONTERM_GUARD_LIST, // Seznam guardu

  // Podminky
  NONTERM_OPTIONAL_ELSE,       // Nepovinna vetev else
  NONTERM_TERNARY_IF,          // Ternarni if
  NONTERM_EXPR_OR_INDENT_EXPR, // Vyraz, pripadne odsazeny

  // Pseudoneterminaly, predani do vyrazove analyzy
  NONTERM_EXPR,       // Vyraz
  NONTERM_EXPR_NO_ID, // Vyraz nezacinajici na ID
  // Pozn. - tento neterminal pravdepodobne nebude implementacne potreba,
  // prozatim jej ponechavam.

  // Pocet neterminalu, napr. pro velikost LL tabulky
  NONTERM_COUNT
} NonterminalType;

/**
 * @brief Vrati citelny nazev neterminalu pro chybove vypisy.
 *
 * @param type Typ neterminalu.
 * @return Retezec s nazvem neterminalu, pro neplatnou hodnotu "?".
 */
const char *nonterm_type_name(NonterminalType type);

#endif // NONTERMINAL_H
