/****************************************************************************************************
 * Parser module for nand2tetris chapter 6 hack assembler project. Contains definitions for all     *
 * the parser routines described in the book.                                                       *
 ****************************************************************************************************/

#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/****************************************************************************************************
 * openfile: open a file containing hack symbolic language source code to be parsed.                *
 ****************************************************************************************************/
FILE* openFile(const char* file_name)
{
    const char* ext = strrchr(file_name, '.');

    if (!ext || strcmp(ext, ".asm") != 0) {
        fprintf(stderr, "Invalid file name. Must be of the form \"source_file_name.asm\"\n");
        return NULL;
    }

    return fopen(file_name, "r");
}

/****************************************************************************************************
 * hasMoreLines: check if input source file has more lines and store the new line into current_line *
 *               buffer.                                                                            *
 ****************************************************************************************************/
bool hasMoreLines(Parser* p)
{
    if (fgets(p->current_line, MAXLINE, p->source_fp) != NULL)
        return true;

    if (ferror(p->source_fp)) {
        fprintf(stderr, "I/O error.\n");
        exit(EXIT_FAILURE);
    }

    return false;
}

/****************************************************************************************************
 * advance: Skips over white space and comments, if necessary.                                      *
 *          Reads the next instruction from the input, and make it the current instruction.         *
 *          This routine should be called only if hasMoreLines is true.                             *
 *          Initially there is no current instruction.                                              *
 ****************************************************************************************************/
void advance(Parser* p)
{
    size_t i, j;

    /* We use a variable last_nonspace to keep track of the last valid character of
       the instruction. */
    int last_nonspace = -1;

    /* Remove leading spaces. */
    for (i = 0; (isspace((unsigned char) p->current_line[i])); i++)
        ;

    /* Read the instruction and store it into current instruction buffer. */
    j = 0;
    while (p->current_line[i] != '\0') {
        if (p->current_line[i] == '/' && p->current_line[i+1] == '/')
            break;

        if (j < MAXINST - 1) {
            if (!isspace((unsigned char) p->current_line[i]))
                last_nonspace = j;
            p->current_instruction[j++] = p->current_line[i];
        }
        i++;
    }

    if (last_nonspace != -1)
        p->current_instruction[last_nonspace + 1] = '\0';
    /* If last_nonspace == -1, then this is not an instruction line. */
    else
        p->current_instruction[0] = '\0';
}

/****************************************************************************************************
 * instructionType: Returns the type of the current instruction:                                    *
 *                  A_INSTRUCTION for @xxx, where xxx is either a decimal number or a symbol.       *
 *                  C_INSTRUCTION for dest=comp;jump                                                *
 *                  L_INSTRUCTION for (xxx), where xxx is a sumbol.                                 *
 ****************************************************************************************************/
insType instructionType(Parser* p)
{
    switch (p->current_instruction[0]) {
        case '@':  return  A_INSTRUCTION;
        case '(':  return  L_INSTRUCTION;
        default:   return  C_INSTRUCTION;
    }
}

/****************************************************************************************************
 * symbol: If the current instruction is (xxx), returns the symbol xxx. If the current instruction  *
 *         is @xxx, returns the symbol or decimal xxx (as a string).                                *
 *         Should be called only if instructionType is A_INSTRUCTION or L_INSTRUCTION.              *
 ****************************************************************************************************/
const char* symbol(Parser* p)
{
    /* As we call symbol only for A_INSTRUCTION or L_INSTRUCTION, then we extract the symbol
       starting from position current_instruction[1] */

    /* Skip possible white spaces after the @ or ( */
    size_t i = 1;
    while (isspace((unsigned char) p->current_instruction[i]))
        i++;

    size_t j = 0;
    while (p->current_instruction[i] != '\0' &&
            p->current_instruction[i] != ')' &&
            !isspace((unsigned char) p->current_instruction[i]))
        p->buffer[j++] = p->current_instruction[i++];
    p->buffer[j] = '\0';

    return p->buffer;
}

/****************************************************************************************************
 * dest: Returns the symbolic dest part of the current C-instruction (8 possibilities).             *
 *       Should be called only if instructionType is C_INSTRUCTION.                                 *
 ****************************************************************************************************/
const char* dest(Parser* p)
{
    size_t i;
    size_t j = 0;

    for (i = 0; p->current_instruction[i] != '=' &&
            p->current_instruction[i] != ';' &&
            p->current_instruction[i] != '\0'; i++) {
        /* We need to check here if there are spaces between dest and the exit loop character.
           If so we will not collect them into the buffer. */
        if (!isspace((unsigned char) p->current_instruction[i]) && j < MAXINST - 1)
            p->buffer[j++] = p->current_instruction[i];
    }

    if (p->current_instruction[i] == '=') {
        p->buffer[j] = '\0';
        return p->buffer;
    }

    p->buffer[0] = '\0';

    return p->buffer;
}

/****************************************************************************************************
 * comp: Returns the symbolic comp part of the current C-instruction (28 possibilities).            *
 *       Should be called only if instructionType is C_INSTRUCTION.                                 *
 ****************************************************************************************************/
const char* comp(Parser* p)
{
    size_t j = 0;

    char* start = strchr(p->current_instruction, '=');

    if (start != NULL)
        start++;
    else
        start = p->current_instruction;

    for (; *start != ';' &&
           *start != '\0';
           start++) {
        if (!isspace((unsigned char) *start) && j < MAXINST - 1)
            p->buffer[j++] = *start;
    }

    /* We return the string here because we don't care if exit loop trigger was ; or \0. */
    p->buffer[j] = '\0';

    return p->buffer;
}

/****************************************************************************************************
 * jump: Returns the symbolic jump part of the current C-instruction (8 possibilities).             *
 *       Should be called only if instructionType is C_INSTRUCTION.                                 *
 ****************************************************************************************************/
const char* jump(Parser* p)
{
    size_t i;
    size_t j = 0;

    /* First we search for ';' character or if not found, until we reach '\0'. */
    for (i = 0; p->current_instruction[i] != ';' && p->current_instruction[i] != '\0'; i++)
        ;

    if (p->current_instruction[i] == '\0') {
        /* No jump. */
        p->buffer[0] = '\0';
        return p->buffer;
    }

    /* Advance from ';' character. */
    i++;

    for (; p->current_instruction[i] != '\0' &&
            j < MAXINST - 1; i++) {
        if (!isspace((unsigned char) p->current_instruction[i]))
            p->buffer[j++] = p->current_instruction[i];
    }

    p->buffer[j] = '\0';

    return p->buffer;
}
