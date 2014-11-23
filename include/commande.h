#ifndef COMMANDE_H
#define COMMANDE_H

#include "registre.h"
#include "commande.h"
#include "emulMips.h"
#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/bits.h"
#include "common/notify.h"
#include "elf/elf.h"
#include "elf/syms.h"
#include "mipself_test.h"
#include "lib.h"
#include "asm.h"

#include <stdint.h>


// On fixe ici une adresse basse dans la mémoire virtuelle. Le premier segment
// ira se loger à cette adresse.
#define START_MEM 0x3000
// nombre max de sections que l'on extraira du fichier ELF
#define NB_SECTIONS 4

// nom de chaque section
#define TEXT_SECTION_STR ".text"
#define RODATA_SECTION_STR ".rodata"
#define DATA_SECTION_STR ".data"
#define BSS_SECTION_STR ".bss"



int dispcmd(interpreteur inter,  registre r,mem memory); // ajouter vm virt en argument
int setcmd(interpreteur inter,  registre r,mem memory);
int loadcmd(interpreteur inter, mem* memory);
int assertcmd(interpreteur inter,  registre r, mem memory);
void debugcmd(interpreteur inter);
void resumecmd(interpreteur inter);
int numero_segment(char* chaine,mem memory);
int disasmcmd(interpreteur inter, mem memory);
int step(interpreteur inter,registre r, mem memory);
void run(interpreteur inter,registre r, mem memory, bp bp);
int quit ( registre r, mem memory);
int check_bp(bp breakpoint,uint32_t PC);
int breakcmd(interpreteur inter, mem memory, bp *bp0);
void free_list(bp);
bp free_bp(bp bp, char* token);
bp ajouter_en_tete(bp bp,char* token);
void print_list(bp bp0);
bp find_by_add(bp bpa,char *token);

#endif
