#include <stdio.h>

#include "ast.h"
#include "nonterminal.h"
#include "parser_common.h"
#include "predictive_parser.h"
#include "semantic.h"
#include "symtable.h"


static void print_block(const Block *b, int depth);

/*
static void print_type(const Type *t) {
  if (t == NULL) { fprintf(stderr, "?"); return; }
  switch (t->kind) {
  case TYPE_INT: fprintf(stderr, "Int"); break;
  case TYPE_DOUBLE: fprintf(stderr, "Double"); break;
  case TYPE_STRING: fprintf(stderr, "String"); break;
  case TYPE_UNIT: fprintf(stderr, "Unit"); break;
  case TYPE_BOOLEAN: fprintf(stderr, "Boolean"); break;
  case TYPE_LIST:
    fprintf(stderr, "List[");
    print_type(t->element);
    fprintf(stderr, "]");
    break;
  }
}
*/

static void print_stmt(const Stmt *s, int depth) {
  fprintf(stderr, "%*s", depth * 2, "");
  switch (s->kind) {
  case STMT_VARDEF: fprintf(stderr, "VARDEF %s\n", s->as.vardef.name); break;
  case STMT_ASSIGN: fprintf(stderr, "ASSIGN %s\n", s->as.assign.name); break;
  case STMT_RETURN: fprintf(stderr, "RETURN\n"); break;
  case STMT_WHILE:
    fprintf(stderr, "WHILE\n");
    print_block(s->as.while_loop.body, depth + 1);
    break;
  case STMT_FOR:
    fprintf(stderr, "FOR %s (strazi: %d)\n", s->as.for_loop.var_name,
            s->as.for_loop.guard_count);
    print_block(s->as.for_loop.body, depth + 1);
    break;
  case STMT_EXPR:
    if (s->as.expr_stmt.expr->kind == EXPR_IF) {
      const Expr *e = s->as.expr_stmt.expr;
      fprintf(stderr, "IF (cond: %s)\n", e->as.if_expr.cond ? "ano" : "NE");
      fprintf(stderr, "%*sthen:\n", depth * 2 + 1, "");
      print_block(e->as.if_expr.then_block, depth + 1);
      if (e->as.if_expr.else_block) {
        fprintf(stderr, "%*selse:\n", depth * 2 + 1, "");
        print_block(e->as.if_expr.else_block, depth + 1);
      }
    } else {
      fprintf(stderr, "EXPR\n");
    }
    break;
  }
}


static void print_block(const Block *b, int depth) {
  if (b == NULL) {
    fprintf(stderr, "%*s(NULL blok)\n", depth * 2, "");
    return;
  }
  for (int i = 0; i < b->count; i++)
    print_stmt(b->items[i], depth);
}

/* TESTOVACI MAIN
int main() {
  ParserContext parser_ctx;
  parser_context_init(&parser_ctx);

  // Koren AST, parser do nej pridava funkce
  Program *program = program_new();

  predictive_parser(&parser_ctx, NONTERM_PROGRAM, program);

  for (int i = 0; i < program->func_count; i++) {
    FuncDef *f = program->funcs[i];
    fprintf(stderr, "AST: funkce %s, main=%d, vraci ", f->name, f->is_main);
    print_type(f->return_type);
    for (int j = 0; j < f->param_count; j++) {
      fprintf(stderr, ", %s: ", f->params[j].name);
      print_type(f->params[j].type);
    }
    
   
    fprintf(stderr, "\n");
    print_block(f->body, 1);
  }

  // Sem pozdeji prijde: semantic_analyze(program), generator kodu
  program_free(program);

  return 0;
}

*/

int main() {
  ParserContext parser_ctx;
  parser_context_init(&parser_ctx);

  Program *program = program_new();
  predictive_parser(&parser_ctx, NONTERM_PROGRAM, program);

  // Docasny vypis stromu pro testovani (pred odevzdanim smazat)
  for (int i = 0; i < program->func_count; i++) {
    fprintf(stderr, "AST: funkce %s\n", program->funcs[i]->name);
    print_block(program->funcs[i]->body, 1);
  }

  SymTable *table = semantic_analyze(program);

  // Sem prijde generator kodu (dostane program a table)

  symtable_free(table);
  program_free(program);
  return 0;
}