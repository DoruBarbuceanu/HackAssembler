/****************************************************************************************************
 * Code module for nand2tetris chapter 6 hack assembler project. Contains declarations for all      *
 * the Code routines described in the book.                                                         *
 ****************************************************************************************************/

#ifndef CODE_H
#define CODE_H

/* Type definition for a "mnemonic - binary representation" association. */
typedef struct
{
    const char* mnemonic;
    const char* binary;
} CodeEntry;

const char* dest_translate(const char* dest_mnemonic);
const char* comp_translate(const char* comp_mnemonic);
const char* jump_translate(const char* jump_mnemonic);

#endif
