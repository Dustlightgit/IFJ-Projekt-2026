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
- Deklarace – snakeCase

## Překlad a testování

- kompilace do `build/` – `make build`
- spouštění `build/main` - `make run`
- testování – `make test` - spusti vsechny unit testy ve slozce `/test/scripts` jednotlivych casti prekladace a zatoven otestuje prekladac pomoci _valgrind_
    - vic info o testovani v `/test/tests.md`
- formátování všech souborů v `src/` – `make format`
- výsledné zabalení do `output/` – `make package`
- přikaz `make` - `build-test`
- čištění složek `build/` a `output/` - `make clean`

- další příkazy `make`:
    - `clean-build`
    - `clean-build-test`
    - `build-test`