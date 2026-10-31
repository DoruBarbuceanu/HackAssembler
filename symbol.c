/****************************************************************************************************
 * Symbol table module. Contains definitions for some routines needed to construct                  *
 * the symbol table. The table is implemented using a linked list.                                  *
 ****************************************************************************************************/

#include "symbol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Symbol {
    char* symbol;
    size_t address;
    struct Symbol* next;
} Symbol;

typedef struct SymbolTable {
    Symbol* head;
    Symbol* tail;
} SymbolTable;

/****************************************************************************************************
 * createSymbolTable: Create an empty symbol table.                                                 *
 ****************************************************************************************************/
SymbolTable* createSymbolTable(void)
{
    SymbolTable* table = malloc(sizeof(SymbolTable));

    if (table == NULL) {
        fprintf(stderr, "Error allocating memory for symbol table.\n");
        exit(EXIT_FAILURE);
    }

    table->head = NULL;
    table->tail = NULL;

    return table;
}

/****************************************************************************************************
 * addEntry: Adds a new <symbol,address> into the table.                                            *
 ****************************************************************************************************/
void addEntry(SymbolTable* table, const char* string, size_t address)
{
    if (table == NULL)
        return;

    /* Alocate memory for a new symbol entry. */
    Symbol* entry = malloc(sizeof(Symbol));
    if (entry == NULL) {
        fprintf(stderr, "Error allocating memory for new symbol.\n");
        exit(EXIT_FAILURE);
    }

    /* Alocate memory for the new symbol string. This may be slow because of strlen but we save
       a lot of memory. */
    entry->symbol = malloc(strlen(string) + 1);
    if (entry->symbol == NULL) {
        free(entry);
        fprintf(stderr, "Error allocating memory for new symbol string.\n");
        exit(EXIT_FAILURE);
    }

    /* Copy the new symbol name and address into the new entry node. */
    strcpy(entry->symbol, string);
    entry->address = address;
    entry->next = NULL;
    
    if (table->tail == NULL) {
        table->head = entry;
        table->tail = entry;
    } else {
        table->tail->next = entry;
        table->tail = entry;
    }
}
/****************************************************************************************************
 * contains: Checks if a given symbol is already in the table.                                      *
 ****************************************************************************************************/
bool contains(const SymbolTable* table, const char* string)
{
    if (table == NULL)
        return false;

    const Symbol* current = table->head;

    while (current) {
        if (strcmp(current->symbol, string) == 0)
            return true;
        current = current->next;
    }

    return false;
}

/****************************************************************************************************
 * getAddress: Returns the address associated with the symbol.                                      *
 ****************************************************************************************************/
size_t getAddress(const SymbolTable* table, const char* string)
{
    if (table == NULL)
        return INVALID_ADDRESS; /* INVALID_ADDRESS defined as 32768. */

    const Symbol* current = table->head;

    while (current) {
        if (strcmp(current->symbol, string) == 0)
            return current->address;
        current = current->next;
    }

    return INVALID_ADDRESS;
}

/****************************************************************************************************
 * clearSymbolTable: Clear the symbol table and free the memory.                                    *
 ****************************************************************************************************/
void clearSymbolTable(SymbolTable* table)
{
    if (table == NULL)
        return;

    Symbol* temp = table->head;

    while (temp) {
        Symbol* next = temp->next;

        free(temp->symbol);
        free(temp);

        temp = next;
    }

    free(table);
}
