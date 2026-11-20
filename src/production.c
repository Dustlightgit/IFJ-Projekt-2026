/*
    IFJ projekt 2026

    xbalaka00 Artur Balak

    production.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - aktualizace pravidel dle nove verze gramatiky: zmeneno
          pravidlo 25 a 44, odstranena pravidla 33, 45 a 46, doplnene
          poznamky k pravidlum.
        - xbalaka00 - komentare pravidel prepsany s ceskou terminologii (straz,
          visici else apod.).
        - xbalaka00 - odstranena testovaci produkce <expr> => ID.
        - xbalaka00 - zdokumentovano kopirovani semantickeho ukazatele
          a kontextove anotace ve statickych produkcich.
        - xbalaka00 - pravidlo 30 pouziva NONTERM_EXPR misto odstraneneho
          NONTERM_EXPR_NO_ID.
        - xbalaka00 - odkazy na excel s gramatikou zmeneny na
          parser_formalisms.xlsx.
        - xbalaka00 - include grammar_symbol.h pro makra GRAMMAR_TERMINAL a GRAMMAR_NONTERMINAL (GrammarSymbol presunut do grammar_symbol.h,
          prompt je v reduction_table.h).
        - xbalaka00 - odstraneny pozustatky testovaciho pravidla <expr> -> *
        - xbalaka00 - cislovani pravidel bylo prevedeno na novy format L<n>, aby ladilo s E<n>
          u precedencniho parseru

    Tento soubor obsahuje staticke produkce LL gramatiky, tedy pravidla bez
    semanticke akce. Cisla pravidel odpovidaji tabulce gramatiky (parser_formalisms.xlsx).
    Pravidla se semantickou akci ("ano") jsou zpracovana callbacky.

*/

#include "production.h"

#include "grammar_symbol.h"
#include "nonterminal.h"
#include "token.h"

/*
    Prompt pro vychozi obsah souboru se statickymi produkcemi. Stejny prompt
    zpusobil zmeny take v production.h a LL_table.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Ahoj. Mám udělané jádro prediktivního parseru založeného na LL tabulce.
        Prozatím jsem do něj vygeneroval pouze testovací pravidla, avšak nastal
        čas přidat tam mnou navrženou gramatiku, která je v přiloženém excelu.

        Vše o formátu pravidel by mělo být v hlavičkových souborech. Pravidla
        existují dvojí - statická a callbacková. Pro začátek vygenerujeme pouze
        statická pravidla.

        V souboru LL_table.c se můžeš inspirovat, jakým způsobem se pravidla
        vkládají. Jedná se o konstanty typu StaticProduction.

        Pravidla pro přehlednost vygeneruj v novém souboru production.c
        odpovídajícího hlavičkovému souboru production.h. K jednotlivým
        pravidlům vytvoř i záznam v .h tak, aby je šlo importovat v LL_table.c
        pro použití v LL tabulce. Tento import rovnou můžeš přidat a smíš smazat
        stará testovací statická pravidla ze souboru LL_table.c. Jinak ale nic
        jiného v kódu neměň.

        Zatím generujeme pouze tvar statických pravidel, nikoliv LL tabulku
        samotnou.

        Drž se stylu v ostatních hlavičkových souborech a dodržuj naming
        conventions včetně komentářů v češtině bez diakritiky a rozlišení
        doxygen/jednoduchých. Dodrž formát hlavičky souboru a přidej změnový
        záznam. Po vzoru ostatních souborů přidej také tento prompt na
        nejvhodnější místo.

        Vygeneruj zatím pouze statická pravidla, které jsou v kolonce Sém. akce
        anotována jako "ano ptr" nebo mají prázdné políčko (obě jsou statické
        pravidla, prakticky v nich není rozdíl). Ostatní pravidla přijdou na
        řadu později.

        Symboly do produkcí dej v "přirozeném pořadí" - parser je bude na stack
        dávat v pořadí snižujícího se indexu.

        Tabulka neterminálů by mohla být zastaralá, protože pravidla jsem od
        jejího vytvoření upravoval a některá přibyla. Pokud to budeš potřebovat,
        přidej si do něj chybějící neterminály z tabulky.

        Prozatím ponechej také testovací pravidlo <expr> -> * protože výrazový
        parser zatím není hotový.

        Vygenerováný kód můžeš zkusit zkompilovat, ale netestuj ho - testování
        pak proběhne na našem parseru.

        Zkus pravidla vhodně anotovat po vzoru testovacích pravidel na základě
        excelu.

        Pokud něco není jasné, zeptej se, než začneš generovat.

    Prompt pro aktualizaci pravidel dle nove verze gramatiky (LL_gramatika).
    Stejny prompt zpusobil zmeny take v production.h, semantic_actions.c,
    semantic_actions.h, nonterminal.h a nonterminal.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Nakonec jsem musel gramatiku ještě přepracovat. Zde je nejnovější
        verze. Aktualizuj prosím pravidla v kódu.

        V prvním kroku bych jen přegeneroval pravidla, vyplnění LL tabulky do
        kódu bude další krok. Bylo změněno pravidlo 25 a 44 pokud si dobře
        vzpomínám. Pravidla 45 a 46 byla zrušena, pravidla 32 a 33 byly
        převedeny do odpovědnosti jádra. Přibyly nová pravidla související s
        ternárním if a mnoho, mnoho poznámek. Byl zrušen jeden neterminál a
        jeden zaveden. Teď už nevím, jestli jsem náhodou nezměnil i něco ve
        sloupci rozlišující callbacky od statických pravidel, kdyžtak pak
        taková pravidla také přegeneruj na druhou variantu.

        Prozatím testovací LL tabulku a vše, co potřebuje, ponech, jak je; po
        testovací sadě bych pouklízel v dalším kroku.

    Prompt pro upravu komentaru na ceskou terminologii (guard, dangling else
    apod.) a pro prejmenovani odkazu na LL_gramatika_1.xlsx na LL_gramatika.xlsx.
    Stejny prompt zpusobil zmeny take v production.h, semantic_actions.c
    a semantic_actions.h, jeho zneni je pouze zde.

    Prompt pro Claude:
        Máme hotové statické produkce a callbacky; než však přejdeme na
        samotnou LL tabulku, chtěl bych ještě upravit komentáře v souborech
        production a semantic_actions. V excelu jsem nepoužíval striktně
        českou terminologii v komentářích a názvech pravidel. Mohl by jsi tyto
        komentáře v kódu (ne v excelu) přepsat tak, aby například vyjádření
        typu guard, dangling else apod. bylo nahrazeno českými výrazy, jako je
        stráž? Samozřejmě, odkazuje-li se text na název/textové vyjádření
        terminálu, neterminálu nebo proměnné/funkce apod., tak toto může
        zůstat doslovné. Kód samotný zatím neměň.

        Dále také změň reference na LL_gramatika_1.xlsx na LL_gramatika.xlsx -
        ta jednička vznikla jen díky souborovému systému a více verzím, uváděl
        bych čistý název. Pro tuto konkrétní spíše kosmetickou změnu není
        potřeba dělat změnový záznam, protože tato změna bude ve stejném
        commitu jako ta předešlá, která již změnové seznamy upravila.

    Prompt pro prejmenovani odkazu na excel s gramatikou na
    parser_formalisms.xlsx. Stejny prompt zpusobil zmeny take v production.h
    a LL_table.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Provedl jsem předělání LL gramatiky. Opravil jsem ten problém, a
        samozřejně vyvstal zase jiný, takže jsem to vyřešil tak, že jazyk
        nepodporuje kombinaci inline then větve s řídící konstrukcí a přítomného
        else.
        Mohl bys všude v souborech změnit odkaz na starý název excelového
        souboru na jeho nový název parser_formalisms.xlsx? Název byl změněn
        proto, že nyní v druhém listu obsahuje i návrh výrazové gramatiky.
        Chtěl bych soubor v aktuální podobě zaverzovat do gitu, aby byla jeho
        podoba uchována, jak je. Padlo totiž rozhodnutí, že přepočet množin po
        této v pořadí třetí úpravě pravidel převezme kolega, který jej provede
        za pomocí AI - chci tedy, aby byla uložena podoba před zásahy AI, ve
        které soubor needitoval nikdo jiný než já.
*/

/*
    Prompt pro odstraneni testovaci produkce <expr> => ID je v souboru
    LL_table.c. Prompt pro kopirovani kontextove anotace ve statickych
    produkcich je v souboru semantic_types.h. Prompt pro odstraneni
    NONTERM_EXPR_NO_ID je v souboru nonterminal.h.
*/

// Symboly jsou v produkcich v prirozenem poradi (zleva doprava), parser je
// na zasobnik vklada od posledniho. Pravidla oznacena (ptr) predavaji
// semanticky ukazatel z neterminalu na leve strane do symbolu produkce.
// Parser pri vkladani staticke produkce vzdy kopiruje semanticky ukazatel
// i kontextovou anotaci do vsech jejich symbolu, znacka (ptr) je jen
// informativni.

// Pravidlo L1 (Seznam deklaraci) (ptr): <defList> => <def> <defList>
const StaticProduction PRODUCTION_DEF_LIST_NEXT = {
    .length = 2,
    .production =
        {
            GRAMMAR_NONTERMINAL(NONTERM_DEF),
            GRAMMAR_NONTERMINAL(NONTERM_DEF_LIST),
        },
};

// Pravidlo L2 (Konec deklaraci): <defList> => epsilon
const StaticProduction PRODUCTION_DEF_LIST_END = {.length = 0};

// Pravidlo L5 (Anotace neni): <main> => epsilon
const StaticProduction PRODUCTION_MAIN_ABSENT = {.length = 0};

// Pravidlo L6 (EOL je): <optionalEOL> => EOL
const StaticProduction PRODUCTION_OPTIONAL_EOL_PRESENT = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_EOL)},
};

// Pravidlo L7 (EOL neni): <optionalEOL> => epsilon
const StaticProduction PRODUCTION_OPTIONAL_EOL_ABSENT = {.length = 0};

// Pravidlo L9 (Pridani carky) (ptr): <paramComma> => , <paramAdd>
// Pozn.: Novy radek mezi koncovou carkou a tokenem ) testuje semanticka akce.
const StaticProduction PRODUCTION_PARAM_LIST_TAIL_NEXT = {
    .length = 2,
    .production =
        {
            GRAMMAR_TERMINAL(TOKEN_COMMA),
            GRAMMAR_NONTERMINAL(NONTERM_PARAM_LIST),
        },
};

// Pravidlo L10 (Konec parametru A): <paramAdd> => epsilon
const StaticProduction PRODUCTION_PARAM_LIST_END = {.length = 0};

// Pravidlo L11 (Konec parametru B): <paramComma> => epsilon
const StaticProduction PRODUCTION_PARAM_LIST_TAIL_END = {.length = 0};

// Pravidlo L12 (Navratovy typ je) (ptr): <optionalType> => : <type>
const StaticProduction PRODUCTION_OPTIONAL_TYPE_PRESENT = {
    .length = 2,
    .production =
        {
            GRAMMAR_TERMINAL(TOKEN_COLON),
            GRAMMAR_NONTERMINAL(NONTERM_TYPE),
        },
};

// Pravidlo L13 (Navratovy typ neni): <optionalType> => epsilon
const StaticProduction PRODUCTION_OPTIONAL_TYPE_ABSENT = {.length = 0};

// Pravidlo L20 (Jednoradkovy vyraz misto bloku) (ptr): <blockOrExpr> => <expr> EOL
// Pozn.: Volani funkce je take expr kvuli FUNEXP.
const StaticProduction PRODUCTION_BLOCK_OR_EXPR_EXPR = {
    .length = 2,
    .production =
        {
            GRAMMAR_NONTERMINAL(NONTERM_EXPR),
            GRAMMAR_TERMINAL(TOKEN_EOL),
        },
};

// Pravidlo L21 (Pouziti bloku) (ptr): <blockOrExpr> => <block>
const StaticProduction PRODUCTION_BLOCK_OR_EXPR_BLOCK = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_BLOCK)},
};

// Pravidlo L23 (Dalsi prikaz) (ptr): <commandList> => <command> <commandList>
// Pozn.: Prikazy konci bud koncem logickeho radku, nebo strednikem.
const StaticProduction PRODUCTION_COMMAND_LIST_NEXT = {
    .length = 2,
    .production =
        {
            GRAMMAR_NONTERMINAL(NONTERM_COMMAND),
            GRAMMAR_NONTERMINAL(NONTERM_COMMAND_LIST),
        },
};

// Pravidlo L24 (Konec seznamu prikazu): <commandList> => epsilon
const StaticProduction PRODUCTION_COMMAND_LIST_END = {.length = 0};

// Pravidlo L25 (Konec prikazu - strednik): <commandEnd> => ;
// Pozn.: Prikaz se strednikem na konci bloku je zakazan, protoze by pri
// spotrebovani DEDENT ze zasobniku nastala chyba.
const StaticProduction PRODUCTION_COMMAND_END_SEMICOLON = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_SEMICOLON)},
};

// Pravidlo L26 (Konec prikazu - EOL): <commandEnd> => EOL
const StaticProduction PRODUCTION_COMMAND_END_EOL = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_EOL)},
};

// Pravidlo L29 (Deklarace modifikovatelne promenne): <valVar> => var
const StaticProduction PRODUCTION_VAR = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_KW_VAR)},
};

// Pravidlo L30 (Prikaz vyrazu) (ptr): <commandSimple> => <exprNoID>
// Pozn.: <exprNoID> je v implementaci nahrazen <expr>, viz pravidlo L30 vyse.
const StaticProduction PRODUCTION_COMMAND_SIMPLE_EXPR = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_EXPR)},
};

// Pravidlo L45 (Jednoduchy prikaz) (ptr): <command> => <commandSimple> <commandEnd>
const StaticProduction PRODUCTION_COMMAND_SIMPLE = {
    .length = 2,
    .production =
        {
            GRAMMAR_NONTERMINAL(NONTERM_COMMAND_SIMPLE),
            GRAMMAR_NONTERMINAL(NONTERM_COMMAND_END),
        },
};

// Pravidlo L46 (Ridici prikaz) (ptr): <command> => <commandControl>
const StaticProduction PRODUCTION_COMMAND_CONTROL = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_COMMAND_CONTROL)},
};

// Pravidlo L55 (Then vetev s blokem) (ptr): <thenBranch> => <block> <optionalElse>
const StaticProduction PRODUCTION_THEN_BRANCH_BLOCK = {
    .length = 2,
    .production =
        {
            GRAMMAR_NONTERMINAL(NONTERM_BLOCK),
            GRAMMAR_NONTERMINAL(NONTERM_OPTIONAL_ELSE),
        },
};

// Pravidlo L56 (Then vetev s prikazem) (ptr): <thenBranch> => <commandSimple> <elseOrCommandEnd>
// Pozn.: else musi nasledovat na stejnem radku jako inline then.
const StaticProduction PRODUCTION_THEN_BRANCH_SIMPLE = {
    .length = 2,
    .production =
        {
            GRAMMAR_NONTERMINAL(NONTERM_COMMAND_SIMPLE),
            GRAMMAR_NONTERMINAL(NONTERM_ELSE_OR_COMMAND_END),
        },
};

// Pravidlo L57 (Then vetev s ridicim prikazem) (ptr): <thenBranch> => <commandControl>
// Pozn.: Bez vetve else, else tak vzdy patri k nejblizsimu if.
const StaticProduction PRODUCTION_THEN_BRANCH_CONTROL = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_COMMAND_CONTROL)},
};

// Pravidlo L58 (Konec prikazu v then vetvi) (ptr): <elseOrCommandEnd> => <commandEnd>
const StaticProduction PRODUCTION_ELSE_OR_COMMAND_END_COMMAND_END = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_COMMAND_END)},
};

// Pravidlo L35 (Return vyrazu) (ptr): <returnValue> => <expr>
const StaticProduction PRODUCTION_RETURN_VALUE_EXPR = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_EXPR)},
};

// Pravidlo L36 (Prazdny return): <returnValue> => epsilon
const StaticProduction PRODUCTION_RETURN_VALUE_EMPTY = {.length = 0};

// Pravidlo L42 (Konec strazi): <guardList> => epsilon
const StaticProduction PRODUCTION_GUARD_LIST_END = {.length = 0};

// Pravidlo L43 (Nasleduje blok) (ptr): <blockOrCommand> => <block>
const StaticProduction PRODUCTION_BLOCK_OR_COMMAND_BLOCK = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_BLOCK)},
};

// Pravidlo L44 (Nasleduje prikaz) (ptr): <blockOrCommand> => <command>
const StaticProduction PRODUCTION_BLOCK_OR_COMMAND_COMMAND = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_COMMAND)},
};

// Pravidlo L49 (Vynechana vetev else): <optionalElse> => epsilon
const StaticProduction PRODUCTION_OPTIONAL_ELSE_ABSENT = {.length = 0};

// Pravidlo L51 (Odsazeni vyrazu v ternarni podmince) (ptr): <exprOrIndentExpr> => EOL INDENT <expr> EOL DEDENT
// Pozn.: Pravidla 51 a 53 maji shodnou pravou stranu, ale lisi se chovanim.
const StaticProduction PRODUCTION_EXPR_OR_INDENT_EXPR_INDENTED = {
    .length = 5,
    .production =
        {
            GRAMMAR_TERMINAL(TOKEN_EOL),
            GRAMMAR_TERMINAL(TOKEN_INDENT),
            GRAMMAR_NONTERMINAL(NONTERM_EXPR),
            GRAMMAR_TERMINAL(TOKEN_EOL),
            GRAMMAR_TERMINAL(TOKEN_DEDENT),
        },
};

// Pravidlo L52 (Neodsazeny vyraz v ternarni podmince) (ptr): <exprOrIndentExpr> => <expr>
const StaticProduction PRODUCTION_EXPR_OR_INDENT_EXPR_PLAIN = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_EXPR)},
};

// Pravidlo L54 (Nedokoncena vetev else): <elseTernaryIf> => epsilon
// Pozn.: Do vyrazovky se vrati neterminal TIF, ktery bude redukovan a pripoji
// vetev else. Uvnitr zavorek odsazeni nehraje roli, vetev else se pripoji na
// zaklade redukce ohranicene zavorkami.
const StaticProduction PRODUCTION_ELSE_TERNARY_IF_ABSENT = {.length = 0};
