#ifndef ASM_H
#define ASM_H

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

#define BREAK 3

int execute_asm(uint32_t u, registre r, mem memory);
int loadmem(uint32_t vAdrr, mem memory, char* type);
#endif
