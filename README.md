<!--
    IFJ projekt 2026

    Původní autor: xsantar00
    Seznam změn:
        - doplneni informaci a novych automatickych unit testech : xsantar00

    Poskytuje informace o projektu a jeho implementaci

-->
# Implementace překladače imperativního jazyka IFJ26

- Termín odevzdávání: 27. 11. v 11:59:59
- Pokusné odevzdání: do konce čtvrtka 19. 11.
- Mezi čtvrtkem 26. 11. a úterým 1. 12. v 8:00 zaneste vytištěnou dokumentaci na fakultu

## Varianta

- vv-BVS – Výškově vyvážený binární vyhledávací strom

## Formát

- Clang-format – LLVM

- Proměnné a funkce - snake_case
- Konstanty a položky enum - SCREAMING_SNAKE_CASE
- Datové typy - PascalCase

- Komentáře česky bez diakritiky
- Kód, git messages anglicky

## Překlad a testování

- **výchozí** chování příkazu **`make` je `build`**
- kompilace do `build/` – `make build`
    - **rekompilace** - `make rebuild` (spustí `clean` a pak `build`)
- **spouštění překladače** v `build/main` - `make run`
- **testování** – `make test` - otestuje překladač pomocí **_valgrind_** a zároveň spustí všechny **unit testy** ve složce `/test/scripts` jednotlivých částí překladače 
    - vždy spustí `rebuild`
    - více informací o testování v `/test/tests.md`
- **formátování** všech souborů v `src/` – `make format`
- výsledné **zabalení** do `output/` – `make package`
- **čištění** složek `build/`, `output/` a zbytků po testech - `make clean`