# Testovací prostředí
- Do testovacího prostředí se dostanete pomocí: `cd test`
	- Vizuální příklad → `user@machine:~/IFJ-Projekt-2026/test$`
- Zde můžete volat unit testy na specifikované komponenty a i specifické unit testy
- Komponety jsou navzájem izolované a nemohou se ovlivnit (mají vlastní build)
- `make test` funguje i bez specifikování `COMPONENT` a `TEST` a testuje vše
- Příklad se specifikací `make test COMPONENT=<nazev> TEST=<nazev>`
- `make test TEST=<nazev>` nefunguje bez specifikování komponenty pomocí `COMPONENT`

# List dostupných specifikací
| COMPONENT | TEST |
| --- | --- |
| `lexer` | `assignment`, `empty_and_comments_only`, `err_backtick_identifier`, `err_double_underscore_int`, `err_empty_annotation`, `err_incomplete_exponent`, `err_incomplete_exponent_no_sign`, `err_inconsistent_dedent`, `err_insufficient_indent_in_parens`, `err_internal_ungetc_overflow`, `err_invalid_annotation`, `err_invalid_escape_sequence`, `err_leading_zero_int`, `err_newline_in_single_string`, `err_non_ascii_character`, `err_single_ampersand`, `err_single_pipe`, `err_single_underscore_id`, `err_space_then_tab_in_indentation`, `err_tab_in_indentation`, `err_token_after_multiline_comment`, `err_trailing_underscore_int`, `err_underscore_start_exponent`, `err_unterminated_block_comment`, `err_unterminated_multiline_string`, `err_unterminated_nested_block_comment`, `err_unterminated_string`, `err_zero_with_underscore`, `function`, `keywords`, `keywords_identifiers_case`, `maximal_munch_and_inline_comment`, `multiline_nested_block_comment_eol`, `multiple_dedent_at_eof`, `numbers_and_dot_notation`, `strings_single_and_multiline`, `suppress_indent_inside_parens`, `unary_minus_numbers` |
| `parser` | `tmp_classic`, `tmp_for_if`, `tmp_if_block_else`, `tmp_if_dangling_else`, `tmp_if_else_if`, `tmp_if_inline_else`, `tmp_return_else_semicolon`, `tmp_while_if` |
| `generator` | `sem_valid_program`, `example_program` |
| `pipeline` | `algebdt`, `boolthen`, `control`, `extkw`, `factorial`, `funexp`, `interpolateds`, `lambdas`, `lists`, `paramlists`, `strings`, `test_ahoj`, `valmem` |

## Aktualně ignorované
| COMPONENT | TEST |
| --- | --- |
| `pipeline` | `algebdt`, `extkw`, `interpolateds`, `lambdas`, `paramlists` |
| `parser` | *všechny testy* |

# MISC
Pro více informací o překladu `IFJ26_examples` pomocí *_scala_* a `linux-x86` interpretu nebo o testování pomocí poskytnutého IFJ26 kódu si přečtěte upravenou verzi `test/IFJ26_examples/README.public.md`.

Pro manuální přeskočení některých testů se používá list `EXCEPTIONS` v souboru `test.py` ve formátu `["COMPONENT", "TEST", "TEST", "TEST"]`
- pro přeskočení celé komponenty stačí `["COMPONENT"]`

Speciální testy vyžadující interpreta jsou specifikovány v listu `INTERPRET_TESTS` obdobným formátem jako `EXCEPTIONS`