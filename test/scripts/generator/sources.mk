SOURCES := generator.c ast.c generator.h ast.h error.h dstring.c dstring.h stack.c stack.h symtable.c symtable.h # alias: dopln .c soubory, na kterych generator.c a ast.c zavisi (napr. error.c, dstring.c)
CFLAGS += -rdynamic
LDFLAGS += -rdynamic
LDLIBS += -ldl