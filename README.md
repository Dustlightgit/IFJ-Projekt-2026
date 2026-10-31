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

- Komentáře česky
- Kód, git messages anglicky

## Překlad a testování

- kompilace do `build/` – `make build`
- spouštění `build/main` - `make run`
- testování – `make test`
- formátování všech souborů v `src/` – `make format`
- výsledné zabalení do `output/` – `make package`
- přikaz `make` - `build-test`
- čištění složek `build/` a `output/` - `make clean`

- další příkazy `make`:
    - `clean-build`
    - `clean-build-test`
    - `build-test`