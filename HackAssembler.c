/****************************************************************************************************
 * Version 2 of the assembler for the nand2tetris project 6. It translates a symbolic language      *
 * hack program into its binary machine code language. It is the complete version with support for  *
 * symbolic references.                                                                             *
 ****************************************************************************************************/

#include "parser.h"
#include "code.h"
#include "symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAXPATHSIZE 256 /* Maximum file path length. */
#define MIN_USABLE_RAM_ADDRESS 16 /* First RAM address for variables address space. */
#define MAX_USABLE_RAM_ADDRESS 16383 /* Last RAM address for variables address space. */

static FILE* openDestFile(char* dest_file, size_t dest_size, const char* source);
static void initializeTable(SymbolTable* table);
static bool isValidSymbol(const char* s);
static bool allDigits(const char* symbol);
static void write_mnemonic(
        FILE *dest_fp,
        const char* (*translator)(const char*),
        const char* kind,
        size_t line,
        const char* dest_file_name,
        const char* buffer);
static void cleanup_and_exit(
        FILE* dest_fp,
        const char* dest_file_name);

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments.\nUsage: \"HackAssembler.exe source_file_name.asm\"\n");
        exit(EXIT_FAILURE);
    }

    Parser p;
    char dest_file_name[MAXPATHSIZE];

    /* Try to open the .asm source file. */
    p.source_fp = openFile(argv[1]);
    if (p.source_fp == NULL) {
        fprintf(stderr, "Can't open %s file.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    /* Try to open the destination .hack file. */
    FILE* dest_fp = openDestFile(dest_file_name, MAXPATHSIZE, argv[1]);
    if (dest_fp == NULL) {
        fprintf(stderr, "Error: Can't save %s file.\n", dest_file_name);
        exit(EXIT_FAILURE);
    }

    /* Creating and initializing the symbol table with all the predefined symbols. */
    SymbolTable* table = createSymbolTable();
    initializeTable(table);

    size_t lineNumber = 0;
    
    /* First pass of traversing the source file. Adding symbol references into the symbol table. */
    while (hasMoreLines(&p)) {
            advance(&p);
            if (*p.current_instruction) {
                if (instructionType(&p) == L_INSTRUCTION) {
                    const char* temp = symbol(&p);

                    if (contains(table, temp)) {
                        fprintf(stderr, "Error: Duplicate symbol \"%s\" encountered.\n", temp);
                        cleanup_and_exit(dest_fp, dest_file_name);
                    }

                    addEntry(table, temp, lineNumber);
                }
                else
                    lineNumber++;
            }
    }

    // Move to the beginning of the source file.
    fseek(p.source_fp, 0L, SEEK_SET);
    
    /* Second pass of traversing the file. */
    insType type;
    lineNumber = 1;
    size_t freeRamAddress = MIN_USABLE_RAM_ADDRESS;

    while (hasMoreLines(&p)) {
        advance(&p);
        if (*p.current_instruction) {
            type = instructionType(&p);

            /* If an A instruction:
               1. check if a valid numeric address.
               2. check if a valid symbol. if not in the table, add it assigning the next free
                  RAM address. if already in the table, get its address.
               3. if none of the above then clean and stop execution. */
            if (type == A_INSTRUCTION) {
                const char* current_symbol = symbol(&p);
                size_t target_address;
                if (allDigits(current_symbol)) {
                    target_address = (size_t) strtoul(current_symbol, NULL, 0);
                    if (target_address >= INVALID_ADDRESS) {
                        fprintf(stderr, "Address out of range on line %zu.\n", lineNumber);
                        cleanup_and_exit(dest_fp, dest_file_name);
                    }
                }
                else if (isValidSymbol(current_symbol)) {
                    if (!contains(table, current_symbol)) {
                        if (freeRamAddress > MAX_USABLE_RAM_ADDRESS) {
                            fprintf(stderr, "ERROR: OUT OF MEMORY\n");
                            cleanup_and_exit(dest_fp, dest_file_name);
                        }
                        addEntry(table, current_symbol, freeRamAddress);
                        target_address = freeRamAddress++;
                    }
                    else {
                        target_address = getAddress(table, current_symbol);
                        if (target_address == INVALID_ADDRESS) {
                            fprintf(stderr, "Error: INVALID_ADDRESS returned by getAddress().\n");
                            cleanup_and_exit(dest_fp, dest_file_name);
                        }
                    }
                }
                else {
                    fprintf(stderr, "Error: Invalid symbol on line %zu.\n", lineNumber);
                    cleanup_and_exit(dest_fp, dest_file_name);
                }

                /* Write the instruction. */
                for (int i = 15; i >= 0; --i)
                    fputc((target_address >> i) & 1 ? '1' : '0', dest_fp);
                fputc('\n', dest_fp);
            }
            else if (type == C_INSTRUCTION) {

                // Write the first 3 bits of the C instruction into the destination file.
                fputs("111", dest_fp);

                // Get and write the comp mnemonic.
                write_mnemonic(dest_fp,
                        comp_translate,
                        "comp",
                        lineNumber,
                        dest_file_name,
                        comp(&p));

                // Get and write the dest mnemonic.
                write_mnemonic(dest_fp,
                        dest_translate,
                        "dest",
                        lineNumber,
                        dest_file_name,
                        dest(&p));

                // Get and write the jump mnemonic.
                write_mnemonic(dest_fp,
                        jump_translate,
                        "jump",
                        lineNumber,
                        dest_file_name,
                        jump(&p));

                fputc('\n', dest_fp);
            }
        }

        lineNumber++;
    }

    clearSymbolTable(table);
    fclose(p.source_fp);
    fclose(dest_fp);

    return 0;
}

/****************************************************************************************************
 * openDestFile: Open the destination .hack file.                                                   *
 ****************************************************************************************************/
FILE* openDestFile(char* dest_file, size_t dest_size, const char* source)
{
    const char *ext = strrchr(source, '.');

    if (!ext || strcmp(ext, ".asm") != 0)
        return NULL;

    size_t base_len = (size_t)(ext - source);

    if (base_len + sizeof(".hack") > dest_size)
        return NULL;

    memcpy(dest_file, source, base_len);
    memcpy(dest_file + base_len, ".hack", sizeof(".hack"));

    return fopen(dest_file, "w");
}

/****************************************************************************************************
 * write_mnemonic: Helper function to avoid repetition in main assembler program.                   *
 *                 Writes the current translated mnemonic into the assembled file. If invalid       *
 *                 mnemonic is encountered, it closes the destination file pointer and it tries     *
 *                 to remove the incomplete assembled file.                                         *
 ****************************************************************************************************/
static void write_mnemonic(
        FILE *dest_fp,
        const char* (*translator)(const char*),
        const char* kind,
        size_t line,
        const char* dest_file_name,
        const char* buffer)
{
    const char* bits = translator(buffer);

    if (bits) {
        fputs(bits, dest_fp);
        return;
    }

    fprintf(stderr,
            "Error: Invalid %s mnemonic on line %zu.\n",
            kind,
            line);
    cleanup_and_exit(dest_fp, dest_file_name);
}

/****************************************************************************************************
 * cleanup_and_exit: Helper function to avoid repetition in main assembler program.                 *
 *                   It closes the dest_fp file pointer and it tries to remove the incomnplete      *
 *                   assembled file.                                                                *
 ****************************************************************************************************/
static void cleanup_and_exit(FILE* dest_fp, const char* dest_file_name)
{
    fclose(dest_fp);

    if (remove(dest_file_name) != 0)
        perror("cleanup error");

    exit(EXIT_FAILURE);
}

/****************************************************************************************************
 * initializeTable: Initialize the symbol table with all the predefined symbols.                    *
 ****************************************************************************************************/
void initializeTable(SymbolTable* table)
{
    const char* symbol[] = {"R0",    "R1",    "R2",    "R3",    "R4",    "R5",    "R6",    "R7",
                            "R8",    "R9",    "R10",   "R11",   "R12",   "R13",   "R14",   "R15",
                            "SP",    "LCL",   "ARG",   "THIS",  "THAT",  "SCREEN","KBD"};
    size_t i;

    for (i = 0; i <=15; i++)
        addEntry(table, symbol[i], i);

    for (i = 0; i <= 4; i++)
        addEntry(table, symbol[i+16], i);

    addEntry(table, symbol[21], 16384);
    addEntry(table, symbol[22], 24576);
}

/****************************************************************************************************
 * isValidSymbol: Checks if the given symbol meets the language specifications by containing only   *
 *                letters, digits, underscores, dots, dollar signs, colons and does not begin       *
 *                with a digit.                                                                     *
 ****************************************************************************************************/
bool isValidSymbol(const char* s)
{
    /* Check first character. */
    if (!isalpha(*s) &&
            *s != '_' &&
            *s != '.' &&
            *s != '$' &&
            *s != ':')
        return false;

    s++;

    /* Check remaining characters. */
    while (*s) {
        if (!isalnum(*s) &&
                *s != '_' &&
                *s != '.' &&
                *s != '$' &&
                *s != ':')
            return false;

        s++;
    }

    return true;
}

/****************************************************************************************************
 * allDigits: Checks if the given character string contains only digits. (For testing if it's a     *
 *            numerical address or a symbol to be added in the symbol table.)                       *
 ****************************************************************************************************/
bool allDigits(const char* symbol)
{
    if (symbol == NULL || symbol[0] == '\0')
        return false;

    for (size_t i = 0; symbol[i] != '\0'; i++)
        if (!isdigit(symbol[i]))
            return false;

    return true;
}
