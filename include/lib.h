#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

/* macros de DEBUG_MSG fournies , etc */
#include "common/notify.h"
#include "registre.h"
#include "commande.h"
#include "emulMips.h"
#include "mem.h"

uint32_t getbits(uint32_t u, size_t start, size_t stop);
uint32_t getRS(uint32_t u);
uint32_t getRT(uint32_t u);
uint32_t getRD(uint32_t u);
int16_t getIm(uint32_t u);
int16_t getOffset(uint32_t u);
uint32_t getSA(uint32_t u);
uint32_t getTarget(uint32_t u);
char* getInstr(uint32_t u);
char* type(uint32_t u);
char* libs(uint32_t u);
char* libs3(uint32_t u);
char* libr(uint32_t u);
char* libn(uint32_t u);
char* getR(uint32_t a);
void dispAssem(uint32_t u);


#endif

