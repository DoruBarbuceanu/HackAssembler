/****************************************************************************************************
 * Parser module for nand2tetris chapter 6 hack assembler project. Contains declarations for all    *
 * the parser routines described in the book.                                                       *
 ****************************************************************************************************/

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>

#define MAXLINE   1024  /* maximum line length */
#define MAXINST   256   /* maximum instruction length */

typedef enum {
    A_INSTRUCTION,
    C_INSTRUCTION,
    L_INSTRUCTION,
} insType;

typedef struct {
    FILE* source_fp;
    char current_line[MAXLINE];
    char current_instruction[MAXINST];
    char buffer[MAXINST];
} Parser;

FILE* openFile(const char* file_name);
bool hasMoreLines(Parser* p);
void advance(Parser* p);
insType instructionType(Parser* p);
const char* symbol(Parser* p);
const char* dest(Parser* p);
const char* comp(Parser* p);
const char* jump(Parser* p);

#endif
