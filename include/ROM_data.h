/*
 * General constants and typedef structres for Wattbot-nt robot
 */

#ifndef  ROM_data_H
#define  ROM_data_H

#include "pico/stdlib.h"

//***************************************************************************
// ROM table to allow fast categorisation of charcaters in a command from 
// HLcontrol
//
// Using simple ASCII chacter set for commands this lookup table categorises 
// a charcater as one of the following types
//
// LETTER    = a-z , A-Z , '_' (others can be added)
// NUMBER    = 0->9
// PLUSMINUS = '+' and '-'
// DOT       = '.'
// TERM      = '\0'
// END       = '\n'
// OTHER     = all other characters in the 256 extended ASCII set
//
// This is uded in the command string parser in "Task_run_cmd" 

enum {LETTER, NUMBER, DOT, PLUSMINUS, NULTERM, END, OTHER};

const uint8_t char_type[256] = {
    NULTERM,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,    END,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // 00->0F
    OTHER  ,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // 10->1F
    OTHER  ,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER, PLUSMINUS,  OTHER, PLUSMINUS,    DOT,  OTHER,  // 20->2F
    NUMBER , NUMBER, NUMBER, NUMBER, NUMBER, NUMBER, NUMBER, NUMBER, NUMBER, NUMBER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // 30->3F
    OTHER  , LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,    LETTER, LETTER,    LETTER, LETTER, LETTER,  // 40->4F
    LETTER , LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,     OTHER,  OTHER,     OTHER,  OTHER, LETTER,  // 50->5F
    OTHER  , LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,    LETTER, LETTER,    LETTER, LETTER, LETTER,  // 40->4F
    LETTER , LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // 70->7F

     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // 80->8F
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // 90->9F
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // A0->AF
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // B0->BF
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // C0->CF
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // D0->DF
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // E0->EF
     OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,  OTHER,     OTHER,  OTHER,     OTHER,  OTHER,  OTHER,  // F0->FF
};

#endif