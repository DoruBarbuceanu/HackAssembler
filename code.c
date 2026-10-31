/****************************************************************************************************
 * Code module for nand2tetris chapter 6 hack assembler project. Contains definitions for all       *
 * the Code routines described in the book.                                                         *
 ****************************************************************************************************/

#include "code.h"
#include <string.h>

/* Dest mnemonics - binary represenations table. */
static const CodeEntry dest_table[] =
{
    {"",    "000"}, {"M",   "001"}, {"D",   "010"}, {"DM",  "011"},
    {"A",   "100"}, {"AM",  "101"}, {"AD",  "110"}, {"ADM", "111"},
    {"MD",  "011"}, {"MA",  "101"}, {"DA",  "110"}, {"AMD", "111"},
    {"DAM", "111"}, {"DMA", "111"}, {"MAD", "111"}, {"MDA", "111"}
};

/* Comp mnemonics - binary represenations table. */
static const CodeEntry comp_table[] =
{
    {"0",   "0101010"}, {"1",   "0111111"}, {"-1",  "0111010"}, {"D",   "0001100"},
    {"A",   "0110000"}, {"M",   "1110000"}, {"!D",  "0001101"}, {"!A",  "0110001"},
    {"!M",  "1110001"}, {"-D",  "0001111"}, {"-A",  "0110011"}, {"-M",  "1110011"},
    {"D+1", "0011111"}, {"A+1", "0110111"}, {"M+1", "1110111"}, {"D-1", "0001110"},
    {"A-1", "0110010"}, {"M-1", "1110010"}, {"D+A", "0000010"}, {"D+M", "1000010"},
    {"D-A", "0010011"}, {"D-M", "1010011"}, {"A-D", "0000111"}, {"M-D", "1000111"},
    {"D&A", "0000000"}, {"D&M", "1000000"}, {"D|A", "0010101"}, {"D|M", "1010101"}
};

/* Jump mnemonics - binary represenations table. */
static const CodeEntry jump_table[] =
{
    {"",    "000"}, {"JGT", "001"}, {"JEQ", "010"}, {"JGE", "011"},
    {"JLT", "100"}, {"JNE", "101"}, {"JLE", "110"}, {"JMP", "111"}
};

/****************************************************************************************************
 * lookup: Helper function to avoid repetition in dest_translate, comp_translate and                *
 *         jump_translate functions below.                                                          *
 ****************************************************************************************************/
static const char* lookup(const CodeEntry* table, size_t count, const char* mnemonic)
{
    for (size_t i = 0; i < count; i++) {
        if (strcmp(mnemonic, table[i].mnemonic) == 0)
            return table[i].binary;
    }

    return NULL;
}

/****************************************************************************************************
 * dest_translate: Returns the binary code of the dest mnemonic. If invalid dest mnemonic,          *
 *                 returns NULL.                                                                    *
 ****************************************************************************************************/
const char* dest_translate(const char* dest_mnemonic)
{
    return lookup(dest_table, sizeof(dest_table) / sizeof(dest_table[0]), dest_mnemonic);
}

/****************************************************************************************************
 * comp_translate: Returns the binary code of the comp mnemonic. If invalid comp mnemonic,          *
 *                 returns NULL.                                                                    *
 ****************************************************************************************************/
const char* comp_translate(const char* comp_mnemonic)
{
    return lookup(comp_table, sizeof(comp_table) / sizeof(comp_table[0]), comp_mnemonic);
}

/****************************************************************************************************
 * jump_translate: Returns the binary code of the jump mnemonic. If invalid jump mnemonic,          *
 *                 returns NULL.                                                                    *
 ****************************************************************************************************/
const char* jump_translate(const char* jump_mnemonic)
{
    return lookup(jump_table, sizeof(jump_table) / sizeof(jump_table[0]), jump_mnemonic);
}
