# Testovací prostředí
- Do testovacího prostředí se dostanete pomocí: `cd test`
	- Vizuální příklad → `user@machine:~/IFJ-Projekt-2026/test$`
- Zde můžete volat unit testy na specifikované komponenty a i specifické unit testy
- Příklad se specifikací `make test COMPONENT=<nazev> TEST=<nazev>`
- `make test` funguje i bez specifikování `COMPONENT` a `TEST`
- `make test TEST=<nazev>` nefunguje bez specifikování komponenty pomocí `COMPONENT`

# List dostupných specifikací
| COMPONENT | TEST |
| --- | --- |
| `lexer` | `assigments` |
| `lexer` | `function` |
| `lexer` | `keywords` |
| `parser` | ... |

# MISC
Pro více informací o překladu `IFJ26_examples` pomocí *_scala_* a `linux-x86` interpretu nebo o testování pomocí poskytnutého IFJ26 kódu si přečtěte upravenou verzi `test/IFJ26_examples/README.public.md`.