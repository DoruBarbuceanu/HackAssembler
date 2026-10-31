/****************************************************************************************************
 * Version 1 of the assembler for the nand2tetris project 6. It translates a symbolic language      *
 * hack program into its binary machine code language. It assumes the source file contains no       *
 * symbolic references. (no labels or variable names).                                              *
 ****************************************************************************************************/
#include "parser.h"
#include "code.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXPATHSIZE 256 /* Maximum file path length. */

static FILE* openDestFile(char* dest_file, size_t dest_size, const char* source);
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
        fprintf(stderr, "Error: Can't open \"%s\" file.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    /* Try to open the destination .hack file. */
    FILE* dest_fp = openDestFile(dest_file_name, MAXPATHSIZE, argv[1]);
    if (dest_fp == NULL) {
        fprintf(stderr, "Error: Can't save \"%s\" file.\n", dest_file_name);
        exit(EXIT_FAILURE);
    }

    size_t lineNumber = 1;
    insType type;

    while (hasMoreLines(&p)) {
        advance(&p);
        if (*p.current_instruction) {

            type = instructionType(&p);

            if (type == C_INSTRUCTION) {

                /* Write the first 3 bits of the C instruction into the destination file. */
                fputs("111", dest_fp);

                /* Get and write the comp mnemonic. */
                write_mnemonic(dest_fp,
                        comp_translate,
                        "comp",
                        lineNumber,
                        dest_file_name,
                        comp(&p));

                /* Get and write the dest mnemonic. */
                write_mnemonic(dest_fp,
                        dest_translate,
                        "dest",
                        lineNumber,
                        dest_file_name,
                        dest(&p));

                /* Get and write the jump mnemonic. */
                write_mnemonic(dest_fp,
                        jump_translate,
                        "jump",
                        lineNumber,
                        dest_file_name,
                        jump(&p));
            } else if (type == A_INSTRUCTION) {

                /* Get the address string and convert it into number. */
                if (*symbol(&p)) {
                    size_t address = (size_t) atoi(p.buffer);
                    if (address > 32767) {
                        fprintf(stderr, "Address out of range on line %zu.\n", lineNumber);
                        cleanup_and_exit(dest_fp, dest_file_name);
                    }
                    for (int i = 15; i >= 0; --i)
                        fputc((address >> i) & 1 ? '1' : '0', dest_fp);
                }
                else {
                    fprintf(stderr,
                            "Missing address value for A instruction on line %zu.\n",
                            lineNumber);
                    cleanup_and_exit(dest_fp, dest_file_name);
                }
            }

            /* Finish the binary instruction by writing a new line character. */
            fputs("\n", dest_fp);
        }

        lineNumber++;
    }

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
