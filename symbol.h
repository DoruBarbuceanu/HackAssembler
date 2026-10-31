/****************************************************************************************************
 * Symbol table module API. Contains declarations for some routines needed to construct             *
 * the symbol table. The table is implemented using a linked list.                                  *
 ****************************************************************************************************/

#ifndef SYMBOL_H
#define SYMBOL_H

#include <stddef.h>
#include <stdbool.h>

/* Return value of getAddress in case no entry in the table contains the given string. */
#define INVALID_ADDRESS ((size_t)32768)

typedef struct SymbolTable SymbolTable;

SymbolTable* createSymbolTable(void);
void addEntry(SymbolTable* table, const char* string, size_t address);
bool contains(const SymbolTable* table, const char* string);
size_t getAddress(const SymbolTable* table, const char* string);
void clearSymbolTable(SymbolTable* table);

#endif
